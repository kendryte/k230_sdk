include $(MPP_SRC_DIR)/userapps/sample/mpp.mk
include $(MPP_SRC_DIR)/userapps/sample/rt-smart.mk

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
BIN = $(MPP_SRC_DIR)/userapps/sample/elf/$(CURRECT_DIR_NAME).elf
LIBPATH = $(MPP_LIB_PATH)
LIBS = $(MPP_LIBS)
AI_LIBPATH=$(MPP_SRC_DIR)/../nncase
KMODEL_LIBPATH=$(MPP_SRC_DIR)/../kmodel

LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR) -I$(LOCAL_SRC_DIR)/../sample_vo

WERROR_FLAGS = -Wno-register -Wno-multichar -Wno-pessimizing-move -Wno-deprecated-declarations -Wno-unused-result -Wno-unused-variable -Wno-format -Wno-return-type -Wno-sign-compare -Wno-unused-label

RISCV64_NN_INCLUDE = \
			-I$(AI_LIBPATH)/riscv64 \
			-I$(AI_LIBPATH)/riscv64/nncase/include \
			-I$(AI_LIBPATH)/riscv64/nncase/include/nncase \
			-I$(AI_LIBPATH)/riscv64/nncase/include/nncase/runtime \
			-I$(AI_LIBPATH)/riscv64/rvvlib/include \

RISCV64_NN_LIBS = \
			-L$(AI_LIBPATH)/riscv64/rvvlib/ -lrvv \
			-L$(AI_LIBPATH)/riscv64/nncase/lib/ -lNncase.Runtime.Native -lnncase.rt_modules.k230 -lfunctional_k230

CXX_FLAGS_AI = -std=c++20 \
			$(WERROR_FLAGS) \
			$(RISCV64_NN_INCLUDE) \
			$(RISCV64_NN_LIBS) \

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c)
# SRCS += $(LOCAL_SRC_DIR)/../sample_vo/vo_test_case.c \
# 		$(LOCAL_SRC_DIR)/../sample_vo/vo_bind_test.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

CPP_SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.cc)

CPP_OBJS = $(patsubst %.cc,%.o,$(CPP_SRCS))

all: $(BIN)
	@-rm -f *.o
	@echo "${PWD}/Makefile all"

$(OBJS): %.o : %.c
	@echo CC $@
	@$(CC) $(CC_CFLAGS) $(CXX_FLAGS_AI) $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(CPP_OBJS): %.o : %.cc
	@echo CXX $@
	@$(CPP) $(CC_CFLAGS) $(CXX_FLAGS_AI) $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(BIN): $(OBJS) $(CPP_OBJS)
	@echo LD $@
	@$(CPP) -o $(BIN)  $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $^ -L$(LIBPATH) $(CXX_FLAGS_AI) -Wl,--start-group $(LIBS) -Wl,--end-group
	cp $(KMODEL_LIBPATH)/mobile_retinaface/test.kmodel $(MPP_SRC_DIR)/userapps/sample/elf/

clean:
	echo "${PWD}/Makefile clean"
	rm -rf $(BIN) $(OBJS)

.PHONY: all clean
