#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#       config
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CURDIR ?= $(shell pwd)
SDK_DIR := $(CURDIR)
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#       config
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#       TOOL CHAINS
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AR=${CROSS_COMPILE}ar
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
CPP=${CROSS_COMPILE}cpp
CC=${CROSS_COMPILE}gcc
CXX=${CROSS_COMPILE}g++
NM=${CROSS_COMPILE}nm
STRIP=${CROSS_COMPILE}strip
OBJCOPY=${CROSS_COMPILE}objcopy
OBJDUMP=${CROSS_COMPILE}objdump
CFG_SOC_BASE_ENV+=" AR AS LD CPP CC NM STRIP OBJCOPY OBJDUMP "
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#       COMPILE TOOLS
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CFG_SOC_BASE_ENV+=" CROSS_COMPILE "
