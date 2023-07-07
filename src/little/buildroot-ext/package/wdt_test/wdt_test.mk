################################################################################
#
# gpio-keys
#
################################################################################
WDT_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/wdt_test"
WDT_TEST_DIR_NAME := wdt_test
WDT_TEST_APP_NAME := wdt_test_demo

WDT_TEST_SITE = $(realpath $(TOPDIR))"/../package/wdt_test/src"
WDT_TEST_SITE_METHOD = local


define WDT_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define WDT_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(WDT_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(WDT_TEST_APP_NAME)
endef

$(eval $(generic-package))
