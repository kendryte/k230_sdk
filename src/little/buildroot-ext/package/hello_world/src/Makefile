CROSS_COMPILE:=riscv64-linux-

CC	:= $(CROSS_COMPILE)gcc
C++	:= $(CROSS_COMPILE)g++
OBJDUMP	:= $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
AR	:= $(CROSS_COMPILE)ar
AS	:= $(CROSS_COMPILE)as
LD	:= $(CROSS_COMPILE)gcc

ifeq ($(DEBUG),1)
	OPTIM   := -O0 -g3
else
	OPTIM   := -Os -g3
endif

ROOT = $(shell pwd)
PROG = $(shell pwd |sed 's/^\(.*\)[/]//' )

DIRS := $(shell find -type d | grep -v .settings |grep -v Debug)
INCLUDES := $(addprefix -I , $(DIRS))
SRCS := $(foreach dir,$(DIRS) , $(wildcard $(dir)/*.c) $(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.S) $(wildcard $(dir)/*.s))
OBJS := $(patsubst %.s,%.o,$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS}))))

### Verbosity control. Use 'make V=1' to get verbose builds.
V = 1
ifeq ($(V),1)
TRACE_CC  =
TRACE_C++ =
TRACE_LD  =
TRACE_AR  =
TRACE_AS  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_C++ = @echo "  C++      " $<
TRACE_LD  = @echo "  LD       " $@
TRACE_AR  = @echo "  AR       " $@
TRACE_AS  = @echo "  AS       " $<
Q=@
endif

# Common compilation options

CFLAGS = \
	-Wall $(OPTIM) $(INCLUDES) \
	-fomit-frame-pointer -fno-strict-aliasing -fno-builtin \
	-ffunction-sections -fdata-sections \
	-DCFG_MAKEFILE

CXXFLAGS = $(CFLAGS)

ASFLAGS = -D__ASSEMBLY__ $(CFLAGS) -c


LDFLAGS = $(OPTIM) -static -Wl,--gc-sections

LIBS	=

# Compilation rules
.SUFFIXES : %.o %.c %.cpp %.S

%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(CFLAGS) -o $@ $<

%.o: %.cpp
	$(TRACE_C++)
	$(Q)$(C++) -c -MMD $(CXXFLAGS) -o $@ $<

%.o: %.S
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(ASFLAGS) -o $@ $<

%.o: %.s
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(ASFLAGS) -o $@ $<


all: $(PROG)

$(PROG): $(OBJS)
	$(LD) -o $@.out $(LDFLAGS) $(OBJS) $(LIBS)
	$(OBJCOPY) -O binary -S $(PROG).out $(PROG).bin
	$(OBJDUMP) -d $(PROG).out > $(PROG).asm
	@echo Completed

clean :
	@rm -f $(OBJS) $(OBJS:.o=.d) $(OBJS:.o=.gcda) $(OBJS:.o=.gcno)
	@rm -f $(PROG).out
	@rm -f $(PROG).bin
	@rm -f $(PROG).asm