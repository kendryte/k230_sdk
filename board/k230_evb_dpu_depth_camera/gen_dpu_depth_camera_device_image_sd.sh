#!/bin/bash

source ${K230_SDK_ROOT}/board/common/gen_image_script/gen_image_comm_func.sh


env_dir="${K230_SDK_ROOT}/board/common/env"

GENIMAGE_CFG_DIR="${K230_SDK_ROOT}/board/common/gen_image_cfg"
GENIMAGE_CFG_SD="${GENIMAGE_CFG_DIR}/genimage-sdcard.cfg"
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




gen_dpu_depth_camera_device_app()
{

	rm -rf lib/tuning-server;

    cp  ${K230_SDK_ROOT}/board/k230_evb_dpu_depth_camera/dpu_depth_camera_rcS  ${BUILD_DIR}/images/little-core/rootfs/etc/init.d/rcS;

    cp  ${K230_SDK_ROOT}/src/common/cdk/user/out/big/sample_sys_init.elf ${BUILD_DIR}/images/big-core/root/bin/;
    cd ${BUILD_DIR}/images/big-core/root/bin;
    echo "cd /bin" > init.sh
    echo "sample_sys_init.elf &" >> init.sh

    cd ${RTSMART_SRC_DIR}/userapps/; python3 ../tools/mkromfs.py ${BUILD_DIR}/images/big-core/root/  ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c;
    cd ${K230_SDK_ROOT};make rtt_update_romfs;

    cp ${BUILD_DIR}/big/rt-smart/rtthread.* ${BUILD_DIR}/images/big-core/;
}

copye_file_to_images;
gen_dpu_depth_camera_device_app;
gen_version;
gen_uboot_bin;
gen_linux_bin;
gen_final_ext2;
gen_rtt_bin;
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

# gen_image_spinor
# gen_image_spinand;



cd  ${BUILD_DIR}/images/
rm -rf  sysimage-sdcard_aes.img  sysimage-sdcard_sm.img  *.vfat









