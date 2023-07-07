################################################################################
#
# door_lock
#
################################################################################
DOOR_LOCK_SITE = $(realpath $(TOPDIR))/../package/door_lock/src
DOOR_LOCK_SITE_METHOD = file

DOOR_LOCK_DEPENDENCIES += libdisp libdrm

DOOR_LOCK_CFLAGS = $(TARGET_CFLAGS) \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/libdrm

DOOR_LOCK_CXXFLAGS = $(DOOR_LOCK_CFLAGS)

DOOR_LOCK_CONF_OPTS += \
	-DCMAKE_CXX_FLAGS="$(DOOR_LOCK_CXXFLAGS)" \
	-DCMAKE_C_FLAGS="$(DOOR_LOCK_CFLAGS)" \
	-DNATIVE_BUILD=OFF

DOOR_LOCK_EXTRA_DOWNLOADS = https://github.com/lvgl/lvgl/archive/refs/tags/v8.3.1.tar.gz

define door_lock_rsync
	rsync -au --chmod=u=rwX,go=rX  --exclude .svn --exclude .git --exclude .hg --exclude .bzr --exclude CVS ${DOOR_LOCK_SITE}/ $(@D)
endef

define DOOR_LOCK_EXTRACT_CMDS
	$(call door_lock_rsync)
	mkdir -p $(@D)/thirdlib/lvgl
	tar -xf $(DOOR_LOCK_DL_DIR)/v8.3.1.tar.gz -C $(@D)/thirdlib/lvgl --strip-components=1
endef

DOOR_LOCK_PRE_BUILD_HOOKS += door_lock_pre_build_hook
define door_lock_pre_build_hook
	$(call door_lock_rsync)
endef

$(eval $(cmake-package))
