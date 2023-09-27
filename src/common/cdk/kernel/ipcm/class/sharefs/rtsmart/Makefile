
RTSMARTTOPDIR ?= ${KERNEL_DIR}
include $(MODULE_DIR)/arch/${__PLATFORM__}/Make_soc.param

CC = riscv64-unknown-linux-musl-gcc
CC_CFLAGS += -mcmodel=medany -march=rv64imafdcv -mabi=lp64d \
		-fvar-tracking -ffreestanding -fno-common -ffunction-sections \
		-fdata-sections -fstrict-volatile-bitfields \
		-O0 -ggdb \
		-DHAVE_CCONFIG_H -D__STDC_ISO_10646__=201206L -D_STDC_PREDEF_H $(KCFLAGS)

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

LIBOUT = ${MODULE_DIR}/out/node_$(LOCAL_ID)
LIB = $(LIBOUT)/libsharefs.a

LOCAL_CFLAGS += -I$(MODULE_DIR)/include \
				-I$(MODULE_DIR)/class/sharefs

SRCS = $(wildcard $(MODULE_DIR)/class/sharefs/rtsmart/*.c) \
	   $(MODULE_DIR)/class/sharefs/client_ipc.c

RTSMART_CFLAGS += -I$(KERNEL_DIR)/include \
		-I$(KERNEL_DIR)/components/dfs/include \
		-I$(KERNEL_DIR)/components/dfs/filesystems/devfs \
		-I$(KERNEL_DIR)/components/dfs/filesystems/romfs \
		-I$(KERNEL_DIR)/components/drivers/include \
		-I$(KERNEL_DIR)/components/drivers/tty/include \
		-I$(KERNEL_DIR)/components/finsh \
		-I$(KERNEL_DIR)/components/libc/compilers/musl \
		-I$(KERNEL_DIR)/components/libc/time \
		-I$(KERNEL_DIR)/components/lwp \
		-I$(KERNEL_DIR)/components/utilities/resource

ifeq ($(__PLATFORM__), k230)
RTSMART_CFLAGS += -I$(KERNEL_DIR)/../bsp/maix3/c908/ \
		-I$(KERNEL_DIR)/../bsp/maix3 \
		-I$(KERNEL_DIR)/components/lwp/arch/risc-v/rv64
endif

OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(LIBOUT) $(LIB)
	@-rm -f $(OBJS)
	echo "${PWD}/Makefile all"

$(OBJS): %.o : %.c
	@$(CC) $(CC_CFLAGS) $(IPCM_CFLAGS) $(RTSMART_CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)

clean:
	echo "${PWD}/Makefile clean"
	-rm -rf $(LIBOUT)

.PHONY: all clean