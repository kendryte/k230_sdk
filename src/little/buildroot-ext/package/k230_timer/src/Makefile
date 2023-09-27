CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC=$(CROSS_COMPILE)gcc

TARGET = k230_timer_demo
OBJS = k230_timer.c

all:
	${CC} ${OBJS} -Wall -g -o ${TARGET}
	@echo "Compile done."
clean:
	@rm -f *.o ${TARGET}
	@echo "Clean done."
