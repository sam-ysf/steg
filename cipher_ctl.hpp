/* cipher_ctl.hpp -- v1.0
   Cipher generation and clean up */

#pragma once

namespace steg {
    // Fwd. decl.
    struct Cipher;

    /*! @brief Closes cipher and deallocates memory
     */
    void cipher_close(Cipher& cph);

    //! Factory method, used for cipher initialization
    //! @param key
    //!     AES key string
    //! @param initvec
    //!     Initialization vector string
    //! @return
    //!     On success, returns a non-null pointer to an initialized cipher
    Cipher* cipher_init(const char* key, const char* initvec);
} // namespace steg
