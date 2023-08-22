BIN=libaudio_mix.a
CC=riscv64-unknown-linux-gnu-gcc
AR=riscv64-unknown-linux-gnu-ar -rcs
# CC=gcc -g
# AR=gcc-ar rc

CFLAGS=
DEFINE=
TARGET_DIR=lib
BUILD_DIR=build

$(shell if [ ! -e $(TARGET_DIR) ];then mkdir -p $(TARGET_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

INCS=-I$(CDK_SRC_DIR)/user/middleware/audio_mix/include
INCS+=-I$(MP4_ROOT)/include
INCS+=-I$(MPP_SRC_DIR)/include


AUDIO_MIX_SRC=./src
SRCS:=$(foreach dir, $(AUDIO_MIX_SRC), $(wildcard $(dir)/*.c $()))
COBJS:=$(SRCS:.c=.o)

all:$(BIN)

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)


$(BIN):$(COBJS)
	$(AR) -o $(TARGET_DIR)/$(BIN)  $(COBJS)
	rm -f  $(COBJS)

clean:
	rm -f $(TARGET_DIR)/$(BIN) $(COBJS)
