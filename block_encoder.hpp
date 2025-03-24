/* block_encoder.hpp -- v1.0
   Facade for encoding messages and writing results to output */

#pragma once

#include "base64.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include <cmath>
#include <span>
#include <type_traits>
#include <utility>

namespace steg {
    // @class
    template <bool b64, typename Talloc>
    class BlockEncoder : private Encoder {
    public:
        //! Factory method, returns a BlockEncoder
        //! @param key AES key string
        //! @param initvec Initialization vector string
        static BlockEncoder* create(const char* key, const char* initvec);

        //! Dtor.
        ~BlockEncoder() override = default;

        BlockEncoder()
            : Encoder(nullptr)
        {}

        //! Encrypts input message and writes resulting image to output
        //! @param inp Input stream
        //! @param out Output stream
        //! @return Boolean flag indicating success or failure
        template <typename Tinp, typename Tout>
        bool run(Tinp& inp, Tout& out);
    private:
        /* Helper
         * Pads the buffer size to a multiple of the digest length
         */
        std::size_t calc_digest_size(std::size_t size) const
        {
            std::size_t mod = size % (Encoder::get())->length;
            if (mod == 0) {
                return size;
            }

            return (size + (Encoder::get()->length - mod));
        }

        /* Helper
         * Generates a digest from the message
         */
        template <typename Tout, bool vvb64 = b64>
        bool encode_digest(Tout& out,
                           char* buff,
                           std::enable_if_t<!vvb64, std::size_t> size)
        {
            // Try to encode raw data to digest
            // and pipe to output
            return Encoder::encode(buff, size) && out.write(buff, size);
        }

        /* Helper
         * Generates a digest from the message and encodes the result to base64
         */
        template <typename Tout, bool vvb64 = b64>
        bool encode_digest(Tout& out,
                           char* buff,
                           std::enable_if_t<vvb64, std::size_t> size)
        {
            // Encode raw data to digest
            if (!Encoder::encode(buff, size)) {
                return false;
            }

            // Encode digest to base64...
            // Calculate base64 size
            std::size_t b64size = ((size + 2) / 3) * 4;

            // Encode to base64
            char* b64buff = Talloc::allocate(b64size);
            base64_encode(std::span{buff, size}, b64buff);

            // Pipe to output
            bool ret = out.write(b64buff, b64size);

            Talloc::deallocate(b64buff);
            return ret;
        }

        /*! Ctor. Private, use factory method create() instead
         */
        explicit BlockEncoder(Cipher* cph)
            : Encoder(cph)
        {}
    };

    /*! Factory method
     */
    template <bool b64, typename Talloc>
    BlockEncoder<b64, Talloc>* BlockEncoder<b64, Talloc>::create(
        const char* key,
        const char* initvec)
    {
        // Use gcrypt to initialize cipher before passing it to the encoder
        Cipher* cph = cipher_init(key, initvec);
        if (cph == nullptr) {
            return nullptr;
        }
        return new BlockEncoder(std::move(cph));
    }

    /*! Encrypts input message and writes resulting image to output
     */
    template <bool b64, typename Talloc>
    template <typename Tinp, typename Tout>
    bool BlockEncoder<b64, Talloc>::run(Tinp& inp, Tout& out)
    {
        // Input read size
        std::size_t inpSize = inp.size();
        if (inpSize == 0) {
            inpSize = 100000; // Default size for unknown size input
        }

        // Padded to a multiple of the block size
        char* buff = Talloc::allocate(calc_digest_size(inpSize) + 1);

        // Run...
        bool ret = false;

        if (std::size_t size = inp.read(buff, inpSize); size != 0) {
            ret = encode_digest(out, buff, calc_digest_size(size));
        }

        // Clean up & return
        Talloc::deallocate(buff);
        return ret;
    }
} // namespace steg
