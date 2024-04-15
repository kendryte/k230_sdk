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
// #include "sdk_autoconf.h"
// #include "k230_board_common.h"
#include <env_internal.h>
#include <linux/delay.h>
#include <dm.h>

sysctl_boot_mode_e sysctl_boot_get_boot_mode(void)
{
	return SYSCTL_BOOT_SDIO1;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "kendryte,k230_canmv_v2");
	if (ofnode_valid(node)) {
#define SDHCI_EMMC_BASE     0x91580000
#define SDHCI_EMMC_CTRL_R   0x52C
#define EMMC_RST_N_OE       3
#define EMMC_RST_N          2
        u32 wifi_regon_ctrl = readl((void*)(SDHCI_EMMC_BASE + SDHCI_EMMC_CTRL_R));
        wifi_regon_ctrl |= (1<<EMMC_RST_N_OE);
        wifi_regon_ctrl &= ~(1<<EMMC_RST_N);
        mdelay(10);
        wifi_regon_ctrl |= (1<<EMMC_RST_N);
    }
    
    node = ofnode_by_compatible(ofnode_null(), "kendryte,k230_canmv");
	if (ofnode_valid(node)) {
        u32 wifi_regon_gpio1_dir = readl((void*)(GPIO_BASE_ADDR0 + 0x4));
        wifi_regon_gpio1_dir |= 1 << 1;
        writel(wifi_regon_gpio1_dir, (void*)(GPIO_BASE_ADDR0 + 0x4));

        // reset gpio1 -> WIFI REGON
        u32 wifi_regon_gpio1_data = readl((void*)(GPIO_BASE_ADDR0 + 0x0));
        wifi_regon_gpio1_data &= ~(1 << 1);
        writel(wifi_regon_gpio1_data, (void*)(GPIO_BASE_ADDR0 + 0x0));
        mdelay(10);
        // reset gpio1 -> WIFI REGON
        wifi_regon_gpio1_data |= 1 << 1;
        writel(wifi_regon_gpio1_data, (void*)(GPIO_BASE_ADDR0 + 0x0));
    }

    return 0;
}
#endif
