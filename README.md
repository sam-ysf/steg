# steg
A command line steganography program
--------------------------------------------------------------------------------

Steganography is the practice of hiding data inside other data. This program is a command line utility that performs steganography by embedding an encrypted payload (using a 128-bit AES block cipher) into an existing image file.

Each bit `{0|1}` of the original payload's binary representation is summed to successive pixel values of the original image, resulting in a modification that is slight enough to look similar to the unaided human eye. In principle, any type of file can be encoded provided that the source image has enough pixels to accomodate the file size.


Usage
--------------------------------------------------------------------------------
The easiest way to run this program is by modifying the included bash scripts
`scripts/encode.sh` and `scripts/decode.sh` using your own parameters.

<strong>IMPORTANT</strong>: users must provide their own AES key and initialization vectors in
separate files.


```
Usage: steg {--encode|--decode|-h}
             -f<encoded-image-source>
            [-o<output-file>]
            [-t<output-file-type>]
             -k<crypt-key-file>
             -v<init-vec-file>
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
  -f<image-source>             Source file for image that the message will be encoded to
  -o<output-file>              Outputs to this file
  -t<output-file-type>         Accepted types: png, bmp, or tga

  -k<crypt-key-file>           AES cryptographic key file
  -v<init-vec-file>            Initialization vector file

  -i<message-file>             Source file of message; if left unspecified, source is the terminal (stdin)
  -b                           Encodes the encrypted output as a base64 string
```

Decode Mode
--------------------------------------------------------------------------------
```
  -f<encoded-image>            Source file of encoded message
  -o<output-file>              Outputs to this filel if left unspecified, outpts to the terminal (stdout)

  -k<crypt-key-file>           AES cryptographic key file

  -v<init-vec-file>            Initialization vector file
  -b                           Required if the encryption output was a base64 string
```

Build
--------------------------------------------------------------------------------
```Bash
mkdir steg/build && cd steg/build
cmake -DCMAKE_BUILD_TYPE={release | debug} ..
cmake --build .
cmake --install . # To install to /usr/local/bin/steg
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

Initialization vector\
<https://wikipedia.org/wiki/Initialization_vector>


--------------------------------------------------------------------------------
This software is entirely in the public domain and is provided as is, without
restricitions. See the LICENSE for more information.
