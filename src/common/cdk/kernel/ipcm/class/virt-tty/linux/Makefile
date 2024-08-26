build_dir=$(CDK_BUILD_DIR)/kernel/ipcm/class/virt-tty/linux

include $(MODULE_DIR)/arch/${__PLATFORM__}/Make_soc.param

KBUILD_EXTRA_SYMBOLS += ${build_dir}/../../cdev/linux/Module.symvers
export KBUILD_EXTRA_SYMBOLS

ccflags-y:= -I$(MODULE_DIR)/class/virt-tty \
	-I$(MODULE_DIR)/include
obj-m := k_virt-tty.o

ifeq ($(VIRT_TTY_ROLE), "server")
k_virt-tty-objs = server_dev.o \
		../server.o \
		../../../sysdeps/linux/os_adapt.o
endif

all:
	echo "${build_dir}/Makefile all"
	mkdir -p ${MODULE_DIR}/out/node_$(LOCAL_ID)
	mkdir -p ${build_dir}
	touch ${build_dir}/Makefile
	make -C ${KERNEL_DIR} M=${build_dir} src=$(shell pwd) ARCH=${arch_type} CROSS_COMPILE=${CROSS_COMPILE} modules O=$(LINUX_BUILD_DIR)
	cp ${build_dir}/k_virt-tty.ko ${MODULE_DIR}/out/node_$(LOCAL_ID)
	-find $(build_dir) -name "*.o" | xargs rm -f
	-find $(build_dir) -name "*.cmd.c" | xargs rm -f

clean:
	-rm $(build_dir)/modules.order	
	-rm $(build_dir)/modules.symvers
	-rm ${build_dir}/.tmp_versions/ -rf
	-rm ${MODULE_DIR}/out/node_$(LOCAL_ID)/k_virt-tty.ko
	-rm ${MODULE_DIR}/out/node_$(LOCAL_ID)/virt-tty

