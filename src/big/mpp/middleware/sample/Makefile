include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)

SAMPLE_RTSP_SERVER = $(LOCAL_SRC_DIR)/sample_rtspserver
SAMPLE_RTSP_CLIENT = $(LOCAL_SRC_DIR)/sample_rtspclient
SAMPLE_DEMUXER= $(LOCAL_SRC_DIR)/sample_demuxer
SAMPLE_MUXER= $(LOCAL_SRC_DIR)/sample_muxer
SAMPLE_PLAYER=$(LOCAL_SRC_DIR)/sample_player
SAMPLE_RTSP_PUSHER=$(LOCAL_SRC_DIR)/sample_rtsppusher

all:
	cd $(SAMPLE_RTSP_SERVER) ; make
	cd $(SAMPLE_RTSP_CLIENT) ; make
	cd $(SAMPLE_DEMUXER) ; make
	cd $(SAMPLE_MUXER) ; make
	cd $(SAMPLE_PLAYER) ; make
	cd $(SAMPLE_RTSP_PUSHER) ; make

clean:
	echo "${PWD}/Makefile clean"
	@cd $(SAMPLE_RTSP_SERVER); make clean
	@cd $(SAMPLE_RTSP_CLIENT); make clean
	@cd $(SAMPLE_DEMUXER); make clean
	@cd $(SAMPLE_MUXER); make clean
	@cd $(SAMPLE_PLAYER); make clean
	@cd $(SAMPLE_RTSP_PUSHER); make clean

.PHONY: all clean
