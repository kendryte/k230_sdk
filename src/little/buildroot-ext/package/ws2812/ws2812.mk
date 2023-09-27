################################################################################
#
# gpio-keys
#
################################################################################
WS2812_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/ws2812"
WS2812_DIR_NAME := ws2812
WS2812_APP_NAME := ws2812_demo

WS2812_SITE = $(realpath $(TOPDIR))"/../package/ws2812/src"
WS2812_SITE_METHOD = local


define WS2812_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define WS2812_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(WS2812_APP_NAME) $(TARGET_DIR)/usr/bin/$(WS2812_APP_NAME)
endef

$(eval $(generic-package))
