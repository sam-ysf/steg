/* image.cpp -- v1.0 */

#include "image.hpp"
#include "error.hpp"
#include "stb.hpp"
#include <cstring>
#include <limits>

steg::Image::~Image()
{
    if (data_ != nullptr) {
        stbi_image_free(data_);
    }
}

steg::Image::Image(Image&& other) noexcept
    : data_(other.data_)
    , width_(other.width_)
    , height_(other.height_)
    , channels_(other.channels_)
{
    other.data_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
}

steg::Image& steg::Image::operator=(Image&& rhs) noexcept
{
    if (this == &rhs) {
        return *this;
    }

    if (data_ != nullptr) {
        stbi_image_free(data_);
    }

    data_ = rhs.data_;
    width_ = rhs.width_;
    height_ = rhs.height_;
    channels_ = rhs.channels_;

    rhs.data_ = nullptr;
    rhs.width_ = 0;
    rhs.height_ = 0;
    rhs.channels_ = 0;

    return *this;
}

/*! Gets image size in pixels.
 */
std::size_t steg::Image::size() const
{
    return static_cast<std::size_t>(width_) * height_ * channels_;
}

/*! Saves image to file.
 */
bool steg::Image::save(const char* path, const ImageType type) const
{
    // Just in case
    if (!path) {
        (Error::get())->log("Error:", "Invalid save path");
        return false;
    }

    auto width = static_cast<int>(width_);
    auto height = static_cast<int>(height_);
    auto channels = static_cast<int>(channels_);

    int write_ret = 0;
    switch (type) {
        case ImageType::kNil:
        {
            return false;
        }

        case ImageType::kPng:
        {
            int stride = channels * width;

            write_ret
                = stbi_write_png(path, width, height, channels, data_, stride);
            break;
        }

        case ImageType::kBmp:
        {
            write_ret = stbi_write_bmp(path, width, height, channels, data_);
            break;
        }

        case ImageType::kTga:
        {
            write_ret = stbi_write_tga(path, width, height, channels, data_);
            break;
        }
    }

    if (write_ret == 0) {
        (Error::get())->log("Error:", "Unable to save image", path);
        return false;
    }

    return true;
}

/*! Loads image from file.
 */
std::size_t steg::Image::open(const char* path)
{
    if (data_ != nullptr) {
        return 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (data == nullptr) {
        (Error::get())->log("Error:", "Unable to load image", path);
        return 0;
    }

    // Just in case
    if (width <= 0 || height <= 0 || channels <= 0) {
        stbi_image_free(data);
        return 0;
    }

    const std::size_t numeric_max = std::numeric_limits<std::size_t>::max();
    if (static_cast<std::size_t>(width)
        > (numeric_max / static_cast<std::size_t>(height))) {
        stbi_image_free(data);
        (Error::get())->log("Error:", "Image dimensions are too large", path);
        return 0;
    }

    std::size_t npixels
        = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (npixels > (numeric_max / static_cast<std::size_t>(channels))) {
        stbi_image_free(data);
        (Error::get())->log("Error:", "Image dimensions are too large", path);
        return 0;
    }

    width_ = static_cast<std::size_t>(width);
    height_ = static_cast<std::size_t>(height);
    channels_ = static_cast<std::size_t>(channels);
    data_ = data;
    return size();
}

/*! Reads back message from image.
 */
std::size_t steg::Image::read(char* buff, std::size_t buffSize) const
{
    // Just in case
    if (data_ == nullptr) {
        return 0;
    }

    // Just in case
    if (buff == nullptr || buffSize == 0) {
        return 0;
    }

    std::size_t npixels = width_ * height_;
    if (buffSize < ((npixels / 8) + 1)) {
        (Error::get())
            ->log("Error:",
                  "Output buffer is too small to accommodate message size");
        return 0;
    }

    std::memset(buff, 0, buffSize);

    char* curr = buff;

    std::size_t i = 0;
    while (i < npixels) {
        unsigned char value = data_[channels_ * i];
        unsigned bit = value & 0x03;
        if (bit == 0x02) {
            break;
        }

        *curr = static_cast<char>(static_cast<unsigned char>(*curr)
                                  | (bit << (i % 8)));
        if ((++i % 8) == 0) {
            ++curr;
        }
    }

    *curr = 0;
    return i / 8;
}

/*! Writes message to image.
 */
std::size_t steg::Image::write(const char* buff, std::size_t buffSize)
{
    // Just in case
    if (data_ == nullptr) {
        return 0;
    }

    // Just in case
    if (buff == nullptr || buffSize == 0) {
        return 0;
    }

    const std::size_t pixelCount = static_cast<std::size_t>(width_) * height_;
    if (buffSize > pixelCount / 8) {
        (Error::get())
            ->log("Error:",
                  "Source image is too small to encode entire message");
        return 0;
    }

    const char* ptr = buff;
    std::size_t i = 0;
    while ((i < pixelCount) && ((i / 8) < buffSize)) {
        const std::size_t offset = i * channels_;
        unsigned char value = data_[offset];
        unsigned char bit = (*ptr >> (i % 8)) & 0x01;
        data_[offset] = (value & ~0x03) | bit;
        if (++i % 8 == 0) {
            ++ptr;
        }
    }

    for (; i < pixelCount; ++i) {
        std::size_t offset = i * channels_;
        data_[offset] = (data_[offset] & ~0x03) | 0x02;
    }

    return buffSize;
}
