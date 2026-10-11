/* cipher_ctl.cpp -- v1.0 */

#include "cipher_ctl.hpp"
#include "cipher.hpp"
#include "error.hpp"
#include <cstring>
#include <gcrypt.h>
#include <memory>

namespace {
    // Helper
    inline void log(unsigned ret)
    {
        (steg::Error::get())
            ->log("Error: ", gcry_strsource(ret), ", ", gcry_strerror(ret));
    }
} // namespace

/*! Initializes cipher
 */
steg::Cipher::~Cipher()
{
    if (hd != nullptr) {
        gcry_cipher_close(static_cast<gcry_cipher_hd_t>(hd));
    }
}

std::unique_ptr<steg::Cipher> steg::cipher_init(const char* const key,
                                                const char* const context)
{
    if (key == nullptr || context == nullptr) {
        return nullptr;
    }

    gcry_cipher_hd_t hd;

    // Create a handle for algorithm ALGO to be used in MODE.  FLAGS may
    // be given as an bitwise OR of the gcry_cipher_flags values.
    unsigned ret
        = gcry_cipher_open(&hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_GCM, 0);
    if (ret != 0) {
        log(ret);
        return {};
    }

    // Retrieve the key length in bytes used with algorithm A.
    // Set KEY of length KEYLEN bytes for the cipher handle HD.
    ret = gcry_cipher_setkey(
        hd, key, gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES128));
    if (ret != 0) {
        gcry_cipher_close(hd);
        log(ret);
        return {};
    }

    auto cipher = std::make_unique<Cipher>();
    cipher->hd = hd;
    std::memcpy(cipher->context.data(), context, cipher->context.size());

    return cipher;
}
