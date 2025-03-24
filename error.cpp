/* error.cpp -- v1.0 */

#include "error.hpp"
#include <cstdio>
#include <gcrypt.h>

void steg::Error::print(const char* str)
{
    ::fprintf(stderr, "%s ", str);
}

steg::Error::Error()
{
    instance_ = nullptr;
}

/*! Pointer to singleton instance.
 */
steg::Error* steg::Error::instance_ = nullptr;
