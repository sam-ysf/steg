/* main.cpp -- v1.0 */

#include "block_decoder.hpp"
#include "block_encoder.hpp"
#include "error.hpp"
#include "image.hpp"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <memory>
#include <string>
#include <unistd.h>

namespace {
    /*! @class: writes to file
     */
    class OutputStream {
        // Output file
        FILE* fd_ = stdout;
    public:
        ~OutputStream()
        {
            if (fd_ && fd_ != stdout) {
                std::fclose(fd_);
            }
        }

        OutputStream() = default;

        OutputStream(OutputStream&& other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = nullptr;
        }

        bool open(const char* const filePath)
        {
            if (fd_ != stdout) {
                return false;
            }

            fd_ = std::fopen(filePath, "wb");
            return fd_ != nullptr;
        }

        std::size_t write(const char* const buff, const std::size_t size)
        {
            // Write from buff to fd
            return std::fwrite(buff, sizeof(char), size, fd_);
        }
    };

    /*! @class: reads from file
     */
    class InputStream {
        // Encapsulated file descriptor
        FILE* fd_ = stdin;
    public:
        ~InputStream()
        {
            // Cleanup
            if (fd_ && fd_ != stdin) {
                std::fclose(fd_);
            }
        }

        /// ctor.
        InputStream() = default;

        InputStream(InputStream&& other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = nullptr;
        }

        /// assignment
        InputStream& operator()(InputStream&& other)
        {
            fd_ = std::move(other.fd_);
            other.fd_ = nullptr;
            return *this;
        }

        /// File size
        std::size_t size() const
        {
            // Get file size
            std::fseek(fd_, 0, SEEK_END);
            std::int64_t size = ftell(fd_);
            std::fseek(fd_, 0, SEEK_SET);

            return size < 0 ? 0 : static_cast<std::size_t>(size);
        }

        /// Open file
        bool open(const char* const filePath)
        {
            if (fd_ != stdin) {
                return false;
            }

            // Open and initialize file
            fd_ = std::fopen(filePath, "r");
            return fd_ != nullptr;
        }

        /// Read from file
        std::size_t read(char* buff, std::size_t size)
        {
            // Sanity check
            if (fd_ == nullptr) {
                return 0;
            }

            // Read raw data
            size = std::fread(buff, sizeof(char), size, fd_);
            if (size > 0 && fd_ == stdin) {
                // Remove trailing newline (stdin has different rules)
                buff[strcspn(buff, "\n")] = 0;
            }

            return size;
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

        static char* allocate(const std::size_t size)
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
               "   -v<init-vec-file>\n"
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
               "-v<init-vec-file>          Initialization vector file",

               "-i<message-file>           Source file of message; if left "
               "unspecified,\n\t"
               "                           source is the terminal (stdin)",

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

               "-v<init-vec-file>          Initialization vector file",
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
        std::unique_ptr<char[]> vec;

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
        std::unique_ptr<char[]> vec;

        // Image input
        steg::Image input;
        // Input message
        OutputStream output;
    };
} // namespace

namespace {
    // Helper: encodes text to image
    template <typename T>
    int encode(EncodeIO& io)
    {
        // Create encoder
        std::unique_ptr<T> encoder(T::create((io.key).get(), (io.vec).get()));

        // Encrypt and save the message
        if (encoder->run(io.input, io.output)
            && (io.output).save(io.outputPath.c_str(), io.outputType)) {
            return 0; // Success
        }

        // Error...
        return 1;
    }

    // Helper: decodes text from image
    template <typename T>
    int decode(DecodeIO& io)
    {
        // Create encoder
        std::unique_ptr<T> decoder(T::create((io.key).get(), (io.vec).get()));

        if (decoder.get() == nullptr) {
            return 1; // Error code
        }

        // Decrypt the message & return
        return decoder->run(io.input, io.output) ? 0 : 1;
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
    std::string vecFilePath;

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
    int mode = 0;

    // Flag specifies whether or not to encode to Base64
    int b64 = 0;

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
                vecFilePath = optarg;
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
                b64 = 1;
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
                            "steganography program that uses gcrypt to "
                            "encode/decode a secret message to/from an "
                            "image file; outputs the result to a new file");
                        print_usage(argv[0]);
                        return 0;
                    }

                    // Encode mode
                    case 1:
                    {
                        if (mode != 0) {
                            (steg::Error::get())
                                ->log("Error: either select encode or "
                                      "decode, I can't do both");
                            print_usage(argv[0]);
                            return 1;
                        }

                        mode = 1;
                        break;
                    }

                    // Decode mode
                    case 2:
                    {
                        if (mode != 0) {
                            (steg::Error::get())
                                ->log("Error: either select encode or "
                                      "decode, I can't do both");
                            print_usage(argv[0]);
                            return 1;
                        }

                        mode = 2;
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
                print_out("steganography program that uses gcrypt to "
                          "encode/decode a secret message to/from an image "
                          "file; outputs the result to a new file",
                          argv[0]);
                print_usage(argv[0]);
                return 0;
            }
        }
    }

    // Ensure all necessary parameters specified; exit otherwise...
    if (mode == 0) {
        (steg::Error::get())
            ->log("Error: you forgot to select the program mode; either "
                  "select encode (--encode) or decode (--decode)");
        print_usage(argv[0]);
        return 1;
    }

    if (imagePath.empty()) {
        (steg::Error::get())
            ->log("Error: no image file specified (one of bmp, bmp, or "
                  "tga formats), exiting");
        return 1;
    }

    if (keyFilePath.empty()) {
        (steg::Error::get())
            ->log("Error: no encryption key specified (use -k), exiting");
        return 1;
    }

    if (vecFilePath.empty()) {
        (steg::Error::get())
            ->log("Error: no initialization vector specified (use -v), "
                  "exiting");
        return 1;
    }

    if (outputPath.empty()) {
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
    if (!vecStream.open(vecFilePath.c_str())) {
        // Handle error
        print_file_error(vecFilePath.c_str());
        return 1;
    }

    std::size_t keySize = keyStream.size();
    std::size_t vecSize = vecStream.size();

    auto* key = new char[keySize];
    auto* vec = new char[vecSize];

    // Just in case
    if (!key || !vec) {
        (steg::Error::get())
            ->log("Error: memory allocation failed - insufficient "
                  "memory available, exiting");
        return 1;
    }

    keyStream.read(key, keySize);
    vecStream.read(vec, vecSize);

    // Go...
    switch (mode) {
        // Encrypt
        case 1:
        {
            EncodeIO io;

            // Load encoded image source
            if (!(io.output).open(imagePath.c_str())) {
                // Handle error
                print_file_error(imagePath.c_str());
                return 1;
            }

            (io.key).reset(key);
            (io.vec).reset(vec);

            // Plain message input;
            // If file specified, try to open it; otherwise, we'll use stdin
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
                for (const char ch: outputType) {
                    type.push_back(static_cast<char>(std::tolower(ch)));
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

            return (
                static_cast<bool>(b64)
                    ? encode<steg::BlockEncoder<true, BasicAllocator>>
                    : encode<steg::BlockEncoder<false, BasicAllocator>>)(io);
        }

        // Decrypt
        case 2:
        {
            DecodeIO io;
            (io.key).reset(key);
            (io.vec).reset(vec);

            // Load source image
            if (!(io.input).open(imagePath.c_str())) {
                // Handle error
                print_file_error(imagePath.c_str());
                return 1;
            }

            // Plain message output;
            // If file specified, try to open it;
            if (!outputPath.empty() && !(io.output).open(outputPath.c_str())) {
                // Handle error
                print_file_error(outputPath.c_str());
                return 1;
            }

            return (
                static_cast<bool>(b64)
                    ? decode<steg::BlockDecoder<true, BasicAllocator>>
                    : decode<steg::BlockDecoder<false, BasicAllocator>>)(io);
        }

        default:
        {
            return 1;
        }
    }

    // Done
    return 0;
}
