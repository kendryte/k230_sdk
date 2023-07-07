SRC_PATH := $(shell pwd)
# INC_FLAGS += -I$(SRC_PATH)/include
# CFLAGS += -Wall -g $(INC_FLAGS)

SMP_SRCS := $(SRC_PATH)/application.c
SMP_SRCS += $(SRC_PATH)/camera.c
SMP_SRCS += $(SRC_PATH)/frame_cache.c
SMP_SRCS += $(SRC_PATH)/kstream.c
SMP_SRCS += $(SRC_PATH)/kuvc.c
SMP_SRCS += $(SRC_PATH)/log.c
SMP_SRCS += $(SRC_PATH)/sample_yuv.c
SMP_SRCS += $(SRC_PATH)/uvc-gadget.c
SMP_SRCS += $(SRC_PATH)/sample_venc.c

# SMP_SRCS := $(wildcard *.c)

IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi
MPI_ROOT=$(RTSMART_SRC_DIR)/userapps/mpp

CC=riscv64-unknown-linux-gnu-gcc
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lm
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBPATH=-L$(IPCMSG_ROOT)/host/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaclient/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/slave/lib
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

TARGET := $(CDK_SRC_DIR)/user/out/little/camera

TARGET_PATH := $(PWD)

# target source
OBJS  := $(SMP_SRCS:%.c=%.o)

CFLAGS += $(INCS)

.PHONY : clean all

all: $(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)
	@cp canaan-camera.sh $(CDK_SRC_DIR)/user/out/little/
	@cp red.jpg $(CDK_SRC_DIR)/user/out/little/

$(TARGET):$(COMM_OBJ) $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)
	@rm -f $(CDK_SRC_DIR)/user/out/little/canaan-camera.sh
	@rm -f $(CDK_SRC_DIR)/user/out/little/red.jpg

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4

