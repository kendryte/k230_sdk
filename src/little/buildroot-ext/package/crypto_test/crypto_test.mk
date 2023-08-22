################################################################################
#
# crypto-test
#
################################################################################
CRYPTO_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/crypto_test"
CRYPTO_TEST_DIR_NAME := crypto_test
CRYPTO_TEST_APP_NAME := crypto_test_demo

CRYPTO_TEST_SITE = $(realpath $(TOPDIR))"/../package/crypto_test/src"
CRYPTO_TEST_SITE_METHOD = local


define CRYPTO_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define CRYPTO_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(CRYPTO_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(CRYPTO_TEST_APP_NAME)
endef

$(eval $(generic-package))
