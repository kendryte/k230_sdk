BIN=libwsd.a

TARGET_DIR=lib
BUILD_DIR=build
$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

# ifeq ($(OS_TYPE),linux)
CC=riscv64-unknown-linux-gnu-g++
AR=riscv64-unknown-linux-gnu-ar -rcs
# endif

INCS+=-I.
INCS+=-I./include
INCS+=-I/usr/local/include

RTSP_CLIENT_SRC=./src
SRCS:=$(foreach dir, $(RTSP_CLIENT_SRC), $(wildcard $(dir)/*.cpp $()))
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp

	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)


$(BIN):$(COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN)  $(COBJS) 
	rm -f  $(COBJS)

clean:
	rm -f $(TARGET_DIR)/$(BIN) $(COBJS)
