OUT = ../drivers
NAME = libvg_lite.so
SOURCES = vg_lite_dump.c \
          vg_lite.c vg_lite_path.c \
		  vg_lite_stroke.c \
		  vg_lite_matrix.c \
		  vg_lite_image.c \
		  ../VGLiteKernel/linux/vg_lite_ioctl.c \
		  ../VGLiteKernel/linux/vg_lite_os.c

CFLAGS += -I../inc -I../VGLiteKernel -I.
CFLAGS += -Wno-missing-braces -D_GNU_SOURCE -lpthread -lrt -g

LIBS += m
LDFLAGS += $(addprefix -l,$(LIBS))
LDFLAGS += -fPIC -shared -Wl,-z,defs

all : $(OUT)/$(NAME)

clean:
	rm -f $(OUT)/$(NAME)

install:
	cp $(OUT)/$(NAME) $(TARGET_DIR)/usr/lib/$(NAME)
	cp $(OUT)/$(NAME) $(STAGING_DIR)/usr/lib/$(NAME)
	cp ../inc/vg_lite.h $(STAGING_DIR)/usr/include/vg_lite.h

$(OUT):
	mkdir $(OUT)

$(OUT)/$(NAME) : $(OUT) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
