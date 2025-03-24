/* image.hpp -- v1.0
   Used for loading, saving, reading, and writing to a source image */

#pragma once

#include <cstdint>

namespace steg {
    //! @class
    class Image {
    public:
        //! Type of image file
        enum ImageType : std::uint8_t { kNil, kPng, kBmp, kTga };

        //! dtor.
        ~Image();

        //! ctor.
        Image() = default;

        //! ctor.
        //! No copy ctor. defined, this is a non-copyable object
        Image(Image&& other) noexcept;

        //! Assignment operator
        //! No copy assignment operator defined, this is a non-copyable object
        Image& operator=(Image&& other) noexcept;

        // Non-copyable object
        Image(Image&) = delete;
        Image(const Image&) = delete;

        //! @return the size of the message
        std::size_t size() const;

        //! Saves file
        //! @param path output image path
        //! @param type output image file type
        //! @return true on success, false otherwise
        bool save(const char* path, ImageType type) const;

        //! Loads file
        //! @param path path/to/image/file
        //! @return image size
        std::size_t open(const char* path);

        //! Reads message from image
        //! @param buff[out] unallocated output buffer
        //! @param buffSize size of buff
        //! @return number of bytes read
        std::size_t read(char* buff, std::size_t buffSize);

        //! Writes message to image
        //! @param buff input message [in]
        //! @param buffSize input message size [in]
        //! @return number of bytes written
        std::size_t write(const char* buff, std::size_t buffSize);
    private:
        // Image data
        unsigned char* data_ = nullptr;

        // Image width, height, no. of channels
        int w_ = 0, h_ = 0, nchanns_ = 0;
    };
} // namespace steg
