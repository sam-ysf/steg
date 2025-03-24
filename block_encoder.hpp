/* block_encoder.hpp -- v1.0
   Facade for encoding messages and writing results to output */

#pragma once

#include "base64.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include <cmath>
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
        inline static BlockEncoder* create(const char* key,
                                           const char* initvec);

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
        inline bool run(Tinp& inp, Tout& out);
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
        bool encode_digest(
            Tout& out,
            char* buff,
            typename std::enable_if<!vvb64, std::size_t>::type size)
        {
            // Encode raw data to digest
            if (Encoder::encode(buff, size)) {
                // Pipe to output and return
                if (out.write(buff, size)) {
                    return true;
                }
            }

            return false;
        }

        /* Helper
         * Generates a digest from the message and encodes the result to base64
         */
        template <typename Tout, bool vvb64 = b64>
        bool encode_digest(
            Tout& out,
            char* buff,
            typename std::enable_if<vvb64, std::size_t>::type size)
        {
            // Encode raw data to digest
            if (!Encoder::encode(buff, size)) {
                return false;
            }

            // Encode digest to base64...
            // Calculate base64 size
            std::size_t b64size;

            if (size % 3) {
                b64size = (size + 3 - (size % 3)) * 4 / 3;
            }

            else {
                b64size = size * 4 / 3;
            }

            // Encode to base64
            char* b64buff = Talloc::allocate(b64size);
            base64_encode(buff, size, b64buff);

            // Pipe to output
            out.write(b64buff, b64size);

            // Cleanup & return
            return (Talloc::deallocate(b64buff), true);
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

        std::size_t size = inp.read(buff, inpSize);
        if (size != 0) {
            ret = encode_digest(out, buff, calc_digest_size(size));
        }

        // Clean up & return
        return (Talloc::deallocate(buff), ret);
    }
} // namespace steg
