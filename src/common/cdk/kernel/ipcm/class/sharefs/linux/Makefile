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
LIB_A  := libsharefs.a
LIB_SO := $(OUT)/libsharefs.so
APP    := ${OUT}/sharefs

SRCS := memmap.c sfs_server.c
SRCS += ../server_ipc.c
SRCS += ../server_ops.c
SRCS_APP := sfs_server_main.c
OBJS := $(patsubst %.c,%.o, $(SRCS))

LIBDEPS := -lpthread -lsharefs

ARFLAGS = cr
LOCAL_CFLAGS += -I..

all: $(APP) $(LIB_SO)

$(APP): $(LIB_A)
	$(CROSS_COMPILE)gcc -o $@ $(SRCS_APP) $(LOCAL_LD_CFLAGS) -L. $(LIBDEPS)

$(LIB_A): $(OBJS)
	mkdir -p $(OUT)
	@$(CROSS_COMPILE)ar $(ARFLAGS) $@ $^
	rm $(OBJS)
	cp $@ $(OUT)
$(LIB_SO): $(SRCS)
	@$(CROSS_COMPILE)gcc $(LOCAL_SO_CFLAGS) $(LOCAL_CFLAGS) -o $@ $^

$(OBJS): %.o : %.c
	@$(CROSS_COMPILE)gcc $(IPCM_CFLAGS) $(LOCAL_CFLAGS) -o $@ -c $<

clean:
	-rm $(LIB_A)
	-rm $(OUT)/$(LIB_A) $(OUT)/$(LIB_SO) $(LIB_SO) $(APP)

.PHONY: all clean
