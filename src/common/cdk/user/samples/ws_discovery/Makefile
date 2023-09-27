MW_ROOT=$(CDK_SRC_DIR)/user/middleware

BIN=$(CDK_SRC_DIR)/user/out/little/wsd_test
CC=riscv64-unknown-linux-gnu-g++
LIBS=-lpthread
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L/usr/local/lib
CFLAGS=

LIBS=-lpthread
LIBS+=-lwsd
LIBPATH+=-L$(MW_ROOT)/ws_discovery/lib

#add common include

#add mpp common include
INCS+=-I$(MW_ROOT)/ws_discovery/include
INCS+=-I/usr/local/include
WSD_DEMO_SRC= ./

SRCS:=$(foreach dir, $(WSD_DEMO_SRC), $(wildcard $(dir)/*.cpp $()))
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp

	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)


$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

	rm -f  $(COBJS)

clean:
	rm -f $(BIN) $(COBJS)
