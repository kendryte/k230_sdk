ifeq ($(wildcard .config), .config)
	include .config
else
	include $(DEFCONFIG)

endif


export RTT_CC		:= gcc
export RTT_CC_PREFIX	:= $(CONFIG_TOOLCHAIN_PREFIX_RTT)
export RTT_EXEC_PATH	:= $(CONFIG_TOOLCHAIN_PATH_RTT)

export LINUX_CC	:= gcc
export LINUX_CC_PREFIX	:= $(CONFIG_TOOLCHAIN_PREFIX_LINUX)
export LINUX_EXEC_PATH	:=$(CONFIG_TOOLCHAIN_PATH_LINUX)

export RTT_SDK_BUILD_DIR	:= $(BUILD_DIR)/big/rt-smart
export BIG_OPENSBI_BUILD_DIR	:= $(BUILD_DIR)/common/big-opensbi
export LITTLE_OPENSBI_BUILD_DIR	:= $(BUILD_DIR)/common/little-opensbi

export CDK_BUILD_DIR			:= $(BUILD_DIR)/common/cdk
export LINUX_BUILD_DIR			:= $(BUILD_DIR)/little/linux
export BUILDROOT_BUILD_DIR		:= $(BUILD_DIR)/little/buildroot-ext
export UBOOT_BUILD_DIR			:= $(BUILD_DIR)/little/uboot

export IMAGE_DIR			:= $(BUILD_DIR)/images
CONFIG_UBOOT_DEFCONFIG	?=$(CONFIG_BOARD_NAME)

LINUX_KERNEL_DEFCONFIG	= $(CONFIG_BOARD_NAME)_defconfig
UBOOT_DEFCONFIG			= $(CONFIG_UBOOT_DEFCONFIG)_defconfig
BUILDROOT_DEFCONFIG		= $(CONFIG_BOARD_NAME)_defconfig

LINUX_DTS_NAME	= $(CONFIG_BOARD_NAME).dtsi
LINUX_DTS_PATH	= $(LINUX_SRC_PATH)/arch/riscv/boot/dts/kendryte/$(LINUX_DTS_NAME)

#memory configuration variables
#rt-smart system memory configuration
export MEM_CFG_RTT_LDS		:= $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/link.lds
export MEM_CFG_RTT_RAMEND	:= $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/board/board.h

#source files will be modified for rt-smart system memory configuration
RTT_SYS_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_RTT_SYS_BASE)+0x20000])
RTT_SYS_SIZE	= $(shell printf '%d\n' $$[(($(CONFIG_MEM_RTT_SYS_SIZE)-0x20000)/1024)-1])
RTT_RAM_END		= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_RTT_SYS_BASE)+$(CONFIG_MEM_RTT_SYS_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])

#ipcm memory configuration
IPCM_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_IPCM_BASE)+0])
IPCM_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_IPCM_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])
IPCM_SHM_PHYS_1TO0	= $(IPCM_BASE)
IPCM_SHM_SIZE_1TO0	= 0x80000
IPCM_SHM_PHYS_0TO1	= $(shell printf '0x%x\n' $$[$(IPCM_SHM_PHYS_1TO0)+$(IPCM_SHM_SIZE_1TO0)])
IPCM_SHM_SIZE_0TO1	= 0x79000
IPCM_VIRT_TTY_PHYS	= $(shell printf '0x%x\n' $$[$(IPCM_SHM_PHYS_0TO1)+$(IPCM_SHM_SIZE_0TO1)])
IPCM_VIRT_TTY_SIZE	= 0x4000
IPCM_NODES_DESC_MEM_BASE	= $(shell printf '0x%x\n' $$[$(IPCM_VIRT_TTY_PHYS)+$(IPCM_VIRT_TTY_SIZE)])

#source files will be modified for ipcm memory configuration
export MEM_CFG_IPCM_RTT	:= $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/board/board.c
export MEM_CFG_IPCM_CDK_CONFIG	:= $(CDK_SRC_PATH)/kernel/ipcm/arch/k230/configs/k230_riscv_rtsmart_config
export MEM_CFG_IPCM_CDK_PLATFORM	:= $(CDK_SRC_PATH)/kernel/ipcm/arch/k230/ipcm_platform.h

BOUNDARY_RESERVED_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)+0])

#mmz memory configuration
MMZ_BASE	= $(shell printf '0x%xUL\n' $$[$(CONFIG_MEM_MMZ_BASE)+0])
MMZ_SIZE	= $(shell printf '0x%xUL\n' $$[$(CONFIG_MEM_MMZ_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])

#source files will be modified for mmz memory configuration
export MEM_CFG_MMZ_MPP := $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/board/mpp/mpp_init.c

#param memory configuration
PARAM_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_PARAM_BASE)+0])
PARAM_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_PARAM_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])

#param memory configuration
AI_MODEL_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_AI_MODEL_BASE)+0])
AI_MODEL_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_AI_MODEL_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])

#linux memory configuration
LINUX_SYS_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_LINUX_SYS_BASE)+0x200000])
LINUX_SYS_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_LINUX_SYS_SIZE)-0x200000-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])

#source files will be modified for linux kernel memory configuration
LINUX_DTSI_NAME	= $(CONFIG_BOARD_NAME).dtsi
LINUX_DTSI_PATH	= $(LINUX_SRC_PATH)/arch/riscv/boot/dts/kendryte/$(LINUX_DTSI_NAME)

#rtt opensbi memory configuration
OPENSBI_RTT_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_RTT_SYS_BASE)+0])
OPENSBI_RTT_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_RTT_SYS_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])
OPENSBI_RTT_JUMP	= $(RTT_SYS_BASE)

#source files will be modified for rtt-opensbi memory configuration
OPENSBI_CONFIG_RTT	= $(OPENSBI_SRC_PATH)/platform/kendryte/fpgac908/config.mk

#linux opensbi memory configuration
OPENSBI_LINUX_BASE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_LINUX_SYS_BASE)+0])
OPENSBI_LINUX_SIZE	= $(shell printf '0x%x\n' $$[$(CONFIG_MEM_LINUX_SYS_SIZE)-$(CONFIG_MEM_BOUNDARY_RESERVED_SIZE)])
OPENSBI_LINUX_PAYLOAD_OFFSET	= 0x200000
OPENSBI_LINUX_JUMP	= $(LINUX_SYS_BASE)

DTB_ADDR = $(shell printf '0x%x\n' $$[$(OPENSBI_LINUX_BASE)+0x2000000]) #32MB offset
FDT_HIGH = $(shell printf '0x%x\n' $$[$(OPENSBI_LINUX_BASE)+0x2000000+0x100000])
KERNEL_ADDR = $(shell printf '0x%x\n' $$[$(OPENSBI_LINUX_BASE)+0x2000000+0x100000+0x2000000])
RAMDISK_ADDR = $(shell printf '0x%x\n' $$[$(OPENSBI_LINUX_BASE)+0x2000000+0x100000])

#source files will be modified for linux-opensbi memory configuration
OPENSBI_CONFIG_LINUX	= $(OPENSBI_SRC_PATH)/platform/generic/config.mk

#linux dts file used to compile the dtb
LINUX_DTS_NAME	= $(CONFIG_BOARD_NAME).dts
LINUX_DTS_PATH	= $(LINUX_SRC_PATH)/arch/riscv/boot/dts/kendryte/$(LINUX_DTS_NAME)

#source files will be modified for uboot memory configuration
UBOOT_DTS_NAME	= $(CONFIG_BOARD_NAME).dts
UBOOT_DTS_PATH	= $(UBOOT_SRC_PATH)/arch/riscv/dts/$(UBOOT_DTS_NAME)

UBOOT_CONFIGH_NAME	= $(CONFIG_BOARD_NAME).h
UBOOT_CONFIGH_PATH	= $(UBOOT_SRC_PATH)/include/configs/$(UBOOT_CONFIGH_NAME)

UBOOT_CONFIG_NAME	= $(CONFIG_BOARD_NAME)_defconfig
UBOOT_CONFIG_PATH	= $(UBOOT_SRC_PATH)/configs/$(UBOOT_CONFIG_NAME)

UBOOT_ENV_PATH = $(K230_SDK_ROOT)/tools/gen_image_cfg/genimage-sdcard.cfg.env  $(K230_SDK_ROOT)/tools/gen_image_cfg/genimage-spinor.cfg.jffs2.env

# Strip quotes and then whitespaces
qstrip = $(strip $(subst ",,$(1)))
#size reserver
realsize = $(shell printf '0x%x' $$[$($(1)) - $($(2))] )

# MEM_CFG_UBOOT_ENV,mem_param_base,0x1000
#$(call MEM_CFG_UBOOT_ENV,mem_param_base, $(call qstrip, $(CONFIG_MEM_PARAM_BASE)))
define MEM_CFG_MODIFY_UBOOT_ENV
	sed -i 's/\"$(1)=.*$$/\"$(1)=$(2)\\0" \\/g' $(UBOOT_CONFIGH_PATH)
	sed -i 's/$(1)=.*$$/$(1)=$(2)/g' $(UBOOT_ENV_PATH)
endef

define CONFIG_MEM_UBOOT_ENV_MODIFY	
	$(call MEM_CFG_MODIFY_UBOOT_ENV,dtb_addr,$(DTB_ADDR))  
	$(call MEM_CFG_MODIFY_UBOOT_ENV,fdt_high,$(FDT_HIGH)) 
	$(call MEM_CFG_MODIFY_UBOOT_ENV,kernel_addr,$(KERNEL_ADDR))
	$(call MEM_CFG_MODIFY_UBOOT_ENV,ramdisk_addr,$(RAMDISK_ADDR)) 
endef 

define CONFIG_MEM_RTT
	set -e; \
	echo "rt-smart system memory base: $(RTT_SYS_BASE)"
	echo "rt-smart system memory size: $(RTT_SYS_SIZE)K"
	echo "rt-smart system memory end: $(RTT_RAM_END)"
	echo "Modify file: $(MEM_CFG_RTT_LDS)"
	echo "Modify file: $(MEM_CFG_RTT_RAMEND)"
	sed -i 's/. =.*rt-smart link start address.*$$/. = $(RTT_SYS_BASE);    \/\*rt-smart link start address\*\//g' $(MEM_CFG_RTT_LDS)
	sed -i 's/SRAM : ORIGIN.*$$/SRAM : ORIGIN = $(RTT_SYS_BASE), LENGTH = $(RTT_SYS_SIZE)K/g' $(MEM_CFG_RTT_LDS)
	sed -i 's/#define RAM_END.*$$/#define RAM_END $(RTT_RAM_END)/g' $(MEM_CFG_RTT_RAMEND)
	echo "IPCM memory base: $(IPCM_BASE)"
	echo "IPCM memory size: $(IPCM_SIZE)"
	echo "IPCM_SHM_PHYS_1TO0: $(IPCM_SHM_PHYS_1TO0)"
	echo "IPCM_SHM_SIZE_1TO0: $(IPCM_SHM_SIZE_1TO0)"
	echo "IPCM_SHM_PHYS_0TO1: $(IPCM_SHM_PHYS_0TO1)"
	echo "IPCM_SHM_SIZE_0TO1: $(IPCM_SHM_SIZE_0TO1)"
	echo "IPCM_VIRT_TTY_PHYS: $(IPCM_VIRT_TTY_PHYS)"
	echo "IPCM_VIRT_TTY_SIZE: $(IPCM_VIRT_TTY_SIZE)"
	echo "IPCM_NODES_DESC_MEM_BASE: $(IPCM_NODES_DESC_MEM_BASE)"
	echo "Modify file: $(MEM_CFG_IPCM_RTT)"
	echo "Modify file: $(MEM_CFG_IPCM_CDK_CONFIG)"
	echo "Modify file: $(MEM_CFG_IPCM_CDK_PLATFORM)"
	sed -i 's/#define MEM_IPCM_BASE.*$$/#define MEM_IPCM_BASE $(IPCM_BASE)/g' $(MEM_CFG_IPCM_RTT)
	sed -i 's/#define MEM_IPCM_SIZE.*$$/#define MEM_IPCM_SIZE $(IPCM_SIZE)/g' $(MEM_CFG_IPCM_RTT)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/virt_tty_phys=.*$$/virt_tty_phys=$(IPCM_VIRT_TTY_PHYS)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/virt_tty_size=.*$$/virt_tty_size=$(IPCM_VIRT_TTY_SIZE)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/shm_phys_1to0=.*$$/shm_phys_1to0=$(IPCM_SHM_PHYS_1TO0)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/shm_size_1to0=.*$$/shm_size_1to0=$(IPCM_SHM_SIZE_1TO0)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/shm_phys_0to1=.*$$/shm_phys_0to1=$(IPCM_SHM_PHYS_0TO1)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_CONFIG) ] || sed -i 's/shm_size_0to1=.*$$/shm_size_0to1=$(IPCM_SHM_SIZE_0TO1)/g' $(MEM_CFG_IPCM_CDK_CONFIG)
	[ ! -f $(MEM_CFG_IPCM_CDK_PLATFORM) ] ||  sed -i 's/#define __NODES_DESC_MEM_BASE__.*$$/#define __NODES_DESC_MEM_BASE__ $(IPCM_NODES_DESC_MEM_BASE)/g' $(MEM_CFG_IPCM_CDK_PLATFORM)
	echo "MMZ memory base: $(MMZ_BASE)"
	echo "MMZ memory size: $(MMZ_SIZE)"
	echo "Modify file: $(MEM_CFG_MMZ_MPP)"
	[ ! -f $(MEM_CFG_MMZ_MPP) ]  || sed -i 's/#define MEM_MMZ_BASE.*$$/#define MEM_MMZ_BASE $(MMZ_BASE)/g' $(MEM_CFG_MMZ_MPP)
	[ ! -f $(MEM_CFG_MMZ_MPP) ]  || sed -i 's/#define MEM_MMZ_SIZE.*$$/#define MEM_MMZ_SIZE $(MMZ_SIZE)/g' $(MEM_CFG_MMZ_MPP)
	echo "LINUX system BASE: $(LINUX_SYS_BASE)"
	echo "LINUX system SIZE: $(LINUX_SYS_SIZE)"
	echo "Modify file: $(LINUX_DTSI_PATH)"
	sed -i 's/reg =.*linux memory config.*$$/reg = <0x0 $(LINUX_SYS_BASE) 0x0 $(LINUX_SYS_SIZE)>;  \/\*linux memory config\*\//g' $(LINUX_DTSI_PATH)
	echo "rt-smart opensbi BASE: $(OPENSBI_RTT_BASE)"
	echo "rt-smart opensbi JUMP: $(OPENSBI_RTT_JUMP)"
	echo "Modify file: $(OPENSBI_CONFIG_RTT)"
	sed -i 's/FW_TEXT_START=.*$$/FW_TEXT_START=$(OPENSBI_RTT_BASE)/g' $(OPENSBI_CONFIG_RTT)
	sed -i 's/FW_JUMP_ADDR=.*$$/FW_JUMP_ADDR=$(OPENSBI_RTT_JUMP)/g' $(OPENSBI_CONFIG_RTT)
	echo "linux opensbi BASE: $(OPENSBI_LINUX_BASE)"
	echo "linux opensbi JUMP: $(OPENSBI_LINUX_JUMP)"
	echo "linux opensbi payload offset: $(OPENSBI_LINUX_PAYLOAD_OFFSET)"
	echo "Modify file: $(OPENSBI_CONFIG_LINUX)"
	sed -i 's/FW_TEXT_START=.*$$/FW_TEXT_START=$(OPENSBI_LINUX_BASE)/g' $(OPENSBI_CONFIG_LINUX)
	sed -i 's/FW_JUMP_ADDR=.*$$/FW_JUMP_ADDR=$(OPENSBI_LINUX_JUMP)/g' $(OPENSBI_CONFIG_LINUX)
	sed -i 's/FW_PAYLOAD_OFFSET=.*$$/FW_PAYLOAD_OFFSET=$(OPENSBI_LINUX_PAYLOAD_OFFSET)/g' $(OPENSBI_CONFIG_LINUX)
	echo "uboot dts memory BASE: $(LINUX_SYS_BASE)"
	echo "uboot dts memory SIZE: $(LINUX_SYS_SIZE)"
	echo "uboot dts memory BASE: $(RTT_SYS_BASE)"
	echo "uboot dts memory SIZE: $(RTT_SYS_SIZE)"
	echo "Modify file: $(UBOOT_DTS_PATH)"
	sed -i 's/0x.*MEM_LINUX_SYS.*$$/0x0 $(OPENSBI_LINUX_BASE) 0x0 $(OPENSBI_LINUX_SIZE)  \/\*MEM_LINUX_SYS\*\//g' $(UBOOT_DTS_PATH)
	sed -i 's/0x.*MEM_RTT_SYS.*$$/0x0 $(OPENSBI_RTT_BASE) 0x0 $(OPENSBI_RTT_SIZE)  \/\*MEM_RTT_SYS\*\//g' $(UBOOT_DTS_PATH)
	echo "Modify file: $(UBOOT_CONFIGH_PATH)"
	
	$(call CONFIG_MEM_UBOOT_ENV_MODIFY)

	echo "Modify file: $(UBOOT_CONFIG_NAME)"
	sed -i 's/CONFIG_SYS_TEXT_BASE=.*$$/CONFIG_SYS_TEXT_BASE=$(OPENSBI_LINUX_BASE)/g' $(UBOOT_CONFIG_PATH)
	$(K230_SDK_ROOT)/tools/menuconfig_to_code.sh
endef

