################################################################################
#
# VG Lite
#
################################################################################

VG_LITE_DIR_NAME := vg_lite
VG_LITE_APP_NAME := vg_lite
VG_LITE_SITE = $(realpath $(TOPDIR))"/../package/vg_lite"
VG_LITE_SITE_METHOD = local
VG_LITE_DEPENDENCIES += libdrm

DRM_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/libdrm -I$(STAGING_DIR)/usr/include
DRM_LDFLAGS = -L$(STAGING_DIR)/usr/lib -ldrm

ifeq ($(BR2_PACKAGE_VG_LITE_DEMOS),y)
define VG_LITE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)/VGLite
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" DRM_CFLAGS="$(DRM_CFLAGS)" DRM_LDFLAGS="$(DRM_LDFLAGS)" SDK_DIR=$(@D) -C $(@D)/test
endef

define VG_LITE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)/VGLite install
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" SDK_DIR=$(@D) -C $(@D)/test install
endef

else
define VG_LITE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)/VGLite
endef

define VG_LITE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)/VGLite install
endef
endif

$(eval $(generic-package))
