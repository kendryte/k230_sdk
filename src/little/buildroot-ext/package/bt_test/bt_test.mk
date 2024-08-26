################################################################################
#
# bt-test
#
################################################################################
BT_TEST_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/bt_test"
BT_TEST_DIR_NAME := bt_test
BT_TEST_APP_NAME := btspp

BT_TEST_SITE = $(realpath $(TOPDIR))"/../package/bt_test/src"
BT_TEST_SITE_METHOD = local

BT_TEST_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/glib-2.0 -I$(STAGING_DIR)/usr/lib/glib-2.0/include
BT_TEST_CFLAGS += -I$(STAGING_DIR)/usr/include/gio-unix-2.0

define BT_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D) CFLAG="$(BT_TEST_CFLAGS)"
endef

define BT_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(BT_TEST_APP_NAME) $(TARGET_DIR)/usr/bin/$(BT_TEST_APP_NAME)
endef

$(eval $(generic-package))
