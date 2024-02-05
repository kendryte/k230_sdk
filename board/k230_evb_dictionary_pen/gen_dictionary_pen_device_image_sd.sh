#!/bin/bash

source ${K230_SDK_ROOT}/board/common/gen_image_script/gen_image_comm_func.sh


env_dir="${K230_SDK_ROOT}/board/common/env"

GENIMAGE_CFG_DIR="${K230_SDK_ROOT}/board/common/gen_image_cfg"
GENIMAGE_CFG_SD="${GENIMAGE_CFG_DIR}/genimage-sdcard-dictionary-pen.cfg"
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




gen_dictionary_pen_device_app()
{
    #crop little-core
	#	rm -rf lib/modules/;
	#	rm -rf lib/libstdc++*;
	#rm -rf lib/modules/
	#rm -rf usr/bin/fio;
	#rm -rf usr/bin/lvgl_demo_widgets;
	#rm -rf usr/bin/ssh*
	#find mnt/ -type f  -not -name k_ipcm.ko  -not -name sharefs    | xargs rm -rf ;
	#	rm -rf app/;
	rm -rf lib/tuning-server;	
	#rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd usr/bin/lvgl_demo_widgets;
	#find . -name *.ko | xargs rm -rf ;

    # fakeroot -- ${K230_SDK_ROOT}/tools/mkcpio-rootfs.sh; 

    #cp ${K230_SDK_ROOT}/src/reference/dictionary_pen_poc/cidianbi_kmodel_v2.5.1/include  ${K230_SDK_ROOT}/src/reference/dictionary_pen_poc/
    #rtapp and ai mode
    cd ${K230_SDK_ROOT}; make dictionary_pen;
    #creat dictionary_pen dir
    mkdir -p ${BUILD_DIR}/images/big-core/app/dictionary_pen/
    cp ${K230_SDK_ROOT}/src/reference/business_poc/dictionary_pen_poc/cidianbi_kmodel/*  ${BUILD_DIR}/images/big-core/app/dictionary_pen/ -rf 
    cp ${K230_SDK_ROOT}/src/reference/business_poc/dictionary_pen_poc/include  ${BUILD_DIR}/images/big-core/app/dictionary_pen/ -rf 
    cp ${K230_SDK_ROOT}/src/reference/business_poc/dictionary_pen_poc/build/dictionary_pen.elf  ${BUILD_DIR}/images/big-core/app/dictionary_pen/ -rf 

    # crop big-core
    # find . -type f  -not -name fastboot_app.elf -not -name init.sh -not -name doorbell.g711u -not -name person_detect_yolov5n.kmodel -not -name imx335-1920x1080_auto.json* -not -name imx335-1920x1080.bin -not -name imx335-1920x1080_manual.json* -not -name imx335-1920x1080.xml* -not -name imx335-2592x1944_auto.json* -not -name imx335-2592x1944.bin -not -name imx335-2592x1944_manual.json* -not -name imx335-2592x1944.xml* | xargs rm -rf

    # rm  ${BUILD_DIR}/images/big-core/app/* -rf;

    cd ${RTSMART_SRC_DIR}/userapps/; python3 ../tools/mkromfs.py ${BUILD_DIR}/images/big-core/root/  ${RTSMART_SRC_DIR}/kernel/bsp/maix3/applications/romfs.c;
    cd ${K230_SDK_ROOT};make rtt_update_romfs;

    cp ${BUILD_DIR}/big/rt-smart/rtthread.* ${BUILD_DIR}/images/big-core/;
}

copye_file_to_images;
gen_dictionary_pen_device_app;
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









