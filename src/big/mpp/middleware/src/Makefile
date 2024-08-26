include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)

LIVE555= $(LOCAL_SRC_DIR)/live555
RTSP_CLIENT= $(LOCAL_SRC_DIR)/rtsp_client
RTSP_SERVER= $(LOCAL_SRC_DIR)/rtsp_server
RTSP_PUSHER= $(LOCAL_SRC_DIR)/rtsp_pusher
MP4_FORMAT = $(LOCAL_SRC_DIR)/mp4_format
MP4_PLAYER = $(LOCAL_SRC_DIR)/mp4_player
KDMEDIA = $(LOCAL_SRC_DIR)/kdmedia
X264    = $(LOCAL_SRC_DIR)/x264/src
FFMPEG  = $(LOCAL_SRC_DIR)/ffmpeg/src

all:
	cd $(LIVE555) ; make
	cd $(RTSP_CLIENT) ; make
	cd $(RTSP_SERVER) ; make
	cd $(MP4_FORMAT) ; make
	cd $(MP4_PLAYER) ; make
	cd $(KDMEDIA) ; make
	cd $(X264) ;  make -j8; make install
	cd $(FFMPEG) ; make -j8; make install
	cd $(RTSP_PUSHER) ; make

clean:
	echo "${PWD}/Makefile clean"
	@cd $(LIVE555); make clean
	@cd $(RTSP_CLIENT); make clean
	@cd $(RTSP_SERVER); make clean
	@cd $(MP4_FORMAT); make clean
	@cd $(MP4_PLAYER); make clean
	@cd $(KDMEDIA); make clean
	@cd $(X264) ; make uninstall;make clean
	@cd $(FFMPEG) ; make uninstall;make clean
	@cd $(RTSP_PUSHER); make clean

.PHONY: all clean
