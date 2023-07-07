#程序版本号
VERSION = 1.0.0     

CROSS_COMPILE = arm-linux-musleabi-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++


# project 根路径
PROJECT_DIR := $(shell pwd)

# userapps 根路径
UROOT_DIR = $(PROJECT_DIR)/../..

# rt-thread 路径
RT_DIR = $(UROOT_DIR)/sdk/rt-thread
INC_DIR =$(UROOT_DIR)/sdk/rt-thread/include
LIB_DIR = ${UROOT_DIR}/sdk/rt-thread/lib


# 配置编译参数
CFLAGS = -march=armv7-a -marm -msoft-float -D__RTTHREAD__ -Wall -O0 -g -gdwarf-2 -n --static

# 加入头文件搜索路径
CFLAGS += -I. -I$(UROOT_DIR) -I$(PROJECT_DIR) -I$(RT_DIR)/components/dfs -I$(RT_DIR)/components/drivers -I$(RT_DIR)/components/finsh -I$(RT_DIR)/components/net -I$(RT_DIR)/components/net/netdev -I$(RT_DIR)/components/net/arpa -I${INC_DIR} -I${INC_DIR}/libc -I${INC_DIR}/sys

# 加入链接文件
LDFLAGS = -march=armv7-a -marm -msoft-float -T ${UROOT_DIR}/linker_scripts/arm/cortex-a/link.lds

# 加入库文件
LDFLAGS += -L$(LIB_DIR) -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive -n --static -Wl,--start-group -lrtthread -Wl,--end-group 

default:
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(LDFLAGS) main.o -o ${TARGET}.elf

clean:
	@rm *.o *.elf

.PHONY: default clean


