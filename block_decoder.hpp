/* block_decoder.hpp -- v1.0
   Facade for decoding messages and writing results to output */

#pragma once

#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "decoder.hpp"
#include <type_traits>

namespace steg {
    //! @class BlockDecoder
    //! @brief Facade for decoding message and writing result to output
    template <bool b64, typename Talloc>
    class BlockDecoder : private Decoder {
    public:
        //! Factory method, returns a BlockDecoder
        //! @param key AES key string
        //! @param initvec Initialization vector string
        //! @return BlockDecoder instance
        inline static BlockDecoder* create(const char* key,
                                           const char* initvec);

        //! Decodes input message and writes to output
        //! @param inp Input stream
        //! @param out Output stream
        //! @return True on success
        template <typename Tinp, typename Tout>
        inline bool run(Tinp& inp, Tout& out);

        ~BlockDecoder() override = default;
    private:
        /*! Helper
         * @brief Decodes from digest to raw data
         * @param out Output stream
         * @param buff Buffer containing data
         * @param size Size of the buffer
         * @return True on success
         */
        template <typename Tout, bool vvb64 = b64>
        bool decode_digest(
            Tout& out,
            char* buff,
            typename std::enable_if<!vvb64, std::size_t>::type size)
        {
            // Decodes from digest to raw data
            bool ret = Decoder::decode(buff, size);
            if (ret) {
                ret = out.write(buff, size); // Pipe to output
            }

            return ret;
        }

        /*! Helper
         * @brief Generates a digest from the message and encodes it to base64
         * @param out Output stream
         * @param buff Buffer containing data
         * @param base64Size Size of the base64 encoded data
         * @return True on success
         */
        template <typename Tout, bool vvb64 = b64>
        bool decode_digest(
            Tout& out,
            char* buff,
            typename std::enable_if<vvb64, std::size_t>::type base64Size)
        {
            // Decode digest from base64
            std::size_t size = base64_decode(buff, base64Size);
            size = size - (size % Decoder::get()->length);

            // Decode raw data from digest
            bool ret = Decoder::decode(buff, size);
            if (ret) {
                ret = out.write(buff, size);
            }

            return ret;
        }

        /*! Ctor. Private, use factory method create() instead
         * @param cph Cipher
         */
        explicit BlockDecoder(Cipher* cph)
            : Decoder(cph)
        {}
    };

    //! Factory method, returns BlockDecoder
    //! @param key AES key string
    //! @param initvec Initialization vector string
    template <bool b64, typename Talloc>
    BlockDecoder<b64, Talloc>* BlockDecoder<b64, Talloc>::create(
        const char* key,
        const char* initvec)
    {
        // Use gcrypt to initialize cipher before passing it to the decoder
        Cipher* cph = cipher_init(key, initvec);
        if (!cph) {
            return nullptr;
        }

        return new BlockDecoder(cph);
    }

    //! Decodes input message and writes to output
    //! @param inp Input stream
    //! @param out Output stream
    //! @return True on success
    template <bool b64, typename Talloc>
    template <typename Tinp, typename Tout>
    bool BlockDecoder<b64, Talloc>::run(Tinp& inp, Tout& out)
    {
        // Input read size
        std::size_t inpSize = inp.size();
        if (inpSize == 0) {
            inpSize = 100000; // Default size for unknown size input
        }

        // Generate the read buffer,
        // padded to a multiple of the block size
        char* buff = Talloc::allocate(inpSize);

        // Run...
        bool ret = false;

        std::size_t size = inp.read(buff, inpSize);
        if (size != 0) {
            ret = decode_digest(out, buff, size);
        }

        // Clean up & return
        return (Talloc::deallocate(buff), ret);
    }
} // namespace steg
