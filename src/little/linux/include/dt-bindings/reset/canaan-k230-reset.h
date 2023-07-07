/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef _DT_BINDINGS_CANAAN_K230_RESET_H_
#define _DT_BINDINGS_CANAAN_K230_RESET_H_

/* reset register offset */
#define K230_RESET_REG_OFFSET_SHIFT     16
#define K230_RESET_REG_OFFSET_MASK      0xffff0000

/* reset type */
#define K230_RESET_TYPE_SHIFT           14
#define K230_RESET_TYPE_MASK            0x0000c000

#define K230_RESET_TYPE_CPU             0
#define K230_RESET_TYPE_HW_AUTO_DONE    1
#define K230_RESET_TYPE_SW_SET_DONE     2

/* reset done bit */
#define K230_RESET_DONE_BIT_SHIFT       7
#define K230_RESET_DONE_BIT_MASK        0x00003F80

/* reset assert&deassert bit */
#define K230_RESET_ASSERT_BIT_SHIFT     0
#define K230_RESET_ASSERT_BIT_MASK      0x0000007F

/*****************************************************************
 *  寄存器中的done表示设置reset后读取该bit可以看到当前状态是否为done，
 *  这种情况属于hardware automatic done；而寄存器没有done但是有assert的
 *  bit表示需要software set done，如何set呢？通过配置assert。
 * 
 * 1. 寄存器中没有done，归类到K230_RESET_TYPE_SW_SET_DONE类型中
 * 2. 寄存器中done和reset都存在，归类到K230_RESET_TYPE_HW_AUTO_DONE中
 * //3. reset在一个寄存器中，done在另一个寄存器中，放到K230_RESET_TYPE_SPECIAL中
 * 3. reset在一个寄存器中，done在另一个寄存器中(只有一个，即cpu1_preset_req在0x4中，cpu1_prst_done在0xc中)，
 *    该寄存器用于调试，在底层驱动中不体现出来。
 * **************************************************************/

/**
 * K230_RESET_TYPE_CPU: 
 *        cpu0_reset, cpu1_reset
 *        > cpu0_reset: 1表示reset，自动清零(W1T)
 *        > cpu1_reset: 1表示reset，需要软件清零(RW)
 * 
 * K230_RESET_TYPE_HW_AUTO_DONE:
 *        ai_reset, vpu_reset, hs_reset, sdio_reset, usb_reset, spi_reset, sec_reset, dma_reset
 *        sram_reset, nonai2d_reset, ddr_reset, isp_reset, dpu_reset, disp_reset, gpu_reset, audio_reset
 *        > 所有的模块: 1表示reset，自动清零(W1T)
 * 
 * K230_RESET_TYPE_SW_SET_DONE:
 *        timer_reset, ls_reset, 
 *        cpu0_clush, cpu1_flush, sram_reset(shrm_apb), isp_reset(csi, vi, csi-dphy, sensor), spi2axi
 *        > timer_reset, ls_reset, isp_reset, sram_reset: 0表示reset，需要软件清零(RW)
 *        > cpu0_clush, cpu1_flush: 1表示reset，硬件自动清零(RWSC)
 *        > spi2axi: 1表示reset，需要软件清零(RW)
 * 
 */

/* cpu0 reset */
#define K230_RESET_CPU0_REG_OFFSET          0x4
#define K230_RESET_CPU0_TYPE                K230_RESET_TYPE_CPU
#define K230_RESET_CPU0_DONE_BIT            12
#define K230_RESET_CPU0_ASSERT_BIT          0

/* cpu1 reset */
#define K230_RESET_CPU1_REG_OFFSET          0xc
#define K230_RESET_CPU1_TYPE                K230_RESET_TYPE_CPU
#define K230_RESET_CPU1_DONE_BIT            12
#define K230_RESET_CPU1_ASSERT_BIT          0

/* cpu0_preset_req(cpu0_rst_done)、cpu1_preset_req(cpu1_rst_done)、tdi_preset_req(tdi_rst_done)用于调试 */
// /* cpu0 apb reset */
// #define K230_RESET_CPU0_APB_REG_OFFSET      0x4
// #define K230_RESET_CPU0_APB_TYPE            K230_RESET_TYPE_CPU
// #define K230_RESET_CPU0_APB_DONE_BIT        13
// #define K230_RESET_CPU0_APB_ASSERT_BIT      2

// /* cpu0 tdi reset */
// #define K230_RESET_CPU0_TDI_REG_OFFSET      0x4
// #define K230_RESET_CPU0_TDI_TYPE            K230_RESET_TYPE_CPU
// #define K230_RESET_CPU0_TDI_DONE_BIT        14
// #define K230_RESET_CPU0_TDI_ASSERT_BIT      1

// /* cpu0 reset cpu1, and cpu1 have done status. */
// /* cpu1 apb reset */
// #define K230_RESET_CPU1_APB_REG_OFFSET      0xc
// #define K230_RESET_CPU1_APB_TYPE            K230_RESET_TYPE_HW_AUTO_DONE
// #define K230_RESET_CPU1_APB_DONE_BIT        13
// #define K230_RESET_CPU1_APB_ASSERT_BIT      3         //0x4, need review

/* ai reset */
#define K230_RESET_AI_REG_OFFSET            0x14
#define K230_RESET_AI_TYPE                  K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_AI_DONE_BIT              31
#define K230_RESET_AI_ASSERT_BIT            0

/* vpu reset */
#define K230_RESET_VPU_REG_OFFSET           0x1c
#define K230_RESET_VPU_TYPE                 K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_VPU_DONE_BIT             31
#define K230_RESET_VPU_ASSERT_BIT           0

/* hs reset */
#define K230_RESET_HS_REG_OFFSET            0x2c
#define K230_RESET_HS_TYPE                  K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_HS_DONE_BIT              4
#define K230_RESET_HS_ASSERT_BIT            0

/* hs ahb reset */
#define K230_RESET_HS_AHB_REG_OFFSET        0x2c
#define K230_RESET_HS_AHB_TYPE              K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_HS_AHB_DONE_BIT          5
#define K230_RESET_HS_AHB_ASSERT_BIT        1

/* sdio0 reset */
#define K230_RESET_SDIO0_REG_OFFSET         0x34
#define K230_RESET_SDIO0_TYPE               K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SDIO0_DONE_BIT           28
#define K230_RESET_SDIO0_ASSERT_BIT         0

/* sdio1 reset */
#define K230_RESET_SDIO1_REG_OFFSET         0x34
#define K230_RESET_SDIO1_TYPE               K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SDIO1_DONE_BIT           29
#define K230_RESET_SDIO1_ASSERT_BIT         1

/* sdio axi reset */
#define K230_RESET_SDIO_AXI_REG_OFFSET      0x34
#define K230_RESET_SDIO_AXI_TYPE            K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SDIO_AXI_DONE_BIT        30
#define K230_RESET_SDIO_AXI_ASSERT_BIT      2

/* usb0 reset */
#define K230_RESET_USB0_REG_OFFSET          0x3c
#define K230_RESET_USB0_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_USB0_DONE_BIT            28
#define K230_RESET_USB0_ASSERT_BIT          0

/* usb1 reset */
#define K230_RESET_USB1_REG_OFFSET          0x3c
#define K230_RESET_USB1_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_USB1_DONE_BIT            29
#define K230_RESET_USB1_ASSERT_BIT          1

/* usb0：配置USB_RST_CTL-usb0_reset_req(bit[0])，会产生bit[28]和bit[30]
   usb1：配置USB_RST_CTL-usb1_reset_req(bit[1])，会产生bit[29]和bit[31]
 */
/* usb0 ahb reset */
#define K230_RESET_USB0_AHB_REG_OFFSET      0x3c
#define K230_RESET_USB0_AHB_TYPE            K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_USB0_AHB_DONE_BIT        30
#define K230_RESET_USB0_AHB_ASSERT_BIT      0

/* usb1 ahb reset */
#define K230_RESET_USB1_AHB_REG_OFFSET      0x3c
#define K230_RESET_USB1_AHB_TYPE            K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_USB1_AHB_DONE_BIT        31
#define K230_RESET_USB1_AHB_ASSERT_BIT      1

/* spi0 reset */
#define K230_RESET_SPI0_REG_OFFSET          0x44
#define K230_RESET_SPI0_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SPI0_DONE_BIT            28
#define K230_RESET_SPI0_ASSERT_BIT          0

/* spi1 reset */
#define K230_RESET_SPI1_REG_OFFSET          0x44
#define K230_RESET_SPI1_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SPI1_DONE_BIT            29
#define K230_RESET_SPI1_ASSERT_BIT          1

/* spi2 reset */
#define K230_RESET_SPI2_REG_OFFSET          0x44
#define K230_RESET_SPI2_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SPI2_DONE_BIT            30
#define K230_RESET_SPI2_ASSERT_BIT          2

/* sec reset */
#define K230_RESET_SEC_REG_OFFSET           0x4c
#define K230_RESET_SEC_TYPE                 K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SEC_DONE_BIT             31
#define K230_RESET_SEC_ASSERT_BIT           0

/* pdma reset */
#define K230_RESET_PDMA_REG_OFFSET          0x54
#define K230_RESET_PDMA_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_PDMA_DONE_BIT            28
#define K230_RESET_PDMA_ASSERT_BIT          0

/* sdma reset */
#define K230_RESET_SDMA_REG_OFFSET          0x54
#define K230_RESET_SDMA_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SDMA_DONE_BIT            29
#define K230_RESET_SDMA_ASSERT_BIT          1

/* decompress reset */
#define K230_RESET_DECOMPRESS_REG_OFFSET    0x5c
#define K230_RESET_DECOMPRESS_TYPE          K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_DECOMPRESS_DONE_BIT      31
#define K230_RESET_DECOMPRESS_ASSERT_BIT    0

/* sram reset */
#define K230_RESET_SRAM_REG_OFFSET          0x64
#define K230_RESET_SRAM_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SRAM_DONE_BIT            28
#define K230_RESET_SRAM_ASSERT_BIT          0

/* shrm axim reset */
#define K230_RESET_SHRM_AXIM_REG_OFFSET     0x64
#define K230_RESET_SHRM_AXIM_TYPE           K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SHRM_AXIM_DONE_BIT       30
#define K230_RESET_SHRM_AXIM_ASSERT_BIT     2

/* shrm axis reset */
#define K230_RESET_SHRM_AXIS_REG_OFFSET     0x64
#define K230_RESET_SHRM_AXIS_TYPE           K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_SHRM_AXIS_DONE_BIT       31
#define K230_RESET_SHRM_AXIS_ASSERT_BIT     3

/* nonai2d reset */
#define K230_RESET_NONAI2D_REG_OFFSET       0x6c
#define K230_RESET_NONAI2D_TYPE             K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_NONAI2D_DONE_BIT         31
#define K230_RESET_NONAI2D_ASSERT_BIT       0

/* ddr controller reset */
#define K230_RESET_MCTL_REG_OFFSET          0x74
#define K230_RESET_MCTL_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_MCTL_DONE_BIT            31
#define K230_RESET_MCTL_ASSERT_BIT          0

/* isp reset */
#define K230_RESET_ISP_REG_OFFSET           0x80
#define K230_RESET_ISP_TYPE                 K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_ISP_DONE_BIT             29
#define K230_RESET_ISP_ASSERT_BIT           6

/* isp dw reset */
#define K230_RESET_ISP_DW_REG_OFFSET        0x80
#define K230_RESET_ISP_DW_TYPE              K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_ISP_DW_DONE_BIT          28
#define K230_RESET_ISP_DW_ASSERT_BIT        5


/* dpu reset */
#define K230_RESET_DPU_REG_OFFSET           0x88
#define K230_RESET_DPU_TYPE                 K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_DPU_DONE_BIT             31
#define K230_RESET_DPU_ASSERT_BIT           0

/* disp reset */
#define K230_RESET_DISP_REG_OFFSET          0x90
#define K230_RESET_DISP_TYPE                K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_DISP_DONE_BIT            31
#define K230_RESET_DISP_ASSERT_BIT          0

/* gpu reset */
#define K230_RESET_V2P5D_REG_OFFSET         0x98
#define K230_RESET_V2P5D_TYPE               K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_V2P5D_DONE_BIT           31
#define K230_RESET_V2P5D_ASSERT_BIT         0

/* audio reset */
#define K230_RESET_AUDIO_REG_OFFSET         0xa4
#define K230_RESET_AUDIO_TYPE               K230_RESET_TYPE_HW_AUTO_DONE
#define K230_RESET_AUDIO_DONE_BIT           31
#define K230_RESET_AUDIO_ASSERT_BIT         0


/*--------------------software assert and deassert-------------------*/
/* timer0 reset */
#define K230_RESET_TIMER0_REG_OFFSET        0x20
#define K230_RESET_TIMER0_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER0_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER0_ASSERT_BIT        0

/* timer1 reset */
#define K230_RESET_TIMER1_REG_OFFSET        0x20
#define K230_RESET_TIMER1_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER1_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER1_ASSERT_BIT        1

/* timer2 reset */
#define K230_RESET_TIMER2_REG_OFFSET        0x20
#define K230_RESET_TIMER2_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER2_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER2_ASSERT_BIT        2

/* timer3 reset */
#define K230_RESET_TIMER3_REG_OFFSET        0x20
#define K230_RESET_TIMER3_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER3_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER3_ASSERT_BIT        3

/* timer4 reset */
#define K230_RESET_TIMER4_REG_OFFSET        0x20
#define K230_RESET_TIMER4_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER4_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER4_ASSERT_BIT        4

/* timer5 reset */
#define K230_RESET_TIMER5_REG_OFFSET        0x20
#define K230_RESET_TIMER5_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER5_DONE_BIT          0                   //NOT USED
#define K230_RESET_TIMER5_ASSERT_BIT        5

/* timer apb reset */
#define K230_RESET_TIMER_APB_REG_OFFSET     0x20
#define K230_RESET_TIMER_APB_TYPE           K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TIMER_APB_DONE_BIT       0                   //NOT USED
#define K230_RESET_TIMER_APB_ASSERT_BIT     6

/* hdi reset */
#define K230_RESET_HDI_REG_OFFSET           0x20
#define K230_RESET_HDI_TYPE                 K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_HDI_DONE_BIT             0                   //NOT USED
#define K230_RESET_HDI_ASSERT_BIT           7

/* wdt0 reset */
#define K230_RESET_WDT0_REG_OFFSET          0x20
#define K230_RESET_WDT0_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_WDT0_DONE_BIT            0                   //NOT USED
#define K230_RESET_WDT0_ASSERT_BIT          12

/* wdt1 reset */
#define K230_RESET_WDT1_REG_OFFSET          0x20
#define K230_RESET_WDT1_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_WDT1_DONE_BIT            0                   //NOT USED
#define K230_RESET_WDT1_ASSERT_BIT          13

/* wdt0 apb reset */
#define K230_RESET_WDT0_APB_REG_OFFSET      0x20
#define K230_RESET_WDT0_APB_TYPE            K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_WDT0_APB_DONE_BIT        0                   //NOT USED
#define K230_RESET_WDT0_APB_ASSERT_BIT      14

/* wdt1 apb reset */
#define K230_RESET_WDT1_APB_REG_OFFSET      0x20
#define K230_RESET_WDT1_APB_TYPE            K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_WDT1_APB_DONE_BIT        0                   //NOT USED
#define K230_RESET_WDT1_APB_ASSERT_BIT      15

/* temper sensor apb reset */
#define K230_RESET_TS_APB_REG_OFFSET        0x20
#define K230_RESET_TS_APB_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_TS_APB_DONE_BIT          0                   //NOT USED
#define K230_RESET_TS_APB_ASSERT_BIT        16

/* mailbox reset */
#define K230_RESET_MAILBOX_REG_OFFSET       0x20
#define K230_RESET_MAILBOX_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_MAILBOX_DONE_BIT         0                   //NOT USED
#define K230_RESET_MAILBOX_ASSERT_BIT       17

/* system timer counter reset */
#define K230_RESET_STC_REG_OFFSET           0x20
#define K230_RESET_STC_TYPE                 K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_STC_DONE_BIT             0                   //NOT USED
#define K230_RESET_STC_ASSERT_BIT           18

/* pmu reset */
#define K230_RESET_PMU_REG_OFFSET           0x20
#define K230_RESET_PMU_TYPE                 K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_PMU_DONE_BIT             0                   //NOT USED
#define K230_RESET_PMU_ASSERT_BIT           19

/* ls apb reset */
#define K230_RESET_LS_APB_REG_OFFSET        0x24
#define K230_RESET_LS_APB_TYPE              K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_LS_APB_DONE_BIT          0                   //NOT USED
#define K230_RESET_LS_APB_ASSERT_BIT        0

/* uart0 reset */
#define K230_RESET_UART0_REG_OFFSET         0x24
#define K230_RESET_UART0_TYPE               K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_UART0_DONE_BIT           0                   //NOT USED
#define K230_RESET_UART0_ASSERT_BIT         1

/* uart1 reset */
#define K230_RESET_UART1_REG_OFFSET         0x24
#define K230_RESET_UART1_TYPE               K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_UART1_DONE_BIT           0                   //NOT USED
#define K230_RESET_UART1_ASSERT_BIT         2

/* uart2 reset */
#define K230_RESET_UART2_REG_OFFSET         0x24
#define K230_RESET_UART2_TYPE               K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_UART2_DONE_BIT           0                   //NOT USED
#define K230_RESET_UART2_ASSERT_BIT         3

/* uart3 reset */
#define K230_RESET_UART3_REG_OFFSET         0x24
#define K230_RESET_UART3_TYPE               K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_UART3_DONE_BIT           0                   //NOT USED
#define K230_RESET_UART3_ASSERT_BIT         4

/* uart4 reset */
#define K230_RESET_UART4_REG_OFFSET         0x24
#define K230_RESET_UART4_TYPE               K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_UART4_DONE_BIT           0                   //NOT USED
#define K230_RESET_UART4_ASSERT_BIT         5

/* i2c0 reset */
#define K230_RESET_I2C0_REG_OFFSET          0x24
#define K230_RESET_I2C0_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_I2C0_DONE_BIT            0                   //NOT USED
#define K230_RESET_I2C0_ASSERT_BIT          6

/* i2c1 reset */
#define K230_RESET_I2C1_REG_OFFSET          0x24
#define K230_RESET_I2C1_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_I2C1_DONE_BIT            0                   //NOT USED
#define K230_RESET_I2C1_ASSERT_BIT          7

/* i2c2 reset */
#define K230_RESET_I2C2_REG_OFFSET          0x24
#define K230_RESET_I2C2_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_I2C2_DONE_BIT            0                   //NOT USED
#define K230_RESET_I2C2_ASSERT_BIT          8

/* i2c3 reset */
#define K230_RESET_I2C3_REG_OFFSET          0x24
#define K230_RESET_I2C3_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_I2C3_DONE_BIT            0                   //NOT USED
#define K230_RESET_I2C3_ASSERT_BIT          9

/* i2c4 reset */
#define K230_RESET_I2C4_REG_OFFSET          0x24
#define K230_RESET_I2C4_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_I2C4_DONE_BIT            0                   //NOT USED
#define K230_RESET_I2C4_ASSERT_BIT          10

/* jamlink0 apb reset */
#define K230_RESET_JAMLINK0_APB_REG_OFFSET  0x24
#define K230_RESET_JAMLINK0_APB_TYPE        K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_JAMLINK0_APB_DONE_BIT    0                   //NOT USED
#define K230_RESET_JAMLINK0_APB_ASSERT_BIT  11

/* jamlink1 apb reset */
#define K230_RESET_JAMLINK1_APB_REG_OFFSET  0x24
#define K230_RESET_JAMLINK1_APB_TYPE        K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_JAMLINK1_APB_DONE_BIT    0                   //NOT USED
#define K230_RESET_JAMLINK1_APB_ASSERT_BIT  12

/* jamlink2 apb reset */
#define K230_RESET_JAMLINK2_APB_REG_OFFSET  0x24
#define K230_RESET_JAMLINK2_APB_TYPE        K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_JAMLINK2_APB_DONE_BIT    0                   //NOT USED
#define K230_RESET_JAMLINK2_APB_ASSERT_BIT  13

/* jamlink3 apb reset */
#define K230_RESET_JAMLINK3_APB_REG_OFFSET  0x24
#define K230_RESET_JAMLINK3_APB_TYPE        K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_JAMLINK3_APB_DONE_BIT    0                   //NOT USED
#define K230_RESET_JAMLINK3_APB_ASSERT_BIT  14

/* codec apb reset */
#define K230_RESET_CODEC_APB_REG_OFFSET     0x24
#define K230_RESET_CODEC_APB_TYPE           K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CODEC_APB_DONE_BIT       0                   //NOT USED
#define K230_RESET_CODEC_APB_ASSERT_BIT     17

/* gpio db reset */
#define K230_RESET_GPIO_DB_REG_OFFSET       0x24
#define K230_RESET_GPIO_DB_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_GPIO_DB_DONE_BIT         0                   //NOT USED
#define K230_RESET_GPIO_DB_ASSERT_BIT       18

/* gpio apb reset */
#define K230_RESET_GPIO_APB_REG_OFFSET      0x24
#define K230_RESET_GPIO_APB_TYPE            K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_GPIO_APB_DONE_BIT        0                   //NOT USED
#define K230_RESET_GPIO_APB_ASSERT_BIT      19

/* adc reset */
#define K230_RESET_ADC_REG_OFFSET           0x24
#define K230_RESET_ADC_TYPE                 K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_ADC_DONE_BIT             0                   //NOT USED
#define K230_RESET_ADC_ASSERT_BIT           20

/* adc apb reset */
#define K230_RESET_ADC_APB_REG_OFFSET       0x24
#define K230_RESET_ADC_APB_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_ADC_APB_DONE_BIT         0                   //NOT USED
#define K230_RESET_ADC_APB_ASSERT_BIT       21

/* pwm apb reset */
#define K230_RESET_PWM_APB_REG_OFFSET       0x24
#define K230_RESET_PWM_APB_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_PWM_APB_DONE_BIT         0                   //NOT USED
#define K230_RESET_PWM_APB_ASSERT_BIT       22


/* cpu flush对应的复位，硬件对reset bit自动清零 */
/* cpu0 flush reset */
#define K230_RESET_CPU0_FLUSH_REG_OFFSET    0x4
#define K230_RESET_CPU0_FLUSH_TYPE          K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CPU0_FLUSH_DONE_BIT      0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CPU0_FLUSH_ASSERT_BIT    4

/* cpu1 flush reset */
#define K230_RESET_CPU1_FLUSH_REG_OFFSET    0xc
#define K230_RESET_CPU1_FLUSH_TYPE          K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CPU1_FLUSH_DONE_BIT      0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CPU1_FLUSH_ASSERT_BIT    4

/* shrm apb复位寄存器，不需要软件清零，会自动清零(W1T) */
/* shrm apb reset */
#define K230_RESET_SHRM_APB_REG_OFFSET      0x64
#define K230_RESET_SHRM_APB_TYPE            K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_SHRM_APB_DONE_BIT        0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_SHRM_APB_ASSERT_BIT      1

/* 寄存器为0x80的模块，当reset=0时表示复位 */
/* csi0 apb reset */
#define K230_RESET_CSI0_REG_OFFSET          0x80
#define K230_RESET_CSI0_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CSI0_DONE_BIT            0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CSI0_ASSERT_BIT          0

/* csi1 apb reset */
#define K230_RESET_CSI1_REG_OFFSET          0x80
#define K230_RESET_CSI1_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CSI1_DONE_BIT            0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CSI1_ASSERT_BIT          1

/* csi2 apb reset */
#define K230_RESET_CSI2_REG_OFFSET          0x80
#define K230_RESET_CSI2_TYPE                K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CSI2_DONE_BIT            0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CSI2_ASSERT_BIT          2

/* csi dphy apb reset */
#define K230_RESET_CSI_DPHY_REG_OFFSET      0x80
#define K230_RESET_CSI_DPHY_TYPE            K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_CSI_DPHY_DONE_BIT        0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_CSI_DPHY_ASSERT_BIT      3

/* vi ahb reset */
#define K230_RESET_ISP_AHB_REG_OFFSET       0x80
#define K230_RESET_ISP_AHB_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_ISP_AHB_DONE_BIT         0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_ISP_AHB_ASSERT_BIT       4

/* sensor0 reset */
#define K230_RESET_M0_REG_OFFSET            0x80
#define K230_RESET_M0_TYPE                  K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_M0_DONE_BIT              0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_M0_ASSERT_BIT            7

/* sensor1 reset */
#define K230_RESET_M1_REG_OFFSET            0x80
#define K230_RESET_M1_TYPE                  K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_M1_DONE_BIT              0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_M1_ASSERT_BIT            8

/* sensor2 reset */
#define K230_RESET_M2_REG_OFFSET            0x80
#define K230_RESET_M2_TYPE                  K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_M2_DONE_BIT              0  //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_M2_ASSERT_BIT            9

/* spi2axi reset */
#define K230_RESET_SPI2AXI_REG_OFFSET       0xa8
#define K230_RESET_SPI2AXI_TYPE             K230_RESET_TYPE_SW_SET_DONE
#define K230_RESET_SPI2AXI_DONE_BIT         0   //没有该bit，是否应该放在SW中，需要review
#define K230_RESET_SPI2AXI_ASSERT_BIT       0

#endif