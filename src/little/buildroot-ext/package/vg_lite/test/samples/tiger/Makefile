include ../../Makefile.def

OUT = ../../bin
NAME = tiger
SOURCES = tiger.c

LIBS += vg_lite vg_lite_util
CFLAGS += -I$(SDK_DIR)/inc -I../util
LDFLAGS += -L$(SDK_DIR)/drivers -L../../lib $(addprefix -l,$(LIBS))

all : $(OUT)/$(NAME)

clean :
	@rm -f $(OUT)/$(NAME)

$(OUT) :
	@mkdir $(OUT)

$(OUT)/libvg_lite_util.so :
	@make -C ../util

$(OUT)/$(NAME) : $(OUT) $(SOURCES) $(OUT)/libvg_lite_util.so
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
