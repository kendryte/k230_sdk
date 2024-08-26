#OSTYPE:=linux
include $(MODULE_DIR)/config.mk
CROSS_COMPILE = $(CFG_COMPILE)
LOCAL_CFLAGS := -fno-aggressive-loop-optimizations \
				-ffunction-sections \
				-fdata-sections \
				-Wall \
				-fno-common \
				-fno-builtin \
				-O2 \
				-Wno-implicit-function-declaration \
				-Wno-unused-variable \
				-Wno-unused-function \
				-fstack-protector
LOCAL_SO_CFLAGS := -fPIC -shared \
				   -Wl,-z,relro \
				   -Wl,-z,noexecstack
LOCAL_LD_CFLAGS := -Wl,-z,relro -Wl,-z,noexecstack

OUT := ${MODULE_DIR}/out/node_$(LOCAL_ID)
APP    := ${OUT}/rtt-ctrl

SRCS = rc_client.c
SRCS += ../client_ipc.c

LOCAL_CFLAGS += -I..

all:
	$(CROSS_COMPILE)gcc $(LOCAL_CFLAGS) -o $(APP) $(SRCS) -lpthread

clean:
	-rm $(APP)

.PHONY: all clean
