IPCMSG_ROOT:= $(firstword $(shell pwd | sed -re '{s/[i]-?[p][c][m][s][g][-,a-z]*/& /;}'))
include $(IPCMSG_ROOT)/config.mk
ifeq ($(SERVER_PLATFORM),x86)
ifeq ($(SERVER_OS_TYPE),rt-smart)
$(error $(SERVER_PLATFORM) not support $(SERVER_OS_TYPE) os)
endif
endif
ifeq ($(CLIENT_PLATFORM),x86)
ifeq ($(CLIENT_OS_TYPE),rt-smart)
$(error $(CLIENT_PLATFORM) not support $(CLIENT_OS_TYPE) os)
endif
endif

all:
	@cd sample/sample_receiver; make
	@cd sample/sample_sender; make

clean:
	@cd sample/sample_receiver; make clean
	@cd sample/sample_sender; make clean
