CC = riscv64-unknown-linux-musl-gcc
CC_CFLAGS += -mcmodel=medany -march=rv64imafdcv -mabi=lp64d \
		-ffreestanding -fno-common -ffunction-sections \
		-fdata-sections -fstrict-volatile-bitfields \
		-O0 -Werror\
		-DHAVE_CCONFIG_H -D__STDC_ISO_10646__=201206L -D_STDC_PREDEF_H -D__KERNEL__   $(KCFLAGS)

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

RTSMART_CFLAGS = -I$(RTSMART_SRC_DIR)/kernel/rt-thread/include \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/dfs/include \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/dfs/filesystems/devfs \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/dfs/filesystems/romfs \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/drivers/include \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/drivers/wlan \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/drivers/tty/include \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/finsh \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/libc/compilers/musl \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/libc/time \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/lwp \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/utilities/resource \
		-I$(RTSMART_SRC_DIR)/kernel/rt-thread/components/lwp/arch/risc-v/rv64 \
		-I$(MPP_SRC_DIR)/kernel/include

BSP_CFLGAS = -I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/c908 \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3 \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/interdrv/sysctl/sysctl_boot \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/interdrv/sysctl/sysctl_power \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/interdrv/sysctl/sysctl_reset \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/interdrv/gpio \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/extdrv/regulator \
		-I$(RTSMART_SRC_DIR)/kernel/bsp/maix3/board/extcomponents/usage
