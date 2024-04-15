CC=riscv64-unknown-linux-musl-gcc
CC_CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Werror -Wall -O0 -g -gdwarf-2 -n --static $(KCFLAGS)
CPP=riscv64-unknown-linux-musl-g++
LINKFLAG=-T $(MPP_SRC_DIR)/userapps/sample/linker_scripts/riscv64/link.lds -lpthread