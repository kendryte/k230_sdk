include $(MPP_SRC_DIR)/userapps/sample/mpp.mk
include $(MPP_SRC_DIR)/userapps/sample/rt-smart.mk

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
BIN = $(MPP_SRC_DIR)/userapps/sample/elf/$(CURRECT_DIR_NAME).elf
LIBPATH = $(MPP_LIB_PATH)
LIBPATH += -L$(RTSMART_SRC_DIR)/userapps/sdk/rt-thread/lib/risc-v/rv64
LIBS = $(MPP_LIBS)
LIBS += -lrtthread

LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR)

DEVICE_CFLAGS = -I$(RTSMART_SRC_DIR)/userapps \
		-I$(RTSMART_SRC_DIR)/userapps/sdk/rt-thread/include \
		-I$(RTSMART_SRC_DIR)/userapps/sdk/rt-thread/components/drivers

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c)

OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(BIN)
	@-rm -f $(OBJS)
	echo "${PWD}/Makefile all"

$(OBJS): %.o : %.c
	@$(CC) -DHAVE_CCONFIG_H $(CC_CFLAGS) $(LOCAL_CFLAGS) $(DEVICE_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $(OBJS) -L$(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	echo "${PWD}/Makefile clean"
	-rm -rf $(BIN)
	-rm -f $(OBJS)

.PHONY: all clean