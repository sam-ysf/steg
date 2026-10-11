# steg
A command line steganography program
--------------------------------------------------------------------------------

Steganography is the practice of hiding data inside other data. This program is a command line utility that performs steganography by embedding an authenticated encrypted payload (using AES-128-GCM) into an existing image file.

Each bit `{0|1}` of the original payload's binary representation is directly encoded into the pixel values of the original image, resulting in a 1-bit modification to each pixel that is slight enough to look similar to the unaided human eye. In principle, any type of file can be encoded provided that the source image has enough pixels to accomodate the file size.


Usage
--------------------------------------------------------------------------------
The easiest way to run this program is by modifying the included bash scripts
`scripts/encode.sh` and `scripts/decode.sh` using your own parameters.

<strong>IMPORTANT</strong>: users must provide an AES-128 key and a 16-byte authentication-context file in separate files. Each encoded message gets a random GCM nonce stored in its payload; the context file is authenticated with the message and must be supplied again when decoding.

The current payload format is versioned and authenticated. Images encoded by older versions are not compatible with this format.


```
Usage: steg {--encode|--decode|-h}
             -f<encoded-image-source>
            [-o<output-file>]
            [-t<output-file-type>]
             -k<crypt-key-file>
             -v<authentication-context-file>
            [-i<message-file>]
            [-b]
```

```
  --encode                     Encode mode
  --decode                     Decode mode
  --help (-h)                  Prints this message
```


Encode Mode
--------------------------------------------------------------------------------
```
  -f<image-source>                  Source file for image that the message will
                                    be encoded to

  -o<output-file>                   Outputs to this file
  -t<output-file-type>              Accepted types: png, bmp, or tga

  -k<crypt-key-file>                AES cryptographic key file
  -v<authentication-context-file>   16-byte authentication context

  -i<message-file>                  Source file of message
  -b                                Encodes the encrypted output as base64
```

Decode Mode
--------------------------------------------------------------------------------
```
  -f<encoded-image>                 Source file of encoded message
  -o<output-file>                   Outputs to this file

  -k<crypt-key-file>                AES cryptographic key file

  -v<authentication-context-file>   16-byte authentication context
  -b                                Required if the encryption output was base64
```

Build
--------------------------------------------------------------------------------
```Bash
$ git clone https://github.com/sam-ysf/steg
$ mkdir steg/build && cd steg/build
$ cmake -DCMAKE_BUILD_TYPE={Release | Debug} ..
$ cmake --build .
$ cmake --install . # To install to /usr/local/bin/steg
```

Sources and acknowledgements
--------------------------------------------------------------------------------
Steganography\
<https://wikipedia.org/wiki/Steganography>

This software uses the stb library for loading and saving images.\
<https://github.com/nothings/stb>

This software uses libgcrypt to perform encryption.\
<https://github.com/gpg/libgcrypt>

AES encryption\
<https://wikipedia.org/wiki/Advanced_Encryption_Standard>

Authenticated context\
<https://wikipedia.org/wiki/Initialization_vector>


--------------------------------------------------------------------------------
This software is entirely in the public domain and is provided as is, without
restricitions. See the LICENSE for more information.
