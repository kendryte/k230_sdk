CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC=$(CROSS_COMPILE)gcc

TARGET = trng_test_demo
OBJS = trng_test.c

all:
	${CC} ${OBJS} -Wall -g -o ${TARGET}
	@echo "Compile done."
clean:
	@rm -f *.o ${TARGET}
	@echo "Clean done."
