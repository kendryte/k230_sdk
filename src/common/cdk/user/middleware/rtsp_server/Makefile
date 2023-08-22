BIN=librtsp_server.a

TARGET_DIR=lib
BUILD_DIR=build
$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

LIVE_ROOT=$(CDK_SRC_DIR)/user/thirdparty/live
CFLAGS=-DNO_OPENSSL=1

# ifeq ($(OS_TYPE),linux)
CC=riscv64-unknown-linux-gnu-g++
AR=riscv64-unknown-linux-gnu-ar -rcs
# endif

INCS+=-I.
INCS+=-I./include
INCS+=-I/usr/local/include
INCS+=-I$(LIVE_ROOT)/BasicUsageEnvironment/include
INCS+=-I$(LIVE_ROOT)/groupsock/include
INCS+=-I$(LIVE_ROOT)/liveMedia/include
INCS+=-I$(LIVE_ROOT)/UsageEnvironment/include

RTSP_SERVER_SRC=./src
SRCS:=$(foreach dir, $(RTSP_SERVER_SRC), $(wildcard $(dir)/*.cpp $()))
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp

	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)


$(BIN):$(COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN)  $(COBJS) 
	rm -f  $(COBJS)

clean:
	rm -f $(TARGET_DIR)/$(BIN) $(COBJS)
