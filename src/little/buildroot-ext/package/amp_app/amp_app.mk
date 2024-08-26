################################################################################
#
# amp-app
#
################################################################################
AMP_APP_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/amp_app"
AMP_APP_DIR_NAME := amp_app
AMP_APP_APP_NAME := amp_test

AMP_APP_SITE = $(realpath $(TOPDIR))"/../package/amp_app/src"
AMP_APP_SITE_METHOD = local

AMP_APP_CFLAGS = $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include
AMP_APP_LDFLAGS = -static -L$(STAGING_DIR)/usr/lib -lz

define AMP_APP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D) CFLAG="$(AMP_APP_CFLAGS)" LDFLAG="$(AMP_APP_LDFLAGS)"
endef

define AMP_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(AMP_APP_APP_NAME) $(TARGET_DIR)/usr/bin/$(AMP_APP_APP_NAME)
endef

$(eval $(generic-package))
