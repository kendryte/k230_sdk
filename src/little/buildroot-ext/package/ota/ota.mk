OTA_LOCAL_PATH:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
OTA_DIR_NAME := ota
OTA_APP_NAME := ota

OTA_SITE = $(OTA_LOCAL_PATH)/src
OTA_SITE_METHOD = local
OTA_INSTALL_STAGING = YES

define OTA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D) 
endef

define OTA_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(OTA_APP_NAME) $(TARGET_DIR)/usr/bin/$(OTA_APP_NAME)
	cp -rf $(@D)/ota.conf $(TARGET_DIR)/etc/ota.conf
endef

$(eval $(generic-package))


