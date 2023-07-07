USB_TEST_LOCAL_PATH:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
USB_TEST_DIR_NAME := usb_test
USB_TEST_APP_NAME := usb_test

USB_TEST_SITE = $(USB_TEST_LOCAL_PATH)/src
USB_TEST_SITE_METHOD = local
USB_TEST_INSTALL_STAGING = YES

define USB_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D) 
endef

define USB_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/usb_test $(TARGET_DIR)/usr/bin/usb_test
	cp -rf $(@D)/gadget-usbtest.sh $(TARGET_DIR)/usr/bin/gadget-usbtest.sh
	cp -rf $(@D)/gadget-storage-mem.sh $(TARGET_DIR)/usr/bin/gadget-storage-mem.sh
	cp -rf $(@D)/gadget-storage.sh $(TARGET_DIR)/usr/bin/gadget-storage.sh
	$(INSTALL) -D -m 0755 $(@D)/test_keyboard $(TARGET_DIR)/usr/bin/test_keyboard
	$(INSTALL) -D -m 0755 $(@D)/test_mouse $(TARGET_DIR)/usr/bin/test_mouse
	cp -rf $(@D)/gadget-hid.sh $(TARGET_DIR)/usr/bin/gadget-hid.sh
	$(INSTALL) -D -m 0755 $(@D)/hid_gadget_test $(TARGET_DIR)/usr/bin/hid_gadget_test
endef

$(eval $(generic-package))


