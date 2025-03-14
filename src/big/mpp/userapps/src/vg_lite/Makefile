include $(MPP_SRC_DIR)/userapps/src/mpp.mk
include $(MPP_SRC_DIR)/userapps/src/rt-smart.mk

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
LIB = $(MPP_SRC_DIR)/userapps/lib/lib$(CURRECT_DIR_NAME).a

LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR)/

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c)

OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(LIB)
	$(MAKE) -C vg_lite_util

$(OBJS): %.o : %.c
	@echo "CC $@"
	@$(CC) $(CC_CFLAGS) -Wno-error $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "AR $@"
	@$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	$(MAKE) -C vg_lite_util clean
	echo "${PWD}/Makefile clean"
	-rm -rf $(LIB)
	-rm -f $(OBJS)

.PHONY: all clean