################################################################################
#
# peephole
#
################################################################################
PEEPHOLE_SITE = $(realpath $(TOPDIR))/../package/peephole/src
PEEPHOLE_SITE_METHOD = file

PEEPHOLE_DEPENDENCIES += libdisp libdrm

PEEPHOLE_CFLAGS = $(TARGET_CFLAGS) \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libdrm

PEEPHOLE_CXXFLAGS = $(PEEPHOLE_CFLAGS)

PEEPHOLE_CONF_OPTS += \
	-DCMAKE_CXX_FLAGS="$(PEEPHOLE_CXXFLAGS)" \
	-DCMAKE_C_FLAGS="$(PEEPHOLE_CFLAGS)" \
	-DNATIVE_BUILD=OFF

# PEEPHOLE_DEPENDENCIES += lvgl
PEEPHOLE_EXTRA_DOWNLOADS = https://github.com/lvgl/lvgl/archive/refs/tags/v8.3.1.tar.gz

define peephole_rsync
	rsync -au --chmod=u=rwX,go=rX  --exclude .svn --exclude .git --exclude .hg --exclude .bzr --exclude CVS ${PEEPHOLE_SITE}/ $(@D)
endef

define PEEPHOLE_EXTRACT_CMDS
	$(call peephole_rsync)
	mkdir -p $(@D)/thirdlib/lvgl
	tar -xf $(PEEPHOLE_DL_DIR)/v8.3.1.tar.gz -C $(@D)/thirdlib/lvgl --strip-components=1
endef

PEEPHOLE_PRE_BUILD_HOOKS += peephole_pre_build_hook
define peephole_pre_build_hook
	$(call peephole_rsync)
endef

$(eval $(cmake-package))
