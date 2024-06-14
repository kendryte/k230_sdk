################################################################################
#
# ffmpeg
#
################################################################################

FFMPEG_CANAAN_VERSION = 6.1
FFMPEG_CANAAN_SOURCE = ffmpeg-$(FFMPEG_CANAAN_VERSION).tar.xz
FFMPEG_CANAAN_SITE = http://ffmpeg.org/releases
FFMPEG_CANAAN_INSTALL_STAGING = YES

FFMPEG_CANAAN_LICENSE = LGPL-2.1+, libjpeg license
FFMPEG_CANAAN_LICENSE_FILES = LICENSE.md COPYING.LGPLv2.1
ifeq ($(BR2_PACKAGE_FFMPEG_CANAAN_GPL),y)
FFMPEG_CANAAN_LICENSE += and GPL-2.0+
FFMPEG_CANAAN_LICENSE_FILES += COPYING.GPLv2
endif

FFMPEG_CANAAN_DEPENDENCIES += host-pkgconf

FFMPEG_CANAAN_CONF_ENV += CFLAGS="$(FFMPEG_CANAAN_CFLAGS)"
FFMPEG_CANAAN_CONF_OPTS += $(call qstrip,$(BR2_PACKAGE_FFMPEG_CANAAN_EXTRACONF))

# Override FFMPEG_CANAAN_CONFIGURE_CMDS: FFmpeg does not support --target and others
define FFMPEG_CANAAN_CONFIGURE_CMDS
	(cd $(FFMPEG_CANAAN_SRCDIR) && rm -rf config.cache && \
	$(TARGET_CONFIGURE_OPTS) \
	$(TARGET_CONFIGURE_ARGS) \
	$(FFMPEG_CANAAN_CONF_ENV) \
	./configure \
		--cross-prefix=riscv64-unknown-linux-gnu- \
		--enable-cross-compile \
		--target-os=linux \
		--cc="$(TARGET_CC)" \
		--arch=riscv64 \
		--libdir="/lib64/" \
		--disable-asm \
		--disable-x86asm \
		--disable-static \
		--enable-shared \
		--disable-autodetect \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-doc \
		--disable-debug \
		--disable-symver \
		--disable-htmlpages \
		--disable-manpages \
		--disable-podpages \
        --disable-txtpages \
		--disable-postproc \
		--disable-encoders \
		--disable-decoders \
		--enable-v4l2_m2m \
		--enable-indev=v4l2 \
		--enable-encoder=h264_v4l2m2m \
		--enable-encoder=hevc_v4l2m2m \
		--enable-encoder=mjpeg_v4l2m2m \
		--enable-encoder=mpeg2video \
		--enable-encoder=aac \
		--enable-encoder=pcm_alaw \
		--enable-encoder=rawvideo \
		--enable-decoder=h264_v4l2m2m \
		--enable-decoder=hevc_v4l2m2m \
		--enable-decoder=mjpeg_v4l2m2m \
		--enable-decoder=mpeg2video \
		--enable-decoder=aac \
		--enable-decoder=pcm_alaw \
		--enable-decoder=rawvideo \
	)
endef

FFMPEG_CANAAN_POST_INSTALL_TARGET_HOOKS += FFMPEG_CANAAN_REMOVE_EXAMPLE_SRC_FILES

$(eval $(autotools-package))
