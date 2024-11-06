#!/bin/bash

set -e;

source ${K230_SDK_ROOT}/.config

function gen_spinor_dts_partion()
{
    local temp_file_name="$1"
    cat >${temp_file_name} <<-EOF
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
EOF


    if [ "${CONFIG_SPI_NOR_SUPPORT_CFG_PARAM}" = "y" ]; then
    cat >>${temp_file_name} <<-EOF
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
                label = "rttapp";
            };
EOF
    fi #CONFIG_SPI_NOR_SUPPORT_CFG_PARAM




    if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ]; then

        cat >>${temp_file_name} <<-EOF
            partition@${CONFIG_SPI_NOR_RTTK_BASE:2} {
                reg = <${CONFIG_SPI_NOR_RTTK_BASE} ${CONFIG_SPI_NOR_RTTK_SIZE}>;
                label = "rttk";
            };

EOF
    fi #CONFIG_SUPPORT_RTSMART



    if [ "${CONFIG_SUPPORT_LINUX}" = "y" ]; then
        cat >>${temp_file_name} <<-EOF
            partition@${CONFIG_SPI_NOR_LK_BASE:2} {
                reg = <${CONFIG_SPI_NOR_LK_BASE} ${CONFIG_SPI_NOR_LK_SIZE}>;
                label = "linux";
            };

            partition@${CONFIG_SPI_NOR_LR_BASE:2} {
                reg = <${CONFIG_SPI_NOR_LR_BASE} ${CONFIG_SPI_NOR_LR_SIZE}>;
                label = "rootfs";
            };
EOF
    fi  #CONFIG_SUPPORT_LINUX



cat >>${temp_file_name} <<-EOF
            partition@2000000 {
                /* 32MB for  update image*/
                reg = <0 0x2000000>;
                label = "all_flash";
            };
EOF


}

#LINUX_DTS_PATH="${K230_SDK_ROOT}/src/little/linux/arch/riscv/boot/dts/kendryte/${CONFIG_LINUX_DTB}.dts"
function modify_linux_dts()
{

	local LINUX_DTS_PATH="${K230_SDK_ROOT}/src/little/linux/arch/riscv/boot/dts/kendryte/${CONFIG_LINUX_DTB}.dts"
		##动态生成partition

	#tl=$(grep 0xdddddddd temp.c  -n | cut  -d:  -f1);
    if [ "${CONFIG_SPI_NOR}" = "y" ]; then
        gen_spinor_dts_partion t.sh

        part_s=$(grep -n partition@0 ${LINUX_DTS_PATH} | head -1 | cut  -d:  -f1 )
        part_e=$(grep -n all_flash ${LINUX_DTS_PATH} | head -1 | cut  -d:  -f1 )
        part_e=$((${part_e}+1))

        [ "${CONFIG_SPI_NOR}" = "y" ] && sed -i -e "${part_s},${part_e}d"  ${LINUX_DTS_PATH}
        [ "${CONFIG_SPI_NOR}" = "y" ] && sed -i  -e "$((${part_s}-1)) r  t.sh" ${LINUX_DTS_PATH}
        echo "modify linux dts  ${LINUX_DTS_PATH}"
    fi

	{
		#linux memory configuration
		local LINUX_SYS_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0x200000])"
		local LINUX_SYS_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_SIZE}-0x200000-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"
		sed -i "s/reg =.*linux memory config.*$/reg = <0x0 ${LINUX_SYS_BASE} 0x0 ${LINUX_SYS_SIZE}>;  \/\*linux memory config\*\//g" ${LINUX_DTS_PATH}
	}

	rm -rf t.sh
}



#${K230_SDK_ROOT}/board/common/gen_image_cfg/${GENIMAGE_CFG_DIR}/genimage-spinor.cfg
function  modify_gen_image_cfg_spinorcfg()
{
	quick_boot_cfg_data_file="cfg_part/fn_ug_quick_boot.bin"
	face_database_data_file="cfg_part/fn_ug_face_data.bin"
	sensor_cfg_data_file="cfg_part/fn_ug_sensor_cfg.bin"
	ai_mode_data_file="cfg_part/fn_ug_ai_mode.bin"
	speckle_data_file="cfg_part/fn_ug_speckle.bin"
	rtapp_data_file="cfg_part/fn_ug_fastboot_app.elf"
	echo "modify genimage_cfg"
	ubifs_max_size=$(( ((${CONFIG_SPI_NOR_LR_SIZE}/65536) - 4) * 65408 ))

	cat >cfg.t <<-EOF

    flash spinor-32M-gd25lx256e {
        #The size of a physical eraseblock in bytes
        pebsize = 65536
        #The size of a logical eraseblock in bytes (for ubifs)
        lebsize = 65408
        #Number of physical eraseblocks on this device. The total size of the device is determined by pebsize * numpebs
        numpebs = 512
        #The minimum size in bytes accessible on this device
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
            image = "little-core/uboot/swap_fn_u-boot-spl.bin"
            size = 0x80000
        }

        partition uboot {
            offset = 0x80000
            image = "little-core/uboot/fn_ug_u-boot.bin"
            size = 0x160000
        }
        partition uboot_env {
            offset = 0x1e0000
            image = "little-core/uboot/jffs2.env"
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
            image = "little-core/uboot/swap_fn_u-boot-spl.bin"
            size = 0x80000
        }

        partition uboot {
            offset = 0x80000
            image = "little-core/uboot/fn_ug_u-boot.bin"
            size = 0x160000
        }
        partition uboot_env {
            offset = 0x1e0000
            image = "little-core/uboot/env.env"
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

	mv cfg.t ${K230_SDK_ROOT}/board/common/gen_image_cfg/${GENIMAGE_CFG_DIR}/genimage-spinor.cfg

}
modify_gen_image_sd_cfg()
{

    local CFG=${K230_SDK_ROOT}/board/common/gen_image_cfg/${GENIMAGE_CFG_DIR}/genimage-sdcard.cfg
    if [  "$CONFIG_SUPPORT_LINUX"  = "y" ]; then
        sed -i "s/gpt = .*/gpt = \"true\"/" ${CFG}
        sed -i "s/partition-type =.*0x83/partition-type-uuid = \"L\"/" ${CFG}  #partition-type = 0x83
        sed -i "s/partition-type =.*0xc/partition-type-uuid = \"F\"/" ${CFG}  #partition-type = 0xc
    else
        sed -i "s/gpt = .*/gpt = \"false\"/" ${CFG}
        sed -i "s/partition-type-uuid =.*L.*/partition-type = 0x83/" ${CFG}  #partition-type = 0x83
        sed -i "s/partition-type-uuid =.*F.*/partition-type = 0xc/" ${CFG}  #partition-type = 0xc
    fi
}

function modify_uboot_file()
{
    if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] && [ "${CONFIG_SUPPORT_LINUX}" != "y" ]; then
        CONFIG_MEM_LINUX_SYS_BASE="${CONFIG_MEM_RTT_SYS_BASE}"
    fi;

	{  #env
		local OPENSBI_LINUX_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0])"
		DTB_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000])" #32MB offset
		FDT_HIGH="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000])"
		KERNEL_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000+0x2000000])"
		RAMDISK_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000])"


		UBOOT_ENV_PATH="${K230_SDK_ROOT}/board/common/env/default.env \
					${K230_SDK_ROOT}/src/little/uboot/include/configs/k230_evb.h \
					${K230_SDK_ROOT}/src/little/uboot/board/canaan/common/k230_img.c \
					${K230_SDK_ROOT}/board/common/env/spinor.jffs2.env"

		sed -i -e "s/ubi.mtd=[0-9]*/ubi.mtd=9/g"   ${UBOOT_ENV_PATH}
		sed -i -e "s/mtdblock[0-9]*/mtdblock9/g"   ${UBOOT_ENV_PATH}
		##sed -i 's/\"$(1)=.*$/\"$(1)=$(2)\\0" \\/g' $(UBOOT_CONFIGH_PATH)

		sed -i -e "s/dtb_addr=0x[0-9a-fA-F]*/dtb_addr=${DTB_ADDR}/g" ${UBOOT_ENV_PATH}  #"dtb_addr=0xa000000 \0" \

		sed -i -e "s/fdt_high=0x[0-9a-fA-F]*/fdt_high=${FDT_HIGH}/g" ${UBOOT_ENV_PATH}
		sed -i -e "s/kernel_addr=0x[0-9a-fA-F]*/kernel_addr=${KERNEL_ADDR}/g" ${UBOOT_ENV_PATH}
		sed -i -e "s/ramdisk_addr=0x[0-9a-fA-F]*/ramdisk_addr=${RAMDISK_ADDR}/g" ${UBOOT_ENV_PATH}
	}


	if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] && [ "${CONFIG_SUPPORT_LINUX}" = "y" ]; then
	{
		local OPENSBI_LINUX_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0])"
		local OPENSBI_LINUX_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		local OPENSBI_RTT_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_BASE}+0])"
		local OPENSBI_RTT_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		UBOOT_DTS="$(cd ${K230_SDK_ROOT}/src/little/uboot/configs/; cat ${UBOOT_DEFCONFIG} |   grep CONFIG_DEFAULT_DEVICE_TREE | cut -d = -f2 | tr -d \" )"

		UBOOT_DTS_PATH="${K230_SDK_ROOT}/src/little/uboot/arch/riscv/dts/${UBOOT_DTS}.dts"
		echo "Modify file: ${UBOOT_DTS_PATH}"
		sed -i "s/0x.*MEM_LINUX_SYS.*$/0x0 ${OPENSBI_LINUX_BASE} 0x0 ${OPENSBI_LINUX_SIZE}  \/\*MEM_LINUX_SYS\*\//g" ${UBOOT_DTS_PATH}
		sed -i "s/0x.*MEM_RTT_SYS.*$/0x0 ${OPENSBI_RTT_BASE} 0x0 ${OPENSBI_RTT_SIZE}  \/\*MEM_RTT_SYS\*\//g" ${UBOOT_DTS_PATH}

		echo "Modify file: ${UBOOT_DEFCONFIG}"
		sed -i "s/CONFIG_SYS_TEXT_BASE=.*$/CONFIG_SYS_TEXT_BASE=${OPENSBI_LINUX_BASE}/g" ${K230_SDK_ROOT}/src/little/uboot/configs/${UBOOT_DEFCONFIG}
	}
    fi

    if [ "${CONFIG_SUPPORT_RTSMART}" != "y" ] && [ "${CONFIG_SUPPORT_LINUX}" = "y" ]; then
	{
		local OPENSBI_LINUX_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0])"
		local OPENSBI_LINUX_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		# local OPENSBI_RTT_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_BASE}+0])"
		# local OPENSBI_RTT_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		UBOOT_DTS="$(cd ${K230_SDK_ROOT}/src/little/uboot/configs/; cat ${UBOOT_DEFCONFIG} |   grep CONFIG_DEFAULT_DEVICE_TREE | cut -d = -f2 | tr -d \" )"

		UBOOT_DTS_PATH="${K230_SDK_ROOT}/src/little/uboot/arch/riscv/dts/${UBOOT_DTS}.dts"
		echo "Modify file: ${UBOOT_DTS_PATH}"
		sed -i "s/.*0x.*MEM_LINUX_SYS.*$/0x0 ${OPENSBI_LINUX_BASE} 0x0 ${OPENSBI_LINUX_SIZE}  \/\*MEM_LINUX_SYS\*\//g" ${UBOOT_DTS_PATH}
		sed -i "s/.*0x.*MEM_RTT_SYS.*$/\/\*0x0 ${OPENSBI_RTT_BASE} 0x0 ${OPENSBI_RTT_SIZE} \*\/ \/\*MEM_RTT_SYS\*\//g" ${UBOOT_DTS_PATH}

		echo "Modify file: ${UBOOT_DEFCONFIG}"
		sed -i "s/CONFIG_SYS_TEXT_BASE=.*$/CONFIG_SYS_TEXT_BASE=${OPENSBI_LINUX_BASE}/g" ${K230_SDK_ROOT}/src/little/uboot/configs/${UBOOT_DEFCONFIG}
	}
    fi

     if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] && [ "${CONFIG_SUPPORT_LINUX}" != "y" ]; then
	{
		# local OPENSBI_LINUX_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0])"
		# local OPENSBI_LINUX_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		local OPENSBI_RTT_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_BASE}+0])"
		local OPENSBI_RTT_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		UBOOT_DTS="$(cd ${K230_SDK_ROOT}/src/little/uboot/configs/; cat ${UBOOT_DEFCONFIG} |   grep CONFIG_DEFAULT_DEVICE_TREE | cut -d = -f2 | tr -d \" )"

		UBOOT_DTS_PATH="${K230_SDK_ROOT}/src/little/uboot/arch/riscv/dts/${UBOOT_DTS}.dts"
		echo "Modify file: ${UBOOT_DTS_PATH}"

        sed -i "s/.*0x.*MEM_LINUX_SYS.*$/\/\*0x0 ${OPENSBI_LINUX_BASE} 0x0 ${OPENSBI_LINUX_SIZE} \*\/ \/\*MEM_LINUX_SYS\*\//g" ${UBOOT_DTS_PATH}
        if (( ${OPENSBI_RTT_SIZE}  < 0x6000000 ))  ; then
            sed -i "s/.*0x.*MEM_RTT_SYS.*$/0x0 ${OPENSBI_RTT_BASE} 0x0 0x8000000  \/\*MEM_RTT_SYS\*\//g" ${UBOOT_DTS_PATH};
        else
            sed -i "s/.*0x.*MEM_RTT_SYS.*$/0x0 ${OPENSBI_RTT_BASE} 0x0 ${OPENSBI_RTT_SIZE}  \/\*MEM_RTT_SYS\*\//g" ${UBOOT_DTS_PATH}
        fi;


		echo "Modify file: ${UBOOT_DEFCONFIG}"
		sed -i "s/CONFIG_SYS_TEXT_BASE=.*$/CONFIG_SYS_TEXT_BASE=${OPENSBI_LINUX_BASE}/g" ${K230_SDK_ROOT}/src/little/uboot/configs/${UBOOT_DEFCONFIG}
	}
    fi




}

function modify_big_code()
{

	{   #define UART_ADDR                   UART3_BASE_ADDR
		#define UART_IRQ                    0x13
		local OPENSBI_PLATFORM="${RTSMART_SRC_DIR}/kernel/bsp/maix3/board/interdrv/uart/drv_uart.c"

		local RTT_UART_REG_BASDE="0x9140${CONFIG_RTT_CONSOLE_ID}000UL" #0x91403000UL
		local RTT_UART_IRQ="0x1${CONFIG_RTT_CONSOLE_ID}" #"0x13"

		sed -i "s/define *UART_ADDR.*$/define UART_ADDR ${RTT_UART_REG_BASDE}/g" ${OPENSBI_PLATFORM}
		sed -i "s/define *UART_IRQ.*$/define UART_IRQ ${RTT_UART_IRQ}/g" ${OPENSBI_PLATFORM}
	}


	{
		#base
		MEM_CFG_RTT_LDS="${RTSMART_SRC_DIR}/kernel/bsp/maix3/link.lds"
		MEM_CFG_RTT_RAMEND="${RTSMART_SRC_DIR}/kernel/bsp/maix3/board/board.h"
		echo "Modify file: ${MEM_CFG_RTT_LDS}"
		echo "Modify file: ${MEM_CFG_RTT_RAMEND}"


		local RTT_SYS_BASE="$(printf '0x%x\n' $(( ${CONFIG_MEM_RTT_SYS_BASE} + 0x20000 )) )"
		local RTT_SYS_SIZE="$(printf '%d\n' $(( (${CONFIG_MEM_RTT_SYS_SIZE} - 0x20000)/1024 -1 )) )"
		local RTT_RAM_END="$(printf '0x%x\n' $(( ${CONFIG_MEM_RTT_SYS_BASE}+${CONFIG_MEM_RTT_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE} )) )"

		sed -i "s/. =.*rt-smart link start address.*$/. = ${RTT_SYS_BASE};    \/\*rt-smart link start address\*\//g" ${MEM_CFG_RTT_LDS}
		sed -i "s/SRAM : ORIGIN.*$/SRAM : ORIGIN = ${RTT_SYS_BASE}, LENGTH = ${RTT_SYS_SIZE}K/g" ${MEM_CFG_RTT_LDS}
		sed -i "s/#define RAM_END.*$/#define RAM_END ${RTT_RAM_END}/g" ${MEM_CFG_RTT_RAMEND}


		##define RT_HW_HEAP_END      ((void *)(((rt_size_t)RT_HW_HEAP_BEGIN) + 4 * 1024 * 1024))
		#((void *)(((rt_size_t)RT_HW_HEAP_BEGIN) + ${RT_HW_HEAP_END_SIZE} ))
		RT_HW_HEAP_END_SIZE="0x2000000"
        if(( $CONFIG_MEM_RTT_SYS_SIZE <= 0x4000000 )) ;then RT_HW_HEAP_END_SIZE="0x400000"; fi;
		if [ "${CONFIG_BOARD_K230D}" = "y" ]; then RT_HW_HEAP_END_SIZE="0x400000"; fi;
		if [ "${CONFIG_BOARD_NAME}" = "k230_evb_doorlock" ]; then RT_HW_HEAP_END_SIZE="0x400000"; fi;
		if [ "${CONFIG_BOARD_NAME}" = "k230_evb_peephole_device" ]; then RT_HW_HEAP_END_SIZE="0x400000"; fi;
        if [ "${CONFIG_BOARD_K230D_CANMV}" = "y" ]; then RT_HW_HEAP_END_SIZE="0x400000"; fi;
		sed -i "s/define *RT_HW_HEAP_END.*$/define RT_HW_HEAP_END \(\(void \*\)\(\(\(rt_size_t\)RT_HW_HEAP_BEGIN\) \+ ${RT_HW_HEAP_END_SIZE} \)\)/g" ${MEM_CFG_RTT_RAMEND}
	}
	{
		# ipc
		MEM_CFG_IPCM_RTT="${RTSMART_SRC_DIR}/kernel/bsp/maix3/board/board.c"
		IPCM_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_IPCM_BASE}+0])"
        IPCM_SIZE="0"
        if (( ${CONFIG_MEM_IPCM_SIZE} > ${CONFIG_MEM_BOUNDARY_RESERVED_SIZE} ))  ; then IPCM_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_IPCM_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])";fi;

		echo "Modify file: ${MEM_CFG_IPCM_RTT}"
		sed -i "s/#define MEM_IPCM_BASE.*$/#define MEM_IPCM_BASE ${IPCM_BASE}/g" ${MEM_CFG_IPCM_RTT}
		sed -i "s/#define MEM_IPCM_SIZE.*$/#define MEM_IPCM_SIZE ${IPCM_SIZE}/g" ${MEM_CFG_IPCM_RTT}

	}

	{ #mmz
		#source files will be modified for mmz memory configuration
		MEM_CFG_MMZ_MPP="${RTSMART_SRC_DIR}/kernel/bsp/maix3/board/mpp/mpp_init.c"
		echo "Modify file: ${MEM_CFG_MMZ_MPP}"

		#mmz memory configuration
		MMZ_BASE="$( printf '0x%xUL\n' $[${CONFIG_MEM_MMZ_BASE}+0])"
		MMZ_SIZE="$( printf '0x%xUL\n' $[${CONFIG_MEM_MMZ_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"

		[ ! -f ${MEM_CFG_MMZ_MPP} ]  || sed -i "s/#define MEM_MMZ_BASE.*$/#define MEM_MMZ_BASE ${MMZ_BASE}/g" ${MEM_CFG_MMZ_MPP}
		[ ! -f ${MEM_CFG_MMZ_MPP} ]  || sed -i "s/#define MEM_MMZ_SIZE.*$/#define MEM_MMZ_SIZE ${MMZ_SIZE}/g" ${MEM_CFG_MMZ_MPP}
	}

    {
        RT_SMART_CONFIG="${RTSMART_SRC_DIR}/kernel/bsp/maix3/rtconfig.h"
        cp ${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/common_rttlinux.config ${RT_SMART_CONFIG}
        if [  "$CONFIG_SUPPORT_LINUX" != "y"  -a   "$CONFIG_SDCAED"  = "y" ] ;then
            cp ${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/common_rttonly.config ${RT_SMART_CONFIG}
            if [ "${CONFIG_BOARD_K230_CANMV}" = "y" ]; then
                cp ${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/k230_canmv_only_rtt.config ${RT_SMART_CONFIG}
            fi;
            if [ "${CONFIG_BOARD_K230D_CANMV}" = "y" ]; then
                cp ${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/k230d_canmv_only_rtt.config ${RT_SMART_CONFIG}
            fi;
        fi
        if [ -f "${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/${CONFIG_RTTHREAD_DEFCONFIG}.config" ];then
            cp ${RTSMART_SRC_DIR}/kernel/bsp/maix3/configs/${CONFIG_RTTHREAD_DEFCONFIG}.config ${RT_SMART_CONFIG}
        fi
    }
}
function modify_cdk_code()
{
	{
		#ipc
		MEM_CFG_IPCM_CDK_CONFIG="${CDK_SRC_DIR}/kernel/ipcm/arch/k230/configs/k230_riscv_rtsmart_config"
		MEM_CFG_IPCM_CDK_PLATFORM="${CDK_SRC_DIR}/kernel/ipcm/arch/k230/ipcm_platform.h"
		echo "Modify file: ${MEM_CFG_IPCM_CDK_CONFIG}"
		echo "Modify file: ${MEM_CFG_IPCM_CDK_PLATFORM}"
		#ipcm memory configuration
		IPCM_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_IPCM_BASE}+0])"
		IPCM_SHM_PHYS_1TO0="${IPCM_BASE}"
		IPCM_SHM_SIZE_1TO0=0x80000
		IPCM_SHM_PHYS_0TO1="$( printf '0x%x\n' $[${IPCM_SHM_PHYS_1TO0}+${IPCM_SHM_SIZE_1TO0}])"
		IPCM_SHM_SIZE_0TO1=0x79000
		IPCM_VIRT_TTY_PHYS="$( printf '0x%x\n' $[${IPCM_SHM_PHYS_0TO1}+${IPCM_SHM_SIZE_0TO1}])"
		IPCM_VIRT_TTY_SIZE=0x4000
		IPCM_NODES_DESC_MEM_BASE="$( printf '0x%x\n' $[${IPCM_VIRT_TTY_PHYS}+${IPCM_VIRT_TTY_SIZE}])"



		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/virt_tty_phys=.*$/virt_tty_phys=${IPCM_VIRT_TTY_PHYS}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/virt_tty_size=.*$/virt_tty_size=${IPCM_VIRT_TTY_SIZE}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/shm_phys_1to0=.*$/shm_phys_1to0=${IPCM_SHM_PHYS_1TO0}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/shm_size_1to0=.*$/shm_size_1to0=${IPCM_SHM_SIZE_1TO0}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/shm_phys_0to1=.*$/shm_phys_0to1=${IPCM_SHM_PHYS_0TO1}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_CONFIG} ] || sed -i "s/shm_size_0to1=.*$/shm_size_0to1=${IPCM_SHM_SIZE_0TO1}/g" ${MEM_CFG_IPCM_CDK_CONFIG}
		[ ! -f ${MEM_CFG_IPCM_CDK_PLATFORM} ] ||  sed -i "s/#define __NODES_DESC_MEM_BASE__.*$/#define __NODES_DESC_MEM_BASE__ ${IPCM_NODES_DESC_MEM_BASE}/g" ${MEM_CFG_IPCM_CDK_PLATFORM}
	}

}

function modify_opensbi_code()
{
    if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] ;then
	{
		#rtt opensbi memory configuration
		local OPENSBI_RTT_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_BASE}+0])"
		local OPENSBI_RTT_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_RTT_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"
		local RTT_SYS_BASE="$(printf '0x%x\n' $(( ${CONFIG_MEM_RTT_SYS_BASE} + 0x20000 )) )"
		local OPENSBI_RTT_JUMP="${RTT_SYS_BASE}"

		#source files will be modified for rtt-opensbi memory configuration
		local OPENSBI_CONFIG_RTT="${K230_SDK_ROOT}/src/common/opensbi/platform/kendryte/fpgac908/config.mk"

		echo "rt-smart opensbi BASE: ${OPENSBI_RTT_BASE}"
		echo "rt-smart opensbi JUMP: ${OPENSBI_RTT_JUMP}"
		echo "Modify file: ${OPENSBI_CONFIG_RTT}"
		sed -i "s/FW_TEXT_START=.*$/FW_TEXT_START=${OPENSBI_RTT_BASE}/g" ${OPENSBI_CONFIG_RTT}
		sed -i "s/FW_JUMP_ADDR=.*$/FW_JUMP_ADDR=${OPENSBI_RTT_JUMP}/g" ${OPENSBI_CONFIG_RTT}
	}
    fi

    if [ "${CONFIG_SUPPORT_LINUX}" = "y" ] ;then
	{
		#linux opensbi memory configuration
		local OPENSBI_LINUX_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0])"
		local OPENSBI_LINUX_SIZE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_SIZE}-${CONFIG_MEM_BOUNDARY_RESERVED_SIZE}])"
		local OPENSBI_LINUX_PAYLOAD_OFFSET=0x200000
		local LINUX_SYS_BASE="$( printf '0x%x\n' $[${CONFIG_MEM_LINUX_SYS_BASE}+0x200000])"
		local OPENSBI_LINUX_JUMP="${LINUX_SYS_BASE}"

		DTB_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000])" #32MB offset
		FDT_HIGH="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000])"
		KERNEL_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000+0x2000000])"
		RAMDISK_ADDR="$( printf '0x%x\n' $[${OPENSBI_LINUX_BASE}+0x2000000+0x100000])"

		#source files will be modified for linux-opensbi memory configuration
		OPENSBI_CONFIG_LINUX="${K230_SDK_ROOT}/src/common/opensbi/platform/generic/config.mk"

		echo "linux opensbi BASE: ${OPENSBI_LINUX_BASE}"
		echo "linux opensbi JUMP: ${OPENSBI_LINUX_JUMP}"
		echo "linux opensbi payload offset: ${OPENSBI_LINUX_PAYLOAD_OFFSET}"
		echo "Modify file: ${OPENSBI_CONFIG_LINUX}"
		sed -i "s/FW_TEXT_START=.*$/FW_TEXT_START=${OPENSBI_LINUX_BASE}/g" ${OPENSBI_CONFIG_LINUX}
		sed -i "s/FW_JUMP_ADDR=.*$/FW_JUMP_ADDR=${OPENSBI_LINUX_JUMP}/g" ${OPENSBI_CONFIG_LINUX}
		sed -i "s/FW_PAYLOAD_OFFSET=.*$/FW_PAYLOAD_OFFSET=${OPENSBI_LINUX_PAYLOAD_OFFSET}/g" ${OPENSBI_CONFIG_LINUX}
	}
    fi

	#opensbi/platform/kendryte/fpgac908/platform.c
	{
		OPENSBI_PLATFORM="${K230_SDK_ROOT}/src/common/opensbi/platform/kendryte/fpgac908/platform.c"
		local RTT_UART_REG_BASDE="0x9140${CONFIG_RTT_CONSOLE_ID}000UL" #0x91403000UL
		sed -i "s/define *UART_ADDR.*$/define UART_ADDR ${RTT_UART_REG_BASDE}/g" ${OPENSBI_PLATFORM}
	}

}

modify_linux_dts;
[ "${CONFIG_SPI_NOR}" = "y" ] && modify_gen_image_cfg_spinorcfg;
[ "${CONFIG_SDCAED}" = "y" ] && modify_gen_image_sd_cfg;
modify_uboot_file;
modify_opensbi_code;
[ "${CONFIG_SUPPORT_RTSMART}" = "y" ] &&  modify_big_code;
[ "${CONFIG_SUPPORT_RTSMART}" = "y" ] &&  modify_cdk_code;
pwd;

#set -x;
