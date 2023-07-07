
OBJS = VGLiteKernel/linux/vg_lite_hal.o VGLiteKernel/vg_lite_kernel.o
TEST_OBJS = unit/test.o

ifeq ($(DEBUG), 1)
EXTRA_CFLAGS += -g -Og -DDEBUG -D_DEBUG
else
EXTRA_CFLAGS += -O2
endif

ifeq ($(KERNELRELEASE),)

PWD = $(shell pwd)
OUT = bin

all: $(OUT)
	mkdir -p out
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) V=$(V) modules

clean:
	rm -fr $(OBJS) *.o vg_lite.ko .*.cmd modules.order Module.symvers vg_lite.mod.c .tmp_versions out $(OUT)/vg_lite.ko
	rm -f ./linux/.*.cmd

$(OUT):
	mkdir -p $(OUT)

else

obj-m := vg_lite.o test_vg_lite.o
vg_lite-objs := $(OBJS)
test_vg_lite-objs := $(TEST_OBJS)

EXTRA_CFLAGS += -I$(M)/inc -I$(M)/VGLiteKernel/linux -I$(M)/VGLiteKernel

endif
