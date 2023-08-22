MP4_ROOT=$(CDK_SRC_DIR)/user/middleware/mp4_format

BIN=$(CDK_SRC_DIR)/user/out/little/sample_demuxer
CC=riscv64-unknown-linux-gnu-gcc
LIBS=-lmp4
LIBPATH=-L$(MP4_ROOT)/lib
CFLAGS=

#add common include
INCS=-I$(MP4_ROOT)/include
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
