CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC=$(CROSS_COMPILE)gcc

TARGET = ws2812_demo
OBJS = ws2812.c

all:
	${CC} ${OBJS} -Wall -g -o ${TARGET}
	@echo "Compile done."
clean:
	@rm -f *.o ${TARGET}
	@echo "Clean done."
