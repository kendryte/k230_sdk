include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)

MIDDLEWARE_SRC= $(LOCAL_SRC_DIR)/src
MIDDLEWARE_SAMPLE= $(LOCAL_SRC_DIR)/sample

all:
	cd $(MIDDLEWARE_SRC) ; make
	cd $(MIDDLEWARE_SAMPLE) ; make

clean:
	echo "${PWD}/Makefile clean"
	@cd $(MIDDLEWARE_SRC); make clean
	@cd $(MIDDLEWARE_SAMPLE); make clean

.PHONY: all clean
