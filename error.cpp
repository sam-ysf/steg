/* error.cpp -- v1.0 */

#include "error.hpp"
#include <cstdio>
#include <gcrypt.h>

void steg::Error::print(const char* str)
{
    std::fprintf(stderr, "%s ", str);
}

/*! Pointer to singleton instance.
 */
std::shared_ptr<steg::Error> steg::Error::instance_;
