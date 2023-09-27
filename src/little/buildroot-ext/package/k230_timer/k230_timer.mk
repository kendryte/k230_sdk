################################################################################
#
# ts-test
#
################################################################################
K230_TIMER_LOCAL_PATH:= $(realpath $(TOPDIR))"/../package/k230_timer"
K230_TIMER_DIR_NAME := k230_timer
K230_TIMER_APP_NAME := k230_timer_demo

K230_TIMER_SITE = $(realpath $(TOPDIR))"/../package/k230_timer/src"
K230_TIMER_SITE_METHOD = local


define K230_TIMER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define K230_TIMER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(K230_TIMER_APP_NAME) $(TARGET_DIR)/usr/bin/$(K230_TIMER_APP_NAME)
endef

$(eval $(generic-package))
