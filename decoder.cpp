/* decoder.cpp -- v1.0 */

#include "decoder.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "error.hpp"
#include <cstddef>
#include <gcrypt.h>

steg::Decoder::~Decoder()
{
    if (cph_) {
        cipher_close(*cph_);
        delete cph_;
    }
}

/*! @brief Decodes data in-place.
 */
bool steg::Decoder::decode(char* const data, const std::size_t size)
{
    Cipher& cph = *cph_;
    // Do an in-place decryption
    std::size_t ret = gcry_cipher_decrypt(
        reinterpret_cast<gcry_cipher_hd_t>(cph.hd), data, size, nullptr, 0);
    if (ret == 0) {
        return true;
    }

    const char* const strerror = gcry_strerror(ret);
    const char* const strsource = gcry_strsource(ret);
    // Report error
    return ((Error::get())->log("Error: ", strsource, ", ", strerror), false);
}

/*! @brief Decodes data.
 */
bool steg::Decoder::decode(const char* const data,
                           const std::size_t size,
                           char* const out,
                           const std::size_t outSize)
{
    Cipher& cph = *cph_;
    // Do an in-place decryption
    std::size_t ret = gcry_cipher_decrypt(
        reinterpret_cast<gcry_cipher_hd_t>(cph.hd), out, outSize, data, size);
    if (ret == 0) {
        return true;
    }

    const char* const strerror = gcry_strerror(ret);
    const char* const strsource = gcry_strsource(ret);
    // Report error
    return ((Error::get())->log("Error: ", strsource, ", ", strerror), false);
}
