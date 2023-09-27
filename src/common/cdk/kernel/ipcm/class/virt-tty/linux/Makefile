build_dir=$(shell pwd)

include $(MODULE_DIR)/arch/${__PLATFORM__}/Make_soc.param

ccflags-y:= -I$(MODULE_DIR)/class/virt-tty \
	-I$(MODULE_DIR)/sysdeps/linux/ \
	-I$(MODULE_DIR)/include \
	-I$(MODULE_DIR)/arch/${__PLATFORM__}/

obj-m := hi_virt-tty.o

ifeq ($(VIRT_TTY_ROLE), "server")
hi_virt-tty-objs = server_dev.o \
		../server.o \
		../../../sysdeps/linux/os_adapt.o \
		../../../message/ipcm.o \
		../../../message/ipcm_data.o \
		../../../message/ipcm_node_discover.o \
		../../../message/ipcm_proc.o \
		../../../sysdeps/linux/os_adapt.o
endif
hi_virt-tty-objs += $(patsubst %.c,%.o, ../../../${PLATFORM_SRCS})

all:
	mkdir -p ${MODULE_DIR}/out/node_$(LOCAL_ID)
	make -C ${KERNEL_DIR} M=${build_dir} ARCH=${arch_type} CROSS_COMPILE=${CROSS_COMPILE} modules O=$(LINUX_BUILD_DIR)
	cp hi_virt-tty.ko ${MODULE_DIR}/out/node_$(LOCAL_ID)
	-find $(build_dir) -name "*.o" | xargs rm -f
	-find $(build_dir) -name "*.cmd.c" | xargs rm -f

clean:
	-rm $(build_dir)/modules.order
	-rm $(build_dir)/modules.symvers
	-rm ${build_dir}/.tmp_versions/ -rf
	-rm ${MODULE_DIR}/out/node_$(LOCAL_ID)/hi_virt-tty.ko
	-rm ${MODULE_DIR}/out/node_$(LOCAL_ID)/virt-tty

