/* encoder.hpp -- v1.0 --
   Message encoder that uses libgcrypt */

#pragma once

#include <cstdint>
#include <memory>

namespace steg {
    // Fwd. decl.
    struct Cipher;

    //! @class: Encoder
    class Encoder {
    public:
        //! Dtor.
        virtual ~Encoder();

        //! Ctor.
        //! @param cipher the cipher implementation
        explicit Encoder(std::unique_ptr<Cipher> cipher);

        Encoder(const Encoder&) = delete;
        Encoder& operator=(const Encoder&) = delete;
        Encoder(Encoder&&) = delete;
        Encoder& operator=(Encoder&&) = delete;

        bool encode(const char* data,
                    std::size_t dataSize,
                    char* out,
                    const unsigned char* nonce,
                    std::size_t nonceSize,
                    const char* header,
                    std::size_t headerSize,
                    char* tag,
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
        // The cipher implementation
        std::unique_ptr<Cipher> cipher_;
    };
} // namespace steg
