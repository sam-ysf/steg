/* error.cpp -- v1.0 */

#include "error.hpp"
#include <cstdio>
#include <gcrypt.h>

/* Pointer to singleton instance.
 */
std::shared_ptr<steg::Error> steg::Error::instance_;

/* Access lock
 */
std::mutex steg::Error::acces_lock_;
