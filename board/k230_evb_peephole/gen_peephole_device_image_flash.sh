#!/bin/bash

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

shrink_rootfs()
{
	# cd ${BUILD_DIR}/images/little-core/;fakeroot cp -r rootfs rootfs_mmc_sd;
	# cd ${BUILD_DIR}/images/big-core/;fackroot cp -r root root_mmc_sd;

	cd ${K230_SDK_ROOT}; make peephole;

	#С���ļ�ϵͳ�ü��޸ģ�
	cp  ${K230_SDK_ROOT}/board/k230_evb_peephole/peephole_device_flash_rcS  ${BUILD_DIR}/images/little-core/rootfs/etc/init.d/rcS;
	cp  ${K230_SDK_ROOT}/board/k230_evb_peephole/peephole_device_flash_rcK  ${BUILD_DIR}/images/little-core/rootfs/etc/init.d/rcK;
	cp  ${K230_SDK_ROOT}/board/k230_evb_peephole/interfaces  ${BUILD_DIR}/images/little-core/rootfs/etc/network/interfaces;
	# cp  ${K230_SDK_ROOT}/board/k230_evb_peephole/inittab  ${BUILD_DIR}/images/little-core/rootfs/etc/inittab;

	cd ${BUILD_DIR}/images/little-core/rootfs/;
	rm -rf usr/games;
	rm -rf usr/doc;
	rm -rf usr/man;
	find usr/share -type f -not -name fonts  | xargs rm -rf ;
	rm -rf usr/bin/fio;
	rm -rf usr/bin/lvgl_demo_widgets;
	rm -rf usr/bin/ssh*
	rm -rf usr/bin/sftp
	rm -rf usr/bin/lat*
	rm -rf usr/bin/hostapd_cli
	rm -rf usr/bin/*test*
	rm -rf usr/bin/k230_timer_demo
	rm -rf usr/bin/gpio_keys_demo
	rm -rf lib/modules/5.10.4/kernel/drivers/gpu/
	rm -rf etc/ssh
	find lib/modules/5.10.4/kernel/drivers/net/usb/ -type f -not -name r8152.ko  -not -name usbnet.ko | xargs rm -rf ;
	rm -rf lib/modules/5.10.4/kernel/drivers/net/wireless/
	find mnt/ -type f  -not -name k_ipcm.ko  -not -name sharefs | xargs rm -rf ;
	rm -rf app/tuning-server;
	rm -rf lib/tuning-server;
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd usr/bin/lvgl_demo_widgets;

	#����ļ�ϵͳ�ü��޸ģ�
	cd ${BUILD_DIR}/images/big-core/root/bin/;
	cp ${K230_SDK_ROOT}/src/reference/business_poc/peephole/big/out/peephole_dev.elf fastboot_app.elf;
	/opt/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/riscv64-unknown-linux-musl-strip fastboot_app.elf;

	echo "/bin/fastboot_app.elf &" > init.sh
	cp ${K230_SDK_ROOT}/src/big/kmodel/peephole/*.kmodel .;
	find . -type f  -not -name fastboot_app.elf -not -name init.sh -not -name imx335-2592x1944.bin \
		-not -name person_detect_yolov5n.kmodel | xargs rm -rf ;
	cp ${K230_SDK_ROOT}/src/reference/business_poc/peephole/big/app/data/doorbell.g711u .;

	#���÷�������
	if [ -f "${K230_SDK_ROOT}/src/big/mpp/userapps/src/vicap/src/isp/sdk/t_frameworks/t_database_c/calibration_data/sensor_cfg.bin" ]; then
		mkdir -p ${cfg_data_file_path}/;
		cp ${K230_SDK_ROOT}/src/big/mpp/userapps/src/vicap/src/isp/sdk/t_frameworks/t_database_c/calibration_data/sensor_cfg.bin ${cfg_data_file_path}/sensor_cfg.bin
	fi
}

copye_file_to_images;
gen_version;
gen_uboot_bin;
gen_linux_bin;
gen_final_ext2;
gen_rtt_bin;
gen_env_bin;
copy_app;

if [ "${CONFIG_SPI_NOR}" = "y" ]; then
	cd  ${BUILD_DIR}/;rm -rf images_bak;cp images images_bak -r;
	shrink_rootfs;
	gen_image_spinor;
	cd  ${BUILD_DIR}/;rm -rf images_spinor;mv  images images_spinor; mv  images_bak images; cp images_spinor/sysimage-spinor32m*.img images;
fi

# cd ${BUILD_DIR}/images/
# rm -rf  sysimage-sdcard_aes.img  sysimage-sdcard_sm.img  *.vfat
