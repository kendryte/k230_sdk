AUDIO_MIX_ROOT=$(CDK_SRC_DIR)/user/middleware/audio_mix

BIN=$(CDK_SRC_DIR)/user/out/little/sample_audio_mix
CC=riscv64-unknown-linux-gnu-gcc
LIBS=-laudio_mix
LIBPATH=-L$(AUDIO_MIX_ROOT)/lib
CFLAGS=

#add common include
INCS=-I$(AUDIO_MIX_ROOT)/include
INCS+=-I$(MPP_SRC_DIR)/include

SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@rm -f $(COBJS)

$(COBJS) : %.o: %.c

	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)


$(BIN):$(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LIBPATH) -Wl,--start-group $(LIBS) -Wl,--end-group

	@rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)
