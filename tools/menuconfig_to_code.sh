#!/bin/bash

quick_boot_cfg_data_file="fh_quick_boot.bin"
face_database_data_file="fh_face_data.bin"
sensor_cfg_data_file="fh_sensor_cfg.bin"
ai_mode_data_file="fh_ai_mode.bin"
speckle_data_file="fh_speckle.bin"
rtapp_data_file="fh_fastboot_app.elf"

source ${K230_SDK_ROOT}/.config

LINUX_DTSI_PATH="${K230_SDK_ROOT}/src/little/linux/arch/riscv/boot/dts/kendryte/${CONFIG_BOARD_NAME}.dtsi"
UBOOT_ENV_PATH="${K230_SDK_ROOT}/tools/gen_image_cfg/genimage-sdcard.cfg.env \
				${K230_SDK_ROOT}/src/little/uboot/include/configs/k230_evb.h \
				${K230_SDK_ROOT}/src/little/uboot/board/canaan/k230_evb/img.c \
				${K230_SDK_ROOT}/tools/gen_image_cfg/genimage-spinor.cfg.jffs2.env"


##动态生成partition
cat >t.sh <<EOF
					partition@0 {
						/* spl boot */
						reg = <0x0 0x00080000>;
						label = "spl_boot";
					};

					partition@80000 {
						/* normal boot */
						reg = <0x00080000 0x00180000>;
						label = "uboot";
					};

					partition@${CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE:2} {
						reg = <${CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE} ${CONFIG_SPI_NOR_QUICK_BOOT_CFG_SIZE}>;
						label = "quick_boot_cfg ";
					};
					partition@${CONFIG_SPI_NOR_FACE_DB_CFG_BASE:2} {
						reg = <${CONFIG_SPI_NOR_FACE_DB_CFG_BASE} ${CONFIG_SPI_NOR_FACE_DB_CFG_SIZE}>;
						label = "face_db";
					};
					partition@${CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE:2} {
						reg = <${CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE} ${CONFIG_SPI_NOR_SENSOR_CFG_CFG_SIZE}>;
						label = "sensor_cfg";
					};
					partition@${CONFIG_SPI_NOR_SPECKLE_CFG_BASE:2} {
						reg = <${CONFIG_SPI_NOR_SPECKLE_CFG_BASE} ${CONFIG_SPI_NOR_SPECKLE_CFG_SIZE}>;
						label = "speckle";
					};

					partition@${CONFIG_SPI_NOR_RTT_APP_BASE:2} {
						reg = <${CONFIG_SPI_NOR_RTT_APP_BASE} ${CONFIG_SPI_NOR_RTT_APP_SIZE}>;
						label = "rtt";
					};
					partition@${CONFIG_SPI_NOR_LK_BASE:2} {
						reg = <${CONFIG_SPI_NOR_LK_BASE} ${CONFIG_SPI_NOR_LK_SIZE}>;
						label = "linux";
					};
					partition@${CONFIG_SPI_NOR_LR_BASE:2} {
						reg = <${CONFIG_SPI_NOR_LR_BASE} ${CONFIG_SPI_NOR_LR_SIZE}>;
						label = "rootfs";
					};

					partition@2000000 {
						/* 32MB for  update image*/
						reg = <0 0x2000000>;
						label = "all_flash";
					};
EOF
#tl=$(grep 0xdddddddd temp.c  -n | cut  -d:  -f1);


part_s=$(grep -n partition@0 ${LINUX_DTSI_PATH} | head -1 | cut  -d:  -f1 )
part_e=$(grep -n all_flash ${LINUX_DTSI_PATH} | head -1 | cut  -d:  -f1 )
part_e=$((${part_e}+1))

sed -i -e "${part_s},${part_e}d"  ${LINUX_DTSI_PATH}
sed -i  -e "$((${part_s}-1)) r  t.sh" ${LINUX_DTSI_PATH}
echo "modify linux dts partition ${LINUX_DTSI_PATH}"

rm -rf t.sh

#cat t.sh




echo "modify genimage_cfg"
ubifs_max_size=$(( ((${CONFIG_SPI_NOR_LR_SIZE}/65536) - 4) * 65408 ))
cat >cfg.t <<EOF

flash spinor-32M-gd25lx256e {
	//The size of a physical eraseblock in bytes
	pebsize = 65536
	//The size of a logical eraseblock in bytes (for ubifs)
	lebsize = 65408
	//Number of physical eraseblocks on this device. The total size of the device is determined by pebsize * numpebs
	numpebs = 512
	//The minimum size in bytes accessible on this device
	minimum-io-unit-size = 1
	vid-header-offset = 64 
	sub-page-size = 1
}
image rootfs.jffs2 {
    flashtype = "spinor-32M-gd25lx256e"
	
	jffs2 {
		extraargs = "-q"
	}
	mountpoint = ""
}

image sysimage-spinor32m_jffs2.img {
	flash  {}
	flashtype = "spinor-32M-gd25lx256e"

	partition uboot_spl_1 {
		offset = 0M
		image = "../little/uboot/u-boot-spl-k230-swap.bin"
		size = 0x80000
	}

	partition uboot {
		offset = 0x80000
		image = "../little/uboot/u-boot.img"
		size = 0x160000
	}
	partition uboot_env {
		offset = 0x1e0000
		image = "jffs2.env"
		size = 0x20000
	}
	partition quick_boot_cfg {
		offset = ${CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE}
		image = "${quick_boot_cfg_data_file}"
		size = ${CONFIG_SPI_NOR_QUICK_BOOT_CFG_SIZE}
	}

	partition face_db {
		offset = ${CONFIG_SPI_NOR_FACE_DB_CFG_BASE}
		image = "${face_database_data_file}"
		size = ${CONFIG_SPI_NOR_FACE_DB_CFG_SIZE}
	}

	partition sensor_cfg {
		offset = ${CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE}
		image = "${sensor_cfg_data_file}"
		size = ${CONFIG_SPI_NOR_SENSOR_CFG_CFG_SIZE}
	}

	partition ai_mode {
		offset = ${CONFIG_SPI_NOR_AI_MODE_CFG_BASE}
		image = "${ai_mode_data_file}"
		size = ${CONFIG_SPI_NOR_AI_MODE_CFG_SIZE}
	}

	partition speckle_cfg {
		offset = ${CONFIG_SPI_NOR_SPECKLE_CFG_BASE}
		image = "${speckle_data_file}"
		size = ${CONFIG_SPI_NOR_SPECKLE_CFG_SIZE}
	}

	partition rtt {
		offset = ${CONFIG_SPI_NOR_RTTK_BASE}
		image = "big-core/rtt_system.bin"
		size = ${CONFIG_SPI_NOR_RTTK_SIZE}
	}
	partition rtt_app {
		offset = ${CONFIG_SPI_NOR_RTT_APP_BASE}
		image = "${rtapp_data_file}"
		size = ${CONFIG_SPI_NOR_RTT_APP_SIZE}
	}

	partition linux {
		offset = ${CONFIG_SPI_NOR_LK_BASE}
		image = "little-core/linux_system.bin"
		size = ${CONFIG_SPI_NOR_LK_SIZE}
	}	
	partition rootfs_ubi {
		offset = ${CONFIG_SPI_NOR_LR_BASE}
		image = "rootfs.jffs2"
		size = ${CONFIG_SPI_NOR_LR_SIZE}
	}
}

image  rootfs.ubifs {
	ubifs{
		extraargs = "-x zlib -U"
		max-size = ${ubifs_max_size}
	}
	flashtype = "spinor-32M-gd25lx256e"
}


image  rootfs.ubi {
	ubi{
		extraargs=""
	}
	partition ubi_rootfs_part {
		image = "rootfs.ubifs"
		size = ${ubifs_max_size}
	}
	flashtype = "spinor-32M-gd25lx256e"
}

image sysimage-spinor32m.img {
	flash  {}
	flashtype = "spinor-32M-gd25lx256e"

	partition uboot_spl_1 {
		offset = 0M
		image = "../little/uboot/u-boot-spl-k230-swap.bin"
		size = 0x80000
	}

	partition uboot {
		offset = 0x80000
		image = "../little/uboot/u-boot.img"
		size = 0x160000
	}
	partition uboot_env {
		offset = 0x1e0000
		image = "env.env"
		size = 0x20000
	}
	partition quick_boot_cfg {
		offset = ${CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE}
		image = "${quick_boot_cfg_data_file}"
		size = ${CONFIG_SPI_NOR_QUICK_BOOT_CFG_SIZE}
	}

	partition face_db {
		offset = ${CONFIG_SPI_NOR_FACE_DB_CFG_BASE}
		image = "${face_database_data_file}"
		size = ${CONFIG_SPI_NOR_FACE_DB_CFG_SIZE}
	}

	partition sensor_cfg {
		offset = ${CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE}
		image = "${sensor_cfg_data_file}"
		size = ${CONFIG_SPI_NOR_SENSOR_CFG_CFG_SIZE}
	}

	partition ai_mode {
		offset = ${CONFIG_SPI_NOR_AI_MODE_CFG_BASE}
		image = "${ai_mode_data_file}"
		size = ${CONFIG_SPI_NOR_AI_MODE_CFG_SIZE}
	}

	partition speckle_cfg {
		offset = ${CONFIG_SPI_NOR_SPECKLE_CFG_BASE}
		image = "${speckle_data_file}"
		size = ${CONFIG_SPI_NOR_SPECKLE_CFG_SIZE}
	}

	partition rtt {
		offset = ${CONFIG_SPI_NOR_RTTK_BASE}
		image = "big-core/rtt_system.bin"
		size = ${CONFIG_SPI_NOR_RTTK_SIZE}
	}
	partition rtt_app {
		offset = ${CONFIG_SPI_NOR_RTT_APP_BASE}
		image = "${rtapp_data_file}"
		size = ${CONFIG_SPI_NOR_RTT_APP_SIZE}
	}

	partition linux {
		offset = ${CONFIG_SPI_NOR_LK_BASE}
		image = "little-core/linux_system.bin"
		size = ${CONFIG_SPI_NOR_LK_SIZE}
	}	
	partition rootfs_ubi {
		offset = ${CONFIG_SPI_NOR_LR_BASE}
		image = "rootfs.ubi"
		size = ${CONFIG_SPI_NOR_LR_SIZE}
	}
}
EOF

mv cfg.t ${K230_SDK_ROOT}/tools/gen_image_cfg/${GENIMAGE_CFG_DIR}/genimage-spinor.cfg


sed -i -e "s/ubi.mtd=[0-9]*/ubi.mtd=8/g"   ${UBOOT_ENV_PATH}
sed -i -e "s/mtdblock[0-9]*/mtdblock8/g"   ${UBOOT_ENV_PATH}
#set -x;



