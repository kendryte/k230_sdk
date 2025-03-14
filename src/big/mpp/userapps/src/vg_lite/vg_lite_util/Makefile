include $(MPP_SRC_DIR)/userapps/src/mpp.mk
include $(MPP_SRC_DIR)/userapps/src/rt-smart.mk

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
LIB = $(MPP_SRC_DIR)/userapps/lib/lib$(CURRECT_DIR_NAME).a

LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR)/ -I$(LOCAL_SRC_DIR)/libpng-1.6.1 -I$(LOCAL_SRC_DIR)/zlib-1.2.7

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c) $(wildcard $(LOCAL_SRC_DIR)/libpng-1.6.1/*.c) $(wildcard $(LOCAL_SRC_DIR)/zlib-1.2.7/*.c)

OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(LIB)

$(OBJS): %.o : %.c
	@echo "CC $@"
	@$(CC) $(CC_CFLAGS) -Wno-error $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "AR $@"
	@$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	echo "${PWD}/Makefile clean"
	-rm -rf $(LIB)
	-rm -f $(OBJS)

.PHONY: all clean