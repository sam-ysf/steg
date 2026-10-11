/* cipher_ctl.hpp -- v1.0
   Cipher generation and clean up */

#pragma once

#include <memory>

namespace steg {
    // Fwd. decl.
    struct Cipher;

    //! Factory method, used for cipher initialization
    //! @param key
    //!     AES key string
    //! @param context
    //!     Associated context authenticated with the message
    //! @return
    //!     On success, returns a non-null pointer to an initialized cipher
    std::unique_ptr<Cipher> cipher_init(const char* key, const char* context);
} // namespace steg
