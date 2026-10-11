#pragma once

#include <cstddef>

namespace steg::detail {

    template <typename OutputType>
    bool write_all(OutputType& output, const char* data, std::size_t size)
    {
        std::size_t written = 0;
        while (written < size) {
            std::size_t nbytes = output.write(data + written, size - written);
            if (nbytes == 0) {
                return false;
            }
            written += nbytes;
        }
        return true;
    }
} // namespace steg::detail