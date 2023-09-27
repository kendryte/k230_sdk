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
#include <asm/asm.h>
#include <asm/io.h>
#include <asm/types.h>
#include <lmb.h>
#include <cpu_func.h>
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <image.h>
#include <gzip.h>
#include <asm/spl.h>
#include "sysctl.h"

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
#include "sdk_autoconf.h"
#include "k230_board_common.h"
#include <env_internal.h>

//weak
int mmc_get_env_dev(void)
{
    int ret = 0;
    if(g_bootmod ==  SYSCTL_BOOT_SDIO1)
        ret = 1;
    return ret;
}
//weak
enum env_location arch_env_get_location(enum env_operation op, int prio)
{
    if(0 != prio){
        return ENVL_UNKNOWN;
    }

    if(g_bootmod ==  SYSCTL_BOOT_NORFLASH){
        return ENVL_SPI_FLASH;
    }
    if(g_bootmod ==  SYSCTL_BOOT_NANDFLASH){
        return ENVL_NAND;
    }
	return ENVL_MMC;
}
#ifndef CONFIG_SPL_BUILD
int board_early_init_f(void)
{
    g_bootmod = sysctl_boot_get_boot_mode(); //init  g_bootmod
    return 0;
}

__weak int board_init(void)
{
	#define USB_IDPULLUP0 		(1<<4)
	#define USB_DMPULLDOWN0 	(1<<8)
	#define USB_DPPULLDOWN0 	(1<<9)

    u32 usb0_test_ctl3 = readl((void*)USB0_TEST_CTL3);
	u32 usb1_test_ctl3 = readl((void*)USB1_TEST_CTL3);

	usb0_test_ctl3 |= USB_IDPULLUP0;
	usb1_test_ctl3 |= USB_IDPULLUP0;

	writel(usb0_test_ctl3, (void*)USB0_TEST_CTL3);
	writel(usb1_test_ctl3, (void*)USB1_TEST_CTL3);

    #define SD_HOST_REG_VOL_STABLE      (1<<4)
    #define SD_CARD_WRITE_PROT          (1<<6)
    u32 sd0_ctrl = readl((void*)SD0_CTRL);
    sd0_ctrl |= SD_HOST_REG_VOL_STABLE | SD_CARD_WRITE_PROT;
    writel(sd0_ctrl, (void*)SD0_CTRL);
	return 0;
}

static int k230_boot_prepare_args(int argc, char *const argv[], ulong buff,
                            en_boot_sys_t *sys, sysctl_boot_mode_e *bootmod)
{
    ulong add_tmp ,len;

    if(argc < 3)
        return CMD_RET_USAGE;

    if(!strcmp(argv[1], "mem")) {
        if(argc < 4)
            return CMD_RET_USAGE;
        add_tmp = simple_strtoul(argv[2],NULL, 0);
        len = simple_strtoul(argv[3],NULL, 0);
        if(add_tmp != buff){
            memmove((void *)buff, (void *)add_tmp, len);
        }
        *sys = BOOT_SYS_ADDR;
        return 0;
    }else  if (!strcmp(argv[1], "sd"))
        *bootmod=SYSCTL_BOOT_SDIO1;
    else if (!strcmp(argv[1], "emmc"))
        *bootmod=SYSCTL_BOOT_SDIO0;
    else if (!strcmp(argv[1], "spinor"))
        *bootmod=SYSCTL_BOOT_NORFLASH;
    else if (!strcmp(argv[1], "spinand"))
        *bootmod=SYSCTL_BOOT_NANDFLASH;
    else if (!strcmp(argv[1], "auto"))
        *bootmod=sysctl_boot_get_boot_mode();


    if(!strcmp(argv[2], "rtt"))
        *sys = BOOT_SYS_RTT;
    else if (!strcmp(argv[2], "linux"))
        *sys=BOOT_SYS_LINUX;
    else if (!strcmp(argv[2], "qbc"))
        *sys=BOOT_QUICK_BOOT_CFG;
    else if (!strcmp(argv[2], "fdb"))
        *sys=BOOT_FACE_DB;
    else if (!strcmp(argv[2], "sensor"))
        *sys=BOOT_SENSOR_CFG;
    else if (!strcmp(argv[2], "ai"))
        *sys=BOOT_AI_MODE;
    else if (!strcmp(argv[2], "speckle"))
        *sys=BOOT_SPECKLE;
    else if (!strcmp(argv[2], "rtapp"))
        *sys=BOOT_RTAPP;
    else if (!strcmp(argv[2], "uboot"))
        *sys=BOOT_SYS_UBOOT;
    else if (!strcmp(argv[2], "auto_boot"))
        *sys=BOOT_SYS_AUTO;


    return 0;

}


/**
 * @brief
 *
 * @param cmdtp
 * @param flag
 * @param argc
 * @param argv
 * @return int
 */
static int do_k230_boot(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
    int ret = 0;
    ulong cipher_addr = CONFIG_CIPHER_ADDR; //加载地址、密文
    en_boot_sys_t sys;
    sysctl_boot_mode_e bootmod = g_bootmod;

    ret = k230_boot_prepare_args(argc, argv, cipher_addr,&sys, &bootmod);
    if(ret)
        return ret;
    g_bootmod = bootmod;
    if(sys == BOOT_SYS_ADDR)
        ret = k230_img_boot_sys_bin((firmware_head_s *) cipher_addr);
    else
        ret = k230_img_load_boot_sys(sys);
    return ret;
}

#define K230_BOOT_HELP  " <auto|sd|emmc|spinor|spinand|mem> <auto_boot|rtt|linux|qbc|fdb|sensor|ai|speckle|rtapp|uboot|addr> [len]\n" \
                        "qbc---quick boot cfg\n" \
                        "fdb---face database\n" \
                        "sensor---sensor cfg\n" \
                        "ai---ai mode cfg\n" \
                        "speckle---speckle cfg\n" \
                        "rtapp---rtt app\n" \
                        "auto_boot---auto boot\n" \
                        "uboot---boot uboot\n"
/*
boot sd/mmc/spinor/spinand/mem  add
k230_boot auto rtt ;k230_boot auto linux;
先实现从sd启动吧；
*/
U_BOOT_CMD_COMPLETE(
	k230_boot, 6, 0, do_k230_boot,
	NULL,
	K230_BOOT_HELP, NULL
);
#endif




