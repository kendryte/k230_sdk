################################################################################
#
# mvx_player
#
################################################################################
MVX_PLAYER_LOCAL_PATH:= $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
MVX_PLAYER_DIR_NAME := mvx_player
MVX_PLAYER_APP_NAME := mvx

MVX_PLAYER_SITE = $(MVX_PLAYER_LOCAL_PATH)/src
MVX_PLAYER_SITE_METHOD = local

MVX_PLAYER_INSTALL_IMAGES := YES


define MVX_PLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) 
endef

define MVX_PLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/mvx_encoder $(TARGET_DIR)/app/mvx_player/mvx_encoder
	$(INSTALL) -D -m 0755 $(@D)/mvx_encoder_multi $(TARGET_DIR)/app/mvx_player/mvx_encoder_multi
	$(INSTALL) -D -m 0755 $(@D)/mvx_decoder $(TARGET_DIR)/app/mvx_player/mvx_decoder
	$(INSTALL) -D -m 0755 $(@D)/mvx_decoder_multi $(TARGET_DIR)/app/mvx_player/mvx_decoder_multi
endef

$(eval $(generic-package))
