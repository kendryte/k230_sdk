/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <init.h>
#include <linux/sizes.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;
void ddr_init_800(void);
void ddr_init_1600(void);
void ddr_init_2133(void);
void ddr_init_1066(void);
void ddr4_init_3200(void);
void ddr_init_2667(void);
void sip_ddr_init_3200_have_wodt(void);
void sip_ddr_init_3200_have_all_odt(void);
void pi_ddr_init_2133(void);

__weak int ddr_init_training(void)
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
	#endif 	

	return 0;
}

int dram_init(void)
{
	//ddr_init_training();	
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_64BIT
	/*
	 * Ensure that we run from first 4GB so that all
	 * addresses used by U-Boot are 32bit addresses.
	 *
	 * This in-turn ensures that 32bit DMA capable
	 * devices work fine because DMA mapping APIs will
	 * provide 32bit DMA addresses only.
	 */
	if (gd->ram_top > SZ_4G)
		return SZ_4G;
#endif
	return gd->ram_top;
}
