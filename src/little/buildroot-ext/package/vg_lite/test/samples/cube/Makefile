include ../../Makefile.def

OUT = ../../bin
NAME = vglite_cube
SOURCES := cube.c glMatrix.c ../drm.c
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

LIBS += vg_lite vg_lite_util
CFLAGS += -I$(SDK_DIR)/inc -I../util
LDFLAGS += -lm -L$(SDK_DIR)/drivers -L../../lib $(addprefix -l,$(LIBS))

all : $(OUT)/$(NAME)

clean :
	@rm -f $(OUT)/$(NAME) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)

$(OUT) :
	@mkdir $(OUT)

%.o : %.c
	$(CC) $(CFLAGS) $(DRM_CFLAGS) -c $< -o $@

$(OUT)/$(NAME) : $(OUT) $(OBJECTS)
	@echo $(CC) $(CFLAGS) $(DRM_CFLAGS) $(LDFLAGS) $(DRM_LDFLAGS) -o $@ $(OBJECTS)
	@$(CC) $(CFLAGS) $(DRM_CFLAGS) $(LDFLAGS) $(DRM_LDFLAGS) -o $@ $(OBJECTS)
