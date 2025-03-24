/* encoder.cpp -- v1.0 */

#include "encoder.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "error.hpp"
#include <cstddef>
#include <gcrypt.h>

steg::Encoder::~Encoder()
{
    if (cph_) {
        cipher_close(*cph_);
        delete cph_;
    }
}

/*! Encodes data in-place
 */
bool steg::Encoder::encode(char* const data, std::size_t size)
{
    Cipher& cph = *cph_;
    // Do an in-place encryption
    std::size_t ret = gcry_cipher_encrypt(
        reinterpret_cast<gcry_cipher_hd_t>(cph.hd), data, size, nullptr, 0);
    if (ret == 0) {
        return true;
    }

    const char* const strerror = gcry_strerror(ret);
    const char* const strsource = gcry_strsource(ret);
    // Report error
    return ((Error::get())->log("Error: ", strsource, ", ", strerror), false);
}

/*! Encodes data
 */
bool steg::Encoder::encode(const char* data,
                           std::size_t size,
                           char* const out,
                           std::size_t outSize)
{
    Cipher& cph = *cph_;
    // Do an in-place encryption
    std::size_t ret = gcry_cipher_encrypt(
        reinterpret_cast<gcry_cipher_hd_t>(cph.hd), out, outSize, data, size);
    if (ret == 0) {
        return true;
    }

    const char* const strerror = gcry_strerror(ret);
    const char* const strsource = gcry_strsource(ret);
    // Report error
    return ((Error::get())->log("Error: ", strsource, ", ", strerror), false);
}
