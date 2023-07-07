################################################################################
#
# aiw4211lv10
#
################################################################################
AIW4211LV10_LOCAL_PATH:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
AIW4211LV10_DIR_NAME := aiw4211lv10
#AIW4211LV10_APP_NAME := 

AIW4211LV10_SITE = $(AIW4211LV10_LOCAL_PATH)/src
AIW4211LV10_SITE_METHOD = local


define AIW4211LV10_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define AIW4211LV10_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/app/link/iotwifi_link $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/app/client/iotwifi_cli $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/app/client/libhal_iotwifi.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0755 $(@D)/app/client/wifi.conf $(TARGET_DIR)/etc/
endef

$(eval $(generic-package))
