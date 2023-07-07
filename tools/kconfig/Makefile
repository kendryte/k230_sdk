# ===========================================================================
ifndef Q
ifeq ("$(origin V)", "command line")
  Q =
else  
  Q = @
endif
endif
#++++++++++
cur_cc   = gcc
cur_cxx  = g++
cur_cflags  := -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
cur_cxxflags := -O2
cur_dir = $(CURDIR)
#++++++++++
cur_shell := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)
# ===========================================================================
PHONY := all clean install uninstall
all: mconf
#all: conf mconf dochecklxdialog
# ===========================================================================
# lxdialog stuff
check-lxdialog  := $(cur_dir)/lxdialog/check-lxdialog.sh

cur_extracflags = $(shell $(cur_shell) $(check-lxdialog) -ccflags)
cur_loadlibs    = $(shell $(cur_shell) $(check-lxdialog) -ldflags $(cur_cc))
cur_extracflags += -DLOCALE
cur_extracflags += $(shell $(cur_shell) $(cur_dir)/check.sh $(cur_cc) $(cur_cflags))

dochecklxdialog:
	$(Q)$(cur_shell) $(check-lxdialog) -check $(cur_cc) $(cur_extracflags) $(cur_loadlibs)

# ===========================================================================
# Shared Makefile for the various kconfig executables:
# conf:	  Used for defconfig, oldconfig and related targets
# mconf:  Used for the menuconfig target
#         Utilizes the lxdialog package
# object files used by all kconfig flavours

lxdialog := lxdialog/checklist.c lxdialog/util.c lxdialog/inputbox.c
lxdialog += lxdialog/textbox.c lxdialog/yesno.c lxdialog/menubox.c

conf-objs     := conf.c  zconf.tab.c
mconf-objs    := mconf.c zconf.tab.c  $(lxdialog)

mconf: $(mconf-objs)
	$(Q)$(cur_cc) -o $@ $^  $(cur_cflags) $(cur_extracflags) $(cur_loadlibs)
	@echo "make mconf over !"

conf: $(conf-objs)
	$(Q)$(cur_cc) -o $@ $^  $(cur_cflags) $(cur_extracflags) $(cur_loadlibs)

zconf.tab.c: zconf.lex.c zconf.hash.c
	cat zconf.tab.c_shipped > zconf.tab.c
zconf.lex.c :
	cat zconf.lex.c_shipped > zconf.lex.c
zconf.hash.c: 
	cat zconf.hash.c_shipped > zconf.hash.c

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ifndef RCS_FIND_IGNORE
RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS -o -name .pc -o -name .hg -o -name .git \) -prune -o
endif

clean:	
	$(Q)rm -rf conf
	$(Q)rm -rf mconf
	$(Q)rm -rf zconf.tab.c zconf.lex.c zconf.hash.c
	$(Q)find . $(RCS_FIND_IGNORE) \
		\( -name '*.o'  \
		-o -name '*.cmd' \
		-o -name '.*.cmd' \
		-o -name '.*.o.cmd'\
		 \) -type f -print | xargs rm -f
	@echo "clean over !"
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
install: all
	@echo "install over !"
uninstall: 
	rm -rf conf
	rm -rf mconf
	@echo "uninstall over !"
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# ===========================================================================

