################################################################################
#
# bluez_inc
#
################################################################################
BLUEZ_INC_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/bluez_inc"
BLUEZ_INC_DIR_NAME := bluez_inc
BLUEZ_INC_APP_NAME := bluez_inc

BLUEZ_INC_SITE = $(realpath $(TOPDIR))"/../package/bluez_inc/src"
BLUEZ_INC_SITE_METHOD = local


define BLUEZ_INC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/examples/peripheral/peripheral $(TARGET_DIR)/usr/bin/peripheral
	$(INSTALL) -D -m 0755 $(@D)/binc/libBinc.so $(TARGET_DIR)/usr/lib/libBinc.so
endef

$(eval $(cmake-package))
