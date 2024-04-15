include $(MPP_SRC_DIR)/middleware/mpp.mk
include $(MPP_SRC_DIR)/middleware/rt-smart.mk

BIN=libmp4.a
# CC=riscv64-unknown-linux-gnu-gcc
# AR=riscv64-unknown-linux-gnu-ar -rcs

CC=riscv64-unknown-linux-musl-gcc
AR=riscv64-unknown-linux-musl-ar -rc

# CC=gcc -g
# AR=gcc-ar rc

CURRECT_DIR_NAME=$(shell pwd)

CFLAGS=
DEFINE=
TARGET_DIR=lib
BUILD_DIR=build

MP4_SRC=src
MOV_SRC=src/libmov/source
FLV_SRC=src/libflv/source

$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

INCS=-I$(CURRECT_DIR_NAME)/include
INCS+=-I$(CURRECT_DIR_NAME)/src/libflv/include
INCS+=-I$(CURRECT_DIR_NAME)/src/libmov/include

MP4_SRCS:= $(foreach dir, $(MP4_SRC), $(wildcard $(dir)/*.c $()))
MP4_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(MP4_SRCS))))
MOV_SRCS:= $(foreach dir, $(MOV_SRC), $(wildcard $(dir)/*.c $()))
MOV_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(MOV_SRCS))))
FLV_SRCS:= $(foreach dir, $(FLV_SRC), $(wildcard $(dir)/*.c $()))
FLV_COBJS:= $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(notdir $(FLV_SRCS))))



all:$(BIN)
	@rm -f $(MP4_COBJS)
	@rm -f $(MOV_COBJS)
	@rm -f $(FLV_COBJS)

$(MOV_COBJS) : $(BUILD_DIR)/%.o: $(MOV_SRC)/%.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(FLV_COBJS) : $(BUILD_DIR)/%.o: $(FLV_SRC)/%.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)

$(MP4_COBJS) : $(BUILD_DIR)/%.o: $(MP4_SRC)/%.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@ $(INCS)  $(DEFINE)


$(BIN):$(MP4_COBJS) $(MOV_COBJS) $(FLV_COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN) $(MP4_COBJS) $(MOV_COBJS) $(FLV_COBJS)
clean:
	@rm -f $(TARGET_DIR)/$(BIN) $(MP4_COBJS) $(MOV_COBJS) $(FLV_COBJS)
	@rm -rf $(TARGET_DIR) $(BUILD_DIR)
