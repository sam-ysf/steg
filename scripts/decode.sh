#!/bin/bash
# This script decides a payload using the supplied parameters

# Edit these -------------------------------------------------------------------
# Specify the path of the binary executable
APP="steg"

# Specify the encoded image filename
ENCODED_FILE="encoded-image"
# Specify the decoded payload's output filename
OUTPUT_FILE="decoded-file"
# Specify the AES-128 key filename
KEY_FILE="key-file.txt"
# Specify the initialization vector
INIT_V_FILE="initvec-file.txt"
# (Optional) Decrypt from base64-encrypted payload, yes / no
USEBASE64=0

# ------------------------------------------------------------------------------
BASE64=""
if [ ${USEBASE64} ] ;
then
    BASE64="-b"
fi

# Run...
${APP} --decode\
       -f ${ENCODED_FILE}\
       -o ${OUTPUT_FILE}\
       -k ${KEY_FILE}\
       -v ${INIT_V_FILE}\
       ${BASE64}
