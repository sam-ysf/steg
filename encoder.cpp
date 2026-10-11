/* encoder.cpp -- v1.0 */

#include "encoder.hpp"
#include "cipher.hpp"
#include "cipher_ctl.hpp"
#include "error.hpp"
#include <cstddef>
#include <gcrypt.h>

steg::Encoder::~Encoder() = default;

steg::Encoder::Encoder(std::unique_ptr<Cipher> cipher)
    : cipher_(std::move(cipher))
{}

bool steg::Encoder::encode(const char* data,
                           std::size_t dataSize,
                           char* out,
                           const unsigned char* nonce,
                           std::size_t nonceSize,
                           const char* header,
                           std::size_t headerSize,
                           char* tag,
                           std::size_t tagSize)
{
    auto* hd = static_cast<gcry_cipher_hd_t>(cipher_->hd);
    unsigned ret = gcry_cipher_setiv(hd, nonce, nonceSize);
    if (ret == 0) {
        ret = gcry_cipher_authenticate(
            hd, cipher_->context.data(), cipher_->context.size());
    }

    if (ret == 0) {
        ret = gcry_cipher_authenticate(hd, header, headerSize);
    }

    if (ret == 0) {
        ret = gcry_cipher_encrypt(hd, out, dataSize, data, dataSize);
    }

    if (ret == 0) {
        ret = gcry_cipher_gettag(hd, tag, tagSize);
    }
    if (ret == 0) {
        return true;
    }

    const char* const strerror = gcry_strerror(ret);
    const char* const strsource = gcry_strsource(ret);
    // Report error
    (Error::get())->log("Error: ", strsource, ", ", strerror);
    return false;
}
