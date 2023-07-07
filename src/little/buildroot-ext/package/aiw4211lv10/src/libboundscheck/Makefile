ifeq ($(CFG_SOC_EXPORT_FLAG),)
SDK_DIR = $(shell cd $(CURDIR)/.. && /bin/pwd)
include $(SDK_DIR)/base.mak
endif

#CC?=gcc

OPTION  = -fPIC
OPTION += -fstack-protector-all
OPTION += -D_FORTIFY_SOURCE=2 -O2
OPTION += -Wformat=2 -Wfloat-equal -Wshadow
OPTION += -Wconversion
OPTION += -Wformat-security
OPTION += -Wextra
OPTION += --param ssp-buffer-size=4
OPTION += -Warray-bounds
OPTION += -Wpointer-arith
OPTION += -Wcast-qual
OPTION += -Wstrict-prototypes
OPTION += -Wmissing-prototypes
OPTION += -Wstrict-overflow=1
OPTION += -Wstrict-aliasing=2
OPTION += -Wswitch -Wswitch-default

LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all

CFLAG   =  -Wall -DNDEBUG -O2 $(OPTION) $(LD_FLAG)

SOURCES=$(wildcard src/*.c)

OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

.PHONY: all clean

LIBS := libboundscheck
CFLAG += -Iinclude
LD_FLAG = -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all

all: $(LIBS)

$(LIBS): $(OBJECTS)
	$(AR) -rcs $@.a $^

.c.o:
	@mkdir -p obj
	$(CC) -c $< $(CFLAG) -o src/$(patsubst %.c,%.o,$(notdir $<))

clean:
	-rm -rf obj lib
	rm -rf src/*.o
	rm -rf ./*.a
	@echo "clean up"
