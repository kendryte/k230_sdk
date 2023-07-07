################################################################################
#
# trng-test
#
################################################################################
TRNG_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/trng_test"
TRNG_TEST_DIR_NAME := trng_test
TRNG_TEST_APP_NAME := trng_test_demo

TRNG_TEST_SITE = $(realpath $(TOPDIR))"/../package/trng_test/src"
TRNG_TEST_SITE_METHOD = local


define TRNG_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define TRNG_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(TRNG_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(TRNG_TEST_APP_NAME)
endef

$(eval $(generic-package))
