export build_config := config.mk
export MODULE_DIR := ${PWD}

prepare:
	@echo "config file ${CFG}"
	chmod +x makeprepare.sh
	./makeprepare.sh ${CFG} ${PLATFORM}

all: prepare ipcm sharefs
	@echo "ipcm make all..."

ipcm: prepare
	@echo "ipcm making..."
	make -f do_make_module ipcm || exit 1
ipcm_clean:
	@echo "ipcm make clean"
	make -f do_make_module ipcm_clean

virt_tty: prepare
	@echo "virt_tty making..."
	find . -name "*.o*" | xargs rm -f
	make -f do_make_module virt_tty || exit 1
virt_tty_clean:
	@echo "virt_tty make clean"
	make -f do_make_module virt_tty_clean

sharefs: prepare
	@echo "sharefs making..."
	make -f do_make_module sharefs || exit 1
sharefs_clean:
	@echo "sharefs make clean"
	make -f do_make_module sharefs_clean

clean:
	@echo "make clean"
	find . -name "*.o*" | xargs rm -f
	find . -name "*.mod.c" | xargs rm -f
	find . -name "*.a" | xargs rm -f
	find . -name "*.ko*"| xargs rm -f
	find . -name "*mod.cmd"| xargs rm -f
	find . -name ".Module.symvers.cmd"| xargs rm -f
	find . -name "*.mod"| xargs rm -f

	-rm -fr out
	find . -name "*.tmp_versions" | xargs rm -rf
	find . -name "modules.order" | xargs rm -f
	find . -name "Module.symvers" | xargs rm -f
	chmod +x makeprepare.sh
	./makeprepare.sh clean
.PHONY: prepare all ipcm ipcm_clean clean

