#/bin/sh
cp ../kernel/bsp/maix3/rtthread.bin ./
export CROSS_COMPILE=~/.tools/gnu_gcc/riscv64--musl--bleeding-edge-2020.08-1/bin/riscv64-linux-
export PLATFORM=kendryte/fpgac908
make FW_PAYLOAD_PATH=rtthread.bin FW_FDT_PATH=hw.dtb
