/* decoder.hpp -- v1.0
   Message decoder that uses libgcrypt */

#pragma once

#include <cstddef>
#include <memory>

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
        explicit Decoder(std::unique_ptr<Cipher> cipher);

        Decoder(const Decoder&) = delete;
        Decoder& operator=(const Decoder&) = delete;
        Decoder(Decoder&&) = delete;
        Decoder& operator=(Decoder&&) = delete;

        bool decode(char* data,
                    std::size_t dataSize,
                    const unsigned char* nonce,
                    std::size_t nonceSize,
                    const char* header,
                    std::size_t headerSize,
                    const char* tag,
                    std::size_t tagSize);

        //! @return Encapsulated cipher
        Cipher* get()
        {
            return cipher_.get();
        }

        //! @return Encapsulated cipher
        const Cipher* get() const
        {
            return cipher_.get();
        }
    private:
        std::unique_ptr<Cipher> cipher_;
    };
} // namespace steg
