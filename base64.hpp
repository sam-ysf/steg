/* base64.hpp -- v1.0
   Base64 encoding and decoding */

#pragma once

#include <cstdint>

namespace steg {
    //! Encodes string to base64
    //! @param value[in]
    //!     Padded input string
    //! @param size[in]
    //!     Data size
    //! @param output[out]
    //!     Output string, padded to necessary base64 expansion
    void base64_encode(const char* value, std::size_t size, char* output);

    //! Decodes string (in-place) from base64
    //! @param value[in/out]
    //!     Input string
    //! @param size[in]
    //!     Data size [in]
    //! @return
    //!     Number of bytes decoded
    std::size_t base64_decode(char* value, std::size_t size);
} // namespace steg
