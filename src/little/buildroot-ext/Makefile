BR2_VERSION		= 9d1d4818c39d97ad7a1cdf6e075b9acae6dfff71

RED=\e[1;31m
NC=\e[0m

SHELL=/bin/bash

BRW_ROOT	= $(PWD)
BRW_SITE	= https://ai.b-bug.org/~yangguang//buildroot-$(BR2_VERSION).tar.gz
BRW_FILE	= $(BR2_DL_DIR)/buildroot-$(BR2_VERSION).tar.gz
BRW_DIR		= $(BRW_ROOT)/buildroot-$(BR2_VERSION)
BRW_PATCH_DIR	= $(BRW_ROOT)/patches


#DEMO_DIR	= $(BRW_ROOT)/demo
#ROOTFS_OVERLAY_PATH	= $(BRW_ROOT)/board/canaan/k510/rootfs_overlay



CONF ?= k230_fpga_defconfig
O ?= $(CONF)
BR2_DL_DIR ?= $(BRW_ROOT)/dl

BRW_BUILD_DIR ?= $(CONF)

define DOWNLOAD
	set -e;\
	if [ ! -d $(BR2_DL_DIR) ]; then \
		if [ -f dl.tar.gz ]; then  \
			tar -zxf dl.tar.gz || { echo -e "\n\nfile dl.tar.gz is error,please delete .\n";exit 1; }  ; \
		fi;	\
		mkdir -p $(BR2_DL_DIR); \
	fi; \
	if [ ! -f $(BRW_FILE) ]; then \
		wget -c $(BRW_SITE) -O $(BRW_FILE); \
	fi; \


endef

define COPYFILES
	set -e; \
	if [ ! -d $(BRW_DIR) ]; then \
		tar --no-same-owner -zxf $(BRW_FILE) -C $(BRW_ROOT); \
		if [ "'ls $(BRW_PATCH_DIR)/*.patch'" != "" ]; then \
			$(BRW_DIR)/support/scripts/apply-patches.sh $(BRW_DIR) $(BRW_PATCH_DIR); \
		fi; \
	fi; \

	
endef

define CLEAN
	set -e; \
	rm -rf $(BRW_BUILD_DIR); \
	rm -rf $(BRW_DIR); \

endef





.PHONY: all
all .DEFAULT: 
	@$(call DOWNLOAD)
	@$(call COPYFILES)
	make -C $(BRW_DIR) $(CONF) O=$(BRW_BUILD_DIR) BR2_EXTERNAL=$(BRW_ROOT)
	make -C $(BRW_BUILD_DIR)

.PHONY: menuconfig
menuconfig:
	make -C $(BRW_BUILD_DIR) menuconfig

.PHONY: build
build:
	make -C $(BRW_BUILD_DIR)

.PHONY: savedefconfig
savedefconfig:
	make -C $(BRW_BUILD_DIR) savedefconfig

.PHONY: clean
clean:
	@$(call CLEAN)


dl:
	rm -rf dl.tar.gz;
	wget https://ai.b-bug.org/k230/downloads/dl/dl.tar.gz;
	tar -zxf dl.tar.gz;
	
help:
	@echo "all---default,example:make CONF=k230_fpga_defconfig"
