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

#define  BOOT_REG_BASE   (0x91102000)
#define  PLL0_CFG0                           (BOOT_REG_BASE + 0x0  )
#define  PLL0_CFG1                           (BOOT_REG_BASE + 0x4  )
#define  PLL0_CTL                            (BOOT_REG_BASE + 0x8  )
#define  PLL0_STAT                           (BOOT_REG_BASE + 0xC  )
#define  PLL1_CFG0                           (BOOT_REG_BASE + 0x10 )
#define  PLL1_CFG1                           (BOOT_REG_BASE + 0x14 )
#define  PLL1_CTL                            (BOOT_REG_BASE + 0x18 )
#define  PLL1_STAT                           (BOOT_REG_BASE + 0x1C )
#define  PLL2_CFG0                           (BOOT_REG_BASE + 0x20 )
#define  PLL2_CFG1                           (BOOT_REG_BASE + 0x24 )
#define  PLL2_CTL                            (BOOT_REG_BASE + 0x28 )
#define  PLL2_STAT                           (BOOT_REG_BASE + 0x2C )
#define  PLL3_CFG0                           (BOOT_REG_BASE + 0x30 )
#define  PLL3_CFG1                           (BOOT_REG_BASE + 0x34 )
#define  PLL3_CTL                            (BOOT_REG_BASE + 0x38 )
#define  PLL3_STAT                           (BOOT_REG_BASE + 0x3C )

static void pd_pll(uint32_t pll_ctl,uint32_t pll_stat)
{
int rdata;
   writel(0x10001,pll_ctl);
   rdata=readl(pll_stat);
   while( (rdata&0x30) != 0x0){
        rdata=readl(pll_stat);
   }
}

static void init_pll(uint32_t pll_ctl,uint32_t pll_stat)
{
   int rdata;
   writel(0x20002,pll_ctl);
   rdata=readl(pll_stat);
   while( (rdata & 0x30) != 0x20){
        rdata=readl(pll_stat);
   }
}

static uint32_t cfg_pll(int fb_div,int ref_div,int out_div,int pllx_cfg0,int pllx_cfg1,int pllx_ctl,int pllx_stat)
{
  int pll_sta;
  int wdata,rdata;
  pd_pll(pllx_ctl,pllx_stat);
  writel(( (fb_div/4) | 0x20000),pllx_cfg1 ); //for minimum long term jitter
  writel(( (fb_div & 0x1fff) | ( (ref_div & 0x3f) << 16 ) | ( (out_div & 0xf) << 24) ),pllx_cfg0 );
  init_pll(pllx_ctl,pllx_stat);
}

uint32_t cfg_pll_npd(int fb_div,int ref_div,int out_div,int pllx_cfg0,int pllx_cfg1,int pllx_ctl,int pllx_stat)
{
  int pll_sta;
  int wdata,rdata;
  writel(( (fb_div/2) | 0x20000),pllx_cfg1 );
  writel(( (fb_div & 0x1fff) | ( (ref_div & 0x3f) << 16 ) | ( (out_div & 0xf) << 24) ),pllx_cfg0 );
  init_pll(pllx_ctl,pllx_stat);

}

static int change_pll_2660(void)
{
    /* enable cache */
    //Note: The recommended value for BWADJ is FBK_DIV/2.Valid values range from 0 to 0xFFF.
    //To minimize long-term jitter, using NB=NF/4 is better. NB = BWADJ[11:0] + 1,
    //So, BWADJ=(NB-1)=[NF/2 -1] or (NF/4 -1)--minimize long term jitter

       cfg_pll(     // 1860Mhz
            110,  //fb_div=NF-1
            0,    //ref_div=NR-1
            0,    //out_div=OD-1
            PLL2_CFG0,
            PLL2_CFG1,
            PLL2_CTL,
            PLL2_STAT
            );

    *(uint32_t*)(0x91100060) =  0x800043fe;//switch ddrc_core_clk source to pll2div4

    udelay(50);
}

#define PWR_REG_BASE    (0x91103000)
#define PMU_PWR_LPI_CTL                     (PWR_REG_BASE + 0x158) 
#define SSYS_CTL_GPIO_CTL                   (PWR_REG_BASE + 0x168) 
#define SSYS_CTL_GPIO_EN0                   (PWR_REG_BASE + 0x170) 
#define SSYS_CTL_GPIO_EN1                   (PWR_REG_BASE + 0x174)
#define SHRM_PWR_LPI_CTL                    (PWR_REG_BASE + 0x68 ) 

#define BOOT_REG_BASE   (0x91102000)
#define SOC_SLEEP_CTL                       (BOOT_REG_BASE + 0x6C ) 
#define SOC_SLEEP_MASK                      (BOOT_REG_BASE + 0x118) 
//

#define PMU_REG_BASE                        (0x91000000)
#define PMU_INT0_TO_CTRL                    (PMU_REG_BASE + 0x0040)
#define PMU_INT1_TO_CTRL                    (PMU_REG_BASE + 0x0044)
#define PMU_INT_TO_CPU                      (PMU_REG_BASE + 0x0048)
#define PMU_INT_DETECT_EN                   (PMU_REG_BASE + 0x004C)
#define PMU_INT_DETECT_CLR                  (PMU_REG_BASE + 0x0054)
#define PMU_SYSCTRL_REG                     (PMU_REG_BASE + 0x0078)
#define PMU_NORMAL_TIMER_VAL                (PMU_REG_BASE + 0x00A0)

#define RTC_REG_BASE                   0x91000c00
#define DATE                           RTC_REG_BASE         + 0x0
#define TIME                           DATE                 + 0x4
#define ALARM_DATE                     TIME                 + 0x4
#define ALARM_TIME                     ALARM_DATE           + 0x4
#define COUNT                          ALARM_TIME           + 0x4
#define INT_CTRL                       COUNT                + 0x4

uint32_t alarm_test(
        uint8_t  century,
        uint8_t  year,
        uint8_t  month,
        uint8_t  day,
        uint8_t  week,
        uint8_t  hour  ,
        uint8_t  minute,
        uint8_t  second,
        
        uint8_t  alarm_century,
        uint8_t  alarm_year,
        uint8_t  alarm_month,
        uint8_t  alarm_day,
        uint8_t  alarm_week,
        uint8_t  alarm_hour,
        uint8_t  alarm_minute,
        uint8_t  alarm_second, 
        uint8_t  alarm_intr_type
        )
{
        uint32_t wdata = 0; 
        uint32_t rdata = 0;
        uint32_t addr;

       uint8_t    leap_year =0;

        writel(0xff<<16, COUNT);
        //set alarm date annd time 
        wdata = ( 0x1f & alarm_day      << 0 ) ; //week
        wdata |= ((0xf  & alarm_month)   << 8 ) ; //day
        wdata |= ((0x7f & alarm_year )   << 16) ; //month
        wdata |= ((0x1  & leap_year  )   << 23) ; //year
        wdata |= ((0x7f & alarm_century) << 24) ;
        writel(wdata,ALARM_DATE);

        wdata = ((0x7  & alarm_week)  << 24 )  ;
        wdata |= ((0x1f & alarm_hour)  << 16 )  ; //h
        wdata |= ((0x3f & alarm_minute) << 8  )  ; //m
        wdata |= ((0x3f & alarm_second) << 0  )  ; //s
        writel(wdata,ALARM_TIME);

        //set date and time 
        wdata = ((0x1f & day)     << 0 ) ; //week
        wdata |= ((0xf  & month)   << 8 ) ; //day
        wdata |= ((0x7f & year )   << 16) ; //month
        wdata |= ((0x1  & leap_year)<<23) ; //year
        wdata |= ((0x7f & century) << 24) ;
        writel(wdata,DATE);

        wdata = ((0x7  & week  ) << 24);
        wdata |= ((0x1f & hour  ) << 16); //h
        wdata |= ((0x3f & minute) << 8 ); //m
        wdata |= ((0x3f & second) << 0 ); //s
        writel(wdata,TIME);

        //enable alarm_int
        wdata =  0x1      ;
        wdata |=  0x1 << 1 ;
        wdata |=  0x1 << 16; //int mode  
        wdata |=  ((0x7f & alarm_intr_type) << 24);
        writel(wdata, INT_CTRL);

       rdata  = readl(INT_CTRL);
       rdata &= 0xfffffffe;
       rdata |=  1<<17;
       writel(rdata,INT_CTRL);
}

/*
    Fout = Fref * NF / NR / OD
    Fvco= Fref * NF / NR
    NF：pll*_fb_div[12:0] + 1
    NR: pll*_ref_div[5:0] + 1
    OD: pll*_out_div[3:0] + 1
 */
void change_pll0(void)
{
    uint32_t wdata,rdata;
    writel(0x0,PMU_PWR_LPI_CTL);
    udelay(500000);
    writel(0x0,SSYS_CTL_GPIO_EN0);
    writel(0x0,SSYS_CTL_GPIO_EN1);
    writel(1<<3,SSYS_CTL_GPIO_CTL);
    writel(0x4,PMU_INT_DETECT_EN);//en int6 from rtc alarm_int
    writel(0x0,PMU_INT0_TO_CTRL);
    writel(0x0,PMU_INT1_TO_CTRL);
    writel(0xffffffff,PMU_INT_DETECT_CLR);

    writel(0x4,PMU_INT_TO_CPU);//enable int6:rtc_alarm
    udelay(200);//wait pmu write_done
    writel(0xffffffff,SOC_SLEEP_MASK);//mask shrm&ddr&pmu

    writel(0x100000,SHRM_PWR_LPI_CTL);//ai_repair/shrm not repair

    // PLL0=1.536GHz
    cfg_pll_npd(0x7f, 0x0, 0x1, PLL0_CFG0, PLL0_CFG1, PLL0_CTL, PLL0_STAT);

    alarm_test(20,100,12,31,6,23,59,59,21,1,1,1,0,0,0,0,1);

    rdata=0x0;
    rdata=readl(SOC_SLEEP_CTL);
    rdata |= (1<<0);//sleep_req
    rdata |= (1<<16);//wen
    writel(rdata,SOC_SLEEP_CTL);

    udelay(100);
    printf("I am wakeup!\n");
    printf("SYS_CTL_INT2_RAW:0x%x SOC_WAKUP_SRC:0x%x \n", *(uint32_t *)0x911020b0, *(uint32_t *)0x91102078);

    writel(0x0,PMU_INT_DETECT_EN);
    writel(0x0,PMU_INT_TO_CPU);
    alarm_test(20,100,12,31,6,23,59,59,21,1,1,1,0,0,0,0,0);
}

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
    #elif defined(CONFIG_CANMV_01STUDIO_LPDDR3_2133)
		canmv_01studio_ddr_init_2133();
    #elif defined(CONFIG_SIPLP4_1600)
		sip_ddr_init_1600();
	#elif defined(CONFIG_SIPLP4_2667)
		change_pll_2660();
		sip_ddr_init_2667();
	#endif

	return 0;
}

int dram_init(void)
{
#ifndef CONFIG_SPL
	ddr_init_training();
#endif
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
