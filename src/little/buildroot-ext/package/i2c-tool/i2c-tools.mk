I2C_TOOL_INSTALL_IMAGES := YES

define I2C_TOOL_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0755 $(TOPDIR)/../package/i2c-tool/i2c-tools.sh $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))



