WLAN_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(WLAN_DIR)/env_config.mk

ifeq ($(CFG_3518EV300), y)
ARCH ?= arm
KDIR ?= 
CROSS_COMPILE ?= arm-himix100-linux-
endif

ifeq ($(CFG_T31), y)
ARCH ?= mips
KDIR ?= 
CROSS_COMPILE ?= mips-linux-uclibc-gnu-
endif

ifeq ($(CFG_K510), y)
ARCH ?= riscv
KDIR ?= 
CROSS_COMPILE ?= 
endif

ifeq ($(CFG_K230), y)
ARCH ?= 
KDIR ?= 
CROSS_COMPILE ?= 
endif

MAIN_PATH :=
OAL_PATH := oal/
OAM_PATH := oam/
WAL_PATH := wal/
HCC_PATH := hcc/
SEC_PATH := libboundscheck/
SDIO_PATH := sdio/

WLAN_CFLAGS += -std=gnu99 -Wno-declaration-after-statement -fsigned-char -freg-struct-return -Wtrampolines -Wfloat-equal -fvisibility=hidden

ifeq ($(CFG_3518EV300), y)
WLAN_CFLAGS += -Wdate-time
endif

WLAN_CFLAGS += -I$(WLAN_DIR)
WLAN_CFLAGS += -I$(WLAN_DIR)hcc
WLAN_CFLAGS += -I$(WLAN_DIR)ipcm
WLAN_CFLAGS += -I$(WLAN_DIR)include
WLAN_CFLAGS += -I$(WLAN_DIR)libboundscheck
WLAN_CFLAGS += -I$(WLAN_DIR)sdio
WLAN_CFLAGS += -I$(WLAN_DIR)oal
WLAN_CFLAGS += -I$(WLAN_DIR)oam
WLAN_CFLAGS += -I$(WLAN_DIR)uart

main-objs += main.o
main-objs := $(addprefix $(MAIN_PATH),$(main-objs))

oal-objs += oal_util.o oal_netbuf.o
oal-objs := $(addprefix $(OAL_PATH),$(oal-objs))

oam-objs  += oam_main.o oam_log.o
oam-objs := $(addprefix $(OAM_PATH),$(oam-objs))

wal-objs += wal_net.o wal_netlink.o
wal-objs := $(addprefix $(WAL_PATH),$(wal-objs))

hcc-objs := hcc_host.o hcc_task.o hcc_adapt.o
hcc-objs := $(addprefix $(HCC_PATH),$(hcc-objs))

sdio-objs := sdio_host.o
sdio-objs := $(addprefix $(SDIO_PATH),$(sdio-objs))

sec-objs += memcpy_s.o memmove_s.o memset_s.o strcpy_s.o strncpy_s.o snprintf_s.o vsnprintf_s.o
sec-objs := $(addprefix $(SEC_PATH),$(sec-objs))

WLAN_OBJS += $(main-objs) $(oal-objs) $(oam-objs) $(wal-objs) $(hcc-objs) $(sec-objs) $(sdio-objs)

EXTRA_CFLAGS += $(WLAN_CFLAGS)
PWD := $(shell pwd)
USER_MODULE_NAME := aiw4211lv10
MODULE_NAME := $(USER_MODULE_NAME)
$(MODULE_NAME)-y += $(WLAN_OBJS)
#obj-m = $(MODULE_NAME).o
obj-$(CONFIG_AIW4211LV10) = $(MODULE_NAME).o

#==================================================================
#                          modules
#==================================================================
.PHONY: all sample sample_clean clean

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

sample:
	$(MAKE) -C sample all

sample_clean:
	$(MAKE) -C sample clean

clean:
	rm -rf $(COBJS) $(MODULE_NAME).o $(MODULE_NAME).ko
	rm -rf ./*.ko
	rm -rf ./*.o
	rm -rf ./.*.cmd
	rm -rf ./*.symvers
	rm -rf ./*.order
	rm -rf ./*.mod.c
	rm -rf ./oal/*.o
	rm -rf ./oal/.*.cmd
	rm -rf ./wal/*.o
	rm -rf ./wal/.*.cmd
	rm -rf ./libboundscheck/*.o
	rm -rf ./libboundscheck/.*.cmd
	rm -rf ./hcc/*.o
	rm -rf ./hcc/.*.cmd
	rm -rf ./oam/*.o
	rm -rf ./oam/.*.cmd
	rm -rf ./sdio/*.o
	rm -rf ./sdio/.*.cmd
	rm -rf .tmp_versions
