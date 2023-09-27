
build_dir=$(CDK_BUILD_DIR)/kernel/ipcm/class/cdev/linux

include $(MODULE_DIR)/arch/${__PLATFORM__}/Make_soc.param

ccflags-y:= -I$(MODULE_DIR)/include \
	    -I$(MODULE_DIR)/sysdeps/linux/ \
	    -I$(MODULE_DIR)/arch/${__PLATFORM__}/

obj-m := k_ipcm.o

k_ipcm-objs := ../../../message/ipcm.o \
		../../../message/ipcm_data.o \
		../../../message/ipcm_node_discover.o \
		../../../message/ipcm_proc.o \
		../../../sysdeps/linux/os_adapt.o

ifdef CDEV
ifeq ($(CDEV),y)
k_ipcm-objs += ipcm_userdev.o
endif
endif
k_ipcm-objs += $(patsubst %.c,%.o, ../../../${PLATFORM_SRCS})

all:
	echo "${build_dir}/Makefile all"
	echo "${k_ipcm-objs}"
	mkdir -p $(MODULE_DIR)/out/node_$(LOCAL_ID)
	mkdir -p ${build_dir}
	touch ${build_dir}/Makefile
	make -C ${KERNEL_DIR} M=${build_dir} src=$(shell pwd) ARCH=${arch_type} CROSS_COMPILE=${CROSS_COMPILE} modules O=$(LINUX_BUILD_DIR)
	cp ${build_dir}/k_ipcm.ko $(MODULE_DIR)/out/node_$(LOCAL_ID)
	-find ${MODULE_DIR} -name "*.o" | xargs rm
	-find ${MODULE_DIR} -name "*.mod.c" | xargs rm

clean:
	echo "${build_dir}/Makefile clean"
	-rm ${build_dir}/modules.order
	-rm ${build_dir}/Module.symvers
	-rm ${build_dir}/.tmp_versions/ -rf
	-rm $(MODULE_DIR)/out/node_$(LOCAL_ID) -r

