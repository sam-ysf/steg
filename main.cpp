/* main.cpp -- v1.0 */

#include "block_decoder.hpp"
#include "block_encoder.hpp"
#include "error.hpp"
#include "image.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <exception>
#include <format>
#include <gcrypt.h>
#include <getopt.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {
    /*! @class: writes to file
     */
    class OutputStream {
        // Output file
        FILE* fd_ = nullptr;

        void discard() noexcept
        {
            if (fd_ != nullptr) {
                std::fclose(fd_);
            }

            fd_ = nullptr;
        }
    public:
        ~OutputStream()
        {
            discard();
        }

        OutputStream() = default;
        OutputStream(const OutputStream&) = delete;
        OutputStream& operator=(const OutputStream&) = delete;
        bool open(const std::string& filePath)
        {
            if (filePath.empty()) {
                return false;
            }

            if (fd_ != nullptr) {
                return false;
            }

            fd_ = std::fopen(filePath.c_str(), "wb");
            return fd_ != nullptr;
        }

        void close()
        {
            if (fd_ == nullptr) {
                return;
            }

            if (std::fflush(fd_) != 0) {
                throw std::runtime_error("Error while flushing file contents");
            }

            if (fsync(fileno(fd_)) != 0) {
                throw std::runtime_error("Error while flushing file contents");
            }

            FILE* fd = fd_;
            fd_ = nullptr;
            if (std::fclose(fd) != 0) {
                throw std::runtime_error("Error closing file");
            }
        }

        std::size_t write(const char* buff, std::size_t size)
        {
            if (fd_ == nullptr) {
                return 0;
            }

            return std::fwrite(buff, sizeof(char), size, fd_);
        }
    };

    /*! @class: reads from file
     */
    class InputStream {
        // Encapsulated file descriptor
        FILE* fd_ = nullptr;
    public:
        ~InputStream()
        {
            // Cleanup
            if (fd_) {
                std::fclose(fd_);
            }
        }

        InputStream() = default;

        InputStream(InputStream&& rhs) noexcept
            : fd_(rhs.fd_)
        {
            rhs.fd_ = nullptr;
        }

        InputStream& operator=(InputStream&& rhs) noexcept
        {
            if (fd_) {
                std::fclose(fd_);
            }

            fd_ = std::move(rhs.fd_);
            rhs.fd_ = nullptr;
            return *this;
        }

        std::optional<std::size_t> size() const
        {
            if (fd_ == nullptr || std::fseek(fd_, 0, SEEK_END) != 0) {
                return std::nullopt;
            }

            std::int64_t fileSize = std::ftell(fd_);
            if (fileSize < 0 || std::fseek(fd_, 0, SEEK_SET) != 0) {
                return std::nullopt;
            }

            return static_cast<std::size_t>(fileSize);
        }

        bool open(const char* const filePath)
        {
            if (fd_) {
                return false;
            }

            // Open and initialize file
            fd_ = std::fopen(filePath, "rb");
            return fd_ != nullptr;
        }

        std::size_t read(char* buff, std::size_t size)
        {
            // Sanity check
            if (fd_ == nullptr) {
                return 0;
            }

            // Read raw data
            return std::fread(buff, sizeof(char), size, fd_);
        }
    };
} // namespace

namespace {
    /*! @class: allocates/deallocates using new/delete
     */
    class BasicAllocator {
    public:
        static void deallocate(const char* const v)
        {
            delete[] v;
        }

        static char* allocate(std::size_t size)
        {
            auto* value = new char[size];
            std::memset(value, 0, size);
            return value;
        }
    };
} // namespace

namespace {

    /*! Helper: Outputs to stdout
     */
    inline void print_out(const char* app, const char* str)
    {
        std::fprintf(stdout, "%s: %s.\n", app, str);
    }

    /*! Helper: Outputs to stderr
     */
    inline void print_file_error(const char* path)
    {
        std::fprintf(
            stderr,
            "Error opening file %s, check that file exists and that file "
            "permissions are correct.\n",
            path);
    }

    /*! Helper: Outputs usage statement to stdout
     */
    inline void print_usage(const char* app)
    {
        printf("---------------------------------------------------------------"
               "------------------\n");
        printf("Usage: %s {--encode|--decode|-h}\n"
               "   -f<encoded-image-source>\n"
               "  [-o<output-file>]\n"
               "  [-t<output-file-type>]\n"
               "   -k<crypt-key-file>\n"
               "   -v<authentication-context-file>\n"
               "  [-i<message-file>]\n"
               "  [-b]\n",
               app);

        printf("\n");
        printf("  %s\n  %s\n  %s\n",
               "--encode                     Encoding mode",
               "--decode                     Decoding mode",
               "--help (-h)                  Prints this message");

        printf("\n");
        printf(
            "------------Encode "
            "Mode----------------------------------------------------------\n");
        printf("\t%s\n\n"
               "\t%s\n\t%s\n\n"
               "\t%s\n\t%s\n\n"
               "\t%s\n"
               "\t%s\n",

               "-f<image-source>           Source file for image that the "
               "message will be\n\t"
               "                           encoded to",

               "-o<output-file>            Outputs to this file",
               "-t<output-file-type>       Accepted types: png, bmp, or tga",

               "-k<crypt-key-file>         AES cryptographic key file",
               "-v<authentication-context-file>  16-byte authenticated context",

               "-i<message-file>           Source file of message",
               "-b                         Encodes the encrypted output as a "
               "base64 string");

        printf("\n");
        printf(
            "------------Decode "
            "Mode----------------------------------------------------------\n");
        printf("\t%s\n\n"
               "\t%s\n\n"
               "\t%s\n\n"
               "\t%s\n"
               "\t%s\n",

               "-f<encoded-image>          Source file of encoded message",

               "-o<output-file>            Outputs to this file; if left "
               "unspecified,\n\t"
               "                           outpts to the terminal (stdout)",

               "-k<crypt-key-file>         AES cryptographic key file",

               "-v<authentication-context-file>  16-byte authenticated context",
               "-b                         Required if the encryption output "
               "was a base64\n\t"
               "                           string");
    }
} // namespace

namespace {

    // @bag
    struct EncodeIO {
        // Cryptographic vars
        std::unique_ptr<char[]> key;
        std::unique_ptr<char[]> initVec;

        // Encoded image output variables
        std::string outputPath;
        steg::Image::ImageType outputType;

        // Encoded image output
        steg::Image output;
        // Message input
        InputStream input;
    };

    // @bag
    struct DecodeIO {
        // Cryptographic vars
        std::unique_ptr<char[]> key;
        std::unique_ptr<char[]> initVec;

        // Image input
        steg::Image input;
        // Input message
        OutputStream output;
    };
} // namespace

namespace {
    // Helper: encodes text to image
    template <typename T>
    bool encode(EncodeIO& io)
    {
        // Create encoder
        std::unique_ptr<T> encoder(
            T::create((io.key).get(), (io.initVec).get()));

        if (encoder.get() == nullptr) {
            return false;
        }

        try {
            // Encrypt and save the message
            return encoder->run(io.input, io.output)
                   && (io.output).save(io.outputPath.c_str(), io.outputType);
        } catch (const std::runtime_error& e) {
            (steg::Error::get())
                ->log(std::format("Error: {}", e.what()).c_str());
            return false;
        }
    }

    // Helper: decodes text from image
    template <typename T>
    bool decode(DecodeIO& io)
    {
        // Create encoder
        std::unique_ptr<T> decoder(
            T::create((io.key).get(), (io.initVec).get()));

        if (decoder.get() == nullptr) {
            return false;
        }

        try {
            // Decrypt the message & return
            bool ret = decoder->run(io.input, io.output);
            io.output.close();
            return ret;
        } catch (const std::runtime_error& e) {
            (steg::Error::get())
                ->log(std::format("Error: {}", e.what()).c_str());
            return false;
        }
    }
} // namespace

int main(int argc, char** argv)
{
    // Image source file
    std::string imagePath;

    // Output file
    std::string outputPath;
    std::string outputType;

    // Key & initialization vector files
    std::string keyFilePath;
    std::string initVecFilePath;

    // Input file
    std::string inputPath;

    // Long command line options
    const option longOptions[] = {
        {.name = "help", .has_arg = no_argument, .flag = nullptr, .val = 0},
        {.name = "encode", .has_arg = no_argument, .flag = nullptr, .val = 0},
        {.name = "decode", .has_arg = no_argument, .flag = nullptr, .val = 0},
        {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0},
    };

    // Null = 0
    // Encode message to file = 1
    // Decode message from file = 2
    enum class Mode : std::uint8_t {
        kNull,
        kEncode,
        kDecode,
    };

    Mode mode = Mode::kNull;

    // Flag specifies whether or not to encode to Base64
    int EnableB64 = 0;

    // Parse command line options...
    int opt = 0;
    int optindex = 0;
    while ((opt = getopt_long(
                argc, argv, "-f:t:o:k:v:i:bh", longOptions, &optindex))
           != -1) {
        switch (opt) {
            // Encoding source
            case 'f':
            {
                imagePath = optarg;
                break;
            }

            // Output file
            case 'o':
            {
                outputPath = optarg;
                break;
            }

            // Output file type
            case 't':
            {
                outputType = optarg;
                break;
            }

            // Key file
            case 'k':
            {
                keyFilePath = optarg;
                break;
            }

            // Initialization vector file
            case 'v':
            {
                initVecFilePath = optarg;
                break;
            }

            // Input message file
            case 'i':
            {
                inputPath = optarg;
                break;
            }

            // Use base 64 encoding
            case 'b':
            {
                EnableB64 = 1;
                break;
            }

            // Long options
            case 0:
            {
                switch (optindex) {
                    // Help mode
                    case 0:
                    {
                        print_out(
                            argv[0],
                            "Steganography program that uses gcrypt to "
                            "encode/decode a secret message to/from an "
                            "image file; outputs the result to a new file");
                        print_usage(argv[0]);
                        return 0;
                    }

                    // Encode mode
                    case 1:
                    {
                        if (mode != Mode::kNull) {
                            (steg::Error::get())
                                ->log("Error: either select encode or "
                                      "decode, I can't do both");
                            print_usage(argv[0]);
                            return 1;
                        }

                        mode = Mode::kEncode;
                        break;
                    }

                    // Decode mode
                    case 2:
                    {
                        if (mode != Mode::kNull) {
                            (steg::Error::get())
                                ->log("Error: either select encode or "
                                      "decode, I can't do both");
                            print_usage(argv[0]);
                            return 1;
                        }

                        mode = Mode::kDecode;
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                break;
            }

            // Print help blurb
            case 'h':
                [[fallthrough]];
            default:
            {
                print_out("Steganography program that uses gcrypt to "
                          "encode/decode a secret message to/from an image "
                          "file; outputs the result to a new file",
                          argv[0]);
                print_usage(argv[0]);
                return 0;
            }
        }
    }

    // Ensure all necessary parameters specified; exit otherwise...
    if (mode == Mode::kNull) {
        (steg::Error::get())
            ->log("Error: you forgot to select the program mode; either "
                  "select encode (--encode) or decode (--decode)");
        print_usage(argv[0]);
        return 1;
    }

    if (imagePath.empty()) {
        (steg::Error::get())
            ->log("Error: no image file specified (use -f to specify a bmp, "
                  "jpg, or tga file), exiting");
        return 1;
    }

    if (keyFilePath.empty()) {
        (steg::Error::get())
            ->log("Error: no encryption key specified (use -k), exiting");
        return 1;
    }

    if (initVecFilePath.empty()) {
        (steg::Error::get())
            ->log("Error: no authentication context specified (use -v), "
                  "exiting");
        return 1;
    }

    if (mode == Mode::kEncode && inputPath.empty()) {
        (steg::Error::get())
            ->log("Error: no input file specified (use -i), exiting");
        return 1;
    }

    if (mode == Mode::kEncode && outputPath.empty()) {
        (steg::Error::get())
            ->log("Error: no output file specified (use -o), exiting");
        return 1;
    }

    // Open file
    InputStream keyStream;
    InputStream vecStream;

    if (!keyStream.open(keyFilePath.c_str())) {
        // Handle error
        print_file_error(keyFilePath.c_str());
        return 1;
    }

    // Open file
    if (!vecStream.open(initVecFilePath.c_str())) {
        // Handle error
        print_file_error(initVecFilePath.c_str());
        return 1;
    }

    std::optional<std::size_t> keySize = keyStream.size();
    std::optional<std::size_t> initVecSize = vecStream.size();

    if (!keySize.has_value()) {
        return 1;
    }

    if (!initVecSize.has_value()) {
        return 1;
    }

    if (std::size_t expectedSize
        = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES128);
        keySize != expectedSize) {
        (steg::Error::get())
            ->log(std::format(
                      "Key is not the required size, should be equal to {}",
                      expectedSize)
                      .c_str());
        return 1;
    }

    if (std::size_t expectedSize
        = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES128);
        initVecSize != expectedSize) {
        (steg::Error::get())
            ->log(
                std::format("Authentication context is not the required size, "
                            "should be equal to {}",
                            expectedSize)
                    .c_str());
        return 1;
    }

    auto key = std::make_unique<char[]>(keySize.value());
    auto initVec = std::make_unique<char[]>(initVecSize.value());
    if (keyStream.read(key.get(), keySize.value()) != keySize.value()
        || vecStream.read(initVec.get(), initVecSize.value())
               != initVecSize.value()) {
        (steg::Error::get())
            ->log("Error: unable to read the complete key or context file");
        return 1;
    }

    // Go...
    switch (mode) {
        // Nothing to do
        case Mode::kNull:
        {
            return 1;
        }

        // Encode & encrypt
        case Mode::kEncode:
        {
            EncodeIO io;

            // Load encoded image source
            if (!(io.output).open(imagePath.c_str())) {
                // Handle error
                print_file_error(imagePath.c_str());
                return 1;
            }

            io.key = std::move(key);
            io.initVec = std::move(initVec);

            // Plain message input;
            // If file specified, try to open it
            if (!inputPath.empty() && !(io.input).open(inputPath.c_str())) {
                // Handle error
                print_file_error(inputPath.c_str());
                return 1;
            }

            // Assign output file variables
            io.outputPath = outputPath;

            io.outputType = [&outputType] {
                using enum steg::Image::ImageType;

                std::string type;
                type.reserve(outputType.size());
                for (char ch: outputType) {
                    type.push_back(static_cast<char>(
                        std::tolower(static_cast<unsigned char>(ch))));
                }

                if (type == "png") {
                    return kPng;
                }

                if (type == "bmp") {
                    return kBmp;
                }

                if (type == "tga") {
                    return kTga;
                }

                return kPng;
            }();

            if (static_cast<bool>(EnableB64)) {
                return encode<steg::BlockEncoder<true, BasicAllocator>>(io) ? 0
                                                                            : 1;
            }

            return encode<steg::BlockEncoder<false, BasicAllocator>>(io) ? 0
                                                                         : 1;
        }

        // Decode & decrypt
        case Mode::kDecode:
        {
            DecodeIO io;
            io.key = std::move(key);
            io.initVec = std::move(initVec);

            // Load source image
            if (!(io.input).open(imagePath.c_str())) {
                // Handle error
                print_file_error(imagePath.c_str());
                return 1;
            }

            if (!(io.output).open(outputPath)) {
                // Handle error
                print_file_error(outputPath.c_str());
                return 1;
            }

            if (static_cast<bool>(EnableB64)) {
                return decode<steg::BlockDecoder<true, BasicAllocator>>(io) ? 0
                                                                            : 1;
            }

            return decode<steg::BlockDecoder<false, BasicAllocator>>(io) ? 0
                                                                         : 1;
        }
    }

    // Done
    return 0;
}
