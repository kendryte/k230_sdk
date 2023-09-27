################################################################################
#
# peephole_device
#
################################################################################
PEEPHOLE_DEVICE_SITE = $(realpath $(TOPDIR))/../package/peephole_device/src
PEEPHOLE_DEVICE_SITE_METHOD = file

PEEPHOLE_DEVICE_DEPENDENCIES += libdisp libdrm

PEEPHOLE_DEVICE_CFLAGS = $(TARGET_CFLAGS) \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libdrm

PEEPHOLE_DEVICE_CXXFLAGS = $(PEEPHOLE_DEVICE_CFLAGS)

PEEPHOLE_DEVICE_CONF_OPTS += \
	-DCMAKE_CXX_FLAGS="$(PEEPHOLE_DEVICE_CXXFLAGS)" \
	-DCMAKE_C_FLAGS="$(PEEPHOLE_DEVICE_CFLAGS)" \
	-DNATIVE_BUILD=OFF

# PEEPHOLE_DEVICE_DEPENDENCIES += lvgl
PEEPHOLE_DEVICE_EXTRA_DOWNLOADS = https://github.com/lvgl/lvgl/archive/refs/tags/v8.3.1.tar.gz

define peephole_device_rsync
	rsync -au --chmod=u=rwX,go=rX  --exclude .svn --exclude .git --exclude .hg --exclude .bzr --exclude CVS ${PEEPHOLE_DEVICE_SITE}/ $(@D)
endef

define PEEPHOLE_DEVICE_EXTRACT_CMDS
	$(call peephole_device_rsync)
	mkdir -p $(@D)/thirdlib/lvgl
	tar -xf $(PEEPHOLE_DEVICE_DL_DIR)/v8.3.1.tar.gz -C $(@D)/thirdlib/lvgl --strip-components=1
endef

PEEPHOLE_DEVICE_PRE_BUILD_HOOKS += peephole_device_pre_build_hook
define peephole_device_pre_build_hook
	$(call peephole_device_rsync)
endef

$(eval $(cmake-package))
