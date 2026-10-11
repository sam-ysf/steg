/* block_decoder.hpp -- v1.0
   Facade for decoding messages and writing results to output */

#pragma once

#include "base64.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "decoder.hpp"
#include "stream_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

namespace steg {
    //! @class BlockDecoder
    //! @brief Facade for decoding message and writing result to output
    template <bool EnableB64, typename AllocType>
    class BlockDecoder : private Decoder {
    public:
        //! Factory method, returns a BlockDecoder
        //! @param key AES key string
        //! @param initVec Initialization vector string
        //! @return BlockDecoder instance
        static BlockDecoder* create(const char* key, const char* initVec);

        //! Decodes input message and writes to output
        //! @param inp Input stream
        //! @param out Output stream
        //! @return True on success
        template <typename InputType, typename OutputType>
        bool run(InputType& inp, OutputType& out);

        ~BlockDecoder() override = default;
    private:
        /*! Helper
         * @brief Decodes from digest to raw data
         * @param out Output stream
         * @param buff Buffer containing data
         * @param size Size of the buffer
         * @return True on success
         */
        template <typename OutputType>
        bool decode_digest(OutputType& out, char* buff, std::size_t size)
        {
            if constexpr (EnableB64) {
                size = base64_decode(buff, size);
            }

            if (size < (Cipher::kEnvelopeHeaderSize + Cipher::kGcmTagSize)) {
                return false;
            }

            bool hasMagic = std::memcmp(buff,
                                        Cipher::kEnvelopeMagic.data(),
                                        Cipher::kEnvelopeMagic.size());
            if (hasMagic != 0) {
                return false;
            }

            std::uint64_t payloadSize = 0;
            for (std::size_t index = 0; index < sizeof(payloadSize); ++index) {
                auto ch = static_cast<unsigned char>(
                    buff[Cipher::kEnvelopeMagic.size() + Cipher::kGcmNonceSize
                         + index]);
                payloadSize = (payloadSize << 8) | ch;
            }

            const std::size_t availableSize
                = size - Cipher::kEnvelopeHeaderSize - Cipher::kGcmTagSize;
            if (payloadSize != availableSize) {
                return false;
            }

            const auto* nonce = reinterpret_cast<unsigned char*>(
                buff + Cipher::kEnvelopeMagic.size());
            char* payload = buff + Cipher::kEnvelopeHeaderSize;
            const char* tag = payload + availableSize;
            if (!Decoder::decode(payload,
                                 availableSize,
                                 nonce,
                                 Cipher::kGcmNonceSize,
                                 buff,
                                 Cipher::kEnvelopeHeaderSize,
                                 tag,
                                 Cipher::kGcmTagSize)) {
                return false;
            }

            return detail::write_all(out, payload, availableSize);
        }

        /*! Ctor. Private, use factory method create() instead
         * @param cph Cipher
         */
        explicit BlockDecoder(std::unique_ptr<Cipher> cipher)
            : Decoder(std::move(cipher))
        {}
    };

    //! Factory method, returns BlockDecoder
    //! @param key AES key string
    //! @param initVec Initialization vector string
    template <bool EnableB64, typename AllocType>
    BlockDecoder<EnableB64, AllocType>* BlockDecoder<EnableB64, AllocType>::
        create(const char* key, const char* initVec)
    {
        // Use gcrypt to initialize cipher before passing it to the decoder
        auto cipher = cipher_init(key, initVec);
        if (!cipher) {
            return nullptr;
        }

        return new BlockDecoder(std::move(cipher));
    }

    //! Decodes input message and writes to output
    //! @param inp Input stream
    //! @param out Output stream
    //! @return True on success
    template <bool EnableB64, typename AllocType>
    template <typename InputType, typename OutputType>
    bool BlockDecoder<EnableB64, AllocType>::run(InputType& input,
                                                 OutputType& output)
    {
        // Input read size
        std::size_t inpSize = input.size();
        if (inpSize == 0) {
            inpSize = 100000; // Default size for unknown size input
        }

        // Generate the read buffer.
        auto deallocate = [](char* value) {
            AllocType::deallocate(value);
        };

        std::unique_ptr<char[], decltype(deallocate)> buff(
            AllocType::allocate(inpSize), deallocate);

        // Run...
        bool ret = false;
        if (std::size_t size = input.read(buff.get(), inpSize); size != 0) {
            ret = decode_digest(output, buff.get(), size);
        }

        return ret;
    }
} // namespace steg
