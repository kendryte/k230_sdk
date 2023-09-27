################################################################################
#
# peephole_phone
#
################################################################################
PEEPHOLE_PHONE_SITE = $(realpath $(TOPDIR))/../package/peephole_phone/src
PEEPHOLE_PHONE_SITE_METHOD = file

PEEPHOLE_PHONE_DEPENDENCIES += libdisp libdrm

PEEPHOLE_PHONE_CFLAGS = $(TARGET_CFLAGS) \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libdrm

PEEPHOLE_PHONE_CXXFLAGS = $(PEEPHOLE_PHONE_CFLAGS)

PEEPHOLE_PHONE_CONF_OPTS += \
	-DCMAKE_CXX_FLAGS="$(PEEPHOLE_PHONE_CXXFLAGS)" \
	-DCMAKE_C_FLAGS="$(PEEPHOLE_PHONE_CFLAGS)" \
	-DNATIVE_BUILD=OFF

# PEEPHOLE_PHONE_DEPENDENCIES += lvgl
PEEPHOLE_PHONE_EXTRA_DOWNLOADS = https://github.com/lvgl/lvgl/archive/refs/tags/v8.3.1.tar.gz

define peephole_phone_rsync
	rsync -au --chmod=u=rwX,go=rX  --exclude .svn --exclude .git --exclude .hg --exclude .bzr --exclude CVS ${PEEPHOLE_PHONE_SITE}/ $(@D)
endef

define PEEPHOLE_PHONE_EXTRACT_CMDS
	$(call peephole_phone_rsync)
	mkdir -p $(@D)/thirdlib/lvgl
	tar -xf $(PEEPHOLE_PHONE_DL_DIR)/v8.3.1.tar.gz -C $(@D)/thirdlib/lvgl --strip-components=1
endef

PEEPHOLE_PHONE_PRE_BUILD_HOOKS += peephole_phone_pre_build_hook
define peephole_phone_pre_build_hook
	$(call peephole_phone_rsync)
endef

$(eval $(cmake-package))
