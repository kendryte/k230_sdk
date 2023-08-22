#! /bin/bash

###### step 1 ######
###### generate ota_package.zip ######
if [ ! -d "$1" ]; then
    echo "[error] ota_package is not exist"
    exit -1
fi

zip -r ota_package.zip $1

###### step 2 ######
###### generate ota_package.bin ######
cat ota_upgrade.sh ota_package.zip > ota_package.bin
echo "generate bin: ota_package.bin"
sync
sync

###### step 3 ######
###### generate ota_package.sign ######
if [ ! -f ota_private.pem ]; then
    echo "[error] ota_private.pem is not exist"
    exit -1
fi
private_pem=ota_private.pem
openssl dgst -md5 -sign $private_pem -out ota_package.sign ota_package.bin
echo "generate signature: ota_package.sign"
sync
sync

###### step 4 ######
###### generate ota_package.kpk ######
cat ota_package.sign ota_package.bin > ota_package.kpk
echo "generate kpk: ota_package.kpk"
sync
sync

###### step 5 ######
###### delete temporary files ######
if [ -f ota_package.bin ]; then
    rm -rf ota_package.bin
fi
if [ -f ota_package.sign ]; then
    rm -rf ota_package.sign
fi
if [ -f ota_package.zip ]; then
    rm -rf ota_package.zip
fi
sync
sync
