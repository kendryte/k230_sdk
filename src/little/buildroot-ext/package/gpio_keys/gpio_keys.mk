################################################################################
#
# gpio-keys
#
################################################################################
GPIO_KEYS_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/gpio_keys"
GPIO_KEYS_DIR_NAME := gpio_keys
GPIO_KEYS_APP_NAME := gpio_keys_demo

GPIO_KEYS_SITE = $(realpath $(TOPDIR))"/../package/gpio_keys/src"
GPIO_KEYS_SITE_METHOD = local


define GPIO_KEYS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define GPIO_KEYS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(GPIO_KEYS_APP_NAME) $(TARGET_DIR)/usr/bin/$(GPIO_KEYS_APP_NAME)
endef

$(eval $(generic-package))
