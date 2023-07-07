#===============================================================================
# export variable
#===============================================================================
ifeq ($(CFG_SOC_EXPORT_FLAG),)
SDK_DIR = $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(SDK_DIR)/base.mak
endif

#CURDIR ?= $(shell pwd)
#CC := $(CROSS_COMPILE)gcc
#===============================================================================
# local variable
#===============================================================================
SAMPLE_IMAGES := iotwifi_link

CFLAGS = -Wno-declaration-after-statement -funsigned-char -freg-struct-return -fno-strict-aliasing -Wtrampolines -Wformat=2 -Wshadow -Wall -fpic -fpie -fno-common -Wfloat-equal -Wextra -fvisibility=hidden
CFLAGS += -D_FORTIFY_SOURCE=2 -O2 -pipe
#规范建议：在Debug版本中实施，Release版本不实施
#CFLAGS += -ftrapv

CFLAGS += -I$(CURDIR)/../../libboundscheck/include
CFLAGS += -I$(CURDIR)/../include
CFLAGS += -D_GNU_SOURCE
$(warning $(CURDIR))
LDFLAGS := -Wl,-z,relro,-z,now -Wl,-z,noexecstack -pie -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined
LIBPATH += -L$(CURDIR)/../../libboundscheck

SAMPLE_OBJ := socchannel_main.o socchannel_host.o socchannel_host_comm.o

DEPEND_LIBS := -lpthread -lboundscheck
.PHONY: all clean
all:$(SAMPLE_IMAGES)

$(SAMPLE_IMAGES): $(SAMPLE_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH) $(DEPEND_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	-@rm -f $(SAMPLE_IMAGES)
	-@rm -f $(SAMPLE_OBJ)
