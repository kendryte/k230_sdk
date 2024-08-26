include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)

LIVE555_USAGEENVIRONMENT= $(LOCAL_SRC_DIR)/UsageEnvironment
LIVE555_BASICUSAGEENVIRONMENT= $(LOCAL_SRC_DIR)/BasicUsageEnvironment
LIVE555_GROUPSOCK = $(LOCAL_SRC_DIR)/groupsock
LIVE555_LIVEMEDIA = $(LOCAL_SRC_DIR)/liveMedia
LIVE555_MEDIASERVER = $(LOCAL_SRC_DIR)/mediaServer

all:
	cd $(LIVE555_LIVEMEDIA) ; make -j4
	cd $(LIVE555_GROUPSOCK) ; make -j4
	cd $(LIVE555_USAGEENVIRONMENT) ; make -j4
	cd $(LIVE555_BASICUSAGEENVIRONMENT) ; make -j4
	cd $(LIVE555_MEDIASERVER) ; make -j4


clean:
	echo "${PWD}/Makefile clean"
	@cd $(LIVE555_LIVEMEDIA); make clean
	@cd $(LIVE555_GROUPSOCK); make clean
	@cd $(LIVE555_USAGEENVIRONMENT); make clean
	@cd $(LIVE555_BASICUSAGEENVIRONMENT); make clean
	@cd $(LIVE555_MEDIASERVER); make clean


.PHONY: all clean
