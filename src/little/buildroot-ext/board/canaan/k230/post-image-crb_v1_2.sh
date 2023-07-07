#!/bin/bash

BOARD_DIR="$(dirname $0)"
GENIMAGE_SD_CFG="${BOARD_DIR}/genimage-sdcard.cfg"
GENIMAGE_SD_TMP="${BUILD_DIR}/genimage-sd.tmp"
GENIMAGE_EMMC_CFG="${BOARD_DIR}/genimage-emmc.cfg"
GENIMAGE_EMMC_TMP="${BUILD_DIR}/genimage-emmc.tmp"


rm -rf "${GENIMAGE_SD_TMP}"
rm -rf "${GENIMAGE_EMMC_TMP}"


if [ -f "${BOARD_DIR}/../../../k510_crb_lp3_v1_2_defconfig/local.mk" ]
then
	OVERRIDE_LINUX="$(cat ${BOARD_DIR}/../../../k510_crb_lp3_v1_2_defconfig/local.mk | grep LINUX)"

	OVERRIDE_UBOOT="$(cat ${BOARD_DIR}/../../../k510_crb_lp3_v1_2_defconfig/local.mk | grep UBOOT)"
fi;


if [ -z "$OVERRIDE_LINUX" ]
then
	KERNEL_VERSION="$(cat ${O}/.config | grep BR2_LINUX_KERNEL_VERSION | awk -F "=" '{print $2}' | sed 's/\"//g')"
	KERNEL_BUILD_DIR="${BUILD_DIR}/linux-$KERNEL_VERSION"
else
	KERNEL_VERSION="custom"
	KERNEL_BUILD_DIR="${BUILD_DIR}/linux-custom"
fi;

if [ -z "$OVERRIDE_UBOOT" ]
then
	UBOOT_VERSION="$(cat ${O}/.config | grep BR2_TARGET_UBOOT_VERSION | awk -F "=" '{print $2}' | sed 's/\"//g')"
	UBOOT_BUILD_DIR="${BUILD_DIR}/uboot-$UBOOT_VERSION"
else
	UBOOT_BUILD_DIR="${BUILD_DIR}/uboot-custom"
fi;


#riscv64-linux-cpp -nostdinc -I ${KERNEL_BUILD_DIR}/include -I ${KERNEL_BUILD_DIR}/arch  -undef -x assembler-with-cpp ${KERNEL_BUILD_DIR}/arch/riscv/boot/dts/k510_evb_lp3_v1_1_sdcard.dts ${BINARIES_DIR}/k510_evb_lp3_v1_1_sdcard.dts.tmp

#${KERNEL_BUILD_DIR}/scripts/dtc/dtc -I dts -o ${BINARIES_DIR}/k510.dtb ${BINARIES_DIR}/k510_evb_lp3_v1_1_sdcard.dts.tmp

#riscv64-linux-cpp -nostdinc -I ${KERNEL_BUILD_DIR}/include -I ${KERNEL_BUILD_DIR}/arch  -undef -x assembler-with-cpp ${KERNEL_BUILD_DIR}/arch/riscv/boot/dts/k510_evb_lp3_v1_1_nfsroot.dts ${BINARIES_DIR}/k510_evb_lp3_v1_1_nfsroot.dts.tmp

#${KERNEL_BUILD_DIR}/scripts/dtc/dtc -I dts -o ${BINARIES_DIR}/k510_nfsroot.dtb ${BINARIES_DIR}/k510_evb_lp3_v1_1_nfsroot.dts.tmp



#riscv64-linux-cpp -nostdinc -I ${KERNEL_BUILD_DIR}/include -I ${KERNEL_BUILD_DIR}/arch  -undef -x assembler-with-cpp ${KERNEL_BUILD_DIR}/arch/riscv/boot/dts/canaan/k510_evb_lp3_v1_1_nfsroot.dts ${BINARIES_DIR}/k510_evb_lp3_v1_1_nfsroot.dts.tmp

#${KERNEL_BUILD_DIR}/scripts/dtc/dtc -I dts -o ${BINARIES_DIR}/k510_nfsroot.dtb ${BINARIES_DIR}/k510_evb_lp3_v1_1_nfsroot.dts.tmp

${UBOOT_BUILD_DIR}/tools/mkenvimage -s 0x2000 -o ${BINARIES_DIR}/uboot-sd.env ${BOARD_DIR}/uboot-sdcard.env
${UBOOT_BUILD_DIR}/tools/mkenvimage -s 0x2000 -o ${BINARIES_DIR}/uboot-emmc.env ${BOARD_DIR}/uboot-emmc.env
${UBOOT_BUILD_DIR}/tools/mkenvimage -s 0x2000 -o ${BINARIES_DIR}/uboot-nfs.env ${BOARD_DIR}/uboot-nfs.env
${UBOOT_BUILD_DIR}/tools/mkenvimage -s 0x2000 -o ${BINARIES_DIR}/uboot-nand.env ${BOARD_DIR}/uboot-nand.env

/usr/bin/python3 ${BOARD_DIR}/app_shaaes.py ${BINARIES_DIR}/u-boot.bin ${BINARIES_DIR}/u-boot_burn.bin
#rm -rf ${BOARD_DIR}/u-boot_burn.bin.aes  ${BOARD_DIR}/u-boot_burn.bin.aes_otp

#${BOARD_DIR}/debian.sh ${KERNEL_VERSION} || { echo "create debian image error "; exit 1; }

riscv64-linux-cpp -nostdinc -I ${KERNEL_BUILD_DIR}/include -I ${KERNEL_BUILD_DIR}/arch  -undef -x assembler-with-cpp ${KERNEL_BUILD_DIR}/arch/riscv/boot/dts/canaan/k510_crb_lp3_v1_2.dts ${BINARIES_DIR}/k510_crb_lp3_v1_2.dts.tmp
riscv64-linux-cpp -nostdinc -I ${KERNEL_BUILD_DIR}/include -I ${KERNEL_BUILD_DIR}/arch  -undef -x assembler-with-cpp ${KERNEL_BUILD_DIR}/arch/riscv/boot/dts/canaan/k510_crb_lp3_hdmi_v1_2.dts ${BINARIES_DIR}/k510_crb_lp3_hdmi_v1_2.dts.tmp

${KERNEL_BUILD_DIR}/scripts/dtc/dtc -I dts -o ${BINARIES_DIR}/k510.dtb ${BINARIES_DIR}/k510_crb_lp3_v1_2.dts.tmp
${KERNEL_BUILD_DIR}/scripts/dtc/dtc -I dts -o ${BINARIES_DIR}/k510-hdmi.dtb ${BINARIES_DIR}/k510_crb_lp3_hdmi_v1_2.dts.tmp

rm -rf  ${BINARIES_DIR}/k510_crb_lp3_v1_2.dts.tmp
rm -rf  ${BINARIES_DIR}/k510_crb_lp3_hdmi_v1_2.dts.tmp

genimage                           \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_SD_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${GENIMAGE_SD_CFG}"

genimage                           \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_EMMC_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${GENIMAGE_EMMC_CFG}"

function nand_image_gen()
{

	GENIMAGE_NAND_CFG="${BOARD_DIR}/genimage-nand.cfg"
	GENIMAGE_NAND_TMP="${BUILD_DIR}/genimage-nand.tmp"
	rm -rf "${GENIMAGE_NAND_TMP}"


	${UBOOT_BUILD_DIR}/tools/mkenvimage -s 0x2000 -o ${BINARIES_DIR}/uboot-emmc.env ${BOARD_DIR}/uboot-emmc.env


	NAND_TARGET_DIR="${TARGET_DIR}/../nand_target" 
	rm -rf "${NAND_TARGET_DIR}"
	mkdir -p "${NAND_TARGET_DIR}"
	fakeroot cp -a  "${TARGET_DIR}"/*  "${NAND_TARGET_DIR}"
	rm -rf ${NAND_TARGET_DIR}/usr/local/bin/ffmpeg
	rm -rf ${NAND_TARGET_DIR}/app


	genimage                           \
		--rootpath "${NAND_TARGET_DIR}"     \
		--tmppath "${GENIMAGE_NAND_TMP}"    \
		--inputpath "${BINARIES_DIR}"  \
		--outputpath "${BINARIES_DIR}" \
		--config "${GENIMAGE_NAND_CFG}"	 ||  { echo "create NAND image error "; exit 1; }
	return 0;	
	
}
function nand_up_tar_gen()
{
	cd "${BINARIES_DIR}";
	cp ${BOARD_DIR}/update_nand.sh ${BINARIES_DIR}/
	md5sum u-boot_burn.bin  uboot-nand.env  k510.dtb bootm-bbl.img  rootfs.ubi > md5.txt
	tar -czvf nand_deploy.tar.gz md5.txt u-boot_burn.bin  uboot-nand.env  k510.dtb bootm-bbl.img  rootfs.ubi update_nand.sh >/dev/null
	rm -rf md5.txt;
	#rm -rf rootfs.ubifs
	#rm -rf update_nand.sh;
	cd -;
}

nand_image_gen
nand_up_tar_gen
exit $?
