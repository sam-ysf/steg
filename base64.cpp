/* base64.cpp -- v1.0 */

#include "base64.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace {
    // All allowed base 64 characters
    const char* const kBase64Chars
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    const char kBase64Lookup[]
        = {62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64,
           64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
           10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
           64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
           36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
} // namespace

/*! Base64 encode
 */
void steg::base64_encode(const std::span<char>& value, char* out)
{
    // Temp. reusable container for the three chars (octets) that are encoded to
    // a four byte array
    char octets[3] = {};

    std::size_t i = 0;
    while (i != value.size()) {
        octets[i % 3] = value[i];
        // Encode octets if next character is at the three character boundary
        if ((++i % 3) == 0) {
            // Encode
            const int indices[4]
                = {(octets[0] & 0xfc) >> 2,
                   ((octets[0] & 0x03) << 4) | ((octets[1] & 0xf0) >> 4),
                   ((octets[1] & 0x0f) << 2) | ((octets[2] & 0xc0) >> 6),
                   (octets[2] & 0x3f)};

            *out++ = kBase64Chars[indices[0]];
            *out++ = kBase64Chars[indices[1]];
            *out++ = kBase64Chars[indices[2]];
            *out++ = kBase64Chars[indices[3]];

            // Zero-out
            std::memset(octets, 0, sizeof(octets));
        }
    }

    // Return if no trailing characters
    if ((i % 3) == 0) {
        return;
    }

    int indices[4] = {64, 64, 64, 64};

    if ((i % 3) == 1) {
        indices[0] = (octets[0] & 0xfc) >> 2;
        indices[1] = (octets[0] & 0x03) << 4;
    }

    else if ((i % 3) == 2) {
        indices[0] = (octets[0] & 0xfc) >> 2;
        indices[1] = ((octets[0] & 0x03) << 4) | ((octets[1] & 0xf0) >> 4);
        indices[2] = (octets[1] & 0x0f) << 2;
    }

    *out++ = kBase64Chars[indices[0]];
    *out++ = kBase64Chars[indices[1]];
    *out++ = kBase64Chars[indices[2]];
    *out++ = kBase64Chars[indices[3]];
}

/*! Base64 in-place decode
 */
std::size_t steg::base64_decode(char* value, std::size_t size)
{
    if (size % 4 != 0) {
        throw std::runtime_error("Invalid Base64 length");
    }

    std::size_t decodedSize = size / 4 * 3;
    if (size != 0 && value[size - 1] == '=') {
        --decodedSize;
        if (value[size - 2] == '=') {
            --decodedSize;
        }
    }

    char* ptr = value;

    // Temp. reusable container for the four bytes (hexets) that are decoded
    // to three octets
    char hexets[4] = {};

    for (std::size_t i = 0; i < size;) {
        // Just in case
        if (static_cast<int>(value[i]) < static_cast<int>('+')) {
            throw std::runtime_error("Error: Out-of-bounds byte in input");
        }

        // Just in case
        if (static_cast<int>(value[i]) > static_cast<int>('z')) {
            throw std::runtime_error("Error: Out-of-bounds byte in input");
        }

        hexets[i % 4] = kBase64Lookup[value[i] - '+'];

        if ((++i % 4) != 0) {
            continue;
        }

        const auto j = static_cast<std::size_t>(ptr - value);

        auto x1 = static_cast<char>((hexets[0] & 0xff) << 2);
        auto y1 = static_cast<char>((hexets[1] & 0x30) >> 4);
        *ptr++ = (x1 | y1);

        if (((j / 4) * 3) + 1 < decodedSize) {
            auto x2 = static_cast<char>((hexets[1] & 0x0f) << 4);
            auto y2 = static_cast<char>((hexets[2] & 0x3c) >> 2);
            *ptr++ = (x2 | y2);
        }

        if (((j / 4) * 3) + 2 < decodedSize) {
            auto x3 = static_cast<char>((hexets[2] & 0x03) << 6);
            char y3 = hexets[3];
            *ptr++ = (x3 | y3);
        }

        std::memset(hexets, 0, sizeof(hexets));
    }

    // Return size
    return decodedSize;
}
