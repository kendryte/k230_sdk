include $(MPP_SRC_DIR)/userapps/src/mpp.mk
include $(MPP_SRC_DIR)/userapps/src/rt-smart.mk

CURRECT_DIR_NAME=$(shell basename `pwd`)
LOCAL_SRC_DIR = $(shell pwd)
LIB = $(MPP_SRC_DIR)/userapps/lib/lib$(CURRECT_DIR_NAME).a

LOCAL_CFLAGS = -I$(LOCAL_SRC_DIR)/

SRCS = $(wildcard $(LOCAL_SRC_DIR)/*.c)

OBJS = $(patsubst %.c,%.o,$(SRCS))

YMLS = $(wildcard $(LOCAL_SRC_DIR)/dewarp/*.yml)

DWCFGS = $(patsubst %.yml,%.bin,$(YMLS))

all: $(LIB) $(DWCFGS)
	@-rm -rf $(OBJS)
	rm -rf build
	mkdir -p $(RTSMART_SRC_DIR)/userapps/root/bin/
	cp $(PWD)/config/* $(RTSMART_SRC_DIR)/userapps/root/bin/
	cp $(PWD)/dewarp/*.bin $(RTSMART_SRC_DIR)/userapps/root/bin/
	echo "${PWD}/Makefile all"

$(OBJS): %.o : %.c
	@$(CC) $(CC_CFLAGS) $(LOCAL_CFLAGS) $(BSP_CFLGAS) $(RTSMART_CFLAGS) $(MPP_USER_CFLGAS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

k230dwmapgen:
	cmake -S $(LOCAL_SRC_DIR)/dewarp/k230dwmapgen -B build
	cmake --build build -j

$(DWCFGS): %.bin : %.yml k230dwmapgen
	./build/k230dwmapgen-dump < $< > $@

clean:
	echo "${PWD}/Makefile clean"
	-rm -rf $(LIB)
	-rm -f $(OBJS)
	-rm -rf build

.PHONY: all clean k230dwmapgen
