IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MW_ROOT=$(CDK_SRC_DIR)/user/middleware
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi

BIN=$(CDK_SRC_DIR)/user/out/little/comm_client
CC=riscv64-unknown-linux-gnu-g++

LIBS=
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lipcmsg
LIBS+=-lpthread
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L/usr/local/lib

#add common include
INCS=-I./ -I../include
#add mpp common include, for k_type.h
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api
#add ipcmsg include
INCS+=-I$(CDK_SRC_DIR)/user/component/ipcmsg/include
INCS+=-I$(CDK_SRC_DIR)/user/component/datafifo/include
#add mapi include
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm

SRCS:=$(wildcard *.cpp)
SRCS +=$(wildcard ../client/*.cpp)
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
	rm -f  $(COBJS)

clean:
	rm -f $(BIN) $(COBJS)
