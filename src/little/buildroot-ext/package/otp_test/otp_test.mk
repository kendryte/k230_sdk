################################################################################
#
# otp-test
#
################################################################################
OTP_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/otp_test"
OTP_TEST_DIR_NAME := otp_test
OTP_TEST_APP_NAME := otp_test_demo

OTP_TEST_SITE = $(realpath $(TOPDIR))"/../package/otp_test/src"
OTP_TEST_SITE_METHOD = local


define OTP_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define OTP_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(OTP_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(OTP_TEST_APP_NAME)
endef

$(eval $(generic-package))
