/* cipher.hpp -- v1.0
   Datatype that defines a cipher handle and the cipher length */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace steg {
    //! @class cipher
    struct Cipher {
        static constexpr std::size_t kGcmNonceSize = 12;
        static constexpr std::size_t kGcmTagSize = 16;
        static constexpr std::size_t kCipherContextSize = 16;
        static constexpr std::array<char, 4> kEnvelopeMagic{'S', 'T', 'G', '2'};
        static constexpr std::size_t kEnvelopeHeaderSize
            = kEnvelopeMagic.size() + kGcmNonceSize + sizeof(std::uint64_t);

        void* hd = nullptr;
        std::array<unsigned char, kCipherContextSize> context{};

        ~Cipher();

        Cipher() = default;
        Cipher(const Cipher&) = delete;
        Cipher& operator=(const Cipher&) = delete;
    };
} // namespace steg
