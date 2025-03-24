/* decoder.hpp -- v1.0 
   Message decoder that uses libgcrypt */

#pragma once

#include <cstdint>

namespace steg {
    // Fwd. decl.
    struct Cipher;

    //! @class Decoder
    //! Base class
    class Decoder {
    public:
        //! Dtor.
        virtual ~Decoder();

        //! Ctor.
        //! @param cph the cipher implementation
        explicit Decoder(Cipher* cph)
            : cph_(cph)
        {}

        //! Default Ctor.
        Decoder() = default;

        //! Decodes data in-place
        //! @param data Input buffer, padded to a multiple of the cipher block length
        //! @param size Size of input buffer
        //! @return True on success, false otherwise
        bool decode(char* data, std::size_t size);

        //! Decodes data
        //! @param data Input buffer, padded to a multiple of the cipher block length
        //! @param size Size of input buffer
        //! @param out Output buffer, will contain result of decoding operation
        //! @param outSize Size of output buffer
        //! @return True on success, false otherwise
        bool decode(const char* data,
                    std::size_t size,
                    char* out,
                    std::size_t outSize);

        //! @return Encapsulated cipher
        Cipher* get()
        {
            return cph_;
        }

        //! @return Encapsulated cipher
        const Cipher* get() const
        {
            return cph_;
        }
    private:
        Cipher* cph_; // > the cipher implementation
    };
} // namespace steg
