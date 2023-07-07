################################################################################
#
# ts-test
#
################################################################################
TS_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/ts_test"
TS_TEST_DIR_NAME := ts_test
TS_TEST_APP_NAME := ts_test_demo

TS_TEST_SITE = $(realpath $(TOPDIR))"/../package/ts_test/src"
TS_TEST_SITE_METHOD = local


define TS_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define TS_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(TS_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(TS_TEST_APP_NAME)
endef

$(eval $(generic-package))
