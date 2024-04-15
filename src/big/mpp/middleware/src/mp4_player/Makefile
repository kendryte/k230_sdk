include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

AR = riscv64-unknown-linux-musl-ar
ARFLAGS = -rc

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
BIN = lib$(CURRECT_DIR_NAME).a
LIBPATH = $(MPP_LIB_PATH)
LIBS = $(MPP_LIBS)

CC_CFLAGS_EX=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Wall -O0 -g -gdwarf-2 -n --static $(KCFLAGS)

MP4_FORMAT = $(MPP_SRC_DIR)/middleware/src/mp4_format

LOCAL_LIBS_PATH = -L$(MP4_FORMAT)/lib
LOCAL_LIBS = -lmp4


LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR) -I$(LOCAL_SRC_DIR)/include -I$(MP4_FORMAT)/include

WERROR_FLAGS = -Wno-register -Wno-multichar -Wno-pessimizing-move -Wno-deprecated-declarations -Wno-unused-result -Wno-unused-variable -Wno-format -Wno-return-type -Wno-sign-compare -Wno-unused-label

CXX_FLAGS = -std=c++20 \
			$(WERROR_FLAGS) \

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c)

OBJS = $(patsubst %.c,%.o,$(SRCS))

CPP_SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.cpp)

CPP_OBJS = $(patsubst %.cpp,%.o,$(CPP_SRCS))

all:$(BIN)
	@-rm -f *.o
	@echo "${PWD}/Makefile all"

$(OBJS): %.o : %.c
	@echo CC $@
	@$(CC) $(CC_CFLAGS_EX) $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) $(LOCAL_LIBS) -c $< -o $@

$(CPP_OBJS): %.o : %.cpp
	@echo CXX $@
	@$(CPP) $(CC_CFLAGS_EX) $(CXX_FLAGS) $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) $(LOCAL_LIBS) -c $< -o $@

$(BIN): $(OBJS) $(CPP_OBJS)
	@echo LD $@
	#@$(CPP) -o $(BIN)  $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $^ -L$(LIBPATH) $(CXX_FLAGS) $(LOCAL_LIBS) $(LOCAL_LIBS_PATH) -Wl,--start-group $(LIBS) -Wl,--end-group
	$(AR) $(ARFLAGS) $@ $^
clean:
	echo "${PWD}/Makefile clean"
	rm -rf $(BIN) $(OBJS)

.PHONY: all clean
