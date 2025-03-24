/* cipher.hpp -- v1.0
   Datatype that defines a cipher handle and the cipher length */

#pragma once

#include <cstdint>

namespace steg {
    //! @class cipher
    struct Cipher {
        // Handle to cipher
        void* hd = nullptr;
        // Digest length
        std::size_t length = 0;
    };
} // namespace steg
