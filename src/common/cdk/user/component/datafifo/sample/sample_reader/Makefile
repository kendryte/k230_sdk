DATAFIFO_ROOT:= $(firstword $(shell pwd | sed -re '{s/[d]-?[a][t][a][f][i][f][o][-,a-z]*/& /;}'))
include $(DATAFIFO_ROOT)/config.mk

BIN=$(CDK_SRC_DIR)/user/out/little/sample_reader
CC=gcc
AR=ar rcs
CFLAGS=
LIBS=-ldatafifo
LIBS+=-lpthread
LIBPATH=-L$(DATAFIFO_ROOT)/slave/lib
DEFINE=
OS_TYPE=$(SLAVE_OS_TYPE)
PLATFRORM=$(SLAVE_PLATFORM)

ifeq ($(PLATFRORM),riscv)
ifeq ($(OS_TYPE),rt-smart)
LIBS+=-lsys
LIBPATH+=-L$(MPP_SRC_DIR)/userapps/lib
BIN=$(CDK_SRC_DIR)/user/out/big/sample_reader.elf
CC=riscv64-unknown-linux-musl-gcc
CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Werror -Wall -O2 -g -gdwarf-2 -n --static
CPP=riscv64-unknown-linux-musl-g++
CPPFLAGS=-mcmodel=medany -march=rv64imac -mabi=lp64
LINKFLAG=-T ./linker_scripts/riscv64/link.lds
endif
ifeq ($(OS_TYPE),linux)
CC=riscv64-unknown-linux-gnu-gcc
AR=riscv64-unknown-linux-gnu-ar
endif
endif

INCS=-I$(DATAFIFO_ROOT)/include/
#add mpp common include
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api

SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@-rm -f $(COBJS)
	echo "${PWD}/Makefile all"

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

ifeq ($(OS_TYPE),rt-smart)
$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(CPPFLAGS) $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
else
$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) $(LIBS)
endif
clean:
	@rm -f $(BIN) $(COBJS)
