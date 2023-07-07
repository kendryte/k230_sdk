IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
MPI_ROOT=$(RTSMART_SRC_DIR)/userapps/mpp

BIN=$(CDK_SRC_DIR)/user/out/little/sample_vdss
CC=riscv64-unknown-linux-gnu-gcc
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
CFLAGS=

#add common include
INCS=-I$(IPCMSG_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm
#add mpp common include
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api


SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@rm -f $(COBJS)

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
clean:
	@rm -f $(BIN) $(COBJS)
