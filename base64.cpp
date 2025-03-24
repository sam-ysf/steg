/* base64.cpp -- v1.0 */

#include "base64.hpp"
#include <cstddef>
#include <cstring>

namespace {
    // All allowed base 64 characters
    const char* kBase64Chars
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    /* base64Lookup --->
     * +===/0123456789=======ABCDEFGHIJKLMNOPQRSTUVWXYZ======abcdefghijklmnopqrstuvwxyz
     */
    const unsigned char kBase64Lookup[]
        = {62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64,
           64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
           10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
           64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
           36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
} // namespace

/*! Base64 encode
 */
void steg::base64_encode(const char* value, std::size_t size, char* out)
{
    // Temp. reusable container for the three chars (octets) that are encoded to
    // a four byte array
    char octets[3] = {};

    std::size_t i = 0;
    while (i != size) {
        octets[i % 3] = value[i];
        // Encode octets if next character is at the three character boundary
        if ((++i % 3) == 0) {
            // Encode
            const int indices[4]
                = {((octets[0] & 0xfc) >> 2),
                   ((octets[0] & 0x03) << 4) | ((octets[1] & 0xf0) >> 4),
                   ((octets[1] & 0x0f) << 2) | ((octets[2] & 0xc0) >> 6),
                   ((octets[2] & 0x3f))};

            *out++ = kBase64Chars[indices[0]];
            *out++ = kBase64Chars[indices[1]];
            *out++ = kBase64Chars[indices[2]];
            *out++ = kBase64Chars[indices[3]];

            // Zero-out
            ::memset(octets, 0, sizeof(octets));
        }
    }

    // Return if no trailing characters
    if (i % 3 == 0) {
        return;
    }

    for (; i % 3; ++i) {
        octets[i % 3] = 0;
    }

    // Encode
    const int indices[4]
        = {((octets[0] & 0xfc) >> 2),
           ((octets[0] & 0x03) << 4) | ((octets[1] & 0xf0) >> 4),
           ((octets[1] & 0x0f) << 2) | ((octets[2] & 0xc0) >> 6),
           ((octets[2] & 0x3f))};

    *out++ = kBase64Chars[indices[0]];
    *out++ = kBase64Chars[indices[1]];
    *out++ = kBase64Chars[indices[2]];
    *out++ = kBase64Chars[indices[3]];
}

/*! Base64 in-place decode
 */
std::size_t steg::base64_decode(char* value, std::size_t size)
{
    // Temp. reusable container for the four bytes (hexets) that are decoded to
    // three octets
    char hexets[4] = {};
    char* ptr = value;

    std::size_t i = 0;
    while (i != size) {
        hexets[i % 4] = kBase64Lookup[value[i] - '+'];

        if ((++i % 4) == 0) {
            *ptr++ = ((hexets[0] & 0xff) << 2) | ((hexets[1] & 0x30) >> 4);
            *ptr++ = ((hexets[1] & 0x0f) << 4) | ((hexets[2] & 0x3c) >> 2);
            *ptr++ = ((hexets[2] & 0x03) << 6) | hexets[3];

            ::memset(hexets, 0, sizeof(hexets));
        }
    }

    // Return size
    return static_cast<std::size_t>(ptr - value);
}
