################################################################################
#
# lvgl
#
################################################################################
LVGL_VERSION = v8.3.7
LVGL_SOURCE = $(LVGL_VERSION).tar.gz
LVGL_SITE = https://github.com/lvgl/lvgl/archive/refs/tags

LVGL_DEPENDENCIES += libdrm
LVGL_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/libdrm -I$(STAGING_DIR)/usr/include
LVGL_LDFLAGS = -L$(STAGING_DIR)/usr/lib -ldrm

define LVGL_EXTRACT_CMDS
	tar zxf $(LVGL_DL_DIR)/$(LVGL_SOURCE) -C $(@D)
	mv $(@D)/lvgl-* $(@D)/lvgl
	cp -r $(TOPDIR)/../package/lvgl/port_src/* $(@D)/
endef

define LVGL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CFLAG="$(LVGL_CFLAGS)" LDFLAG="$(LVGL_LDFLAGS)" all
endef

define LVGL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/lvgl_demo_widgets $(TARGET_DIR)/usr/bin/lvgl_demo_widgets
endef

$(eval $(generic-package))