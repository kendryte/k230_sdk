################################################################################
#
# hello_world
#
################################################################################
HELLO_WORLD_LOCAL_PATH:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
HELLO_WORLD_DIR_NAME := hello_world
HELLO_WORLD_APP_NAME := hello

HELLO_WORLD_SITE = $(HELLO_WORLD_LOCAL_PATH)/src
HELLO_WORLD_SITE_METHOD = local


define HELLO_WORLD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) 
endef

define HELLO_WORLD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(HELLO_WORLD_DIR_NAME).out $(TARGET_DIR)/app/$(HELLO_WORLD_DIR_NAME)/$(HELLO_WORLD_APP_NAME)
endef

$(eval $(generic-package))
