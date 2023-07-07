CASES = $(wildcard samples/*)

.PHONY: all install cases

all:
	$(MAKE) -C samples/util
	$(MAKE) -C samples/cube
	$(MAKE) -C samples/vglite_drm
	$(MAKE) -C samples/tiger
	$(MAKE) -C samples/imgIndex
	$(MAKE) -C samples/linearGrad

install:
	cp lib/libvg_lite_util.so $(TARGET_DIR)/usr/lib/libvg_lite_util.so
	cp lib/libvg_lite_util.so $(STAGING_DIR)/usr/lib/libvg_lite_util.so
	cp samples/util/vg_lite_util.h $(STAGING_DIR)/usr/include/vg_lite_util.h
	cp bin/* $(TARGET_DIR)/usr/bin

clean:
	@rm -rf bin
	@$(MAKE) -C samples/util clean
	@$(MAKE) -C samples/cube clean
	@$(MAKE) -C samples/vglite_drm clean
	@$(MAKE) -C samples/tiger clean
	@$(MAKE) -C samples/imgIndex clean
	@$(MAKE) -C samples/linearGrad clean
