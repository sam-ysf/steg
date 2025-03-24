/* encoder.hpp -- v1.0 -- 
   Message encoder that uses libgcrypt */

#pragma once

#include <cstdint>

namespace steg {
    // Fwd. decl.
    struct Cipher;

    //! @class: Encoder
    class Encoder {
    public:
        //! Dtor.
        virtual ~Encoder();

        //! Ctor.
        //! @param cph the cipher implementation
        explicit Encoder(Cipher* cph)
            : cph_(cph)
        {}

        //! Default Ctor.
        Encoder() = default;

        //! Encodes data in-place
        //! @param data Input buffer, padded to a multiple of the cipher block length
        //! @param size Size of input buffer
        //! @return True on success, false otherwise
        bool encode(char* data, std::size_t size);

        //! Encodes data
        //! @param data Input buffer, padded to a multiple of the cipher block length
        //! @param size Size of input buffer
        //! @param out Output buffer, will contain result of encoding operation
        //! @param outSize Size of output buffer
        //! @return True on success, false otherwise
        bool encode(const char* data,
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
        // The cipher implementation
        Cipher* cph_;
    };
} // namespace steg
