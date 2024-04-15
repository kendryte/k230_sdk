#CROSS_COMPILE := riscv64-linux-
CROSS_COMPILE := riscv64-unknown-linux-gnu-

CC	:= $(CROSS_COMPILE)gcc
C++	:= $(CROSS_COMPILE)g++
OBJDUMP	:= $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
AR	:= $(CROSS_COMPILE)ar
AS	:= $(CROSS_COMPILE)as
LD	:= $(CROSS_COMPILE)gcc

CFLAGS = \
	-fomit-frame-pointer -fno-strict-aliasing -fno-builtin \
	-ffunction-sections -fdata-sections \
	-DCFG_MAKEFILE -DTARGET_BOARD_TYPE_def=\"$(BR2_TARGET_BOARD_TYPE)\"

PROG_1 = mvx_encoder
PROG_2 = mvx_encoder_multi
PROG_3 = mvx_decoder
PROG_4 = mvx_decoder_multi

all: ${PROG_1} ${PROG_2} ${PROG_3} ${PROG_4}
.PHONY: all

${PROG_1}: mvx_encoder.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(C++) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread -g -o $@

${PROG_2}: mvx_encoder_multi.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(C++) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread -g -o $@

${PROG_3}: mvx_decoder.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(C++) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread -g -o $@
	
${PROG_4}: mvx_decoder_multi.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(C++) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread -g -o $@
	
clean:
	rm -f *.o
	rm -f *.d
	rm -f ${PROG_1} ${PROG_2} ${PROG_3} ${PROG_4} 

