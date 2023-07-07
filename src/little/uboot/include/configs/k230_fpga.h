/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_INIT_RAM_ADDR 0x80300000
#define CONFIG_SYS_INIT_RAM_SIZE 0x100000

#define CONFIG_SYS_CACHELINE_SIZE   64

#define CONFIG_SYS_NS16550_MEM32
#define DWC2_UTMI_WIDTH 16

#define CONFIG_EXTRA_ENV_SETTINGS \
	"opensbi_addr=0x0\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"dtb_addr=0x20000000\0" \
	"kernel_addr=0x00200000\0" \
	"avail_addr=0x10000000\0" \
	"boot_vector=0\0" \
	"ipaddr=10.99.105.22\0" \
	"serverip=10.99.105.4\0" \
	"console_port=console=ttyS1,115200\0" \
	"set_bootargs=root=/dev/nfs rw nfsroot=10.99.105.4:/home/canaan/Bingo/rootfs ip=10.99.105.22:10.99.105.4:10.99.105.254:255.255.255.0:k230:eth0:off console=ttyS0,115200n8 debug loglevel=7\0" \
	"bootcmd_load=usb start; tftp 0x2000000 bootm-sbi.img;tftp 0x4000000 rootfs.cpio.gz;tftp 0x3f00000 hw.dtb\0" \
	"bootcmd_usb=run bootcmd_load; bootm 0x2000000 - 0x3f00000 \0" 	\
	"bootcmd_sd= setenv bootargs root=/dev/mmcblk0p3 loglevel=8 rw rootdelay=4   rootfstype=ext2 console=ttyS0,115200  crashkernel=256M-:128M earlycon=sbi; \
			mmc read 0x100000 0xf000 0xa000 ; k230_boot mem 0x100000 0x1000000 ; \0" \
	"bootcmd=run bootcmd_sd \0" \
	"\0"
#ifdef CONFIG_SPL_BUILD
#define CONFIG_MALLOC_F_ADDR          0x80200000
#define CONFIG_SPL_PANIC_ON_RAW_IMAGE
#endif 
#define CONFIG_DDR_CLK_FREQ 2133000000
#endif /* __CONFIG_H */
