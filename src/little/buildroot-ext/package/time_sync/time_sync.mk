################################################################################
#
# amp-app
#
################################################################################
TIME_SYNC_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/time_sync"
TIME_SYNC_DIR_NAME := time_sync
TIME_SYNC_APP_NAME := time_sync

TIME_SYNC_SITE = $(realpath $(TOPDIR))"/../package/time_sync/src"
TIME_SYNC_SITE_METHOD = local

TIME_SYNC_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include
TIME_SYNC_LDFLAGS = 

define TIME_SYNC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D) CFLAG="$(TIME_SYNC_CFLAGS)" LDFLAG="$(TIME_SYNC_LDFLAGS)"
endef

define TIME_SYNC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(TIME_SYNC_APP_NAME) $(TARGET_DIR)/usr/bin/$(TIME_SYNC_APP_NAME)
endef

$(eval $(generic-package))
