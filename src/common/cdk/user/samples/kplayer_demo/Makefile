KPLAYER_ROOT=$(CDK_SRC_DIR)/user/middleware/kplayer

BIN=$(CDK_SRC_DIR)/user/out/little/sample_kplayer
CC=riscv64-unknown-linux-gnu-gcc

IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MP4_ROOT=$(CDK_SRC_DIR)/user/middleware/mp4_format
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lmp4
LIBS+=-lkplayer
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L$(MP4_ROOT)/lib
LIBPATH+=-L$(KPLAYER_ROOT)/lib
CFLAGS=

#add common include
INCS=-I$(KPLAYER_ROOT)/include
INCS+=-I$(MPP_SRC_DIR)/include

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
