################################################################################
#
# touch_emu
#
################################################################################
TOUCH_EMU_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/touch_emu"
TOUCH_EMU_DIR_NAME := touch_emu
TOUCH_EMU_APP_NAME := touch_emu

TOUCH_EMU_SITE = $(realpath $(TOPDIR))"/../package/touch_emu/src"
TOUCH_EMU_SITE_METHOD = local


define TOUCH_EMU_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define TOUCH_EMU_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(TOUCH_EMU_APP_NAME) $(TARGET_DIR)/usr/bin/$(TOUCH_EMU_APP_NAME)
endef

$(eval $(generic-package))
