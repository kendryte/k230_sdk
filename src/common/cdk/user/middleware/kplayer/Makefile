BIN=libkplayer.a
CC=riscv64-unknown-linux-gnu-gcc
AR=riscv64-unknown-linux-gnu-ar -rcs
# CC=gcc -g
# AR=gcc-ar rc
IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MP4_ROOT=$(CDK_SRC_DIR)/user/middleware/mp4_format
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lmp4
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
LIBPATH+=-L$(MP4_ROOT)/lib
LIBPATH+=-L/usr/local/lib

CFLAGS=
DEFINE=
TARGET_DIR=lib
BUILD_DIR=build

$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

INCS=-I$(CDK_SRC_DIR)/user/middleware/kplayer/include
INCS+=-I$(CDK_SRC_DIR)/user/middleware/mp4_format/include
INCS+=-I$(MP4_ROOT)/include
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api


AUDIO_MIX_SRC=./src
SRCS:=$(foreach dir, $(AUDIO_MIX_SRC), $(wildcard $(dir)/*.c $()))
COBJS:=$(SRCS:.c=.o)

all:$(BIN)

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS) $(LIBPATH) $(LIBS)


$(BIN):$(COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN) $(COBJS)
	rm -f  $(COBJS)

clean:
	rm -f $(TARGET_DIR)/$(BIN) $(COBJS)
