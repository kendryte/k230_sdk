##############################################################################
#
# hw-c910
#
##############################################################################

HW_C910_INSTALL_IMAGES = YES

define HW_C910_INSTALL_IMAGES_CMDS
	mkdir -p $(BINARIES_DIR)/hw/
	cp $(TOPDIR)/../package/hw-c910/hw/* $(BINARIES_DIR)/hw/ -raf
	cp $(BINARIES_DIR)/../host/bin/dtc $(BINARIES_DIR)/hw/ -raf
endef

$(eval $(generic-package))
