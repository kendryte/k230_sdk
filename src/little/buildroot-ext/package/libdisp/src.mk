################################################################################
#
# libdisp
#
################################################################################
LIBDISP_SITE = $(realpath $(TOPDIR))/../package/libdisp/src
LIBDISP_SITE_METHOD = local
LIBDISP_INSTALL_STAGING = YES

LIBDISP_DEPENDENCIES = libdrm
LIBDISP_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/libdrm
LIBDISP_LDFLAGS = -L$(STAGING_DIR)/usr/lib -ldrm

define LIBDISP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CFLAG="$(LIBDISP_CFLAGS)" LDFLAG="$(LIBDISP_LDFLAGS)" all
endef

define LIBDISP_INSTALL_STAGING_CMDS
	cp -rf $(@D)/libdisp.so  $(STAGING_DIR)/usr/lib/libdisp.so
	cp -rf $(@D)/disp.h $(STAGING_DIR)/usr/include/disp.h
endef

define LIBDISP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libdisp.so $(TARGET_DIR)/usr/lib/libdisp.so
endef

$(eval $(generic-package))