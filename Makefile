red=\e[1;31m
NC=\e[0m

SHELL=/bin/bash

BUILD_TYPE ?= product
NATIVE_BUILD ?= 1

MPP_SRC_DIR="${K230_SDK_ROOT}/src/big/mpp"
CDK_SRC_DIR="${K230_SDK_ROOT}/src/common/cdk"
BUILDROOT_EXT_SRC_PATH="src/little/buildroot-ext"
POST_COPY_ROOTFS_PATH=tools/post_copy_rootfs

#DOWNLOAD_URL=https://ai.b-bug.org/k230
DOWNLOAD_URL?=https://kendryte-download.canaan-creative.com/k230

ifeq ($(NATIVE_BUILD),1)
RTT_TOOLCHAIN_URL = $(DOWNLOAD_URL)/toolchain/riscv64-unknown-linux-musl-rv64imafdcv-lp64d-20230420.tar.bz2
LINUX_TOOLCHAIN_URL = $(DOWNLOAD_URL)/toolchain/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0.tar.bz2
endif

export K230_SDK_ROOT := $(shell pwd)
ifeq ("$(origin CONF)", "command line")
$(shell echo CONF=$(CONF)>.last_conf;rm -rf .config)
else
$(shell [ -f .last_conf ] || ( echo CONF=k230_evb_defconfig>.last_conf; rm -rf .config ) )
endif

ifneq ($(wildcard .last_conf),)
include .last_conf
endif

CONF ?= k230_evb_defconfig
export BUILD_DIR := $(K230_SDK_ROOT)/output/$(CONF)

DEFCONFIG = configs/$(CONF)

include repo.mak
include parse.mak

ADD_PATH := :$(RTT_EXEC_PATH):$(LINUX_EXEC_PATH)
TMP_PATH := $(addsuffix $(ADD_PATH), $(PATH))
export PATH=$(TMP_PATH)
export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH)
export CDK_SRC_DIR=$(K230_SDK_ROOT)/$(CDK_SRC_PATH)
export MPP_SRC_DIR=$(K230_SDK_ROOT)/$(MPP_SRC_PATH)



KCONFIG_PATH= tools/kconfig/
KCONFIG_MCONF_EXE = tools/kconfig/mconf
KCONFIG_CFG = Kconfig

GEN_IMAGE_SCRIPT ?= gen_image.sh
ifeq ($(CONF), k230_evb_doorlock_defconfig)
GEN_IMAGE_SCRIPT = gen_doorlock_image.sh
endif

ifeq ($(CONF), k230d_doorlock_defconfig)
GEN_IMAGE_SCRIPT = gen_doorlock_image.sh
endif

ifeq ($(CONF), k230_evb_doorlock_ov9286_defconfig)
GEN_IMAGE_SCRIPT = gen_doorlock_ov9286_image.sh
endif

define CLEAN
	set -e; \
	echo "clean test"

endef

define BUILD_IMAGE
	set -e; \
	echo "build SDK images"

endef

#include .config

.PHONY: all
all .DEFAULT: check_src prepare_memory linux mpp cdk-kernel cdk-kernel-install cdk-user cdk-user-install rt-smart-apps rt-smart-kernel big-core-opensbi little-core-opensbi buildroot uboot build-image

.PHONY: test_url
test_url:
	source tools/get_download_url.sh

ifeq ($(NATIVE_BUILD),1)
.PHONY: download_toolchain
download_toolchain:test_url
	@set -e; \
	if [ ! -f toolchain/.toolchain_ready ];then \
	echo "download toolchain"; \
	echo $(DOWNLOAD_URL); \
	wget -qc -P $(K230_SDK_ROOT)/toolchain $(RTT_TOOLCHAIN_URL); \
	wget -qc -P $(K230_SDK_ROOT)/toolchain $(LINUX_TOOLCHAIN_URL); \
	fi;

.PHONY: extract_toolchain
extract_toolchain: download_toolchain
	@echo "test1"
	@set -e; \
	if [ ! -f toolchain/.toolchain_ready ];then \
	echo "extract toolchain"; \
	for file in $(shell find ./toolchain -name "*.bz2"); do echo $$file;tar jxf $$file -C $(K230_SDK_ROOT)/toolchain; done; \
	fi;

.PHONY: prepare_toolchain
prepare_toolchain: extract_toolchain
	@if [ ! -f toolchain/.toolchain_ready ];then \
	touch toolchain/.toolchain_ready; \
	fi;
	@echo "toolchain is ready"
endif

.PHONY: prepare_sourcecode check_toolchain
prepare_sourcecode:prepare_toolchain
	@echo "prepare source code"
#ai
	@echo "download nncase sdk"
	@rm -rf src/big/utils/; rm -rf src/big/ai;
	@wget -qc $(DOWNLOAD_URL)/downloads/kmodel/kmodel_v2.2.0.tgz -O - | tar -xzC src/big/
	@wget -qc $(DOWNLOAD_URL)/downloads/nncase/nncase_k230_v2.2.0.tgz -O - | tar -xzC src/big/

#big utils
	@echo "download big utils"
	@wget -qc $(DOWNLOAD_URL)/downloads/big/utils/utils.tar.gz -O - | tar -xzC src/big/
	@cd src/big/utils/lib/;ln -s opencv_thead opencv;cd -

#wifi firmware
	@echo "download little firmware"
	@mkdir -p  ./src/little/utils/firmware/ || exit 1
	@wget -qc $(DOWNLOAD_URL)/downloads/firmware/iot_wifi/AiW4211L_demo_allinone.bin -O ./src/little/utils/firmware/AiW4211L_demo_allinone.bin || exit 1

#tuning-server
	@echo "download tuninig-server"
	@mkdir -p ${BUILDROOT_EXT_SRC_PATH}/package/tuning-server
	@wget -qc $(DOWNLOAD_URL)/downloads/tunning_server/tuning-server-package_v0.1.1.tar.bz2 -O ${BUILDROOT_EXT_SRC_PATH}/package/tuning-server/tuning-server-package_v0.1.1.tar.bz2
	@tar -jxf ${BUILDROOT_EXT_SRC_PATH}/package/tuning-server/tuning-server-package_v0.1.1.tar.bz2 -C ${POST_COPY_ROOTFS_PATH}/
	@mkdir -p tools/tuning-tool-client/
	@wget -qc $(DOWNLOAD_URL)/downloads/tunning_tools/tunning_client/Kendyte_ISP_Tool_TuningClient_RC22.5_Pre_596062-20221116.7z -P tools/tuning-tool-client/

#buildroot
	@echo "download buildroot dl"
	@wget $(DOWNLOAD_URL)/downloads/dl/dl.tar.gz -O - | tar -xzC src/little/buildroot-ext/

	@touch src/.src_fetched
check_toolchain:
	@if  [ ! -f  $(CONFIG_TOOLCHAIN_PATH_LINUX)/$(CONFIG_TOOLCHAIN_PREFIX_LINUX)gcc ] || \
		 [ !   -f $(CONFIG_TOOLCHAIN_PATH_RTT)/$(CONFIG_TOOLCHAIN_PREFIX_RTT)gcc  ]; then \
		 echo "please run command: source tools/get_download_url.sh && make prepare_sourcecode"; exit 1;  \
	fi;


#.PHONY: check_src
check_src:check_toolchain
	@if [ ! -f src/.src_fetched ];then \
	echo "Please run command: source tools/get_download_url.sh && make prepare_sourcecode";exit 1; \
	fi;


#.PHONY: defconfig
defconfig:   $(DEFCONFIG)   .last_conf
	@cp $(DEFCONFIG) .config;
	@touch $@

.PHONY: savedefconfig
savedefconfig:
	@cp .config $(DEFCONFIG)

.PHONY: prepare_menuconfig
prepare_menuconfig:
	@if [ ! -f $(KCONFIG_MCONF_EXE) ];then cd $(KCONFIG_PATH);make mconf conf;cd -;fi

.PHONY: menuconfig
menuconfig: prepare_menuconfig
	@$(KCONFIG_MCONF_EXE) $(KCONFIG_CFG)



#.PHONY: prepare_memory
prepare_memory: defconfig  .config  tools/menuconfig_to_code.sh  parse.mak
	@echo "prepare memory"
	@if [ ! -f tools/kconfig/conf ];then cd $(KCONFIG_PATH);make  conf;cd -;fi
	@mkdir -p  include/generated/  include/config/;
	@./tools/kconfig/conf --silentoldconfig  --olddefconfig $(KCONFIG_CFG)
	@cp include/generated/autoconf.h src/little/uboot/board/canaan/k230_evb/sdk_autoconf.h
	@cp include/generated/autoconf.h src/big/mpp/include/comm/k_autoconf_comm.h

#	#@cp include/generated/autoconf.h src/little/buildroot-ext/package/feature_opreation/src/sdk_autoconf.h
	@rm -rf include;
	@./tools/menuconfig_to_code.sh
	@touch $@

.PHONY: mpp-kernel
mpp-kernel: check_src
	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(MPP_SRC_PATH); \
	make -C kernel || exit $?; \
	cp kernel/lib/* $(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/mpp/lib/; \
	cd -;

.PHONY: mpp-kernel-clean
mpp-kernel-clean:
	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(MPP_SRC_DIR); \
	make clean -C kernel; \
	rm $(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/mpp/lib/* -rf; \
	cd -;

.PHONY: mpp-apps
mpp-apps:check_src
	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(MPP_SRC_DIR); \
	make -C userapps/src || exit $?; \
	mkdir -p userapps/sample/elf; \
	mkdir -p userapps/sample/fastboot_elf; \
	make -C userapps/sample || exit $?; \
	mkdir -p $(RTSMART_SRC_DIR)/userapps/root/bin/; \
	source $(K230_SDK_ROOT)/.config; [ "$${CONFIG_BOARD_K230D}" != "y" ] && cp userapps/sample/fastboot_elf/* $(RTSMART_SRC_DIR)/userapps/root/bin/; \
	cp $(RTSMART_SRC_DIR)/init.sh $(RTSMART_SRC_DIR)/userapps/root/bin/; \
	cd -;

.PHONY: mpp-apps-clean
mpp-apps-clean:
	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(MPP_SRC_DIR); \
	make clean -C userapps/src; \
	make clean -C userapps/sample; \
	cd -;

.PHONY: mpp
mpp: mpp-kernel mpp-apps

.PHONY: mpp-clean
mpp-clean: mpp-kernel-clean mpp-apps-clean

.PHONY: poc
poc:check_src
	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(K230_SDK_ROOT)/src/reference/business_poc/doorlock/big; \
	mkdir -p build; cd build; cmake ../; \
	make && make install; rm ./* -rf; \
	cd -;

	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(K230_SDK_ROOT)/src/reference/business_poc/peephole/big; \
	mkdir -p build; cd build; cmake ../; \
	make && make install; rm ./* -rf; \
	cd -;

	@export PATH=$(RTT_EXEC_PATH):$(PATH); \
	export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH); \
	cd $(K230_SDK_ROOT)/src/reference/business_poc/doorlock_ov9286/big; \
	mkdir -p build; cd build; cmake ../; \
	make && make install; rm ./* -rf; \
	cd -;

.PHONY: cdk-kernel
cdk-kernel: linux
	@echo "cdk-kernel";


.PHONY: cdk-kernel-install
cdk-kernel-install: check_src
	@mkdir -p $(LINUX_BUILD_DIR)/rootfs/mnt; \
	cd $(CDK_SRC_PATH)/kernel/ipcm; \
	cp out/node_0/* $(LINUX_BUILD_DIR)/rootfs/mnt/; \
	cp out/node_1/*.a $(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH)/kernel/bsp/maix3/board/ipcm/lib/; \
	cd -


.PHONY: cdk-kernel-clean
cdk-kernel-clean:
	@echo "cdk-kernel-clean";

.PHONY: cdk-user
cdk-user:check_src
	@export PATH=$(RTT_EXEC_PATH):$(LINUX_EXEC_PATH):$(PATH);export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH);export LINUX_BUILD_DIR=$(LINUX_BUILD_DIR); \
	cd $(CDK_SRC_PATH)/user/;make || exit $?;cd -

.PHONY: cdk-user-install
cdk-user-install:check_src
	@mkdir -p $(LINUX_BUILD_DIR)/rootfs/mnt; \
	cd $(K230_SDK_ROOT)/$(CDK_SRC_PATH)/user/; \
	cp out/little/* $(LINUX_BUILD_DIR)/rootfs/mnt/; \
	cd $(K230_SDK_ROOT)/; \
	python3 $(RT-SMART_SRC_PATH)/tools/mkromfs.py $(RT-SMART_SRC_PATH)/userapps/root $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/applications/romfs.c



.PHONY: cdk-user-clean
cdk-user-clean:
	@export PATH=$(RTT_EXEC_PATH):$(LINUX_EXEC_PATH):$(PATH);export RTSMART_SRC_DIR=$(K230_SDK_ROOT)/$(RT-SMART_SRC_PATH);export LINUX_BUILD_DIR=$(LINUX_BUILD_DIR); \
	cd $(CDK_SRC_PATH)/user/;make clean;cd -;


.PHONY: rt-smart-apps
rt-smart-apps: defconfig prepare_memory  check_src
	@export RTT_CC=$(RTT_CC); \
	export RTT_CC_PREFIX=$(RTT_CC_PREFIX); \
	export RTT_EXEC_PATH=$(RTT_EXEC_PATH); \
	cp -rf $(UNITTEST_SRC_PATH)/testcases $(RT-SMART_SRC_PATH)/userapps; \
	cd $(RT-SMART_SRC_PATH)/userapps; \
	mkdir -p $(RTSMART_SRC_DIR)/userapps/root/bin/; \
	cp configs/def_config_riscv64 .config; \
	scons -j16 || exit $?; \
	cd -;
	python3 $(RT-SMART_SRC_PATH)/tools/mkromfs.py $(RT-SMART_SRC_PATH)/userapps/root $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/applications/romfs.c

.PHONY: rt-smart-apps-clean
rt-smart-apps-clean: defconfig
	@cd $(RT-SMART_SRC_PATH)/userapps;scons -c;cd -


.PHONY: rt-smart-kernel
rt-smart-kernel: defconfig  prepare_memory  check_src
	@export RTT_CC=$(RTT_CC); \
	export RTT_CC_PREFIX=$(RTT_CC_PREFIX); \
	export RTT_EXEC_PATH=$(RTT_EXEC_PATH); \
	cd $(RT-SMART_SRC_PATH)/kernel/bsp/maix3; \
	rm -f rtthread.elf; \
	scons -j16 || exit $?; \
	mkdir -p $(RTT_SDK_BUILD_DIR); \
	cp rtthread.bin rtthread.elf $(RTT_SDK_BUILD_DIR)/; \
	cd -;


.PHONY: rt-smart-kernel-clean
rt-smart-kernel-clean: defconfig prepare_memory
	@export RTT_CC=$(RTT_CC);export RTT_CC_PREFIX=$(RTT_CC_PREFIX);export RTT_EXEC_PATH=$(RTT_EXEC_PATH); \
	cd $(RT-SMART_SRC_PATH)/kernel/bsp/maix3;scons -c;cd -

#.PHONY: linux
#linux: defconfig
#	cd $(LINUX_SRC_PATH); \
#	make ARCH=riscv $(LINUX_KERNEL_DEFCONFIG) O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv; \
#	make -j16 O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv; \
#	make O=$(LINUX_BUILD_DIR) modules_install INSTALL_MOD_PATH=$(LINUX_BUILD_DIR)/rootfs/ CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv; \
#	cd -

.PHONY: linux-config
linux-config:
	cd $(LINUX_SRC_PATH); \
	make ARCH=riscv $(LINUX_KERNEL_DEFCONFIG) O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv || exit $?; \
	cd -

.PHONY: linux-build
linux-build:
	cd $(LINUX_SRC_PATH); \
	make -j16 O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv || exit $?; \
	make O=$(LINUX_BUILD_DIR) modules_install INSTALL_MOD_PATH=$(LINUX_BUILD_DIR)/rootfs/ CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv || exit $?; \
	cd -

.PHONY: linux-menuconfig
linux-menuconfig:
	cd $(LINUX_SRC_PATH); \
	make O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv menuconfig; \
	cd -

.PHONY: linux-savedefconfig
linux-savedefconfig:
	cd $(LINUX_SRC_PATH); \
	make O=$(LINUX_BUILD_DIR) CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) ARCH=riscv savedefconfig; \
	cd -

.PHONY: linux
linux: check_src  defconfig prepare_memory linux-config linux-build

.PHONY: linux-rebuild
linux-rebuild: linux-build


#.PHONY: linux-config
#linux-config: defconfig
#	@export PATH=$(CONFIG_TOOLCHAIN_PATH_LINUX):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
#	echo $(PATH); echo $(CONFIG_TOOLCHAIN_PATH_LINUX); \
#	cd $(LINUX_SRC_PATH);make ARCH=riscv k230_kernel_little_defconfig O=$(LINUX_BUILD_DIR);cd -

#PHONY: linux-build
#linux-build: linux-config
#	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
#	cd $(LINUX_BUILD_DIR);make;make modules_install INSTALL_MOD_PATH=$(LINUX_BUILD_DIR)/rootfs/;cd -
#	cd $(LINUX_SRC_PATH);make O=$(LINUX_BUILD_DIR);make O=$(LINUX_BUILD_DIR) modules_install INSTALL_MOD_PATH=$(LINUX_BUILD_DIR)/rootfs/;cd -

.PHONY: linux-clean
linux-clean: defconfig
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(LINUX_SRC_PATH);make O=$(LINUX_BUILD_DIR) clean;cd -


.PHONY: big-core-opensbi
big-core-opensbi: rt-smart-kernel
	@mkdir -p $(BIG_OPENSBI_BUILD_DIR); \
	cp $(RT-SMART_SRC_PATH)/kernel/bsp/maix3/rtthread.bin $(OPENSBI_SRC_PATH)/; \
	cd $(OPENSBI_SRC_PATH); \
	export CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX); \
	export PLATFORM=kendryte/fpgac908; \
	make FW_FDT_PATH=hw.dtb FW_PAYLOAD_PATH=rtthread.bin O=$(BIG_OPENSBI_BUILD_DIR) OPENSBI_QUIET=1 || exit $?; \
	cd -

.PHONY: big-core-opensbi-clean
big-core-opensbi-clean:
	rm -rf $(BIG_OPENSBI_BUILD_DIR)

.PHONY:rt-smart
rt-smart: mpp rt-smart-apps big-core-opensbi

.PHONY:rt-smart-clean
rt-smart-clean: mpp-clean big-core-opensbi-clean rt-smart-kernel-clean rt-smart-apps-clean

.PHONY: little-core-opensbi
little-core-opensbi: linux
	@mkdir -p $(LITTLE_OPENSBI_BUILD_DIR); \
	cd $(OPENSBI_SRC_PATH); \
	make CROSS_COMPILE=$(LINUX_EXEC_PATH)/$(LINUX_CC_PREFIX) PLATFORM=generic FW_PAYLOAD_PATH=$(LINUX_BUILD_DIR)/arch/riscv/boot/Image  O=$(LITTLE_OPENSBI_BUILD_DIR) K230_LITTLE_CORE=1 OPENSBI_QUIET=1 || exit $?; \
	cd -


.PHONY: little-core-opensbi-clean
little-core-opensbi-clean:
	rm -rf $(LITTLE_OPENSBI_BUILD_DIR)

.PHONY: buildroot
buildroot: defconfig prepare_memory  check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH); \
	cd $(BUILDROOT-EXT_SRC_PATH); \
	make CONF=$(BUILDROOT_DEFCONFIG) BRW_BUILD_DIR=$(BUILDROOT_BUILD_DIR) BR2_TOOLCHAIN_EXTERNAL_PATH=$(LINUX_EXEC_PATH)/../ BR2_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX=$(LINUX_CC_PREFIX) || exit $?; \
	cd -

.PHONY: buildroot-rebuild
buildroot-rebuild: defconfig  prepare_memory check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH); \
	cd $(BUILDROOT-EXT_SRC_PATH); \
	make CONF=$(BUILDROOT_DEFCONFIG) BRW_BUILD_DIR=$(BUILDROOT_BUILD_DIR) BR2_TOOLCHAIN_EXTERNAL_PATH=$(LINUX_EXEC_PATH)/../ BR2_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX=$(LINUX_CC_PREFIX) build || exit $?; \
	cd -

.PHONY: buildroot-menuconfig
buildroot-menuconfig: defconfig  prepare_memory
	@export PATH=$(LINUX_EXEC_PATH):$(PATH); \
	cd $(BUILDROOT-EXT_SRC_PATH); \
	make CONF=$(BUILDROOT_DEFCONFIG) BRW_BUILD_DIR=$(BUILDROOT_BUILD_DIR) BR2_TOOLCHAIN_EXTERNAL_PATH=$(LINUX_EXEC_PATH)/../ BR2_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX=$(LINUX_CC_PREFIX) menuconfig; \
	cd -

.PHONY: buildroot-savedefconfig
buildroot-savedefconfig: defconfig prepare_memory check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH); \
	cd $(BUILDROOT-EXT_SRC_PATH); \
	make CONF=$(BUILDROOT_DEFCONFIG) BRW_BUILD_DIR=$(BUILDROOT_BUILD_DIR) BR2_TOOLCHAIN_EXTERNAL_PATH=$(LINUX_EXEC_PATH)/../ BR2_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX=$(LINUX_CC_PREFIX) savedefconfig; \
	cd -

.PHONY: buildroot-clean
buildroot-clean: defconfig
	@export PATH=$(LINUX_EXEC_PATH):$(PATH); \
	cd $(BUILDROOT-EXT_SRC_PATH); \
	make CONF=$(BUILDROOT_DEFCONFIG) BRW_BUILD_DIR=$(BUILDROOT_BUILD_DIR) BR2_TOOLCHAIN_EXTERNAL_PATH=$(LINUX_EXEC_PATH)/../ BR2_TOOLCHAIN_EXTERNAL_CUSTOM_PREFIX=$(LINUX_CC_PREFIX) clean; \
	cd -


#.PHONY: uboot
#uboot: defconfig
#	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
#	cd $(UBOOT_SRC_PATH); \
#	make $(UBOOT_DEFCONFIG) O=$(UBOOT_BUILD_DIR);make -C $(UBOOT_BUILD_DIR); \
#	cd -

.PHONY: uboot
uboot: defconfig prepare_memory check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(UBOOT_SRC_PATH); \
	make $(UBOOT_DEFCONFIG) O=$(UBOOT_BUILD_DIR) || exit $?;make -C $(UBOOT_BUILD_DIR) || exit $?; \
	cd -

.PHONY: uboot-rebuild
uboot-rebuild: defconfig  prepare_memory  check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(UBOOT_SRC_PATH); \
	make -C $(UBOOT_BUILD_DIR) || exit $?; \
	cd -

.PHONY: uboot-menuconfig
uboot-menuconfig: defconfig  prepare_memory check_src
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(UBOOT_SRC_PATH); \
	make -C $(UBOOT_BUILD_DIR) menuconfig; \
	cd -

.PHONY: uboot-savedefconfig
uboot-savedefconfig: defconfig  prepare_memory
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(UBOOT_SRC_PATH); \
	make -C $(UBOOT_BUILD_DIR) savedefconfig; \
	cd -

.PHONY: uboot-clean
uboot-clean: defconfig
	@export PATH=$(LINUX_EXEC_PATH):$(PATH);export CROSS_COMPILE=$(LINUX_CC_PREFIX);export ARCH=riscv; \
	cd $(UBOOT_SRC_PATH); \
	make -C $(UBOOT_BUILD_DIR) clean; \
	cd -


.PHONY: build-image
build-image: defconfig  prepare_memory  check_src
	mkdir -p $(BUILD_DIR)/images/big-core
	cp -rf $(RT-SMART_SRC_PATH)/userapps/root $(BUILD_DIR)/images/big-core/
	cp -f $(RTT_SDK_BUILD_DIR)/rtthread.* $(BUILD_DIR)/images/big-core/
	cp -rf $(BIG_OPENSBI_BUILD_DIR)/ $(BUILD_DIR)/images/big-core/
	mkdir -p $(BUILD_DIR)/images/little-core
	cp -rf $(BUILDROOT_BUILD_DIR)/images/* $(BUILD_DIR)/images/little-core/
	cp -rf $(LINUX_BUILD_DIR)/arch/riscv/boot/Image $(BUILD_DIR)/images/little-core/
	cp -rf $(LITTLE_OPENSBI_BUILD_DIR)/ $(BUILD_DIR)/images/little-core/
	cp -f $(LITTLE_OPENSBI_BUILD_DIR)/platform/generic/firmware/fw_*.bin $(BUILD_DIR)/images/little-core/
	cp -f $(LITTLE_OPENSBI_BUILD_DIR)/platform/generic/firmware/fw_*.elf $(BUILD_DIR)/images/little-core/
	mkdir -p $(BUILD_DIR)/images/little-core/ko-apps
	cp -rf $(LINUX_BUILD_DIR)/rootfs/* $(BUILD_DIR)/images/little-core/ko-apps/
	rm $(BUILD_DIR)/images/little-core/rootfs -rf
	mkdir -p $(BUILD_DIR)/images/little-core/rootfs
	cd $(BUILD_DIR)/images/little-core/rootfs; \
	fakeroot -- cpio -idm < ../rootfs.cpio ; \
	cd $(K230_SDK_ROOT)
	cp -rf $(K230_SDK_ROOT)/tools/ota/ota_public.pem  	$(BUILD_DIR)/images/little-core/rootfs/etc/
	cp -rf $(K230_SDK_ROOT)/tools/post_copy_rootfs/*  	$(BUILD_DIR)/images/little-core/rootfs/
	fakeroot -- cp -rf $(BUILD_DIR)/images/little-core/ko-apps/* $(BUILD_DIR)/images/little-core/rootfs/
	set -e ;cd $(BUILD_DIR)/images/little-core/rootfs/; \
	fakeroot -- $(K230_SDK_ROOT)/tools/mkcpio-rootfs.sh; \
	cd ../; \
	tar -zcf rootfs-final.tar.gz rootfs; \
	$(K230_SDK_ROOT)/tools/$(GEN_IMAGE_SCRIPT); \
	cd $(K230_SDK_ROOT)




.PHONY: clean
clean:
	@rm -rf defconfig
	@rm -rf prepare_memory
	@$(call CLEAN)



help:
	@echo "Usage: "
	@echo "make CONF=k230_evb_defconfig --$$(ls configs | tr '\n' '/')"
	@echo "make"
	@echo "Supported compilation options"
	@echo "make                          -- Build all for k230";
	@echo "make prepare_sourcecode       -- down source code";
	@echo "make little-core-opensbi      -- Build little core opensbi for k230";
	@echo "make big-core-opensbi         -- Build big core opensbi for k230";
	@echo "make mpp-apps                 -- Build mpp kernel driver user api lib and sample code for k230";
	@echo "make rt-smart                 -- Build mpp rtsmart kernel and userapps and opensbi for k230";
	@echo "make rt-smart-kernel          -- Build rtsmart kernel for k230";
	@echo "make rt-smart-apps            -- Build rtsmart userapps for k230";
	@echo "make cdk-kernel               -- Build cdk kernel code";
	@echo "make cdk-kernel-install       -- Install compiled products of cdk kernel to rt-smart and rootfs";
	@echo "make cdk-user                 -- Build cdk user code";
	@echo "make cdk-user-install         -- Install compiled products of cdk user to rt-smart and rootfs";
	@echo "make uboot                    -- Build k230 uboot code with defconfig";
	@echo "make uboot-menuconfig         -- Menuconfig for k230 uboot, select save will save to output/k230_evb_defconfig/little/uboot/.config";
	@echo "make uboot-savedefconfig      -- Save uboot configuration to output/k230_evb_defconfig/little/uboot/defconfig";
	@echo "make uboot-rebuild            -- Rebuild k230 uboot";
	@echo "make uboot-clean              -- Carry out clean in k230 uboot build directory, run make uboot-rebuild will build all source code";
	@echo "make linux                    -- Build k230 linux code with defconfig";
	@echo "make linux-rebuild            -- Rebuild k230 linux kernel";
	@echo "make linux-menuconfig         -- Menuconfig for k230 linux kernel, select save will save to output/k230_evb_defconfig/little/linux/.config";
	@echo "make linux-savedefconfig      -- Save linux kernel configuration to output/k230_evb_defconfig/little/linux/defconfig";
	@echo "make linux-clean              -- Carry out clean in linux kernel build directory, run make linux-rebuild will build all source code";
	@echo "make buildroot   	     -- Build k230 buildroot with defconfig";
	@echo "make buildroot-rebuild        -- Rebuild k230 buildroot ";
	@echo "make buildroot-menuconfig     -- Menuconfig for k230 buildroot, select save will save to output/k230_evb_defconfig/little/buildroot-ext/.config";
	@echo "make buildroot-savedefconfig  -- Save k230 buildroot configuration to src/little/buildroot-ext/configs/k230_evb_defconfig";
	@echo "make buildroot-clean          -- Clean the k230 buildroot build directory, after clean, run make buildroot-rebuild will fail because the build cirectory is not exist. Run make buildroot to build";
	@echo "make build-image              -- Build k230 rootfs image";
