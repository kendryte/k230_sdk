IPCMSG_ROOT=$(CDK_SRC_DIR)/user/component/ipcmsg
DATAFIFO_ROOT=$(CDK_SRC_DIR)/user/component/datafifo
MPI_ROOT=$(MPP_SRC_DIR)
MAPI_ROOT=$(CDK_SRC_DIR)/user/mapi

BIN=$(CDK_SRC_DIR)/user/out/big/sample_vicap_dump.elf
CC=riscv64-unknown-linux-musl-gcc
CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -Werror -Wall -O0 -g -gdwarf-2 -n --static
CPP=riscv64-unknown-linux-musl-g++
LINKFLAG=-T ./linker_scripts/riscv64/link.lds
LIBS=-lipcmsg
LIBS+=-lpthread
LIBS+=-lmapi
LIBS+=-lsys
LIBS+=-lvvi
LIBS+=-lvb
LIBS+=-lvdss

LIBS+=-lvo
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
LIBS+=-lvb
LIBS+=-lvdec
LIBS+=-lvicap
LIBS+=-lsensor
LIBS+=-lvenc
LIBS+=-ldatafifo
#LIBS+=-lvin_drv
LIBS+=-lai
LIBS+=-lao
LIBS+=-laenc
LIBS+=-ladec
LIBS+=-lvdec



LIBPATH=-L$(IPCMSG_ROOT)/slave/lib
LIBPATH+=-L$(MPI_ROOT)/userapps/lib
LIBPATH+=-L$(MAPI_ROOT)/mediaserver/lib
LIBPATH+=-L$(DATAFIFO_ROOT)/host/lib

#add common include
INCS=-I$(IPCMSG_ROOT)/include
INCS+=-I$(MPI_ROOT)/include
INCS+=-I$(MPI_ROOT)/include/comm
INCS+=-I$(MPI_ROOT)/userapps/api
INCS+=-I$(MAPI_ROOT)/include
INCS+=-I$(MAPI_ROOT)/include/api
INCS+=-I$(MAPI_ROOT)/include/comm


SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@rm -f $(COBJS)

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(LINKFLAG) -Wl,--whole-archive -Wl,--no-whole-archive -n --static $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

clean:
	@rm -f $(BIN) $(COBJS)
