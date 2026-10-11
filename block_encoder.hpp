/* block_encoder.hpp -- v1.0
   Facade for encoding messages and writing results to output */

#pragma once

#include "base64.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "stream_utils.hpp"
#include <cstring>
#include <gcrypt.h>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <utility>

namespace steg {
    // @class
    template <bool EnableB64, typename AllocType>
    class BlockEncoder : private Encoder {
    public:
        //! Factory method, returns a BlockEncoder
        //! @param key AES key string
        //! @param initVec Initialization vector string
        static BlockEncoder* create(const char* key, const char* initVec);

        //! Dtor.
        ~BlockEncoder() override = default;

        //! Encrypts input message and writes resulting image to output
        //! @param inp Input stream
        //! @param out Output stream
        //! @return Boolean flag indicating success or failure
        template <typename InputType, typename OutputType>
        bool run(InputType& inp, OutputType& out);
    private:
        /*! Ctor. Private, use factory method create() instead
         */
        explicit BlockEncoder(std::unique_ptr<Cipher> cipher)
            : Encoder(std::move(cipher))
        {}
    };

    /*! Factory method
     */
    template <bool EnableB64, typename AllocType>
    BlockEncoder<EnableB64, AllocType>* BlockEncoder<EnableB64, AllocType>::
        create(const char* key, const char* initVec)
    {
        // Use gcrypt to initialize cipher before passing it to the encoder
        auto cipher = cipher_init(key, initVec);
        if (cipher == nullptr) {
            return nullptr;
        }
        return new BlockEncoder(std::move(cipher));
    }

    /*! Encrypts input message and writes resulting image to output
     */
    template <bool EnableB64, typename AllocType>
    template <typename InputType, typename OutputType>
    bool BlockEncoder<EnableB64, AllocType>::run(InputType& inp,
                                                 OutputType& out)
    {
        // Input read size
        std::optional<std::size_t> inpSize = inp.size();
        if (!inpSize.has_value()) {
            return false;
        }

        if (inpSize.value() > std::numeric_limits<std::size_t>::max()
                                  - Cipher::kEnvelopeHeaderSize
                                  - Cipher::kGcmTagSize) {
            return false;
        }

        const std::size_t envelopeSize = Cipher::kEnvelopeHeaderSize
                                         + inpSize.value()
                                         + Cipher::kGcmTagSize;
        auto deallocate = [](char* value) {
            AllocType::deallocate(value);
        };
        std::unique_ptr<char[], decltype(deallocate)> buff(
            AllocType::allocate(envelopeSize), deallocate);

        std::memcpy(buff.get(),
                    Cipher::kEnvelopeMagic.data(),
                    Cipher::kEnvelopeMagic.size());
        auto* nonce = reinterpret_cast<unsigned char*>(
            buff.get() + Cipher::kEnvelopeMagic.size());
        gcry_randomize(nonce, Cipher::kGcmNonceSize, GCRY_STRONG_RANDOM);

        const std::uint64_t payloadSize = inpSize.value();
        for (std::size_t index = 0; index < sizeof(payloadSize); ++index) {
            buff.get()[Cipher::kEnvelopeMagic.size() + Cipher::kGcmNonceSize
                       + index]
                = static_cast<char>(payloadSize >> (56 - (index * 8)));
        }

        char* payload = buff.get() + Cipher::kEnvelopeHeaderSize;
        if (inp.read(payload, inpSize.value()) != inpSize.value()) {
            return false;
        }

        char* tag = payload + inpSize.value();
        if (!Encoder::encode(payload,
                             inpSize.value(),
                             payload,
                             nonce,
                             Cipher::kGcmNonceSize,
                             buff.get(),
                             Cipher::kEnvelopeHeaderSize,
                             tag,
                             Cipher::kGcmTagSize)) {
            return false;
        }

        if constexpr (EnableB64) {
            if (envelopeSize > std::numeric_limits<std::size_t>::max() - 2) {
                return false;
            }
            const std::size_t groups = (envelopeSize + 2) / 3;
            if (groups > std::numeric_limits<std::size_t>::max() / 4) {
                return false;
            }
            const std::size_t b64Size = groups * 4;
            std::unique_ptr<char[], decltype(deallocate)> b64buff(
                AllocType::allocate(b64Size), deallocate);
            base64_encode(std::span{buff.get(), envelopeSize}, b64buff.get());
            return detail::write_all(out, b64buff.get(), b64Size);
        }

        return detail::write_all(out, buff.get(), envelopeSize);
    }
} // namespace steg
