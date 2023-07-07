IPCMSG_ROOT:= $(firstword $(shell pwd | sed -re '{s/[i]-?[p][c][m][s][g][-,a-z]*/& /;}'))
include $(IPCMSG_ROOT)/config.mk

BIN=$(CDK_SRC_DIR)/user/out/little/sample_receiver
CC=gcc
AR=ar rcs
CFLAGS=
LIBS=-lipcmsg
LIBS+=-lpthread
LIBPATH=-L$(IPCMSG_ROOT)/slave/lib
DEFINE=
OS_TYPE=$(SERVER_OS_TYPE)
TRANSPORT=$(SERVER_TRANSPORT)
PLATFRORM=$(SERVER_PLATFORM)

ifeq ($(PLATFRORM),riscv)
ifeq ($(OS_TYPE),rt-smart)
BIN=$(CDK_SRC_DIR)/user/out/big/sample_receiver.elf
CC=riscv64-unknown-linux-musl-gcc
CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Werror -Wall -O2 -g -gdwarf-2 -n --static
CPP=riscv64-unknown-linux-musl-g++
LINKFLAG=-T ./linker_scripts/riscv64/link.lds
endif
ifeq ($(OS_TYPE),linux)
CC=riscv64-unknown-linux-gnu-gcc
AR=riscv64-unknown-linux-gnu-ar
endif
endif

#add common include
INCS=-I$(IPCMSG_ROOT)/include
INCS+=-I$(IPCMSG_ROOT)/sample
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
