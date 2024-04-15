#!/bin/bash
set -e;

source ${K230_SDK_ROOT}/board/common/gen_image_script/gen_image_comm_func.sh


env_dir="${K230_SDK_ROOT}/board/common/env"

GENIMAGE_CFG_DIR="${K230_SDK_ROOT}/board/common/gen_image_cfg"
GENIMAGE_CFG_SD="${GENIMAGE_CFG_DIR}/genimage-sdcard.cfg"
GENIMAGE_CFG_SD_AES="${GENIMAGE_CFG_DIR}/genimage-sdcard_aes.cfg"
GENIMAGE_CFG_SD_SM="${GENIMAGE_CFG_DIR}/genimage-sdcard_sm.cfg"
#GENIMAGE_CFG_SD_DDR4="${GENIMAGE_CFG_DIR}/genimage-sdcard_ddr4.cfg"
GENIMAGE_CFG_SPI_NOR="${GENIMAGE_CFG_DIR}/genimage-spinor.cfg"
GENIMAGE_CFG_SPI_NAND="${GENIMAGE_CFG_DIR}/genimage-spinand.cfg"
GENIMAGE_CFG_SD_REMOTE="${GENIMAGE_CFG_DIR}/genimage-sdcard_remote.cfg"

cfg_data_file_path="${GENIMAGE_CFG_DIR}/data"
quick_boot_cfg_data_file="${GENIMAGE_CFG_DIR}/data/quick_boot.bin"
face_database_data_file="${GENIMAGE_CFG_DIR}/data/face_data.bin"
sensor_cfg_data_file="${GENIMAGE_CFG_DIR}/data/sensor_cfg.bin"
ai_mode_data_file="${BUILD_DIR}/images/big-core/ai_mode.bin" #"${GENIMAGE_CFG_DIR}/data/ai_mode.bin"
speckle_data_file="${GENIMAGE_CFG_DIR}/data/speckle.bin"
rtapp_data_file="${BUILD_DIR}/images/big-core/fastboot_app.elf"

copye_file_to_images;
gen_version;
if [ "${CONFIG_SUPPORT_LINUX}" = "y" ]; then	
	add_dev_firmware;
	gen_linux_bin;
	gen_final_ext2;
fi
[ "${CONFIG_SUPPORT_RTSMART}" = "y" ] &&  gen_rtt_bin;
gen_uboot_bin;
gen_env_bin;
copy_app;

if [ "${CONFIG_REMOTE_TEST_PLATFORM}" = "y" ] ; then
	gen_image ${GENIMAGE_CFG_SD_REMOTE}   sysimage-sdcard.img
else
	gen_image ${GENIMAGE_CFG_SD}   sysimage-sdcard.img
fi

if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ] ; then
	gen_image  ${GENIMAGE_CFG_SD_AES}  sysimage-sdcard_aes.img
	gen_image ${GENIMAGE_CFG_SD_SM}  sysimage-sdcard_sm.img
fi

if [ "${CONFIG_SPI_NOR}" = "y" ]; then
	cd  ${BUILD_DIR}/;rm -rf images_bak;cp images images_bak -r;
	shrink_rootfs_common
	gen_image_spinor
	cd  ${BUILD_DIR}/;rm -rf images_spinor;mv  images images_spinor; mv  images_bak images; cp images_spinor/sysimage-spinor32m*.img images;
fi
[ "${CONFIG_SPI_NAND}" = "y" ] && gen_image_spinand;



cd  ${BUILD_DIR}/images/
rm -rf  sysimage-sdcard_aes.img  sysimage-sdcard_sm.img  *.vfat









