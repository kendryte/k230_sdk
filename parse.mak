ifeq ($(wildcard .config), .config)
	include .config
else
	include $(DEFCONFIG)
endif

# Strip quotes and then whitespaces
qstrip = $(strip $(subst ",,$(1)))

ifdef CONFIG_BUILD_RELEASE_VER
CONFIG_DBGLV := 0
endif 
ifeq ("$(origin debug)", "command line")
  CONFIG_DBGLV = $(debug)
endif
CONFIG_DBGLV ?=0
export KCFLAGS=-DDBGLV=$(CONFIG_DBGLV)


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
CONFIG_BOARD_NAME_NO_QUOTES = $(call qstrip ,$(CONFIG_BOARD_NAME))

CONFIG_UBOOT_DEFCONFIG	?= $(CONFIG_BOARD_NAME_NO_QUOTES)
CONFIG_UBOOT_DEFCONFIG := $(call qstrip ,$(CONFIG_UBOOT_DEFCONFIG))

LINUX_KERNEL_DEFCONFIG	= $(CONFIG_BOARD_NAME_NO_QUOTES)_defconfig
UBOOT_DEFCONFIG			=$(CONFIG_UBOOT_DEFCONFIG)_defconfig
BUILDROOT_DEFCONFIG		= $(CONFIG_BOARD_NAME_NO_QUOTES)_defconfig



ifneq ($(CONFIG_DBGLV),0)  
#debug version
	ifneq ($(wildcard $(LINUX_SRC_PATH)/arch/riscv/configs/$(CONFIG_BOARD_NAME_NO_QUOTES)_d_defconfig),)
		LINUX_KERNEL_DEFCONFIG = $(CONFIG_BOARD_NAME_NO_QUOTES)_d_defconfig
	endif 
	ifneq ($(wildcard $(UBOOT_SRC_PATH)/configs/$(CONFIG_UBOOT_DEFCONFIG)_d_defconfig),)
		UBOOT_DEFCONFIG			= $(CONFIG_UBOOT_DEFCONFIG)_d_defconfig
	endif 
	ifneq ($(wildcard $(BUILDROOT-EXT_SRC_PATH)/configs/$(CONFIG_BOARD_NAME_NO_QUOTES)_d_defconfig),)
		BUILDROOT_DEFCONFIG		= $(CONFIG_BOARD_NAME_NO_QUOTES)_d_defconfig
	endif 
endif 

export UBOOT_DEFCONFIG 

