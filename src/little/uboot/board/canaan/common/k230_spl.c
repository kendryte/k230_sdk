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
#include <asm-generic/sections.h>

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
#include <init.h>
#include "k230_board_common.h"
#include "mmc.h"
#include "sdk_autoconf.h"
//spl 相关代码
int quick_boot(void);

//weak
void board_boot_order(u32 *spl_boot_list)
{
    if(g_bootmod ==  SYSCTL_BOOT_SDIO0){
        spl_boot_list[0] = BOOT_DEVICE_MMC1;
        spl_boot_list[1] = BOOT_DEVICE_MMC2;
    }else  if(g_bootmod ==  SYSCTL_BOOT_NORFLASH){
        spl_boot_list[0] = BOOT_DEVICE_SPI;
        spl_boot_list[1] = BOOT_DEVICE_SPI;
    }else  if(g_bootmod ==  SYSCTL_BOOT_NANDFLASH){
        spl_boot_list[0] = BOOT_DEVICE_NAND;
        spl_boot_list[1] = BOOT_DEVICE_SPI;
    }else  if(g_bootmod ==  SYSCTL_BOOT_SDIO1){
        spl_boot_list[0] = BOOT_DEVICE_MMC2;
        spl_boot_list[1] = BOOT_DEVICE_MMC1;
    }
}

//weak
void spl_board_prepare_for_boot(void){
    cache_flush();
    icache_disable();
    dcache_disable();
    // csi_l2cache_flush_invalid();
    asm volatile(".long 0x0170000b\n":::"memory");
}

static void device_disable(void)
{
    uint32_t value;

    // disable ai power
    if (readl(0x9110302c) & 0x2)
        writel(0x30001, 0x91103028);
    // disable vpu power
    if (readl(0x91103080) & 0x2)
        writel(0x30001, 0x9110307c);
    // disable dpu power
    if (readl(0x9110310c) & 0x2)
        writel(0x30001, 0x91103108);
    // disable disp power
    if (readl(0x91103040) & 0x2)
        writel(0x30001, 0x9110303c);
    // check disable status
    value = 1000000;
    while ((!(readl(0x9110302c) & 0x1) || !(readl(0x91103080) & 0x1) ||
        !(readl(0x9110310c) & 0x1) || !(readl(0x91103040) & 0x1)) && value)
        value--;
    // disable ai clk
    value = readl(0x91100008);
    value &= ~((1 << 0));
    writel(value, 0x91100008);
    // disable vpu clk
    value = readl(0x9110000c);
    value &= ~((1 << 0));
    writel(value, 0x9110000c);
    // disable dpu clk
    value = readl(0x91100070);
    value &= ~((1 << 0));
    writel(value, 0x91100070);
    // disable mclk
    value = readl(0x9110006c);
    value &= ~((1 << 0) | (1 << 1) | (1 << 2));
    writel(value, 0x9110006c);
}

__weak void quick_boot_board_init(void)
{
	/* Nothing to do! */
}

//weak;
int spl_board_init_f(void)
{
    int ret = 0;

    device_disable();
    g_bootmod = sysctl_boot_get_boot_mode();

    record_boot_time_info_to_sram("ds");
    ddr_init_training();
    record_boot_time_info_to_sram("dd");
    /* Clear the BSS. */
    //record_boot_time_info_to_sram("bs");
    memset(__bss_start, 0, (ulong)&__bss_end - (ulong)__bss_start);
    //record_boot_time_info_to_sram("be");

   

    // /* load/boot image from boot device */
    //if(quick_boot() == 1){//默认非快起；
    if(quick_boot()){//默认快起
        quick_boot_board_init();

        //record_boot_time_info("ls");
        ret += k230_img_load_boot_sys(BOOT_SYS_AUTO);
    }
    
    ret = k230_img_load_boot_sys(BOOT_SYS_UBOOT);
    if(ret )
        printf("uboot boot failed\n");
    //while(1);
    //board_init_r(NULL, 0);
    return ret;
}

//1 快起 other：非快起
int quick_boot(void)
{
    int ret = 1 ;
    #if defined(CONFIG_LINUX_RUN_CORE_ID) && (CONFIG_LINUX_RUN_CORE_ID == 1)
    return 0; //非快起，uboot运行在大核core1;
    #endif 

    if((g_bootmod == SYSCTL_BOOT_SDIO0) || (g_bootmod == SYSCTL_BOOT_SDIO1)){
        if(mmc_init_device(mmc_get_env_dev()))
            return 0;//正常boot；
    }
        
    env_init();
    env_load();
    ret  = env_get_yesno("quick_boot");
    return ret;
}



