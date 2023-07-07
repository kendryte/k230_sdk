CROSS_COMPILE = riscv64-unknown-linux-gnu-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

CFLAGS = $(CFLAG) -Wall -fpic
LDFLAGS = $(LDFLAG) -shared

TARGET = libdisp.so

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)
	@echo "Clean object files done."

	@rm -f *~
	@echo "Clean tempreator files done."

	@rm -f $(TARGET)
	@echo "Clean target files done."

	@echo "Clean done."