#!/bin/sh

${HOST_DIR}/bin/mkimage -A riscv -O linux -T kernel -C none -a 0x00200000 -e 0x00200000 -n Linux -d ${BINARIES_DIR}/Image ${BINARIES_DIR}/uImage

rm -rf ${BUILD_DIR}/boot_ext4
mkdir ${BUILD_DIR}/boot_ext4
cp ${BINARIES_DIR}/uImage ${BUILD_DIR}/boot_ext4/
cp ${BINARIES_DIR}/fw_dynamic.* ${BUILD_DIR}/boot_ext4/
cp ${BINARIES_DIR}/readme.txt ${BUILD_DIR}/boot_ext4/
cp ${BINARIES_DIR}/fw_dynamic.bin ${BUILD_DIR}/boot_ext4/fw_jump.bin
cp ${BUILD_DIR}/linux-custom/arch/riscv/boot/dts/thead/*.dtb ${BUILD_DIR}/boot_ext4/
cp ${BUILD_DIR}/boot_ext4/light_mpw.dtb ${BUILD_DIR}/boot_ext4/hw.dtb

${HOST_DIR}/usr/bin/make_ext4fs -l 30M ${BINARIES_DIR}/boot.ext4 ${BUILD_DIR}/boot_ext4

exit $?
