/* image.cpp -- v1.0 */

#include "image.hpp"
#include "error.hpp"
#include "stb.hpp"

steg::Image::~Image()
{
    if (data_ != nullptr) {
        stbi_image_free(data_);
    }
}

steg::Image::Image(Image&& other) noexcept
    : data_(other.data_)
    , w_(other.w_)
    , h_(other.h_)
    , nchanns_(other.nchanns_)
{
    other.data_ = nullptr;
    other.w_ = 0;
    other.h_ = 0;
    other.nchanns_ = 0;
}

steg::Image& steg::Image::operator=(Image&& other) noexcept
{
    data_ = other.data_;
    w_ = other.w_;
    h_ = other.h_;
    nchanns_ = other.nchanns_;

    other.data_ = nullptr;
    other.w_ = 0;
    other.h_ = 0;
    other.nchanns_ = 0;

    return *this;
}

/*! Gets image size in pixels.
 */
std::size_t steg::Image::size() const
{
    // Width x Height x # channels
    return w_ * h_ * nchanns_;
}

/*! Saves image to file.
 */
bool steg::Image::save(const char* path, const ImageType type) const
{
    // Just in case
    if (!path) {
        return ((Error::get())->log("Error:", "Invalid save path"), false);
    }

    switch (type) {
        case kPng:
        {
            const int stride = nchanns_ * w_;
            const int ret
                = stbi_write_png(path, w_, h_, nchanns_, data_, stride);
            if (ret == 0)
                (Error::get())->log("Error:", "Unable to save image", path);
            return ret != 0;
        }

        case kBmp:
        {
            const int ret = stbi_write_bmp(path, w_, h_, nchanns_, data_);
            if (ret == 0)
                (Error::get())->log("Error:", "Unable to save image", path);
            return ret != 0;
        }

        case kTga:
        {
            const int ret = stbi_write_tga(path, w_, h_, nchanns_, data_);
            if (ret == 0)
                (Error::get())->log("Error:", "Unable to save image", path);
            return ret != 0;
        }

        default:
        {
            return false;
        }
    }
}

/*! Loads image from file.
 */
std::size_t steg::Image::open(const char* path)
{
    // Return if image already loaded
    if (data_ != nullptr) {
        return 0;
    }

    // Get the data
    unsigned char* data = stbi_load(path, &w_, &h_, &nchanns_, 0);
    if (data == nullptr) {
        constexpr const char* kMessage = "Unable to load image";
        return ((Error::get())->log("Error:", kMessage, path), 0);
    }

    data_ = data;
    return (w_ * h_ * nchanns_);
}

/*! Reads back message from image.
 */
std::size_t steg::Image::read(char* buff, const std::size_t buffSize)
{
    // Zero-out buffer
    ::memset(buff, 0, buffSize);

    // Width x height
    const std::size_t size = w_ * h_;

    // Just in case
    // Ensure that buffer is large enough
    if ((buffSize * 8) < size) {
        constexpr const char* kMessage
            = "Output buffer is too small to accommodate "
              "message size, exiting";
        return (Error::get())->log("Error:", kMessage), 0;
    }

    // Unapply steganography
    std::size_t i = 0;
    while (i != size) {
        const unsigned char r = data_[(nchanns_ * i)];

        int bit = (r & 0x03);
        // Check for terminating character
        if (bit == 0x02) {
            break; // Reached end of message
        }

        // Parse bit
        *buff |= (bit << (i % 8));
        if ((++i % 8) == 0) {
            ++buff;
        }
    }

    // Terminate and return
    return ((*buff = 0), (i / 8));
}

/*! Writes message to image.
 */
std::size_t steg::Image::write(const char* buff, std::size_t buffSize)
{
    // Width x height
    const std::size_t size = w_ * h_;

    // Ensure that file size is large enough to hold image
    const char* ptr = buff;
    if ((buffSize * 8) > size) {
        constexpr const char* kMessage
            = "Source image is too small to encode entire "
              "message, exiting";
        return ((Error::get())->log("Error:", kMessage), 0);
    }

    // Apply steganography
    // Insert message
    std::size_t i = 0;
    while (i != size && (static_cast<std::size_t>(ptr - buff) != buffSize)) {
        const std::size_t j = nchanns_ * i;

        // Encode single bit
        unsigned char& r = data_[j];
        const int bit = (static_cast<unsigned char>(*ptr >> (i % 8)) & 0x01);

        // Clear the two lower-order bits
        // and add low order bit
        r = ((r & ~0x03) | bit);
        // When done encoding all bits, advance message pointer to next byte
        if ((++i % 8) == 0) {
            ++ptr;
        }
    }

    // "Zero-out" remaining cells using 0x02 as terminating character
    for (; i != size; ++i) {
        unsigned char& r = data_[(nchanns_ * i)];
        r = ((r & ~0x03) | 0x02);
    }

    return size;
}
