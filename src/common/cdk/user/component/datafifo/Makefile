DATAFIFO_ROOT:= $(firstword $(shell pwd | sed -re '{s/[d]-?[a][t][a][f][i][f][o][-,a-z]*/& /;}'))
include $(DATAFIFO_ROOT)/config.mk

ifeq ($(HOST_PLATFORM),x86)
ifeq ($(HOST_OS_TYPE),rt-smart)
$(error $(HOST_PLATFORM) not support $(HOST_OS_TYPE) os)
endif
endif
ifeq ($(SLAVE_PLATFORM),x86)
ifeq ($(SLAVE_OS_TYPE),rt-smart)
$(error $(SLAVE_PLATFORM) not support $(SLAVE_OS_TYPE) os)
endif
endif

all:
	@cd sample/sample_reader; make
	@cd sample/sample_writer; make

clean:
	@cd sample/sample_reader; make clean
	@cd sample/sample_writer; make clean

distclean: clean
	@rm -rf host/build host/lib
	@rm -rf slave/build slave/lib

