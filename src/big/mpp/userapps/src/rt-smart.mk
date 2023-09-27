CC = riscv64-unknown-linux-musl-gcc
CC_CFLAGS = -mcmodel=medany -march=rv64imafdcv -mabi=lp64d -O0 -Werror $(KCFLAGS)

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc
