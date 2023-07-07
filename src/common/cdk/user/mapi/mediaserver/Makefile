include ../config.mk
CURRECT_DIR_NAME=$(shell basename `pwd`)
BIN=libmapi.a
CC=gcc
AR=ar rcs
CFLAGS=
DEFINE=
TARGET_DIR=lib
BUILD_DIR=build
MAPI_SRC_MSG=
MAPI_SRC_LOG=log
MAPI_SRC_SYS=sys
MAPI_SRC_VVI=vvi
MAPI_SRC_AI=ai
MAPI_SRC_AO=ao
MAPI_SRC_AENC=aenc
MAPI_SRC_ADEC=adec
MAPI_SRC_VICAP=vicap
MAPI_SRC_SENSOR=sensor
MAPI_SRC_ISP=isp

MAPI_SRC_VDSS=vdss
MAPI_SRC_VENC=venc
MAPI_SRC_VDEC=vdec

$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

ifeq ($(CURRECT_DIR_NAME),mediaserver)
OS_TYPE=$(SERVER_OS_TYPE)
PLATFRORM=$(SERVER_PLATFORM)
MAPI_SRC_MSG=msg_server
INCS=-Imsg_server/include
endif

ifeq ($(CURRECT_DIR_NAME),mediaclient)
OS_TYPE=$(CLIENT_OS_TYPE)
PLATFRORM=$(CLIENT_PLATFORM)
MAPI_SRC_MSG=msg_client
INCS=-Imsg_client/include
LIBPATH=-L$(CDK_SRC_DIR)/compoent/ipcmsg/slave/lib
endif

ifeq ($(OS_TYPE),rt-smart)
CC=riscv64-unknown-linux-musl-gcc
AR=riscv64-unknown-linux-musl-ar -rc
CFLAGS=-mcmodel=medany -march=rv64imafdcv -mabi=lp64d -O0 -g -gdwarf-2
endif

ifeq ($(OS_TYPE),linux)
CC=riscv64-unknown-linux-gnu-gcc
AR=riscv64-unknown-linux-gnu-ar -rcs
endif

INCS+=-Ivenc

#add common include
INCS+=-I../include
INCS+=-I../include/api
INCS+=-I../include/comm
INCS+=-I../include/msg


#add mpp common include
INCS+=-I$(MPP_SRC_DIR)/include
INCS+=-I$(MPP_SRC_DIR)/include/comm
INCS+=-I$(MPP_SRC_DIR)/userapps/api

#add ipcmsg include
INCS+=-I$(CDK_SRC_DIR)/user/component/ipcmsg/include
INCS+=-I$(CDK_SRC_DIR)/user/component/datafifo/include

MSG_SRCS:= $(foreach dir, $(MAPI_SRC_MSG), $(wildcard $(dir)/*.c $()))
MSG_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(MSG_SRCS))))
SYS_SRCS:= $(foreach dir, $(MAPI_SRC_SYS), $(wildcard $(dir)/*.c $()))
SYS_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(SYS_SRCS))))
VVI_SRCS:= $(foreach dir, $(MAPI_SRC_VVI), $(wildcard $(dir)/*.c $()))
VVI_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(VVI_SRCS))))
LOG_SRCS:= $(foreach dir, $(MAPI_SRC_LOG), $(wildcard $(dir)/*.c $()))
LOG_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(LOG_SRCS))))
AI_SRCS:= $(foreach dir, $(MAPI_SRC_AI), $(wildcard $(dir)/*.c $()))
AI_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(AI_SRCS))))
AO_SRCS:= $(foreach dir, $(MAPI_SRC_AO), $(wildcard $(dir)/*.c $()))
AO_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(AO_SRCS))))
AENC_SRCS:= $(foreach dir, $(MAPI_SRC_AENC), $(wildcard $(dir)/*.c $()))
AENC_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(AENC_SRCS))))
ADEC_SRCS:= $(foreach dir, $(MAPI_SRC_ADEC), $(wildcard $(dir)/*.c $()))
ADEC_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(ADEC_SRCS))))


VDSS_SRCS:= $(foreach dir, $(MAPI_SRC_VDSS), $(wildcard $(dir)/*.c $()))
VDSS_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(VDSS_SRCS))))

VENC_SRCS:= $(foreach dir, $(MAPI_SRC_VENC), $(wildcard $(dir)/*.c $()))
VENC_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(VENC_SRCS))))

VDEC_SRCS:= $(foreach dir, $(MAPI_SRC_VDEC), $(wildcard $(dir)/*.c $()))
VDEC_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(VDEC_SRCS))))

VICAP_SRCS:= $(foreach dir, $(MAPI_SRC_VICAP), $(wildcard $(dir)/*.c $()))
VICAP_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(VICAP_SRCS))))

SENSOR_SRCS:= $(foreach dir, $(MAPI_SRC_SENSOR), $(wildcard $(dir)/*.c $()))
SENSOR_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(SENSOR_SRCS))))

ISP_SRCS:= $(foreach dir, $(MAPI_SRC_ISP), $(wildcard $(dir)/*.c $()))
ISP_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(ISP_SRCS))))

all:$(BIN)
	@rm -f $(MSG_COBJS) $(SYS_COBJS) $(VVI_COBJS) $(VDSS_COBJS) $(VENC_COBJS) $(VDEC_COBJS) $(LOG_COBJS) $(AI_COBJS) $(AO_COBJS) $(AENC_COBJS) $(ADEC_COBJS) $(VICAP_COBJS) $(SENSOR_COBJS) $(ISP_COBJS)

$(MSG_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_MSG)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(SYS_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_SYS)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(VVI_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_VVI)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(VDSS_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_VDSS)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(VENC_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_VENC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(VDEC_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_VDEC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(LOG_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_LOG)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(AI_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_AI)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(AO_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_AO)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(AENC_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_AENC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(ADEC_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_ADEC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(VICAP_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_VICAP)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(SENSOR_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_SENSOR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(ISP_COBJS) : $(BUILD_DIR)/%.o: $(MAPI_SRC_ISP)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(BIN):$(MSG_COBJS) $(SYS_COBJS) $(VVI_COBJS) $(VDSS_COBJS) $(VENC_COBJS) $(VDEC_COBJS) $(LOG_COBJS) $(AI_COBJS) $(AO_COBJS) $(AENC_COBJS) $(ADEC_COBJS)  $(VICAP_COBJS) $(SENSOR_COBJS) $(ISP_COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN) $(MSG_COBJS) $(SYS_COBJS) $(VVI_COBJS) $(VDSS_COBJS)  $(VENC_COBJS) $(VDEC_COBJS)  $(LOG_COBJS) $(AI_COBJS) $(AO_COBJS) $(AENC_COBJS) $(ADEC_COBJS) $(VICAP_COBJS) $(SENSOR_COBJS) $(ISP_COBJS)
clean:
	@rm -f $(TARGET_DIR)/$(BIN) $(MSG_COBJS) $(SYS_COBJS) $(VVI_COBJS) $(VDSS_COBJS) $(VENC_COBJS) $(VDEC_COBJS)  $(LOG_COBJS) $(AI_COBJS) $(AO_COBJS) $(AENC_COBJS) $(ADEC_COBJS) $(VICAP_COBJS) $(SENSOR_COBJS) $(ISP_COBJS)