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

	cd ${K230_SDK_ROOT}; make poc;

	#小核文件系统裁剪修改；
	cp  ${K230_SDK_ROOT}/board/k230_evb_doorlock/doorlock_rcS  ${BUILD_DIR}/images/little-core/rootfs/etc/init.d/rcS;
	cp  ${K230_SDK_ROOT}/board/k230_evb_doorlock/inittab  ${BUILD_DIR}/images/little-core/rootfs/etc/inittab;

	cd ${BUILD_DIR}/images/little-core/rootfs/; 
	#	rm -rf lib/modules/;
	#	rm -rf lib/libstdc++*;
	rm -rf usr/bin/fio;
	rm -rf usr/bin/lvgl_demo_widgets;
	rm -rf usr/bin/ssh*
	rm -rf usr/bin/sftp
	rm -rf usr/bin/lat*
	rm -rf usr/bin/hostapd_cli
	rm -rf usr/bin/*test*
	rm -rf usr/bin/k230_timer_demo
	rm -rf usr/bin/gpio_keys_demo
	rm -rf lib/modules/5.10.4+/kernel/drivers/gpu/
	rm -rf /lib/modules/5.10.4+/kernel/drivers/net/wireless/aich/aiw4211lv10/aiw4211lv10.ko
	find mnt/ -type f  -not -name k_ipcm.ko  -not -name sharefs    | xargs rm -rf ;
	#	rm -rf app/;
	rm -rf lib/tuning-server;	
	rm -rf usr/bin/stress-ng  bin/bash usr/sbin/sshd usr/bin/trace-cmd usr/bin/lvgl_demo_widgets;
	#find . -name *.ko | xargs rm -rf ;

	
	#大核文件系统裁剪修改；
	cd ${BUILD_DIR}/images/big-core/root/bin/; 
		cp ${K230_SDK_ROOT}/src/reference/business_poc/doorlock/big/out/door_lock.elf fastboot_app.elf;
		/opt/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/riscv64-unknown-linux-musl-strip fastboot_app.elf;

		echo "/bin/fastboot_app.elf /bin/retinaface.kmodel /bin/mbface.kmodel &" > init.sh
		cp ${K230_SDK_ROOT}/src/big/kmodel/door_lock/*.kmodel .;
		find . -type f  -not -name fastboot_app.elf -not -name init.sh -not -name H1280W720_conf.bin -not -name H1280W720_ref.bin \
			-not -name mbface.kmodel -not -name retinaface.kmodel  | xargs rm -rf ;

	#配置分区数据
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
	shrink_rootfs
	gen_image_spinor;
	cd  ${BUILD_DIR}/;rm -rf images_spinor;mv  images images_spinor; mv  images_bak images; cp images_spinor/sysimage-spinor32m*.img images;
fi
if [ "${CONFIG_SPI_NAND}" = "y" ]; then 
 	gen_image_spinand; 
fi



cd  ${BUILD_DIR}/images/
rm -rf  sysimage-sdcard_aes.img  sysimage-sdcard_sm.img  *.vfat









