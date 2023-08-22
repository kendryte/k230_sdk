IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
MPI_ROOT=$(RTSMART_SRC_DIR)/userapps/mpp
MP4_ROOT=$(CDK_SRC_DIR)/user/middleware/mp4_format

BIN=$(CDK_SRC_DIR)/user/out/little/sample_muxer
CC=riscv64-unknown-linux-gnu-gcc
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lmp4
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L$(MP4_ROOT)/lib
CFLAGS=

#add common include
INCS=-I$(IPCMSG_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm
INCS+=-I$(MP4_ROOT)/include
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

	@rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)
