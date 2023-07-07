IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
MPI_ROOT=$(RTSMART_SRC_DIR)/userapps/mpp
LIVE_ROOT=$(CDK_SRC_DIR)/user/thirdparty/live

BIN=$(CDK_SRC_DIR)/user/out/little/backchannel_client
CC=riscv64-unknown-linux-gnu-g++
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lliveMedia
LIBS+=-lUsageEnvironment
LIBS+=-lBasicUsageEnvironment
LIBS+=-lgroupsock
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L$(LIVE_ROOT)/BasicUsageEnvironment
LIBPATH+=-L$(LIVE_ROOT)/groupsock
LIBPATH+=-L$(LIVE_ROOT)/liveMedia
LIBPATH+=-L$(LIVE_ROOT)/UsageEnvironment
LIBPATH+=-L/usr/local/lib
CFLAGS=-DNO_OPENSSL=1

#add common include
INCS=-I$(IPCMSG_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm
#add mpp common include
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api
INCS+=-I./include
INCS+=-I/usr/local/include
INCS+=-I$(LIVE_ROOT)/BasicUsageEnvironment/include
INCS+=-I$(LIVE_ROOT)/groupsock/include
INCS+=-I$(LIVE_ROOT)/liveMedia/include
INCS+=-I$(LIVE_ROOT)/UsageEnvironment/include

RTSP_DEMO_SRC=./src ./

SRCS:=$(foreach dir, $(RTSP_DEMO_SRC), $(wildcard $(dir)/*.cpp $()))
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp

	$(CC) -c $(CFLAGS) $< -o $@ $(INCS) -DNO_OPENSSL=1


$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

	rm -f  $(COBJS)

clean:
	rm -f $(BIN) $(COBJS)
