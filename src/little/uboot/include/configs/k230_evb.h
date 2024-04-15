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

#if CONFIG_CMD_DFU
#define DEFAULT_BOOTCMD_ENV "bootcmd=k230_dfu; \0"
#else
#define DEFAULT_BOOTCMD_ENV "bootcmd=k230_boot auto auto_boot; \0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"dtb_addr=0xa000000 \0" \
	"fdt_high=0xa100000\0" \
	"kernel_addr=0xc100000\0" \
	"ramdisk_addr=0xa100000\0" \
	"ipaddr=10.99.105.44\0" \
	"serverip=10.10.1.94\0" \
	"gatewayip=10.99.105.254\0" \
	"netmask=255.255.255.0\0" \
	"console_port=console=ttyS1,115200\0" \
	"usb_load=usb start; dhcp; tftp $ramdisk_addr jiangxiangbing/rtt_systems.bin; k230_boot mem $ramdisk_addr 0x$filesize; tftp $kernel_addr jiangxiangbing/fw_payload.img;tftp $ramdisk_addr jiangxiangbing/rootfs-final.cpio.gz;tftp $dtb_addr jiangxiangbing/k230.dtb\0" \
	"bootcmd_usb=run usb_load; bootm $kernel_addr - $dtb_addr \0" \
	"bootcmd_baremetal= mmc dev 1; mmc read 0 0x5000 0xa000; boot_baremetal 1 0 1400000;\0" \
	DEFAULT_BOOTCMD_ENV \
	"upspiuboot=usb start; dhcp;  tftp 0xc100000 10.10.1.94:wjx/u-boot.img && sf probe 0:0;sf erase 0x80000 0x180000; sf update  0x$fileaddr 0x80000  0x$filesize; \0" \
	"upspiimg=usb start; dhcp;  tftp 0x9000000 10.10.1.94:wjx/sysimage-spinor32m.img;sf probe 0:0;sf erase 0 0x2000000;sf write   0x$fileaddr  0 0x$filesize; \0" \
	"upsduboot=usb start; dhcp;  tftp 0xc100000 10.10.1.94:wjx/u-boot.img && mmc dev 1; mmc write  0x$fileaddr 0x1000  0xc00; \0" \
	"upsdimg=usb start; dhcp;  tftp 0x9000000 10.10.1.94:wjx/sysimage-sdcard.img.gz;gzwrite mmc 1 0x$fileaddr  0x$filesize; \0" \
	"\0"
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_PANIC_ON_RAW_IMAGE
#endif 

#endif /* __CONFIG_H */
