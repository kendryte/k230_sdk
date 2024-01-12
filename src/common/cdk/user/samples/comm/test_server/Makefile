IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi

BIN=$(CDK_SRC_DIR)/user/out/big/comm_server.elf
CC=riscv64-unknown-linux-musl-g++
#CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Werror -Wall -O0 -g -gdwarf-2 -n --static
CFLAGS=-march=rv64imafdcv -mabi=lp64d -mcmodel=medany
LINKFLAG=-T ./linker_scripts/riscv64/link.lds

LIBS=
LIBS+=-lmapi
LIBS+=-ldatafifo
LIBS+=-lipcmsg
LIBS+=-lsys
LIBS+=-lvvi
LIBS+=-lvb
LIBS+=-lvdss
LIBS+=-lvenc
LIBS+=-lai
LIBS+=-lao
LIBS+=-laenc
LIBS+=-ladec
LIBS+=-lvicap
LIBS+=-lcam_device
LIBS+=-l3a
LIBS+=-lauto_ctrol
LIBS+=-lbinder
LIBS+=-lbuffer_management
LIBS+=-lcam_caldb
LIBS+=-lcam_device
LIBS+=-lcam_engine
LIBS+=-lcameric_drv
LIBS+=-lcameric_reg_drv
LIBS+=-lcommon
LIBS+=-lebase
LIBS+=-lfpga
LIBS+=-lhal
LIBS+=-lisi
LIBS+=-lisp_drv
LIBS+=-loslayer
LIBS+=-lsys
LIBS+=-lt_common_c
LIBS+=-lt_database_c
LIBS+=-lt_json_c
LIBS+=-lt_mxml_c
LIBS+=-lswitch
LIBS+=-lvideo_in
LIBS+=-lvirtual_hal
LIBS+=-lstart_engine
LIBS+=-lcmd_buffer
LIBS+=-lvdec
LIBS+=-lvicap
LIBS+=-lsensor
LIBS+=-lvo
LIBS+=-lconnector
LIBS+=-ldpu
LIBS+=-ldma

LIBS+=-lpthread
LIBS+=-lrtthread
LIBPATH=-L$(MAPI_ROOT)/mediaserver/lib
LIBPATH+=-L$(IPCMSG_ROOT)/slave/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/host/lib
LIBPATH+=-L$(MPP_SRC_DIR)/userapps/lib
LIBPATH+=-L$(RTSMART_SRC_DIR)/userapps/sdk/rt-thread/lib/risc-v/rv64

#add common include
INCS=-I./ -I../include
#add mpp common include, for k_type.h k_vb_comm.h
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api
#add ipcmsg include
INCS+=-I$(CDK_SRC_DIR)/user/component/ipcmsg/include
INCS+=-I$(CDK_SRC_DIR)/user/component/datafifo/include
#
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api

SRCS:=$(wildcard *.cpp)
SRCS +=$(wildcard ../server/*.cpp)
COBJS:=$(SRCS:.cpp=.o)

all:$(BIN)

$(COBJS) : %.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group
	rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)
