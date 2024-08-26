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

void ddr_init_800(void);
void ddr_init_1600(void);
void ddr_init_2133(void);
void ddr_init_1066(void);
void ddr4_init_3200(void);
void ddr_init_2667(void);
void sip_ddr_init_3200_have_wodt(void);
void sip_ddr_init_3200_have_all_odt(void);
void pi_ddr_init_2133(void);
void sip_ddr_init_1600(void);
void sip_ddr_init_2667(void);
void canmv_01studio_ddr_init_2133(void);

int ddr_init_training(void)
{
	#if defined(CONFIG_TARGET_K230_FPGA)
		return 0; //fpga not need init;
	#endif

	if( (readl((const volatile void __iomem *)0x980001bcULL) & 0x1 ) != 0 ){
		return 0; //have init ,not need reinit;
	}

	#ifdef CONFIG_LPDDR3_800
		printf("CONFIG_LPDDR3_800 \n");
		ddr_init_800();
	#elif defined(CONFIG_LPDDR3_1600)
		printf("CONFIG_LPDDR3_1600 \n");
		ddr_init_1600();
	#elif defined(CONFIG_LPDDR3_2133)
		//printf("CONFIG_LPDDR3_2133 \n");
		ddr_init_2133();
	#elif  defined(CONFIG_LPDDR4_1066)
		printf("CONFIG_LPDDR4_1066 \n");
		ddr_init_1066();
	#elif  defined(CONFIG_LPDDR4_2667)
		printf("CONFIG_LPDDR4_2667 \n");
		ddr_init_2667();
	#elif  defined(CONFIG_LPDDR4_3200)
		printf("CONFIG_LPDDR4_3200 \n");
		ddr4_init_3200();
	#elif  defined(CONFIG_SIPLP4_3200_WODT)
		//printf("CONFIG_SIPLP4_3200_WODT \n");
		sip_ddr_init_3200_have_wodt();
	#elif  defined(CONFIG_SIPLP4_3200_WALLODT)
		sip_ddr_init_3200_have_all_odt();
	#elif defined(CONFIG_CANMV_LPDDR3_2133)
		pi_ddr_init_2133();
    #elif defined(CONFIG_CANMV_01STUDIO_LPDDR3_2133)
		canmv_01studio_ddr_init_2133();
    #elif defined(CONFIG_SIPLP4_1600)
		sip_ddr_init_1600();
	#elif defined(CONFIG_SIPLP4_2667)
		change_pll_2660();
		sip_ddr_init_2667();
	#elif  defined(CONFIG_CANMV_V3_LPDDR4_2667)
		printf("CONFIG_LPDDR4_2667 \n");
		ddr_init_2667();
	#endif

	return 0;
}

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
