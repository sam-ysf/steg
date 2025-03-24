#!/bin/bash
# This script encodes a payload using the supplied parameters

# Edit these -------------------------------------------------------------------
# Specify the path of the binary executable
APP="steg"

# Specify the source image filename
ENCLOSING_FILE="raw-image.png"
# Specify the payload filename
INPUT_FILE="payload-file"
# Specify the encoded image filename
OUTPUT_FILE="encoded-image"
# Specify the encoded image filetype
OUTPUT_FILE_TYPE="png"
# Specify the AES-128 key filename
KEY_FILE="key-file.txt"
# Specify the initialization vector
INIT_V_FILE="initvec-file.txt"
# (Optional) Encode encrypted payload to base64, yes / no
USEBASE64=0

# ------------------------------------------------------------------------------
BASE64=""
if [ ${USEBASE64} ] ;
then
    BASE64="-b"
fi

# Run...
${APP} --encode\
       -f ${ENCLOSING_FILE}\
       -o ${OUTPUT_FILE}\
       -t ${OUTPUT_FILE_TYPE}\
       -k ${KEY_FILE}\
       -v ${INIT_V_FILE}\
       -i ${INPUT_FILE}\
       ${BASE64}
