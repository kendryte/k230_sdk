include ../../Makefile.def

OUT = ../../lib
NAME = libvg_lite_util.so
SOURCES = \
	load_png.c save_png.c fbdev.c vg_lite_util.c \
	$(wildcard libpng-1.6.1/*.c) \
	$(wildcard zlib-1.2.7/*.c)

CFLAGS += -I$(SDK_DIR)/inc -I.
CFLAGS += -Ilibpng-1.6.1 -Izlib-1.2.7
CFLAGS += -DHAVE_FBDEV

LIBS += vg_lite m
LDFLAGS += -L$(SDK_DIR)/drivers -L$(OUT) $(addprefix -l,$(LIBS))
LDFLAGS += -fPIC -shared  -Wl,-z,defs

all : $(OUT)/$(NAME)

clean :
	@rm -f $(OUT)/$(NAME)

$(OUT) :
	@mkdir $(OUT)

$(OUT)/$(NAME) : $(OUT) $(SOURCES)
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
