CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC=$(CROSS_COMPILE)gcc

TARGET = crypto_test_demo
OBJS = crypto_test.c

all:
	${CC} ${OBJS} -Wall -g -o ${TARGET}
	@echo "Compile done."
clean:
	@rm -f *.o ${TARGET}
	@echo "Clean done."
