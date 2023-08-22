CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC=$(CROSS_COMPILE)gcc

TARGET = touch_emu
OBJS = touch_emu.c

all:
	${CC} ${OBJS} -Wall -g -o ${TARGET}
	@echo "Compile done."
clean:
	@rm -f *.o ${TARGET}
	@echo "Clean done."
