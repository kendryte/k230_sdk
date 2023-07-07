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

#ifndef __SYSCTL_CLK_H__
#define __SYSCTL_CLK_H__

/* created by yangfan */

#include <stdint.h>
#include <stdbool.h>
// #include "platform.h"

#define SYSCTL_BASE_ADDR         0x91100000
#define SYSCTL_CLK_BASE_ADDR    (SYSCTL_BASE_ADDR + 0x0000)     /* SYSCTL_BASE_ADDR=0x9110_0000 */

typedef struct sysctl_clk {
    volatile uint32_t cpu0_clk_cfg;             /* 0x00 */
    volatile uint32_t reserved_0;               /* 0x04 */
    volatile uint32_t reserved_1;               /* 0x08 */
    volatile uint32_t reserved_2;               /* 0x0c */
    volatile uint32_t pmu_clk_cfg;              /* 0x10 */
    volatile uint32_t reserved0[1];             /* 0x14 */
    volatile uint32_t hs_clken_cfg;             /* 0x18 */
    volatile uint32_t hs_sdclk_cfg;             /* 0x1c */
    volatile uint32_t hs_spi_cfg;               /* 0x20 */
    volatile uint32_t ls_clken_cfg0;            /* 0x24 */
    volatile uint32_t ls_clken_cfg1;            /* 0x28 */
    volatile uint32_t uart_i2c_clkdiv_cfg;      /* 0x2c */
    volatile uint32_t ls_clkdiv_cfg;            /* 0x30 */
    volatile uint32_t reserved_3;               /* 0x34 */
    volatile uint32_t reserved_4;               /* 0x38 */
    volatile uint32_t reserved_5;               /* 0x3c */
    volatile uint32_t reserved_6;               /* 0x40 */
    volatile uint32_t reserved_7;               /* 0x44 */
    volatile uint32_t reserved1[2];             /* 0x48 0x4c */
    volatile uint32_t sysctl_clken_cfg;         /* 0x50 */
    volatile uint32_t timer_clk_cfg;            /* 0x54 */
    volatile uint32_t sysctl_clk_div_cfg;       /* 0x58 */
    volatile uint32_t shrm_clk_cfg;             /* 0x5c */
    volatile uint32_t ddr_clk_cfg;              /* 0x60 */
    volatile uint32_t reserved_8;               /* 0x64 */
    volatile uint32_t reserved_9;               /* 0x68 */
    volatile uint32_t reserved_10;              /* 0x6c */
    volatile uint32_t reserved_11;              /* 0x70 */
    volatile uint32_t reserved_12;              /* 0x74 */
    volatile uint32_t reserved_13;              /* 0x78 */
    volatile uint32_t reserved2[1];             /* 0x7c */
    volatile uint32_t sec_clk_div;              /* 0x80 */
    volatile uint32_t reserved3[31];             /* 0x84 0x88 0x8c 0x90 0x94 0x98 0x9c 0xa0-0xac 0xb0-0xbc 0xc0-0xcc 0xd0-0xdc 0xe0-0xec 0xf0-0xfc*/
    volatile uint32_t usb_test_clk_div;         /* 0x100 */
    volatile uint32_t dphy_test_clk_div;        /* 0x104 */
    volatile uint32_t spi2axi_clk_div;          /* 0x108 */
} sysctl_clk_t;

typedef enum {
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /***********************************************ROOT******************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /* clock root */
    /* sysctl_boot clk tree
        osc24m----------------->|-->pll0----->|--------->pll0      clk output--->to sysctl_clock module
                                |             |--div2--->pll0_div2 clk output--->to sysctl_clock module
                                |             |--div3--->pll0_div3 clk output--->to sysctl_clock module
                                |             |--div4--->pll0_div4 clk output--->to sysctl_clock module
                                |
                                |-->pll1----->|--------->pll1      clk output--->to sysctl_clock module
                                |             |--div2--->pll1_div2 clk output--->to sysctl_clock module
                                |             |--div3--->pll1_div3 clk output--->to sysctl_clock module
                                |             |--div4--->pll1_div4 clk output--->to sysctl_clock module
                                |
                                |-->pll2----->|--------->pll2      clk output--->to sysctl_clock module
                                |             |--div2--->pll2_div2 clk output--->to sysctl_clock module
                                |             |--div3--->pll2_div3 clk output--->to sysctl_clock module
                                |             |--div4--->pll2_div4 clk output--->to sysctl_clock module
                                |
                                |-->pll3----->|--------->pll3      clk output--->to sysctl_clock module
                                |             |--div2--->pll3_div2 clk output--->to sysctl_clock module
                                |             |--div3--->pll3_div3 clk output--->to sysctl_clock module
                                |             |--div4--->pll3_div4 clk output--->to sysctl_clock module
                                |
                                |----------------------------------------------->to sysctl_clock module
    */
    SYSCTL_CLK_ROOT_OSC_IN = 0,                 /* 24M */
    SYSCTL_CLK_ROOT_TIMERX_PULSE_IN,            /* 50M */
    SYSCTL_CLK_ROOT_PLL0,                       /* 1.6G */
    SYSCTL_CLK_ROOT_PLL0_DIV_2,                 /* 800M */
    SYSCTL_CLK_ROOT_PLL0_DIV_3,                 /* 533M */
    SYSCTL_CLK_ROOT_PLL0_DIV_4,                 /* 400M */
    SYSCTL_CLK_ROOT_PLL1,                       /* 2.376G */
    SYSCTL_CLK_ROOT_PLL1_DIV_2,                 /* 1.188G */
    SYSCTL_CLK_ROOT_PLL1_DIV_3,                 /* 792M */
    SYSCTL_CLK_ROOT_PLL1_DIV_4,                 /* 594M */
    SYSCTL_CLK_ROOT_PLL2,                       /* 2.667G */
    SYSCTL_CLK_ROOT_PLL2_DIV_2,                 /* 1.3335G */
    SYSCTL_CLK_ROOT_PLL2_DIV_3,                 /* 889M */
    SYSCTL_CLK_ROOT_PLL2_DIV_4,                 /* 666.75M */
    SYSCTL_CLK_ROOT_PLL3,                       /* 1.6G */
    SYSCTL_CLK_ROOT_PLL3_DIV_2,                 /* 800M */
    SYSCTL_CLK_ROOT_PLL3_DIV_3,                 /* 533M */
    SYSCTL_CLK_ROOT_PLL3_DIV_4,                 /* 400M */
    SYSCTL_CLK_ROOT_MAX,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************CPU0 clock***************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /* cpu0 clock tree
      pll0_div2--->DIV--->GATE--->|--------------------->cpu0 core clk
                                  |---GATE--->|---DIV--->cpu0 plic clk
                                  |---------->|---DIV--->cpu0 axi clk
                                  |---GATE--->|--------->noc & ddrc p4 clk

      pll0_div4--->DIV--->|---GATE--->apb clk

      // root node: pll0_div2
      1. cpu0_src           -->cpu0 core DIV & GATE
      2. cpu0_plic          -->cpu0 plic clk gate & div
      3. cpu0_aclk          -->cpu0 axi clk div
      4. cpu0_noc_ddrcp4    -->ddrc axi4clk & noc AXI clock gate

      // root node: pll0_div4
      1. cpu0_pclk          -->cpu0 apb pclk, DIV & GATE
    */
    SYSCTL_CLK_CPU_0_SRC,                     /* defualt 800MHz ---> select pll0_div_2 */
    SYSCTL_CLK_CPU_0_PLIC,                    /* 400MHz */
    SYSCTL_CLK_CPU_0_ACLK,                    /* 400MHz */
    SYSCTL_CLK_CPU_0_NOC_DDRCP4,              /* 400MHz */

    SYSCTL_CLK_CPU_0_PCLK,                    /* 200MHz */

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************pmu clock****************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  pmu system clock tree
      osc24m--->GATE--->pmu apb clk

      //root node: osc24m
      1. pmu_pclk: pmu apb clk gate
    */
    SYSCTL_CLK_PMU_PCLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************hs clock*****************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  hs system clock tree
      pll0_div4-------->DIV------>|--->GATE--->hs hclk high
                                  |--->DIV--->GATE--->|---hs ahb clk--->|---GATE--->sd0 ahb clk
                                                                        |---GATE--->sd1 ahb clk
                                                                        |---GATE--->usb0 ahb clk
                                                                        |---GATE--->usb1 ahb clk
                                                                        |---GATE--->ssi1 ahb clk
                                                                        |---GATE--->ssi2 ahb clk

      pll0_div4---->|---DIV--->GATE--->ssi0 axi clk
                    |---DIV--->GATE--->ssi1 clk
                    |---DIV--->GATE--->ssi2 clk
                    |---DIV--->GATE--->qspi axi clk--->|---GATE--->ssi1 axi clk
                                                       |---GATE--->ssi2 axi clk

                  +-+
      pll0_div2-->|M \
                  |U |--->GATE-->ospi core clk
      pll2_div4-->|X /
                  +-+

      pll2_div4--->DIV--->GATE--->sd axi clk--->|---GATE--->sd0 axi clk
                                                |---GATE--->sd1 axi clk
                                                |---GATE--->sd0 base clk
                                                |---GATE--->sd1 base clk

      pll0_div4--->DIV--->GATE--->sd card clk---->|---GATE--->sd0 card clk tx
                                                  |---GATE--->sd1 card clk tx

                                          +-+
      osc24m----------------------------->|M \
                                          |U |--->|---GATE--->usb0 reference clk
      pll0-------->DIV16--->DIV---------->|X /    |---GATE--->usb1 reference clk
                                          +-+

      osc24m--->DIV--->GATE---sd timer clk--->|---GATE--->sd0 timer clk
                                              |---GATE--->sd1 timer clk

      //root node: pll0_div4
      0. hs_hclk_high_src: hs hclk high DIV
      1. hs_hclk_high: hs hclk high GATE;
      2. hs_hclk_src: hs ahb clk DIV & GATE;
      3. sd0_hclk_gate: sd0 ahb clk gate;
      4. sd1_hclk_gate: sd1 ahb clk gate;
      5. usb0_hclk_gate: usb0 ahb clk gate;
      6. usb1_hclk_gate: usb1 ahb clk gate;
      7. ssi1_hclk_gate: ssi1 ahb clk gate;
      8. ssi2_hclk_gate: ssi2 ahb clk gate;

      //root node: pll0_div4
      1. ssi0_aclk: ssi0 axi clk DIV & GATE;
      2. ssi1_clk: ssi1 clk DIV & GATE;
      3. ssi2_clk: ssi2 clk DIV & GATE;
      4. qspi_aclk_src: qspi sxi clk DIV & GATE;
      5. ssi1_aclk_gate: ssi1 axi clk gate;
      6. ssi2_aclk_gate: ssi2 axi clk gate;

      //root node: pll0_div2 & pll2_div4
      1. ssi0_clk: ospi core clk MUX & GATE;

      //root node: pll2_div4
      1. sd_aclk_src: sd axi clk DIV & GATE;
      2. sd0_aclk_gate: sd0 axi clk gate;
      3. sd1_aclk_gate: sd1 axi clk gate;
      4. sd0_bclk_gate: sd0 base clk gate;
      5. sd1_bclk_gate: sd1 base clk gate;

      //root node: pll0_div4
      1. sd_cclk_src: sd card clk gate & div;
      2. sd0_cclk_gate: sd0 card clk gate;
      3. sd1_cclk_gate: sd1 card clk gate;

      //需要修改，linux部分已经修改了
      //root node: pll0
      1. pll0_div16
      //root node: pll0_div16
      1. usb_ref_50m: usbx reference clk DIV;
      //root node: osc24m & usb_ref_50m
      2. usb0_ref_clk: usb0 reference clk gate & MUX;
      3. usb1_ref_clk: usb1 reference clk gate & MUX;

      //root node: osc24m
      1. sd_tmclk_src: sdx timer clk DIV & GATE;
      2. sd0_tmclk_gate: sd0 timer clk gate;
      3. sd1_tmclk_gate: sd1 timer clk gate;
    */
    SYSCTL_CLK_HS_HCLK_HIGH_SRC,
    SYSCTL_CLK_HS_HCLK_HIGH_GATE,
    SYSCTL_CLK_HS_HCLK_SRC,
    SYSCTL_CLK_SD_0_HCLK_GATE,
    SYSCTL_CLK_SD_1_HCLK_GATE,
    SYSCTL_CLK_USB_0_HCLK_GATE,
    SYSCTL_CLK_USB_1_HCLK_GATE,
    SYSCTL_CLK_SSI_1_HCLK_GATE,
    SYSCTL_CLK_SSI_2_HCLK_GATE,

    SYSCTL_CLK_SSI_0_ACLK,
    SYSCTL_CLK_SSI_1_CLK,
    SYSCTL_CLK_SSI_2_CLK,
    SYSCTL_CLK_QSPI_ACLK_SRC,
    SYSCTL_CLK_SSI_1_ACLK_GATE,
    SYSCTL_CLK_SSI_2_ACLK_GATE,

    SYSCTL_CLK_SSI_0_CLK,

    SYSCTL_CLK_SD_ACLK_SRC,
    SYSCTL_CLK_SD_0_ACLK_GATE,
    SYSCTL_CLK_SD_1_ACLK_GATE,
    SYSCTL_CLK_SD_0_BCLK_GATE,
    SYSCTL_CLK_SD_1_BCLK_GATE,

    SYSCTL_CLK_SD_CCLK_SRC,
    SYSCTL_CLK_SD_0_CCLK_GATE,
    SYSCTL_CLK_SD_1_CCLK_GATE,


    SYSCTL_CLK_PLL0_DIV_16,
    SYSCTL_CLK_USB_REF_50M,
    SYSCTL_CLK_USB_0_REF_CLK,
    SYSCTL_CLK_USB_1_REF_CLK,

    SYSCTL_CLK_SD_TMCLK_SRC,
    SYSCTL_CLK_SD_0_TMCLK_GATE,
    SYSCTL_CLK_SD_1_TMCLK_GATE,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************ls clock*****************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  ls system clock tree
      pll0_div4---------->DIV----->-----GATE--->ls apb clk--->|---GATE--->uart0 apb clk
                                                              |---GATE--->uart1 apb clk
                                                              |---GATE--->uart2 apb clk
                                                              |---GATE--->uart3 apb clk
                                                              |---GATE--->uart4 apb clk
                                                              |---GATE--->i2c0 apb clk
                                                              |---GATE--->i2c1 apb clk
                                                              |---GATE--->i2c2 apb clk
                                                              |---GATE--->i2c3 apb clk
                                                              |---GATE--->i2c4 apb clk
                                                              |---GATE--->gpio apb clk
                                                              |---GATE--->pwm apb clk
                                                              |---GATE--->jamlink0 apb clk
                                                              |---GATE--->jamlink1 apb clk
                                                              |---GATE--->jamlink2 apb clk
                                                              |---GATE--->jamlink3 apb clk
                                                              |---GATE--->audio apb clk
                                                              |---GATE--->adc apb clk
                                                              |---GATE--->codec apb clk

      pll0_div16----->|---DIV--->GATE--->uart0 core clk
                      |---DIV--->GATE--->uart1 core clk
                      |---DIV--->GATE--->uart2 core clk
                      |---DIV--->GATE--->uart3 core clk
                      |---DIV--->GATE--->uart4 core clk

      pll0_div16----->DIV---->|---GATE--->jamlink0 CO clk
                              |---GATE--->jamlink1 CO clk
                              |---GATE--->jamlink2 CO clk
                              |---GATE--->jamlink3 CO clk

      pll0_div4------>|---DIV--->GATE--->i2c0 core clk
                      |---DIV--->GATE--->i2c1 core clk
                      |---DIV--->GATE--->i2c2 core clk
                      |---DIV--->GATE--->i2c3 core clk
                      |---DIV--->GATE--->i2c4 core clk

      pll0_div4------>|---fraDIV--->GATE--->codec adc mclk
                      |---fraDIV--->GATE--->codec dac mclk
                      |---fraDIV--->GATE--->audio dev clk
                      |---fraDIV--->GATE--->pdm clk
                      |---DIV--->GATE--->adc clk

      osc24m----->DIV--->GATE--->gpio debounce clk

      //root node: pll0_div4    clock tree有误
      1. ls_pclk_src: low system apb clk div & gate;
      2. uart0_pclk_gate: uart0 apb clk gate;
      3. uart1_pclk_gate: uart1 apb clk gate;
      4. uart2_pclk_gate: uart2 apb clk gate;
      5. uart3_pclk_gate: uart3 apb clk gate;
      6. uart4_pclk_gate: uart4 apb clk gate;
      7. i2c0_pclk_gate: i2c0 apb clk gate;
      8. i2c1_pclk_gate: i2c1 apb clk gate;
      9. i2c2_pclk_gate: i2c2 apb clk gate;
      10. i2c3_pclk_gate: i2c3 apb clk gate;
      11. i2c4_pclk_gate: i2c4 apb clk gate;
      12. gpio_pclk_gate: gpio apb clk gate;
      13. pwm_pclk_gate: pwm apb clk gate;
      14. jamlink0_pclk_gate: jamlink0 apb clk gate;
      15. jamlink1_pclk_gate: jamlink1 apb clk gate;
      16. jamlink2_pclk_gate: jamlink2 apb clk gate;
      17. jamlink3_pclk_gate: jamlink3 apb clk gate;
      18. audio_pclk_gate: audio apb clk gate;
      19. adc_pclk_gate: adc apb clk gate;
      20. codec_pclk_gate: codec apb clk gate;

      //root node: pll0_div16
      1. uart0_clk: uart0 core clk div & gate;
      2. uart1_clk: uart1 core clk div & gate;
      3. uart2_clk: uart2 core clk div & gate;
      4. uart3_clk: uart3 core clk div & gate;
      5. uart4_clk: uart4 core clk div & gate;

      //root node: pll0_div16
      1. jamlinkCO_div: jamlinkx CO div;
      2. jamlink0CO_gate: jamlink0 CO clk gate;
      3. jamlink1CO_gate: jamlink1 CO clk gate;
      4. jamlink2CO_gate: jamlink2 CO clk gate;
      5. jamlink3CO_gate: jamlink3 CO clk gate;

      //root node: pll0_div4
      1. i2c0_clk: i2c0 core clk div & gate;
      2. i2c1_clk: i2c1 core clk div & gate;
      3. i2c2_clk: i2c2 core clk div & gate;
      4. i2c3_clk: i2c3 core clk div & gate;
      5. i2c4_clk: i2c4 core clk div & gate;

      //root node: pll0_div4
      1. codec_adc_mclk: codec adc mclk fradiv & gate;
      2. codec_dac_mclk: codec dac mclk fradiv & gate;
      3. audio_dev_clk: audio dev clk fradiv & gate;
      4. pdm_clk: pdm clk fradiv & gate;
      5. adc_clk: adc clk div & gate;

      //root node: osc24m
      1. gpio_dbclk: gpio debounce clk div & gate;
    */
    SYSCTL_CLK_LS_PCLK_SRC,
    SYSCTL_CLK_UART_0_PCLK_GATE,
    SYSCTL_CLK_UART_1_PCLK_GATE,
    SYSCTL_CLK_UART_2_PCLK_GATE,
    SYSCTL_CLK_UART_3_PCLK_GATE,
    SYSCTL_CLK_UART_4_PCLK_GATE,
    SYSCTL_CLK_I2C_0_PCLK_GATE,
    SYSCTL_CLK_I2C_1_PCLK_GATE,
    SYSCTL_CLK_I2C_2_PCLK_GATE,
    SYSCTL_CLK_I2C_3_PCLK_GATE,
    SYSCTL_CLK_I2C_4_PCLK_GATE,
    SYSCTL_CLK_GPIO_PCLK_GATE,
    SYSCTL_CLK_PWM_PCLK_GATE,
    SYSCTL_CLK_JAMLINK_0_PCLK_GATE,
    SYSCTL_CLK_JAMLINK_1_PCLK_GATE,
    SYSCTL_CLK_JAMLINK_2_PCLK_GATE,
    SYSCTL_CLK_JAMLINK_3_PCLK_GATE,
    SYSCTL_CLK_ADC_PCLK_GATE,

    SYSCTL_CLK_UART_0_CLK,
    SYSCTL_CLK_UART_1_CLK,
    SYSCTL_CLK_UART_2_CLK,
    SYSCTL_CLK_UART_3_CLK,
    SYSCTL_CLK_UART_4_CLK,

    SYSCTL_CLK_JAMLINKCO_DIV,
    SYSCTL_CLK_JAMLINK_0_CO_GATE,
    SYSCTL_CLK_JAMLINK_1_CO_GATE,
    SYSCTL_CLK_JAMLINK_2_CO_GATE,
    SYSCTL_CLK_JAMLINK_3_CO_GATE,

    SYSCTL_CLK_I2C_0_CLK,
    SYSCTL_CLK_I2C_1_CLK,
    SYSCTL_CLK_I2C_2_CLK,
    SYSCTL_CLK_I2C_3_CLK,
    SYSCTL_CLK_I2C_4_CLK,

    // SYSCTL_CLK_SUM_DIV,
    // SYSCTL_CLK_STEP_DIV,
    // SYSCTL_CLK_PDM_CLK_GATE,
    SYSCTL_CLK_ADC_CLK,

    SYSCTL_CLK_GPIO_DBCLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************sysctl clock*************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  sysctl clock tree
      pll0_div16----------------->sysctl apb clk----->|---GATE--->wdt0 apb clk
                                                      |---GATE--->wdt1 apb clk
                                                      |---GATE--->timer apb clk
                                                      |---GATE--->iomux apb clk
                                                      |---GATE--->mailbox apb clk

      pll0_div4--->DIV---GATE--->hdi core clk
      pll1_div4--->DIV--->GATE--->time stamp clk
      osc24m------>DIV----------->temp sensor clk

      osc24m--->|---DIV--->GATE--->wdt0 clk
                |---div--->GATE--->wdt1 clk

      //root node: pll0_div16
      1. sysctl_pclk: sysctl apb clk; //sysctl_pclk have no gate & div.
      2. wdt0_pclk_gate: wdt0 apb clk gate;
      3. wdt1_pclk_gate: wdt1 apb clk gate;
      4. timer_pclk_gate: timer apb clk gate;
      5. iomux_pclk_gate: iomux apb clk gate;
      6. mailbox_pclk_gate: mailbox apb clk gate;

      //root node: pll0_div4
      1. hdi_clk: hdi core clk div & gate;

      //root node: pll1_div4
      1. stc_clk: time stamp clk div & gate;

      //root node: osc24m
      1. ts_clk: temp sensor clk div;

      //root node: osc24m
      1. wdt0_clk: wdt0 clk div & gate;
      2. wdt1_clk: wdt1 clk div & gate;
    */
    SYSCTL_CLK_PCLK,
    SYSCTL_CLK_WDT_0_PCLK_GATE,
    SYSCTL_CLK_WDT_1_PCLK_GATE,
    SYSCTL_CLK_TIMER_PCLK_GATE,
    SYSCTL_CLK_IOMUX_PCLK_GATE,
    SYSCTL_CLK_MAILBOX_PCLK_GATE,

    SYSCTL_CLK_HDI_CLK,

    SYSCTL_CLK_STC_CLK,

    SYSCTL_CLK_TS_CLK,

    SYSCTL_CLK_WDT_0_CLK,
    SYSCTL_CLK_WDT_1_CLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************timer clock**************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  timer clock tree
                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer0 core clk
      timerx_pulse_in-------->|X /
                              +-+

                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer1 core clk
      timerx_pulse_in-------->|X /
                              +-+

                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer2 core clk
      timerx_pulse_in-------->|X /
                              +-+

                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer3 core clk
      timerx_pulse_in-------->|X /
                              +-+

                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer4 core clk
      timerx_pulse_in-------->|X /
                              +-+

                              +-+
      pll0_div16----->DIV---->|M \
                              |U |---GATE--->timer5 core clk
      timerx_pulse_in-------->|X /
                              +-+

      //需要修改，linux已经修改
      //root node: pll0_div16 & timerx_pulse_in
      1. timerx_pulse_in: external input, maxsize is 1MHz;

      2. timer0_clk_src: timer0 core clk DIV;
      3. timer0_clk: timer0 core clk GATE & MUX;

      4. timer1_clk_src: timer1 core clk DIV;
      5. timer1_clk: timer1 core clk GATE & MUX;

      6. timer2_clk_src: timer2 core clk DIV;
      7. timer2_clk: timer2 core clk GATE & MUX;

      8. timer3_clk_src: timer3 core clk DIV;
      9. timer3_clk: timer3 core clk GATE & MUX;

      10. timer4_clk_src: timer4 core clk DIV;
      11. timer4_clk: timer4 core clk GATE & MUX

      12. timer5_clk_src: timer5 core clk DIV;
      13. timer5_clk: timer5 core clk GATE & MUX
    */
    SYSCTL_CLK_TIMERX_PULSE_IN,
    SYSCTL_CLK_TIMER_0_SRC,
    SYSCTL_CLK_TIMER_0_CLK,
    SYSCTL_CLK_TIMER_1_SRC,
    SYSCTL_CLK_TIMER_1_CLK,
    SYSCTL_CLK_TIMER_2_SRC,
    SYSCTL_CLK_TIMER_2_CLK,
    SYSCTL_CLK_TIMER_3_SRC,
    SYSCTL_CLK_TIMER_3_CLK,
    SYSCTL_CLK_TIMER_4_SRC,
    SYSCTL_CLK_TIMER_4_CLK,
    SYSCTL_CLK_TIMER_5_SRC,
    SYSCTL_CLK_TIMER_5_CLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************shrm clock***************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  shrm system clock tree
                  +-+
      pll0_div2-->|M \
                  |U |---GATE--->shrm sram clk--->|---DIV2--->GATE--->shrm axi slave clk
      pll3_div2-->|X /                            |---------->GATE--->decompress axi clk
                  +-+

      pll0_div4--->DIV--->GATE---shrm apb clk

      pll0_div4--->GATE--->shrm axi master clk--->|---GATE--->gsdma axi clk
                                                  |---GATE--->nonai2d axi clk
                                                  |---GATE--->peri dma axi clk

      //root node: pll0_div2 & pll3_div2
      1. shrm_src: MUX & GATE;
      2. shrm_div2: div2;
      3. shrm_axis_clk_gate: shrm axi slave clk gate;
      4. decompress_aclk_gate: decompress axi clk gate;

      //root node: pll0_div4
      1. shrm_pclk: shrm apb clk div & gate;

      //root node: pll0_div4
      1. shrm_axim_clk_gate: shrm axi master clk gate;
      2. gsdma_aclk_gate: gsdma axi clk gate;
      3. nonai2d_aclk_gate: nonai2d axi clk gate;
      4. pdma_aclk_gate: peri dma axi clk gate;
    */
    SYSCTL_CLK_SHRM_SRC,
    SYSCTL_CLK_SHRM_DIV_2,
    SYSCTL_CLK_SHRM_AXIS_CLK_GATE,
    SYSCTL_CLK_DECOMPRESS_ACLK_GATE,

    SYSCTL_CLK_SHRM_PCLK,

    SYSCTL_CLK_SHRM_AXIM_CLK_GATE,
    SYSCTL_CLK_NONAI2D_ACLK_GATE,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************sec clock****************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  sec system clock tree
      pll0_div4-->---DIV--->GATE--->sec apb clk
      pll1_div4--->---DIV--->GATE--->sec fix clk  // clock tree有误
      pll1_div4--->DIV--->GATE--->sec axi clk

      //root node: pll0_div4
      1. sec_pclk: sec apb clk DIV & GATE;

      //root node: pll1_div4
      1. sec_aclk: sec axi clk DIV & GATE;
      2. sec_fixclk: sec fix clk DIV & GATE;
    */
    SYSCTL_CLK_SEC_PCLK,
    SYSCTL_CLK_SEC_FIXCLK,

    SYSCTL_CLK_SEC_ACLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************usb test mode clock******************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  usb test mode clock tree
      pll1--->DIV--->GATE--->usb 480m clk
      pll0_div4--->DIV--->GATE--->usb 100m clk

      //root node: pll1
      1. usb_clk480: usb 480m clk DIV & GATE;

      //root node: pll0_div4
      1. usb_clk100: usb 100m clk DIV & GATE;
    */
    SYSCTL_CLK_USB_CLK_480,
    SYSCTL_CLK_USB_CLK_100,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************dphy dft mode clock******************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  dphy dft mode clock tree
      pll0--->DIV--->GATE--->dphy dft mode clk

      //root node: pll0
      1. dphy_test_clk: dphy dft mode clk DIV & GATE;
    */
    SYSCTL_CLK_DPHY_TEST_CLK,

    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /********************************************spi2axi clock************************************************/
    /*********************************************************************************************************/
    /*********************************************************************************************************/
    /*  spi2axi clock tree
      pll0_div4--->DIV--->GATE--->spi2axi axi clk

      //root node: pll0_div4
      1. spi2axi_aclk: spi2axi clk DIV & GATE;
    */
    SYSCTL_CLK_SPI2AXI_ACLK,

    SYSCTL_CLK_NODE_MAX,
}sysctl_clk_node_e;

// typedef struct sysctl_attr {
//     sysctl_clk_node_e clk;
//     volatile uint32_t sysctl_readonly;
//     volatile uint32_t sysctl_readwrite;
//     volatile uint32_t sysctl_noreadwrite;
// } sysctl_clk_attr_t;

// typedef struct sysctl_attr {
//     bool sysctl_read;
//     bool sysctl_write;
// } sysctl_clk_attr_t;

#define SYSCTL_READ_ENABLE (1 << 0)
#define SYSCTL_READ_DISABLE (0 << 0)
#define SYSCTL_WRITE_ENABLE (1 << 1)
#define SYSCTL_WRITE_DISABLE (0 << 1)
// typedef enum
// {
//     ERROR_RO = 0,
//     ERROR_RW,
//     ERROR_NOTRW,
// } sysctl_err_t;


/**********************************************************************************************************************
*
* 针对根时钟的API。24M、PLL0-3, 这5个时钟是根时钟
* 这里假设大核对根时钟具有读写权限，因此在这些API中没有判断clk的属性，因为这些API只针对根时钟。
*
**********************************************************************************************************************/
/* 获取锁相环 bypass状态, 如果是bypas，则锁相环输出的是24M OSC时钟 */
bool sysctl_boot_get_root_clk_bypass(sysctl_clk_node_e clk);
void sysctl_boot_set_root_clk_bypass(sysctl_clk_node_e clk, bool enable);

/* Enable pll, enable 24M clock&pll */
/* 打开或者关闭 root时钟 针对锁相环和24M时钟 */
bool sysctl_boot_get_root_clk_en(sysctl_clk_node_e clk);
void sysctl_boot_set_root_clk_en(sysctl_clk_node_e clk, bool enable);

/* 获取锁相环锁定状态 */
bool sysctl_boot_get_root_clk_lock(sysctl_clk_node_e clk);

/* 获取root时钟频率 */
uint32_t sysctl_boot_get_root_clk_freq(sysctl_clk_node_e clk);

/* 设置锁相环时钟频率 计算公式 pll_out_freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1) */
bool sysctl_boot_set_root_clk_freq(sysctl_clk_node_e clk, uint32_t fbdiv, uint32_t refdiv, uint32_t outdiv, uint32_t bwadj);


/**********************************************************************************************************************
*
* 针对时钟树中trunk和leaf节点的API。即，除了5个根时钟之外的其它时钟。
*
**********************************************************************************************************************/
/* 设置时钟数上叶子节点时钟源, 请根据时钟树来设置, 很多时钟节点只有一个时钟源，因此设置会返回false */
bool sysctl_clk_set_leaf_parent(sysctl_clk_node_e leaf, sysctl_clk_node_e parent);
/* 获取时钟树上叶子节点时钟源 */
sysctl_clk_node_e sysctl_clk_get_leaf_parent(sysctl_clk_node_e leaf);

/* 设置时钟节点enable,注意:只设置本时钟节点的enable，不会设置上游时钟的enable。
   同linux kernel的区别: linux kernel clock framework 会自动设置上游时钟的enable，测试代码没有kernel框架，因此只设置本节点时钟的enable */
void sysctl_clk_set_leaf_en(sysctl_clk_node_e leaf, bool enable);

/* 获取本时钟节点的enable状态 */
bool sysctl_clk_get_leaf_en(sysctl_clk_node_e leaf);

/* 设置本时钟节点的分频系数 */
bool sysctl_clk_set_leaf_div(sysctl_clk_node_e leaf, uint32_t numerator, uint32_t denominator);

/* 获取本时钟节点的分频系数 */
double sysctl_clk_get_leaf_div(sysctl_clk_node_e leaf);

// /* 设置本时钟节点的相位 */
// bool sysctl_clk_set_phase(sysctl_clk_node_e leaf, uint32_t degree);

// /* 获取本时钟节点的相位 */
// uint32_t sysctl_clk_get_phase(sysctl_clk_node_e leaf);

/* calc clock freqency */
/* 计算当前时钟节点的频率, 这个API会搜索整个时钟路径，从时钟源开始计算每一级的分频，最终得出当前时钟频率 */
uint32_t sysctl_clk_get_leaf_freq(sysctl_clk_node_e leaf);
#endif
