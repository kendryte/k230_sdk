#!/bin/sh

CONTAINER_VER="1.0"
PRODUCT_NAME="k230"
FILES=" sw-description \
        sw-description.sig \
        file_test.txt \
        script_test.sh \
        encrypted.ciphertext"
        
rm -f ${PRODUCT_NAME}_${CONTAINER_VER}.swu
openssl dgst -sha256 -sign ../ota_private.pem sw-description > sw-description.sig
openssl enc -aes-256-cbc -in encrypted.plain -out encrypted.ciphertext -K b6882ccaa19c5cd81f416df91a0991a9d9014dfc894264bc00959fd8c643de8d -iv d17398d1e7388504c3872ab6e0c66a6f
for i in $FILES;do
    echo $i;done | cpio -ov -H crc >  ${PRODUCT_NAME}_${CONTAINER_VER}.swu
    
rm -f sw-description.sig
rm -f encrypted.ciphertext

#cpio -idv < xxx.swu