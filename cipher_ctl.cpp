/* cipher_ctl.cpp -- v1.0 */

#include "cipher_ctl.hpp"
#include "cipher.hpp"
#include "error.hpp"
#include <gcrypt.h>

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
struct steg::Cipher* steg::cipher_init(const char* const key,
                                       const char* const initvec)
{
    gcry_cipher_hd_t hd;
    std::size_t keylen = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES128);
    std::size_t blklen = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES128);

    // Create a handle for algorithm ALGO to be used in MODE.  FLAGS may
    // be given as an bitwise OR of the gcry_cipher_flags values.
    unsigned ret
        = gcry_cipher_open(&hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, 0);
    if (ret != 0) {
        log(ret);
        return nullptr;
    }

    // Retrieve the key length in bytes used with algorithm A.
    // Set KEY of length KEYLEN bytes for the cipher handle HD.
    ret = gcry_cipher_setkey(hd, key, keylen);
    if (ret != 0) {
        gcry_cipher_close(hd);
        log(ret);
        return nullptr;
    }

    ret = gcry_cipher_setiv(hd, initvec, blklen);
    if (ret != 0) {
        gcry_cipher_close(hd);
        log(ret);
        return nullptr;
    }

    return new Cipher({
        .hd = hd,
        .length = blklen // AES block size in bytes
    });
}

/*! Closes cipher and deallocates memory
 */
void steg::cipher_close(steg::Cipher& cph)
{
    auto* hd = static_cast<gcry_cipher_hd_t>(cph.hd);
    gcry_cipher_close(hd);
}
