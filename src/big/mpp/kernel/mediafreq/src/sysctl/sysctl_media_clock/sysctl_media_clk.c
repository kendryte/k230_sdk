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

#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include <math.h>
#include "sysctl_boot.h"
#include "sysctl_media_clk.h"
#include "ioremap.h"
#include "board.h"

/* created by yangfan */
/* please refer to the sysctl_media_clk.h file for API description */
#define OSC_CLOCK_FREQ_24M (24000000)

extern volatile sysctl_boot_t* sysctl_boot;

volatile sysctl_media_clk_t* sysctl_media_clk = (volatile sysctl_media_clk_t*)SYSCTL_CLK_BASE_ADDR;

/* if PLL bypass, the output clock is 24m clock. If there is no bypass, the clock comes from PLL */
bool sysctl_media_boot_get_root_clk_bypass(sysctl_media_clk_node_e clk)
{
    switch(clk)
    {
        case SYSCTL_CLK_ROOT_PLL0:
        case SYSCTL_CLK_ROOT_PLL0_DIV_2:
        case SYSCTL_CLK_ROOT_PLL0_DIV_3:
        case SYSCTL_CLK_ROOT_PLL0_DIV_4:
            return ((sysctl_boot->pll[0].cfg1 >> 19) & 0x1) ? true:false;

        case SYSCTL_CLK_ROOT_PLL1:
        case SYSCTL_CLK_ROOT_PLL1_DIV_2:
        case SYSCTL_CLK_ROOT_PLL1_DIV_3:
        case SYSCTL_CLK_ROOT_PLL1_DIV_4:
            return ((sysctl_boot->pll[1].cfg1 >> 19) & 0x1) ? true:false;

        case SYSCTL_CLK_ROOT_PLL2:
        case SYSCTL_CLK_ROOT_PLL2_DIV_2:
        case SYSCTL_CLK_ROOT_PLL2_DIV_3:
        case SYSCTL_CLK_ROOT_PLL2_DIV_4:
            return ((sysctl_boot->pll[2].cfg1 >> 19) & 0x1) ? true:false;

        case SYSCTL_CLK_ROOT_PLL3:
        case SYSCTL_CLK_ROOT_PLL3_DIV_2:
        case SYSCTL_CLK_ROOT_PLL3_DIV_3:
        case SYSCTL_CLK_ROOT_PLL3_DIV_4:
            return ((sysctl_boot->pll[3].cfg1 >> 19) & 0x1) ? true:false;

        default:
            return false;
    }
}

/* get PLL output frequency. 
*  freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1) 
*/
uint32_t sysctl_media_boot_get_root_clk_freq(sysctl_media_clk_node_e clk)
{
    uint32_t refdiv;    /* reference clock divide */
    uint32_t outdiv;    /* output clock divide */
    uint32_t fbdiv;     /* feedback clock divide */
    uint32_t freq;

    switch(clk)
    {
        case SYSCTL_CLK_ROOT_OSC_IN:
            return OSC_CLOCK_FREQ_24M;  /* 24MHz */

        case SYSCTL_CLK_ROOT_PLL0:
        case SYSCTL_CLK_ROOT_PLL0_DIV_2:
        case SYSCTL_CLK_ROOT_PLL0_DIV_3:
        case SYSCTL_CLK_ROOT_PLL0_DIV_4:
        {
            if(true == sysctl_media_boot_get_root_clk_bypass(clk))
            {
                freq = OSC_CLOCK_FREQ_24M;
            }
            else
            {
                refdiv = (sysctl_boot->pll[0].cfg0 >> 16) & 0x3F;    /* bit 16~21 */
                outdiv = (sysctl_boot->pll[0].cfg0 >> 24) & 0xF;     /* bit 24~27 */
                fbdiv  = (sysctl_boot->pll[0].cfg0 >> 0)  & 0x1FFF;   /* bit 0~12 */
                freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1);
            }
            switch(clk)
            {
                case SYSCTL_CLK_ROOT_PLL0:
                    return freq;
                case SYSCTL_CLK_ROOT_PLL0_DIV_2:
                    return freq/2;
                case SYSCTL_CLK_ROOT_PLL0_DIV_3:
                    return freq/3;
                default:
                    return freq/4;
            }
        }

        case SYSCTL_CLK_ROOT_PLL1:
        case SYSCTL_CLK_ROOT_PLL1_DIV_2:
        case SYSCTL_CLK_ROOT_PLL1_DIV_3:
        case SYSCTL_CLK_ROOT_PLL1_DIV_4:
        {
            if(true == sysctl_media_boot_get_root_clk_bypass(clk))
            {
                freq = OSC_CLOCK_FREQ_24M;
            }
            else
            {
                refdiv = (sysctl_boot->pll[1].cfg0 >> 16) & 0x3F;    /* bit 16~21 */
                outdiv = (sysctl_boot->pll[1].cfg0 >> 24) & 0xF;     /* bit 24~27 */
                fbdiv  = (sysctl_boot->pll[1].cfg0 >> 0)  & 0x1FFF;   /* bit 0~12 */
                freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1);
            }
            switch(clk)
            {
                case SYSCTL_CLK_ROOT_PLL1:
                    return freq;
                case SYSCTL_CLK_ROOT_PLL1_DIV_2:
                    return freq/2;
                case SYSCTL_CLK_ROOT_PLL1_DIV_3:
                    return freq/3;
                default:
                    return freq/4;
            }
        }

        case SYSCTL_CLK_ROOT_PLL2:
        case SYSCTL_CLK_ROOT_PLL2_DIV_2:
        case SYSCTL_CLK_ROOT_PLL2_DIV_3:
        case SYSCTL_CLK_ROOT_PLL2_DIV_4:
        {
            if(true == sysctl_media_boot_get_root_clk_bypass(clk))
            {
                freq = OSC_CLOCK_FREQ_24M;
            }
            else
            {
                refdiv = (sysctl_boot->pll[2].cfg0 >> 16) & 0x3F;    /* bit 16~21 */
                outdiv = (sysctl_boot->pll[2].cfg0 >> 24) & 0xF;     /* bit 24~27 */
                fbdiv  = (sysctl_boot->pll[2].cfg0 >> 0)  & 0x1FFF;   /* bit 0~12 */
                freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1);
            }
            switch(clk)
            {
                case SYSCTL_CLK_ROOT_PLL2:
                    return freq;
                case SYSCTL_CLK_ROOT_PLL2_DIV_2:
                    return freq/2;
                case SYSCTL_CLK_ROOT_PLL2_DIV_3:
                    return freq/3;
                default:
                    return freq/4;
            }
        }

        case SYSCTL_CLK_ROOT_PLL3:
        case SYSCTL_CLK_ROOT_PLL3_DIV_2:
        case SYSCTL_CLK_ROOT_PLL3_DIV_3:
        case SYSCTL_CLK_ROOT_PLL3_DIV_4:
        {
            if(true == sysctl_media_boot_get_root_clk_bypass(clk))
            {
                freq = OSC_CLOCK_FREQ_24M;
            }
            else
            {
                refdiv = (sysctl_boot->pll[3].cfg0 >> 16) & 0x3F;    /* bit 16~21 */
                outdiv = (sysctl_boot->pll[3].cfg0 >> 24) & 0xF;     /* bit 24~27 */
                fbdiv  = (sysctl_boot->pll[3].cfg0 >> 0)  & 0x1FFF;   /* bit 0~12 */
                freq = (double)OSC_CLOCK_FREQ_24M * (double)(fbdiv+1) / (double)(refdiv+1) / (double)(outdiv+1);
            }
            switch(clk)
            {
                case SYSCTL_CLK_ROOT_PLL3:
                    return freq;
                case SYSCTL_CLK_ROOT_PLL3_DIV_2:
                    return freq/2;
                case SYSCTL_CLK_ROOT_PLL3_DIV_3:
                    return freq/3;
                default:
                    return freq/4;
            }
        }

        default:
            return 0;
    }
}


/***************************************************************************
*  
* API of trunk and leaf node
*
**************************************************************************/
/* 设置时钟树上叶子节点的时钟源，即根据寄存器设计文档来描述MUX；如果时钟节点只有一个时钟源，则返回false。 */
bool sysctl_media_clk_set_leaf_parent(sysctl_media_clk_node_e leaf, sysctl_media_clk_node_e parent)
{
    volatile uint32_t ret;

    switch(leaf)
    {
        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
        {
            if(SYSCTL_CLK_ROOT_PLL0 == parent)
            {
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xfffffff9;
                sysctl_media_clk->cpu1_clk_cfg = ret | (0 << 1);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL1_DIV_2 == parent)
            {
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xfffffff9;
                sysctl_media_clk->cpu1_clk_cfg = ret | (1 << 1);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL3 == parent)
            {
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xfffffff9;
                sysctl_media_clk->cpu1_clk_cfg = ret | (2 << 1);
                return true;
            }
            else
            {
                return false;
            }
        }
        case SYSCTL_CLK_CPU_1_PLIC:
        case SYSCTL_CLK_CPU_1_ACLK:
        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            return false;
        case SYSCTL_CLK_CPU_1_PCLK:
            return false;       /* always pll0_div4 */

        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
        {
            if(SYSCTL_CLK_ROOT_PLL0_DIV_2 == parent)
            {
                ret = sysctl_media_clk->ai_clk_cfg;
                ret &= 0xfffffffb;
                sysctl_media_clk->ai_clk_cfg = ret | (0 << 2);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL3_DIV_2 == parent)
            {
                ret = sysctl_media_clk->ai_clk_cfg;
                ret &= 0xfffffffb;
                sysctl_media_clk->ai_clk_cfg = ret | (1 << 2);
                return true;
            }
            else
            {
                return false;
            }
        }
        case SYSCTL_CLK_AI_ACLK:
        case SYSCTL_CLK_AI_DDRCP3:
            return false;       /* always pll0_div4 */

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
        case SYSCTL_CLK_VPU_ACLK_SRC:
        case SYSCTL_CLK_VPU_ACLK:
        case SYSCTL_CLK_VPU_DDRCP2:
        case SYSCTL_CLK_VPU_CFG:
            return false;

        /*--------------------------- AUDIO CLOCK ------------------------------------*/
        case SYSCTL_CLK_AUDIO_PCLK_GATE:
        case SYSCTL_CLK_CODEC_PCLK_GATE:
            return false;
        case SYSCTL_CLK_CODEC_ADC_MCLK:
        case SYSCTL_CLK_CODEC_DAC_MCLK:
        case SYSCTL_CLK_AUDIO_DEV_CLK:
        case SYSCTL_CLK_PDM_CLK:
            return false;

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
            return false;

        case SYSCTL_CLK_MCLK_0:
        {
            if(SYSCTL_CLK_ROOT_PLL1_DIV_3 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xffffffe7;
                sysctl_media_clk->mclk_cfg = ret | (0 << 3);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL1_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xffffffe7;
                sysctl_media_clk->mclk_cfg = ret | (1 << 3);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL0_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xffffffe7;
                sysctl_media_clk->mclk_cfg = ret | (2 << 3);
                return true;
            }
            else
            {
                return false;
            }
        }
        case SYSCTL_CLK_MCLK_1:
        {
            if(SYSCTL_CLK_ROOT_PLL1_DIV_3 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfffff3ff;
                sysctl_media_clk->mclk_cfg = ret | (0 << 10);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL1_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfffff3ff;
                sysctl_media_clk->mclk_cfg = ret | (1 << 10);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL0_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfffff3ff;
                sysctl_media_clk->mclk_cfg = ret | (2 << 10);
                return true;
            }
            else
            {
                return false;
            }
        }
        case SYSCTL_CLK_MCLK_2:
        {
            if(SYSCTL_CLK_ROOT_PLL1_DIV_3 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfff9ffff;
                sysctl_media_clk->mclk_cfg = ret | (0 << 17);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL1_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfff9ffff;
                sysctl_media_clk->mclk_cfg = ret | (1 << 17);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL0_DIV_4 == parent)
            {
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfff9ffff;
                sysctl_media_clk->mclk_cfg = ret | (2 << 17);
                return true;
            }
            else
            {
                return false;
            }
        }

        case SYSCTL_CLK_ISP_CLK:
        {
            if(SYSCTL_CLK_ROOT_PLL0_DIV_4 == parent)
            {
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xdfffffff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (0 << 29);
                return true;
            }
            else if(SYSCTL_CLK_ROOT_PLL2_DIV_4 == parent)
            {
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xdfffffff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (1 << 29);
                return true;
            }
            else
            {
                return false;
            }
        }
        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            return false;

        case SYSCTL_CLK_ISP_HCLK:
            return false;

        case SYSCTL_CLK_ISP_ACLK_GATE:
        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            return false;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
        case SYSCTL_CLK_ISP_VSECLK_GATE:
            return false;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
        case SYSCTL_CLK_DPU_ACLK_GATE:
        case SYSCTL_CLK_DPU_PCLK:
            return false;

        default:
            return false;
    }
}

/* 获取时钟树上叶子节点的时钟源，即读取MUX对应的寄存器的值；默认返回SYSCTL_CLK_ROOT_MAX */
sysctl_media_clk_node_e sysctl_media_clk_get_leaf_parent(sysctl_media_clk_node_e leaf)
{
    switch(leaf)
    {
        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
        {
            switch((sysctl_media_clk->cpu1_clk_cfg >> 1) & 0x3)
            {
                case 0:
                    return SYSCTL_CLK_ROOT_PLL0;
                case 1:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_2;
                case 2:
                    return SYSCTL_CLK_ROOT_PLL3;
                default:
                    return SYSCTL_CLK_ROOT_MAX;
            }
        }
        case SYSCTL_CLK_CPU_1_PLIC:
        case SYSCTL_CLK_CPU_1_ACLK:
            return SYSCTL_CLK_CPU_1_SRC;
        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            return SYSCTL_CLK_CPU_1_ACLK;
        case SYSCTL_CLK_CPU_1_PCLK:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
        {
            if(0 == ((sysctl_media_clk->ai_clk_cfg >> 2) & 0x1))
                return SYSCTL_CLK_ROOT_PLL0_DIV_2;
            else
                return SYSCTL_CLK_ROOT_PLL3_DIV_2;
        }
        case SYSCTL_CLK_AI_ACLK:
        case SYSCTL_CLK_AI_DDRCP3:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
            return SYSCTL_CLK_ROOT_PLL0_DIV_2;
        case SYSCTL_CLK_VPU_ACLK_SRC:
            return SYSCTL_CLK_VPU_SRC;
        case SYSCTL_CLK_VPU_ACLK:
        case SYSCTL_CLK_VPU_DDRCP2:
            return SYSCTL_CLK_VPU_ACLK_SRC;
        case SYSCTL_CLK_VPU_CFG:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        /*--------------------------- AUDIO CLOCK ------------------------------------*/
        case SYSCTL_CLK_AUDIO_PCLK_GATE:
        case SYSCTL_CLK_CODEC_PCLK_GATE:
            return SYSCTL_CLK_LS_PCLK_SRC;

        case SYSCTL_CLK_CODEC_ADC_MCLK:
        case SYSCTL_CLK_CODEC_DAC_MCLK:
        case SYSCTL_CLK_AUDIO_DEV_CLK:
        case SYSCTL_CLK_PDM_CLK:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        /*--------------------------- DDR CLOCK ------------------------------------*/
        case SYSCTL_CLK_DDRC_CORE_CLK:
        {
            switch((sysctl_media_clk->ddr_clk_cfg >> 0) & 0x3)
            {
                case 0:
                    return SYSCTL_CLK_ROOT_PLL0_DIV_2;
                case 1:
                    return SYSCTL_CLK_ROOT_PLL0_DIV_3;
                case 2:
                    return SYSCTL_CLK_ROOT_PLL2_DIV_4;
                default:
                    return SYSCTL_CLK_ROOT_MAX;
            }
        }
        case SYSCTL_CLK_DDRC_BYPASS_GATE:
            return SYSCTL_CLK_ROOT_PLL2_DIV_4;
        case SYSCTL_CLK_DDRC_PCLK:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
            return SYSCTL_CLK_ROOT_PLL1_DIV_4;
        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
            return SYSCTL_CLK_ROOT_PLL2_DIV_4;

        case SYSCTL_CLK_MCLK_0:
        {
            switch((sysctl_media_clk->mclk_cfg >> 3) & 0x3)
            {
                case 0:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_3;
                case 1:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_4;
                case 2:
                    return SYSCTL_CLK_ROOT_PLL0_DIV_4;
                default:
                    return SYSCTL_CLK_ROOT_MAX;
            }
        }
        case SYSCTL_CLK_MCLK_1:
        {
            switch((sysctl_media_clk->mclk_cfg >> 10) & 0x3)
            {
                case 0:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_3;
                case 1:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_4;
                case 2:
                    return SYSCTL_CLK_ROOT_PLL0_DIV_4;
                default:
                    return SYSCTL_CLK_ROOT_MAX;
            }
        }
        case SYSCTL_CLK_MCLK_2:
        {
            switch((sysctl_media_clk->mclk_cfg >> 17) & 0x3)
            {
                case 0:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_3;
                case 1:
                    return SYSCTL_CLK_ROOT_PLL1_DIV_4;
                case 2:
                    return SYSCTL_CLK_ROOT_PLL0_DIV_4;
                default:
                    return SYSCTL_CLK_ROOT_MAX;
            }
        }

        case SYSCTL_CLK_ISP_CLK:
        {
            if(0 == ((sysctl_media_clk->isp_clkdiv_cfg >> 29) & 0x1))
                return SYSCTL_CLK_ROOT_PLL0_DIV_4;
            else
                return SYSCTL_CLK_ROOT_PLL2_DIV_4;
        }
        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            return SYSCTL_CLK_ISP_CLK;

        case SYSCTL_CLK_ISP_HCLK:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        case SYSCTL_CLK_ISP_ACLK_GATE:
        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
        case SYSCTL_CLK_ISP_VSECLK_GATE:
            return SYSCTL_CLK_ROOT_PLL0_DIV_3;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
            return SYSCTL_CLK_ROOT_PLL1_DIV_4;
        case SYSCTL_CLK_DPU_ACLK_GATE:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;
        case SYSCTL_CLK_DPU_PCLK:
            return SYSCTL_CLK_ROOT_PLL0_DIV_4;

        default:
            return SYSCTL_CLK_ROOT_MAX;
    }
}

/* 设置时钟节点enable,注意:只设置本时钟节点的enable，不会设置上游时钟的enable。*/
void sysctl_media_clk_set_leaf_en(sysctl_media_clk_node_e leaf, bool enable)
{
    switch(leaf)
    {
        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
            if(enable == true)
                sysctl_media_clk->cpu1_clk_cfg |= (1 << 0);
            else
                sysctl_media_clk->cpu1_clk_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_CPU_1_PLIC:
            if(enable == true)
                sysctl_media_clk->cpu1_clk_cfg |= (1 << 15);
            else
                sysctl_media_clk->cpu1_clk_cfg &= ~(1 << 15);
            break;

        case SYSCTL_CLK_CPU_1_ACLK:
            break;

        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            if(enable == true)
                sysctl_media_clk->ddr_clk_cfg |= (1 << 3);
            else
                sysctl_media_clk->ddr_clk_cfg &= ~(1 << 3);
            break;

        case SYSCTL_CLK_CPU_1_PCLK:
            if(enable == true)
                sysctl_media_clk->cpu1_clk_cfg |= (1 << 19);
            else
                sysctl_media_clk->cpu1_clk_cfg &= ~(1 << 19);
            break;

        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
            if(enable == true)
                sysctl_media_clk->ai_clk_cfg |= (1 << 0);
            else
                sysctl_media_clk->ai_clk_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_AI_ACLK:
            if(enable == true)
                sysctl_media_clk->ai_clk_cfg |= (1 << 10);
            else
                sysctl_media_clk->ai_clk_cfg &= ~(1 << 10);
            break;

        case SYSCTL_CLK_AI_DDRCP3:
            if(enable == true)
                sysctl_media_clk->ddr_clk_cfg |= (1 << 6);
            else
                sysctl_media_clk->ddr_clk_cfg &= ~(1 << 6);
            break;

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
            if(enable == true)
                sysctl_media_clk->vpu_clk_cfg |= (1 << 0);
            else
                sysctl_media_clk->vpu_clk_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_VPU_ACLK_SRC:
            break;

        case SYSCTL_CLK_VPU_ACLK:
            if(enable == true)
                sysctl_media_clk->vpu_clk_cfg |= (1 << 5);
            else
                sysctl_media_clk->vpu_clk_cfg &= ~(1 << 5);
            break;

        case SYSCTL_CLK_VPU_DDRCP2:
            if(enable == true)
                sysctl_media_clk->ddr_clk_cfg |= (1 << 5);
            else
                sysctl_media_clk->ddr_clk_cfg &= ~(1 << 5);
            break;

        case SYSCTL_CLK_VPU_CFG:
            if(enable == true)
                sysctl_media_clk->vpu_clk_cfg |= (1 << 10);
            else
                sysctl_media_clk->vpu_clk_cfg &= ~(1 << 10);
            break;

        case SYSCTL_CLK_AUDIO_PCLK_GATE:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 13);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 13);
            break;

        case SYSCTL_CLK_ADC_PCLK_GATE:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 15);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 15);
            break;

        case SYSCTL_CLK_CODEC_PCLK_GATE:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 14);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 14);
            break;

        case SYSCTL_CLK_CODEC_ADC_MCLK:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 29);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 29);
            break;

        case SYSCTL_CLK_CODEC_DAC_MCLK:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 30);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 30);
            break;

        case SYSCTL_CLK_AUDIO_DEV_CLK:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 28);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 28);
            break;

        case SYSCTL_CLK_PDM_CLK:
            if(enable == true)
                sysctl_media_clk->ls_clken_cfg0 |= (1 << 31);
            else
                sysctl_media_clk->ls_clken_cfg0 &= ~(1 << 31);
            break;

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 0);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 6);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 6);
            break;

        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 7);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 7);
            break;

        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 8);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 8);
            break;

        case SYSCTL_CLK_MCLK_0:
            if(enable == true)
                sysctl_media_clk->mclk_cfg |= (1 << 0);
            else
                sysctl_media_clk->mclk_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_MCLK_1:
            if(enable == true)
                sysctl_media_clk->mclk_cfg |= (1 << 1);
            else
                sysctl_media_clk->mclk_cfg &= ~(1 << 1);
            break;

        case SYSCTL_CLK_MCLK_2:
            if(enable == true)
                sysctl_media_clk->mclk_cfg |= (1 << 2);
            else
                sysctl_media_clk->mclk_cfg &= ~(1 << 2);
            break;

        case SYSCTL_CLK_ISP_CLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 12);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 12);
            break;

        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 17);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 17);
            break;

        case SYSCTL_CLK_ISP_HCLK:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 13);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 13);
            break;

        case SYSCTL_CLK_ISP_ACLK_GATE:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 14);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 14);
            break;

        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            if(enable == true)
                sysctl_media_clk->ddr_clk_cfg |= (1 << 4);
            else
                sysctl_media_clk->ddr_clk_cfg &= ~(1 << 4);
            break;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 15);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 15);
            break;

        case SYSCTL_CLK_ISP_VSECLK_GATE:
            if(enable == true)
                sysctl_media_clk->isp_clken_cfg |= (1 << 16);
            else
                sysctl_media_clk->isp_clken_cfg &= ~(1 << 16);
            break;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
            if(enable == true)
                sysctl_media_clk->dpu_clk_cfg |= (1 << 0);
            else
                sysctl_media_clk->dpu_clk_cfg &= ~(1 << 0);
            break;

        case SYSCTL_CLK_DPU_ACLK_GATE:
            if(enable == true)
                sysctl_media_clk->dpu_clk_cfg |= (1 << 1);
            else
                sysctl_media_clk->dpu_clk_cfg &= ~(1 << 1);
            break;

        case SYSCTL_CLK_DPU_PCLK:
            if(enable == true)
                sysctl_media_clk->dpu_clk_cfg |= (1 << 2);
            else
                sysctl_media_clk->dpu_clk_cfg &= ~(1 << 2);
            break;

        /*--------------------------- DISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_DISP_GPU:
            if(enable == true)
                sysctl_media_clk->vo_clk_cfg |= (1 << 6);
            else
                sysctl_media_clk->vo_clk_cfg &= ~(1 << 6);
            break;

        case SYSCTL_CLK_DPIPCLK:
            if(enable == true)
                sysctl_media_clk->vo_clk_cfg |= (1 << 2);
            else
                sysctl_media_clk->vo_clk_cfg &= ~(1 << 2);
            break;

        default:
            break;
    }
}

/* 获取本时钟节点的enable状态 */
bool sysctl_meida_clk_get_leaf_en(sysctl_media_clk_node_e leaf)
{
    switch(leaf)
    {
        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
            return (0 == ((sysctl_media_clk->cpu1_clk_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_CPU_1_PLIC:
            return (0 == ((sysctl_media_clk->cpu1_clk_cfg >> 15) & 0x1)) ? false : true;
        case SYSCTL_CLK_CPU_1_ACLK:
            return true;
        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            return (0 == ((sysctl_media_clk->ddr_clk_cfg >> 3) & 0x1)) ? false : true;
        case SYSCTL_CLK_CPU_1_PCLK:
            return (0 == ((sysctl_media_clk->cpu1_clk_cfg >> 19) & 0x1)) ? false : true;

        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
            return (0 == ((sysctl_media_clk->ai_clk_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_AI_ACLK:
            return (0 == ((sysctl_media_clk->ai_clk_cfg >> 10) & 0x1)) ? false : true;
        case SYSCTL_CLK_AI_DDRCP3:
            return (0 == ((sysctl_media_clk->ddr_clk_cfg >> 6) & 0x1)) ? false : true;

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
            return (0 == ((sysctl_media_clk->vpu_clk_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_VPU_ACLK_SRC:
            return true;
        case SYSCTL_CLK_VPU_ACLK:
            return (0 == ((sysctl_media_clk->vpu_clk_cfg >> 5) & 0x1)) ? false : true;
        case SYSCTL_CLK_VPU_DDRCP2:
            return (0 == ((sysctl_media_clk->ddr_clk_cfg >> 5) & 0x1)) ? false : true;
        case SYSCTL_CLK_VPU_CFG:
            return (0 == ((sysctl_media_clk->vpu_clk_cfg >> 10) & 0x1)) ? false : true;

        case SYSCTL_CLK_AUDIO_PCLK_GATE:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 13) & 0x1)) ? false : true;
        case SYSCTL_CLK_CODEC_PCLK_GATE:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 14) & 0x1)) ? false : true;

        case SYSCTL_CLK_CODEC_ADC_MCLK:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 29) & 0x1)) ? false : true;
        case SYSCTL_CLK_CODEC_DAC_MCLK:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 30) & 0x1)) ? false : true;
        case SYSCTL_CLK_AUDIO_DEV_CLK:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 28) & 0x1)) ? false : true;
        case SYSCTL_CLK_PDM_CLK:
            return (0 == ((sysctl_media_clk->ls_clken_cfg0 >> 31) & 0x1)) ? false : true;

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 6) & 0x1)) ? false : true;
        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 7) & 0x1)) ? false : true;
        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 8) & 0x1)) ? false : true;

        case SYSCTL_CLK_MCLK_0:
            return (0 == ((sysctl_media_clk->mclk_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_MCLK_1:
            return (0 == ((sysctl_media_clk->mclk_cfg >> 1) & 0x1)) ? false : true;
        case SYSCTL_CLK_MCLK_2:
            return (0 == ((sysctl_media_clk->mclk_cfg >> 2) & 0x1)) ? false : true;

        case SYSCTL_CLK_ISP_CLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 12) & 0x1)) ? false : true;
        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 17) & 0x1)) ? false : true;

        case SYSCTL_CLK_ISP_HCLK:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 13) & 0x1)) ? false : true;

        case SYSCTL_CLK_ISP_ACLK_GATE:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 14) & 0x1)) ? false : true;
        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 4) & 0x1)) ? false : true;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 15) & 0x1)) ? false : true;
        case SYSCTL_CLK_ISP_VSECLK_GATE:
            return (0 == ((sysctl_media_clk->isp_clken_cfg >> 16) & 0x1)) ? false : true;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
            return (0 == ((sysctl_media_clk->dpu_clk_cfg >> 0) & 0x1)) ? false : true;
        case SYSCTL_CLK_DPU_ACLK_GATE:
            return (0 == ((sysctl_media_clk->dpu_clk_cfg >> 1) & 0x1)) ? false : true;
        case SYSCTL_CLK_DPU_PCLK:
            return (0 == ((sysctl_media_clk->dpu_clk_cfg >> 2) & 0x1)) ? false : true;

        default:
            return true;
    }
}

/* 设置本时钟节点的分频系数, freq = root_freq * numerator / denominator */
bool sysctl_media_clk_set_leaf_div(sysctl_media_clk_node_e leaf, uint32_t numerator, uint32_t denominator)
{
    volatile uint32_t ret;

    if(denominator == 0)
        return false;

    switch(leaf)
    {
        /*--------------------------- CPU0 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_0_SRC:
        {
            if((numerator > 16) || (numerator < 1) || (denominator != 16))
                return false;
            else
            {
                /* 1/16 --- 16/16 */
                ret = sysctl_media_clk->cpu0_clk_cfg;
                ret &= 0xffffffe1;
                sysctl_media_clk->cpu0_clk_cfg = ret | (((numerator - 1) << 1) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_0_PLIC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu0_clk_cfg;
                ret &= 0xffffe3ff;
                sysctl_media_clk->cpu0_clk_cfg = ret | (((denominator - 1) << 10) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_0_ACLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu0_clk_cfg;
                ret &= 0xfffffe3f;
                sysctl_media_clk->cpu0_clk_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_0_NOC_DDRCP4:
            return false;
        case SYSCTL_CLK_CPU_0_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu0_clk_cfg;
                ret &= 0xffffc7ff;
                sysctl_media_clk->cpu0_clk_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->cpu1_clk_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_1_PLIC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xfff8ffff;
                sysctl_media_clk->cpu1_clk_cfg = ret | (((denominator - 1) << 16) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_1_ACLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu1_clk_cfg;
                ret &= 0xffff8fff;
                sysctl_media_clk->cpu1_clk_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            return false;
        
        case SYSCTL_CLK_CPU_1_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->cpu0_clk_cfg;
                ret &= 0xffffc7ff;
                sysctl_media_clk->cpu0_clk_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }
        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->ai_clk_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->ai_clk_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_AI_ACLK:
        case SYSCTL_CLK_AI_DDRCP3:
            return false;

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
        {
            if((numerator > 16) || (numerator < 1) || (denominator != 16))
                return false;
            else
            {
                /* 1/16 --- 16/16 */
                ret = sysctl_media_clk->vpu_clk_cfg;
                ret &= 0xffffffe1;
                sysctl_media_clk->vpu_clk_cfg = ret | (((numerator - 1) << 1) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_VPU_ACLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->vpu_clk_cfg;
                ret &= 0xfffffc3f;
                sysctl_media_clk->vpu_clk_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_VPU_ACLK:
        case SYSCTL_CLK_VPU_DDRCP2:
            return false;
        case SYSCTL_CLK_VPU_CFG:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->vpu_clk_cfg;
                ret &= 0xffff87ff;
                sysctl_media_clk->vpu_clk_cfg = ret | (((denominator - 1) << 11) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- PMU CLOCK ------------------------------------*/
        case SYSCTL_CLK_PMU_PCLK:
            return false;

        /*--------------------------- HS CLOCK ------------------------------------*/
        case SYSCTL_CLK_HS_HCLK_HIGH_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_sdclk_cfg;
                ret &= 0xfffffff8;
                sysctl_media_clk->hs_sdclk_cfg = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_HS_HCLK_HIGH_GATE:
            return false;
        
        case SYSCTL_CLK_HS_HCLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_sdclk_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->hs_sdclk_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SD_0_HCLK_GATE:
        case SYSCTL_CLK_SD_1_HCLK_GATE:
        case SYSCTL_CLK_USB_0_HCLK_GATE:
        case SYSCTL_CLK_USB_1_HCLK_GATE:
        case SYSCTL_CLK_SSI_1_HCLK_GATE:
        case SYSCTL_CLK_SSI_2_HCLK_GATE:
            return false;

        case SYSCTL_CLK_SSI_0_ACLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_spi_cfg;
                ret &= 0xfffff1ff;
                sysctl_media_clk->hs_spi_cfg = ret | (((denominator - 1) << 9) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SSI_1_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_spi_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->hs_spi_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SSI_2_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_spi_cfg;
                ret &= 0xfffffe3f;
                sysctl_media_clk->hs_spi_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_QSPI_ACLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_spi_cfg;
                ret &= 0xffff8fff;
                sysctl_media_clk->hs_spi_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SSI_1_ACLK_GATE:
        case SYSCTL_CLK_SSI_2_ACLK_GATE:
            return false;

        case SYSCTL_CLK_SSI_0_CLK:
            return false;

        case SYSCTL_CLK_SD_ACLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_sdclk_cfg;
                ret &= 0xfffffe3f;
                sysctl_media_clk->hs_sdclk_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SD_0_ACLK_GATE:
        case SYSCTL_CLK_SD_1_ACLK_GATE:
        case SYSCTL_CLK_SD_0_BCLK_GATE:
        case SYSCTL_CLK_SD_1_BCLK_GATE:
            return false;

        case SYSCTL_CLK_SD_CCLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_sdclk_cfg;
                ret &= 0xffff8fff;
                sysctl_media_clk->hs_sdclk_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SD_0_CCLK_GATE:
        case SYSCTL_CLK_SD_1_CCLK_GATE:
            return false;

        case SYSCTL_CLK_PLL0_DIV_16:
            return false;
        case SYSCTL_CLK_USB_REF_50M:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->hs_spi_cfg;
                ret &= 0xfffc7fff;
                sysctl_media_clk->hs_spi_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_USB_0_REF_CLK:
        case SYSCTL_CLK_USB_1_REF_CLK:
            return false;

        case SYSCTL_CLK_SD_TMCLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->hs_sdclk_cfg;
                ret &= 0xfff07fff;
                sysctl_media_clk->hs_sdclk_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SD_0_TMCLK_GATE:
        case SYSCTL_CLK_SD_1_TMCLK_GATE:
            return false;

        /*--------------------------- LS CLOCK ------------------------------------*/
        case SYSCTL_CLK_LS_PCLK_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/4, 1/8 --- 1/8 */
                ret = sysctl_media_clk->ls_clkdiv_cfg;
                ret &= 0xfffffff8;
                sysctl_media_clk->ls_clkdiv_cfg = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_UART_0_PCLK_GATE:
        case SYSCTL_CLK_UART_1_PCLK_GATE:
        case SYSCTL_CLK_UART_2_PCLK_GATE:
        case SYSCTL_CLK_UART_3_PCLK_GATE:
        case SYSCTL_CLK_UART_4_PCLK_GATE:
        case SYSCTL_CLK_I2C_0_PCLK_GATE:
        case SYSCTL_CLK_I2C_1_PCLK_GATE:
        case SYSCTL_CLK_I2C_2_PCLK_GATE:
        case SYSCTL_CLK_I2C_3_PCLK_GATE:
        case SYSCTL_CLK_I2C_4_PCLK_GATE:
        case SYSCTL_CLK_GPIO_PCLK_GATE:
        case SYSCTL_CLK_PWM_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_0_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_1_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_2_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_3_PCLK_GATE:
        case SYSCTL_CLK_AUDIO_PCLK_GATE:
        case SYSCTL_CLK_ADC_PCLK_GATE:
        case SYSCTL_CLK_CODEC_PCLK_GATE:
            return false;

        case SYSCTL_CLK_UART_0_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xfffffff8;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_UART_1_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_UART_2_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xfffffe3f;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_UART_3_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xfffff1ff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 9) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_UART_4_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xffff8fff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_JAMLINKCO_DIV:
        {
            if((numerator != 1) || (denominator < 2) || (denominator > 512) || (denominator % 2 != 0))
                return false;
            else
            {
                /* 1/2, 1/4, 1/8 --- 1/512 */
                ret = sysctl_media_clk->ls_clkdiv_cfg;
                ret &= 0x807fffff;
                sysctl_media_clk->ls_clkdiv_cfg = ret | (((denominator/2 - 1) << 23) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_JAMLINK_0_CO_GATE:
        case SYSCTL_CLK_JAMLINK_1_CO_GATE:
        case SYSCTL_CLK_JAMLINK_2_CO_GATE:
        case SYSCTL_CLK_JAMLINK_3_CO_GATE:
            return false;

        case SYSCTL_CLK_I2C_0_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xfffc7fff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_I2C_1_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xffe3ffff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 18) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_I2C_2_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xff1fffff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 21) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_I2C_3_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xf8ffffff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 24) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_I2C_4_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->uart_i2c_clkdiv_cfg;
                ret &= 0xc7ffffff;
                sysctl_media_clk->uart_i2c_clkdiv_cfg = ret | (((denominator - 1) << 27) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_CODEC_ADC_MCLK:
        {
            if((numerator < 0x10) || (numerator > 0x1B9) || (denominator < 0xC35) || (denominator > 0x3D09))
                return false;
            if(numerator * 2 > denominator)      //根据文档描述，max_sum的设定值至少是inc_step设定值的2倍以上
                return false;

            ret = sysctl_media_clk->codec_adc_mclkdiv_cfg;
            ret &= 0xf8000000;
            sysctl_media_clk->codec_adc_mclkdiv_cfg = ret | ((numerator << 14) | (denominator << 0) | (1 << 31));
            return true;
        }
        case SYSCTL_CLK_CODEC_DAC_MCLK:
        {
            if((numerator < 0x10) || (numerator > 0x1B9) || (denominator < 0xC35) || (denominator > 0x3D09))
                return false;
            if(numerator * 2 > denominator)      //根据文档描述，max_sum的设定值至少是inc_step设定值的2倍以上
                return false;

            ret = sysctl_media_clk->codec_dac_mclkdiv_cfg;
            ret &= 0xf8000000;
            sysctl_media_clk->codec_dac_mclkdiv_cfg = ret | ((numerator << 14) | (denominator << 0) | (1 << 31));
            return true;
        }
        case SYSCTL_CLK_AUDIO_DEV_CLK:
        {
            if((numerator < 0x4) || (numerator > 0x1B9) || (denominator < 0xC35) || (denominator > 0xF424))
                return false;
            if(numerator * 2 > denominator)      //根据文档描述，max_sum的设定值至少是inc_step设定值的2倍以上
                return false;

            ret = sysctl_media_clk->audio_clkdiv_cfg;
            ret &= 0x80000000;
            sysctl_media_clk->audio_clkdiv_cfg = ret | ((numerator << 16) | (denominator << 0) | (1 << 31));
            // ret |= ((numerator << 16) | (denominator << 0));
            // ret |= (1 << 31);
            // sysctl_media_clk->audio_clkdiv_cfg = ret;
            return true;
        }

        //review。注意，此处需要与clock_provider.dtsi保持一致，dtsi可能会被改变。
        //后者不需要按照dtsi再修改，此处代码已经写好，即在配置寄存器的时候，配置两个寄存器即可。
        case SYSCTL_CLK_PDM_CLK:
        {
            if((numerator < 0x2) || (numerator > 0x1B9) || (denominator < 0xC35) || (denominator > 0x1E848))
                return false;
            if(numerator * 2 > denominator)      //根据文档描述，max_sum的设定值至少是inc_step设定值的2倍以上
                return false;

            // sysctl_media_clk->pdm_clkdiv_cfg0 = ((numerator << 32) | (denominator << 0) | (1 << 63));
            sysctl_media_clk->pdm_clkdiv_cfg0 = (denominator << 0);
            sysctl_media_clk->pdm_clkdiv_cfg1 = ((numerator << 0) | (1 << 31));
            return true;
        }
    #if 0
        case SYSCTL_CLK_SUM_DIV:
        {
            if((numerator != 1) || (denominator < 3125) || (denominator > 125000))
                return false;
            else
            {
                /* 1/3125 --- 1/125000 */
                ret = sysctl_media_clk->pdm_clkdiv_cfg0;
                ret &= 0xfffe0000;
                sysctl_media_clk->pdm_clkdiv_cfg0 = ret | ((denominator) << 0);
                return true;
            }
        }

        case SYSCTL_CLK_STEP_DIV:
        {
            if((numerator > 441) || (numerator < 2) || (denominator != 1))
                return false;
            else
            {
                /* 2/1 --- 441/1 */
                ret = sysctl_media_clk->pdm_clkdiv_cfg1;
                ret &= 0xffff0000;
                sysctl_media_clk->pdm_clkdiv_cfg1 = ret | (((numerator) << 0) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_PDM_CLK_GATE:
            return false;
#endif

        case SYSCTL_CLK_ADC_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 1024))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/1024 */
                ret = sysctl_media_clk->ls_clkdiv_cfg;
                ret &= 0xffffe007;
                sysctl_media_clk->ls_clkdiv_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_GPIO_DBCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 1024))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/1024 */
                ret = sysctl_media_clk->ls_clkdiv_cfg;
                ret &= 0xff801fff;
                sysctl_media_clk->ls_clkdiv_cfg = ret | (((denominator - 1) << 13) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- SYSCTL CLOCK ------------------------------------*/
        case SYSCTL_CLK_PCLK:
            return false;
        case SYSCTL_CLK_WDT_0_PCLK_GATE:
        case SYSCTL_CLK_WDT_1_PCLK_GATE:
        case SYSCTL_CLK_TIMER_PCLK_GATE:
        case SYSCTL_CLK_IOMUX_PCLK_GATE:
        case SYSCTL_CLK_MAILBOX_PCLK_GATE:
            return false;

        case SYSCTL_CLK_HDI_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->sysctl_clk_div_cfg;
                ret &= 0x8fffffff;
                sysctl_media_clk->sysctl_clk_div_cfg = ret | (((denominator - 1) << 28) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_STC_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->sysctl_clk_div_cfg;
                ret &= 0xfff07fff;
                sysctl_media_clk->sysctl_clk_div_cfg = ret | (((denominator - 1) << 15) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_TS_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 256))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/256 */
                ret = sysctl_media_clk->sysctl_clk_div_cfg;
                ret &= 0xf00fffff;
                sysctl_media_clk->sysctl_clk_div_cfg = ret | (((denominator - 1) << 20) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- TIMER CLOCK ------------------------------------*/
        case SYSCTL_CLK_TIMERX_PULSE_IN:
            return false;
        case SYSCTL_CLK_TIMER_0_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->timer_clk_cfg;
                ret &= 0xfffffff8;
                sysctl_media_clk->timer_clk_cfg = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_TIMER_0_CLK:
            return false;

        case SYSCTL_CLK_TIMER_1_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->timer_clk_cfg;
                ret &= 0xffffffc7;
                sysctl_media_clk->timer_clk_cfg = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_TIMER_1_CLK:
            return false;

        case SYSCTL_CLK_TIMER_2_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->timer_clk_cfg;
                ret &= 0xfffffe3f;
                sysctl_media_clk->timer_clk_cfg = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_TIMER_2_CLK:
            return false;

        case SYSCTL_CLK_TIMER_3_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->timer_clk_cfg;
                ret &= 0xfffff1ff;
                sysctl_media_clk->timer_clk_cfg = ret | (((denominator - 1) << 9) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_TIMER_3_CLK:
            return false;

        case SYSCTL_CLK_TIMER_4_SRC:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->timer_clk_cfg;
                ret &= 0xffff8fff;
                sysctl_media_clk->timer_clk_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_TIMER_4_CLK:
            return false;

        /*--------------------------- SHRM CLOCK ------------------------------------*/
        case SYSCTL_CLK_SHRM_SRC:
            return false;
        case SYSCTL_CLK_SHRM_DIV_2:
            return false;
        case SYSCTL_CLK_SHRM_AXIS_CLK_GATE:
        case SYSCTL_CLK_DECOMPRESS_ACLK_GATE:
            return false;

        case SYSCTL_CLK_SHRM_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->shrm_clk_cfg;
                ret &= 0xffe3ffff;
                sysctl_media_clk->shrm_clk_cfg = ret | (((denominator - 1) << 18) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_SHRM_AXIM_CLK_GATE:
        case SYSCTL_CLK_GSDMA_ACLK_GATE:
        case SYSCTL_CLK_NONAI2D_ACLK_GATE:
        case SYSCTL_CLK_PDMA_ACLK_GATE:
            return false;

        /*--------------------------- DDR CLOCK ------------------------------------*/
        case SYSCTL_CLK_DDRC_CORE_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->ddr_clk_cfg;
                ret &= 0xffffc3ff;
                sysctl_media_clk->ddr_clk_cfg = ret | (((denominator - 1) << 10) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_DDRC_BYPASS_GATE:
            return false;
        case SYSCTL_CLK_DDRC_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->ddr_clk_cfg;
                ret &= 0xfffc3fff;
                sysctl_media_clk->ddr_clk_cfg = ret | (((denominator - 1) << 14) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xffffffe0;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 64))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/64 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xfffff81f;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 5) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 64))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/64 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xfffe07ff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 11) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 64))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/64 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xff81ffff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 17) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_MCLK_0:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfffffc1f;
                sysctl_media_clk->mclk_cfg = ret | (((denominator - 1) << 5) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_MCLK_1:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xfffe0fff;
                sysctl_media_clk->mclk_cfg = ret | (((denominator - 1) << 12) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_MCLK_2:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->mclk_cfg;
                ret &= 0xff07ffff;
                sysctl_media_clk->mclk_cfg = ret | (((denominator - 1) << 19) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_ISP_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xe3ffffff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 26) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            return false;

        case SYSCTL_CLK_ISP_HCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->isp_clkdiv_cfg;
                ret &= 0xfc7fffff;
                sysctl_media_clk->isp_clkdiv_cfg = ret | (((denominator - 1) << 23) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_ISP_ACLK_GATE:
        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            return false;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
        case SYSCTL_CLK_ISP_VSECLK_GATE:
            return false;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
        {
            if((numerator > 16) || (numerator < 1) || (denominator != 16))
                return false;
            else
            {
                /* 1/16, 2/16, 3/16 --- 16/16 */
                ret = sysctl_media_clk->dpu_clk_cfg;
                ret &= 0xffffff87;
                sysctl_media_clk->dpu_clk_cfg = ret | (((numerator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_DPU_ACLK_GATE:
            return false;
        case SYSCTL_CLK_DPU_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->dpu_clk_cfg;
                ret &= 0xfffffc7f;
                sysctl_media_clk->dpu_clk_cfg = ret | (((denominator - 1) << 7) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- DISPLAY CLOCK ------------------------------------*/
        case SYSCTL_CLK_DISP_HCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->disp_clk_div;
                ret &= 0xfffffff8;
                sysctl_media_clk->disp_clk_div = ret | (((denominator - 1) << 0) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_DISP_ACLK_GATE:
            return false;

        case SYSCTL_CLK_DISP_CLKEXT:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->disp_clk_div;
                ret &= 0xfff0ffff;
                sysctl_media_clk->disp_clk_div = ret | (((denominator - 1) << 16) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_DISP_GPU:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->disp_clk_div;
                ret &= 0xff0fffff;
                sysctl_media_clk->disp_clk_div = ret | (((denominator - 1) << 20) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_DPIPCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 256))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/256 */
                ret = sysctl_media_clk->disp_clk_div;
                ret &= 0xfffff807;
                sysctl_media_clk->disp_clk_div = ret | (((denominator - 1) << 3) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_DISP_CFGCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->disp_clk_div;
                ret &= 0xffff07ff;
                sysctl_media_clk->disp_clk_div = ret | (((denominator - 1) << 11) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_DISP_REFCLK_GATE:
            return false;

        /*--------------------------- SEC CLOCK ------------------------------------*/
        case SYSCTL_CLK_SEC_PCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->sec_clk_div;
                ret &= 0xfffffff1;
                sysctl_media_clk->sec_clk_div = ret | (((denominator - 1) << 1) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_SEC_FIXCLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 32))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/32 */
                ret = sysctl_media_clk->sec_clk_div;
                ret &= 0xfffff83f;
                sysctl_media_clk->sec_clk_div = ret | (((denominator - 1) << 6) | (1 << 31));
                return true;
            }
        }

        case SYSCTL_CLK_SEC_ACLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->sec_clk_div;
                ret &= 0xffffc7ff;
                sysctl_media_clk->sec_clk_div = ret | (((denominator - 1) << 11) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- USB TEST MODE CLOCK ------------------------------------*/
        case SYSCTL_CLK_USB_CLK_480:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->usb_test_clk_div;
                ret &= 0xfffffff1;
                sysctl_media_clk->usb_test_clk_div = ret | (((denominator - 1) << 1) | (1 << 31));
                return true;
            }
        }
        case SYSCTL_CLK_USB_CLK_100:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->usb_test_clk_div;
                ret &= 0xffffff8f;
                sysctl_media_clk->usb_test_clk_div = ret | (((denominator - 1) << 4) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- DPHY DFT MODE CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPHY_TEST_CLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 16))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/16 */
                ret = sysctl_media_clk->dphy_test_clk_div;
                ret &= 0xffffffe1;
                sysctl_media_clk->dphy_test_clk_div = ret | (((denominator - 1) << 1) | (1 << 31));
                return true;
            }
        }

        /*--------------------------- SPI2AXI CLOCK ------------------------------------*/
        case SYSCTL_CLK_SPI2AXI_ACLK:
        {
            if((numerator != 1) || (denominator < 1) || (denominator > 8))
                return false;
            else
            {
                /* 1/1, 1/2, 1/3 --- 1/8 */
                ret = sysctl_media_clk->spi2axi_clk_div;
                ret &= 0xfffffff1;
                sysctl_media_clk->spi2axi_clk_div = ret | (((denominator - 1) << 1) | (1 << 31));
                return true;
            }
        }

        default:
            return false;
    }
}

/* 获取本时钟节点的分频系数 */
double sysctl_media_clk_get_leaf_div(sysctl_media_clk_node_e leaf)
{
    switch(leaf)
    {
        /*--------------------------- CPU0 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_0_SRC:
            return (double)(((sysctl_media_clk->cpu0_clk_cfg >> 1) & 0xF) + 1) / 16.0;    /* 1/16 --- 16/16 */
        case SYSCTL_CLK_CPU_0_PLIC:
            return 1.0/(double)(((sysctl_media_clk->cpu0_clk_cfg >> 10) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_CPU_0_ACLK:
            return 1.0/(double)(((sysctl_media_clk->cpu0_clk_cfg >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_CPU_0_NOC_DDRCP4:
            return 1;
        case SYSCTL_CLK_CPU_0_PCLK:
            return 1.0/(double)(((sysctl_media_clk->cpu0_clk_cfg >> 15) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        /*--------------------------- CPU1 CLOCK ------------------------------------*/
        case SYSCTL_CLK_CPU_1_SRC:
            return 1.0/(double)(((sysctl_media_clk->cpu1_clk_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_CPU_1_PLIC:
            return 1.0/(double)(((sysctl_media_clk->cpu1_clk_cfg >> 16) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_CPU_1_ACLK:
            return 1.0/(double)(((sysctl_media_clk->cpu1_clk_cfg >> 12) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_CPU_1_NOC_DDRCP0:
            return 1;
        case SYSCTL_CLK_CPU_1_PCLK:
            return 1.0/(double)(((sysctl_media_clk->cpu0_clk_cfg >> 15) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        /*--------------------------- AI CLOCK ------------------------------------*/
        case SYSCTL_CLK_AI_SRC:
            return 1.0/(double)(((sysctl_media_clk->ai_clk_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_AI_ACLK:
        case SYSCTL_CLK_AI_DDRCP3:
            return 1;

        /*--------------------------- VPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_VPU_SRC:
            return (double)(((sysctl_media_clk->vpu_clk_cfg >> 1) & 0xF) + 1) / 16.0;    /* 1/16 --- 16/16 */
        case SYSCTL_CLK_VPU_ACLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->vpu_clk_cfg >> 6) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */
        case SYSCTL_CLK_VPU_ACLK:
        case SYSCTL_CLK_VPU_DDRCP2:
            return 1;
        case SYSCTL_CLK_VPU_CFG:
            return 1.0/(double)(((sysctl_media_clk->vpu_clk_cfg >> 11) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */

        /*--------------------------- PMU CLOCK ------------------------------------*/
        case SYSCTL_CLK_PMU_PCLK:
            return 1;

        /*--------------------------- HS CLOCK ------------------------------------*/
        case SYSCTL_CLK_HS_HCLK_HIGH_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_sdclk_cfg >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_HS_HCLK_HIGH_GATE:
            return 1;
        case SYSCTL_CLK_HS_HCLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_sdclk_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SD_0_HCLK_GATE:
        case SYSCTL_CLK_SD_1_HCLK_GATE:
        case SYSCTL_CLK_USB_0_HCLK_GATE:
        case SYSCTL_CLK_USB_1_HCLK_GATE:
        case SYSCTL_CLK_SSI_1_HCLK_GATE:
        case SYSCTL_CLK_SSI_2_HCLK_GATE:
            return 1;

        case SYSCTL_CLK_SSI_0_ACLK:
            return 1.0/(double)(((sysctl_media_clk->hs_spi_cfg >> 9) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SSI_1_CLK:
            return 1.0/(double)(((sysctl_media_clk->hs_spi_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SSI_2_CLK:
            return 1.0/(double)(((sysctl_media_clk->hs_spi_cfg >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_QSPI_ACLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_spi_cfg >> 12) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SSI_1_ACLK_GATE:
        case SYSCTL_CLK_SSI_2_ACLK_GATE:
            return 1;

        case SYSCTL_CLK_SSI_0_CLK:
            return 1;

        case SYSCTL_CLK_SD_ACLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_sdclk_cfg >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SD_0_ACLK_GATE:
        case SYSCTL_CLK_SD_1_ACLK_GATE:
        case SYSCTL_CLK_SD_0_BCLK_GATE:
        case SYSCTL_CLK_SD_1_BCLK_GATE:
            return 1;

        case SYSCTL_CLK_SD_CCLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_sdclk_cfg >> 6) & 0x7) + 1);      /* maxinum = 1/2, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SD_0_CCLK_GATE:
        case SYSCTL_CLK_SD_1_CCLK_GATE:
            return 1;

        case SYSCTL_CLK_PLL0_DIV_16:
            return 1.0/16;
        case SYSCTL_CLK_USB_REF_50M:
            return 1.0/(double)(((sysctl_media_clk->hs_spi_cfg >> 15) & 0x7) + 1);        /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_USB_0_REF_CLK:
        case SYSCTL_CLK_USB_1_REF_CLK:
            return 1;

        case SYSCTL_CLK_SD_TMCLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->hs_sdclk_cfg >> 15) & 0x1F) + 1);      /* maxinum = 1/24, 1/24 --- 1/32 */
        case SYSCTL_CLK_SD_0_TMCLK_GATE:
        case SYSCTL_CLK_SD_1_TMCLK_GATE:
            return 1;

        /*--------------------------- LS CLOCK ------------------------------------*/
        case SYSCTL_CLK_LS_PCLK_SRC:
            return 1.0/(double)(((sysctl_media_clk->ls_clkdiv_cfg >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_UART_0_PCLK_GATE:
        case SYSCTL_CLK_UART_1_PCLK_GATE:
        case SYSCTL_CLK_UART_2_PCLK_GATE:
        case SYSCTL_CLK_UART_3_PCLK_GATE:
        case SYSCTL_CLK_UART_4_PCLK_GATE:
        case SYSCTL_CLK_I2C_0_PCLK_GATE:
        case SYSCTL_CLK_I2C_1_PCLK_GATE:
        case SYSCTL_CLK_I2C_2_PCLK_GATE:
        case SYSCTL_CLK_I2C_3_PCLK_GATE:
        case SYSCTL_CLK_I2C_4_PCLK_GATE:
        case SYSCTL_CLK_GPIO_PCLK_GATE:
        case SYSCTL_CLK_PWM_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_0_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_1_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_2_PCLK_GATE:
        case SYSCTL_CLK_JAMLINK_3_PCLK_GATE:
        case SYSCTL_CLK_AUDIO_PCLK_GATE:
        case SYSCTL_CLK_ADC_PCLK_GATE:
        case SYSCTL_CLK_CODEC_PCLK_GATE:
            return 1;

        case SYSCTL_CLK_UART_0_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_UART_1_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_UART_2_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_UART_3_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 9) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_UART_4_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 12) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        case SYSCTL_CLK_JAMLINKCO_DIV:
            return 1.0/(double)(2 * (((sysctl_media_clk->ls_clkdiv_cfg >> 23) & 0xFF) + 1));      /* 1/2, 1/4, 1/8 --- 1/512 */
        case SYSCTL_CLK_JAMLINK_0_CO_GATE:
        case SYSCTL_CLK_JAMLINK_1_CO_GATE:
        case SYSCTL_CLK_JAMLINK_2_CO_GATE:
        case SYSCTL_CLK_JAMLINK_3_CO_GATE:
            return 1;

        case SYSCTL_CLK_I2C_0_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 15) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_I2C_1_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 18) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_I2C_2_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 21) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_I2C_3_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 24) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_I2C_4_CLK:
            return 1.0/(double)(((sysctl_media_clk->uart_i2c_clkdiv_cfg >> 27) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
#if 0
        case SYSCTL_CLK_CODEC_ADC_MCLK:
            return (double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 14) & 0x1B9)/(double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 0) & 0x3D09);
        case SYSCTL_CLK_CODEC_DAC_MCLK:
            return (double)((sysctl_media_clk->codec_dac_mclkdiv_cfg >> 14) & 0x1B9)/(double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 0) & 0x3D09);
        case SYSCTL_CLK_AUDIO_DEV_CLK:
            return (double)((sysctl_media_clk->audio_clkdiv_cfg >> 16) & 0x1B9)/(double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 0) & 0xF424);

        //review。注意，此处需要与clock_provider.dtsi保持一致，dtsi可能会被改变。
        //后者不需要按照dtsi再修改，此处代码已经写好，即在配置寄存器的时候，配置两个寄存器即可。
        // case SYSCTL_CLK_PDM_CLK:
        //     return (double)((sysctl_media_clk->pdm_clkdiv_cfg1 >> 0) & 0x1B9)/(double)((sysctl_media_clk->pdm_clkdiv_cfg0 >> 0) & 0x1E848);
        case SYSCTL_CLK_SUM_DIV:
            return 1.0/(double)((sysctl_media_clk->pdm_clkdiv_cfg0 >> 0) & 0x1E848);
        case SYSCTL_CLK_STEP_DIV:
            return (double)((sysctl_media_clk->pdm_clkdiv_cfg1 >> 0) & 0x1B9) / 1.0;
        case SYSCTL_CLK_PDM_CLK_GATE:
            return 1;
#endif
        case SYSCTL_CLK_CODEC_ADC_MCLK:
            return (double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 14) & 0x1fff)/(double)((sysctl_media_clk->codec_adc_mclkdiv_cfg >> 0) & 0x3fff);
        case SYSCTL_CLK_CODEC_DAC_MCLK:
            return (double)((sysctl_media_clk->codec_dac_mclkdiv_cfg >> 14) & 0x1fff)/(double)((sysctl_media_clk->codec_dac_mclkdiv_cfg >> 0) & 0x3fff);
        case SYSCTL_CLK_AUDIO_DEV_CLK:
            return (double)((sysctl_media_clk->audio_clkdiv_cfg >> 16) & 0x7fff)/(double)((sysctl_media_clk->audio_clkdiv_cfg >> 0) & 0xffff);       
        case SYSCTL_CLK_PDM_CLK:
            return (double)((sysctl_media_clk->pdm_clkdiv_cfg1 >> 0) & 0xffff)/(double)((sysctl_media_clk->pdm_clkdiv_cfg0 >> 0) & 0x1ffff);


#if 0
        //review。注意，此处需要与clock_provider.dtsi保持一致，dtsi可能会被改变。
        //后者不需要按照dtsi再修改，此处代码已经写好，即在配置寄存器的时候，配置两个寄存器即可。
        // case SYSCTL_CLK_PDM_CLK:
        //     return (double)((sysctl_media_clk->pdm_clkdiv_cfg1 >> 0) & 0x1B9)/(double)((sysctl_media_clk->pdm_clkdiv_cfg0 >> 0) & 0x1E848);
        case SYSCTL_CLK_SUM_DIV:
            return 1.0/(double)((sysctl_media_clk->pdm_clkdiv_cfg0 >> 0) & 0x1ffff);
        case SYSCTL_CLK_STEP_DIV:
            return (double)((sysctl_media_clk->pdm_clkdiv_cfg1 >> 0) & 0xffff) / 1.0;
        case SYSCTL_CLK_PDM_CLK_GATE:
            return 1;
#endif

        case SYSCTL_CLK_ADC_CLK:
            return 1.0/(double)(((sysctl_media_clk->ls_clkdiv_cfg >> 3) & 0x3FF) + 1);      /* 1/1, 1/2, 1/3 --- 1/1024 */

        case SYSCTL_CLK_GPIO_DBCLK:
            return 1.0/(double)(((sysctl_media_clk->ls_clkdiv_cfg >> 13) & 0x3FF) + 1);      /* 1/1, 1/2, 1/3 --- 1/1024 */

        /*--------------------------- SYSCTL CLOCK ------------------------------------*/
        case SYSCTL_CLK_PCLK:
            // return 1.0/(double)(((sysctl_media_clk->sysctl_clk_div_cfg >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
            return 1;
        case SYSCTL_CLK_WDT_0_PCLK_GATE:
        case SYSCTL_CLK_WDT_1_PCLK_GATE:
        case SYSCTL_CLK_TIMER_PCLK_GATE:
        case SYSCTL_CLK_IOMUX_PCLK_GATE:
        case SYSCTL_CLK_MAILBOX_PCLK_GATE:
            return 1;

        case SYSCTL_CLK_HDI_CLK:
            return 1.0/(double)(((sysctl_media_clk->sysctl_clk_div_cfg >> 28) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        case SYSCTL_CLK_STC_CLK:
            return 1.0/(double)(((sysctl_media_clk->sysctl_clk_div_cfg >> 15) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */

        case SYSCTL_CLK_TS_CLK:
            return 1.0/(double)(((sysctl_media_clk->sysctl_clk_div_cfg >> 20) & 0xFF) + 1);      /* 1/1, 1/2, 1/3 --- 1/256 */

        /*--------------------------- TIMER CLOCK ------------------------------------*/
        case SYSCTL_CLK_TIMERX_PULSE_IN:
            return 1;
        case SYSCTL_CLK_TIMER_0_SRC:
            return 1.0/(double)(((sysctl_media_clk->timer_clk_cfg >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_TIMER_0_CLK:
            return 1;

        case SYSCTL_CLK_TIMER_1_SRC:
            return 1.0/(double)(((sysctl_media_clk->timer_clk_cfg >> 3) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_TIMER_1_CLK:
            return 1;

        case SYSCTL_CLK_TIMER_2_SRC:
            return 1.0/(double)(((sysctl_media_clk->timer_clk_cfg >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_TIMER_2_CLK:
            return 1;

        case SYSCTL_CLK_TIMER_3_SRC:
            return 1.0/(double)(((sysctl_media_clk->timer_clk_cfg >> 9) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_TIMER_3_CLK:
            return 1;
        
        case SYSCTL_CLK_TIMER_4_SRC:
            return 1.0/(double)(((sysctl_media_clk->timer_clk_cfg >> 12) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_TIMER_4_CLK:
            return 1;


        /*--------------------------- SHRM CLOCK ------------------------------------*/
        case SYSCTL_CLK_SHRM_SRC:
            return 1;
        case SYSCTL_CLK_SHRM_DIV_2:
            return 1.0/2;
        case SYSCTL_CLK_SHRM_AXIS_CLK_GATE:
        case SYSCTL_CLK_DECOMPRESS_ACLK_GATE:
            return 1;

        case SYSCTL_CLK_SHRM_PCLK:
            return 1.0/(double)(((sysctl_media_clk->shrm_clk_cfg >> 18) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        case SYSCTL_CLK_SHRM_AXIM_CLK_GATE:
        case SYSCTL_CLK_GSDMA_ACLK_GATE:
        case SYSCTL_CLK_NONAI2D_ACLK_GATE:
        case SYSCTL_CLK_PDMA_ACLK_GATE:
            return 1;

        /*--------------------------- DDR CLOCK ------------------------------------*/
        case SYSCTL_CLK_DDRC_CORE_CLK:
            return 1.0/(double)(((sysctl_media_clk->ddr_clk_cfg >> 10) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */
        case SYSCTL_CLK_DDRC_BYPASS_GATE:
            return 1;
        case SYSCTL_CLK_DDRC_PCLK:
            return 1.0/(double)(((sysctl_media_clk->ddr_clk_cfg >> 14) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */

        /*--------------------------- ISP CLOCK ------------------------------------*/
        case SYSCTL_CLK_ISP_CFG_CLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 0) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */
        case SYSCTL_CLK_CSI_0_PIXEL_CLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 5) & 0x3F) + 1);      /* 1/1, 1/2, 1/3 --- 1/64 */
        case SYSCTL_CLK_CSI_1_PIXEL_CLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 11) & 0x3F) + 1);      /* 1/1, 1/2, 1/3 --- 1/64 */
        case SYSCTL_CLK_CSI_2_PIXEL_CLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 17) & 0x3F) + 1);      /* 1/1, 1/2, 1/3 --- 1/64 */

        case SYSCTL_CLK_MCLK_0:
            return 1.0/(double)(((sysctl_media_clk->mclk_cfg >> 5) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */
        case SYSCTL_CLK_MCLK_1:
            return 1.0/(double)(((sysctl_media_clk->mclk_cfg >> 12) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */
        case SYSCTL_CLK_MCLK_2:
            return 1.0/(double)(((sysctl_media_clk->mclk_cfg >> 19) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */

        case SYSCTL_CLK_ISP_CLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 26) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_ISP_MEM_CLK_GATE:
            return 1;

        case SYSCTL_CLK_ISP_HCLK:
            return 1.0/(double)(((sysctl_media_clk->isp_clkdiv_cfg >> 23) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        case SYSCTL_CLK_ISP_ACLK_GATE:
        case SYSCTL_CLK_DDRC_P1CLK_GATE:
            return 1;

        case SYSCTL_CLK_ISP_DWECLK_GATE:
        case SYSCTL_CLK_ISP_VSECLK_GATE:
            return 1;

        /*--------------------------- DPU CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPU_CLK:
            return (double)(((sysctl_media_clk->dpu_clk_cfg >> 3) & 0xF) + 1) / 16.0;    /* 1/16 --- 16/16 */
        case SYSCTL_CLK_DPU_ACLK_GATE:
            return 1;
        case SYSCTL_CLK_DPU_PCLK:
            return 1.0/(double)(((sysctl_media_clk->dpu_clk_cfg >> 7) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        /*--------------------------- DISPLAY CLOCK ------------------------------------*/
        case SYSCTL_CLK_DISP_HCLK:
            return 1.0/(double)(((sysctl_media_clk->disp_clk_div >> 0) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_DISP_ACLK_GATE:
            return 1;

        case SYSCTL_CLK_DISP_CLKEXT:
            return 1.0/(double)(((sysctl_media_clk->disp_clk_div >> 16) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */
        case SYSCTL_CLK_DISP_GPU:
            return 1.0/(double)(((sysctl_media_clk->disp_clk_div >> 20) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */

        case SYSCTL_CLK_DPIPCLK:
            return 1.0/(double)(((sysctl_media_clk->disp_clk_div >> 3) & 0xFF) + 1);      /* 1/1, 1/2, 1/3 --- 1/256 */
        case SYSCTL_CLK_DISP_CFGCLK:
            return 1.0/(double)(((sysctl_media_clk->disp_clk_div >> 11) & 0x1F) + 1);      /* 1/1, 1/2, 1/3 --- 1/32 */

        case SYSCTL_CLK_DISP_REFCLK_GATE:
            return 1;

        /*--------------------------- SEC CLOCK ------------------------------------*/
        case SYSCTL_CLK_SEC_PCLK:
            return 1.0/(double)(((sysctl_media_clk->sec_clk_div >> 1) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_SEC_FIXCLK:
            return 1.0/(double)(((sysctl_media_clk->sec_clk_div >> 6) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        case SYSCTL_CLK_SEC_ACLK:
            return 1.0/(double)(((sysctl_media_clk->sec_clk_div >> 11) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        /*--------------------------- USB TEST MODE CLOCK ------------------------------------*/
        case SYSCTL_CLK_USB_CLK_480:
            return 1.0/(double)(((sysctl_media_clk->usb_test_clk_div >> 1) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */
        case SYSCTL_CLK_USB_CLK_100:
            return 1.0/(double)(((sysctl_media_clk->usb_test_clk_div >> 4) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        /*--------------------------- DPHY DFT MODE CLOCK ------------------------------------*/
        case SYSCTL_CLK_DPHY_TEST_CLK:
            return 1.0/(double)(((sysctl_media_clk->dphy_test_clk_div >> 1) & 0xF) + 1);      /* 1/1, 1/2, 1/3 --- 1/16 */

        /*--------------------------- SPI2AXI CLOCK ------------------------------------*/
        case SYSCTL_CLK_SPI2AXI_ACLK:
            return 1.0/(double)(((sysctl_media_clk->spi2axi_clk_div >> 1) & 0x7) + 1);      /* 1/1, 1/2, 1/3 --- 1/8 */

        default:
            return 1;
    }
}

/* 计算本时钟节点的频率。它会搜索整个时钟路径，从时钟源开始计算每一级的分频，最终得出当前时钟节点的频率
*  1. 对于根节点的clock，利用sysctl_boot_get_root_clk_freq(node)获取clock的频率（此时分频系数div=1）；
*  2. 对于叶子节点的clock，利用while循环搜索叶子节点的根节点并计算出整条路径上的div，找到根节点后，利用
*     sysctl_boot_get_root_clk_freq(node) * div计算出叶子节点的频率。
 */
uint32_t sysctl_media_clk_get_leaf_freq(sysctl_media_clk_node_e leaf)
{
    double div = 1.0;
    sysctl_media_clk_node_e node;

    node = leaf;

    if(node == SYSCTL_CLK_TIMERX_PULSE_IN)
    {
        return (uint32_t)(50000000 * div);
    }

    /* calc leaf chain div */
    while(node > SYSCTL_CLK_ROOT_MAX)
    {
        div *= sysctl_media_clk_get_leaf_div(node);
        node = sysctl_media_clk_get_leaf_parent(node);
    }

    /* get root freq and calc leaf freq */
    return (uint32_t)(sysctl_media_boot_get_root_clk_freq(node) * div);
}

/**
 * 实际上，本函数主要针对小数分频。
 * 
 * method=1/2，本函数会根据父节点时钟、实际需要的时钟，计算分频系数；
 * method=3，本函数会根据输入节点、实际需要的时钟，计算分频系数
*/
int sysctl_media_clk_find_approximate(sysctl_media_clk_node_e leaf,
                                     uint32_t mul_min, 
                                     uint32_t mul_max, 
                                     uint32_t div_min, 
                                     uint32_t div_max, 
                                     sysctl_media_clk_mul_div_methord_e method,
                                     unsigned long rate, 
                                     unsigned long parent_rate,
                                     uint32_t *div, 
                                     uint32_t *mul)
{
    long abs_min;
    long abs_current;
    /* we used interger to instead of float */
    long perfect_divide;

    uint32_t a,b;
    int i = 0;
    int n = 0;
    unsigned long mul_ulong,div_ulong;

    volatile uint32_t codec_clk[9] = {
        2048000,
        3072000,
        4096000,
        6144000,
        8192000,
        11289600,
        12288000,
        24576000,
        49152000
    };
    volatile uint32_t codec_div[9][2] = {
        {3125, 16},
        {3125, 24},
        {3125, 32},
        {3125, 48},
        {3125, 64},
        {15625, 441},
        {3125, 96},
        {3125, 192},
        {3125, 384}
    };

    volatile uint32_t pdm_clk[20] = {
        128000,
        192000,
        256000,
        384000,
        512000,
        768000,
        1024000,
        1411200,
        1536000,
        2048000,
        2822400,
        3072000,
        4096000,
        5644800,
        6144000,
        8192000,
        11289600,
        12288000,
        24576000,
        49152000
    };
    volatile uint32_t pdm_div[20][2] = {
        {3125, 1},
        {6250, 3},
        {3125, 2},
        {3125, 3},
        {3125, 4},
        {3125, 6},
        {3125, 8},
        {125000, 441},
        {3125, 12},
        {3125, 16},
        {62500, 441},
        {3125, 24},
        {3125, 32},
        {31250, 441},
        {3125, 48},
        {3125, 64},
        {15625, 441},
        {3125, 96},
        {3125, 192},
        {3125, 384}
    };

    switch(method) {

        /* only mul can be changeable, 1/8,2/8,3/8...*/
        case SYSCTL_CLK_MUL_CHANGEABLE: {
            perfect_divide = (long)((parent_rate*1000) / rate);
            abs_min = abs(perfect_divide - (long)(((long)div_max*1000)/(long)mul_min));

            for(i = mul_min + 1; i <= mul_max; i++) {
                abs_current = abs(perfect_divide - (long)((long)((long)div_max*1000)/(long)i));
                if(abs_min > abs_current ) {
                    abs_min = abs_current;
                    *mul = i;
                }
            }

            *div = div_min;
            break;
        } 
        /* only div can be changeable, 1/1,1/2,1/3...*/
        case SYSCTL_CLK_DIV_CHANGEABLE: {
            perfect_divide = (long)((parent_rate*1000) / rate);
            abs_min = abs(perfect_divide - (long)(((long)div_min*1000)/(long)mul_max));
            *div = div_min;

            for(i = div_min + 1; i <= div_max; i++) {
                abs_current = abs(perfect_divide - (long)((long)((long)i*1000)/(long)mul_max));
                if(abs_min > abs_current ) {
                    abs_min = abs_current;
                    *div = i;
                }
            }
            *mul = mul_min;
            break;
        }

        case SYSCTL_CLK_MUL_DIV_CHANGEABLE: {
            if((SYSCTL_CLK_CODEC_ADC_MCLK == leaf) || (SYSCTL_CLK_CODEC_DAC_MCLK == leaf))
            {
                for(int i=0;i<9;i++)
                {
                    if(0 == (rate - codec_clk[i]))
                    {
                        *div = codec_div[i][0];
                        *mul = codec_div[i][1];
                    }
                }
            }
            else if((SYSCTL_CLK_AUDIO_DEV_CLK == leaf) || (SYSCTL_CLK_PDM_CLK == leaf))
            {
                for(int i=0;i<20;i++)
                {
                    if(0 == (rate - pdm_clk[i]))
                    {
                        *div = pdm_div[i][0];
                        *mul = pdm_div[i][1];
                    }
                }
            }
            else
            {
                return -1;
            }
            break;
        }

        default:{
            return -3;
        }
    }
    return 0;
}

bool sysctl_media_clk_set_leaf_parent_div(sysctl_media_clk_node_e leaf, sysctl_media_clk_node_e parent, uint32_t numerator, uint32_t denominator)
{
    uint32_t reg, value;

    if (denominator == 0)
        return false;

    switch (leaf) {
    /*--------------------------- CPU1 CLOCK ------------------------------------*/
    case SYSCTL_CLK_CPU_1_SRC:
        if ((numerator != 1) || (denominator < 1) || (denominator > 8))
            return false;

        if (SYSCTL_CLK_ROOT_PLL0 == parent)
            value = 0;
        else if (SYSCTL_CLK_ROOT_PLL1_DIV_2 == parent)
            value = 1;
        else if (SYSCTL_CLK_ROOT_PLL3 == parent)
            value = 2;
        else
            return false;

        reg = sysctl_media_clk->cpu1_clk_cfg;
        reg |= ((1 << 31) | (7 << 3));
        sysctl_media_clk->cpu1_clk_cfg = reg;

        reg &= ~(3 << 1);
        reg |= (value << 1);
        sysctl_media_clk->cpu1_clk_cfg = reg;

        reg &= ~(7 << 3);
        reg |= ((1 << 31) | ((denominator - 1) << 3));
        sysctl_media_clk->cpu1_clk_cfg = reg;
        break;
    /*--------------------------- AI CLOCK ------------------------------------*/
    case SYSCTL_CLK_AI_SRC:
        if ((numerator != 1) || (denominator < 1) || (denominator > 8))
            return false;

        if (SYSCTL_CLK_ROOT_PLL0_DIV_2 == parent)
            value = 0;
        else if (SYSCTL_CLK_ROOT_PLL3_DIV_2 == parent)
            value = 1;
        else
            return false;

        reg = sysctl_media_clk->ai_clk_cfg;
        reg |= ((1 << 31) | (7 << 3));
        sysctl_media_clk->ai_clk_cfg = reg;

        reg &= ~(1 << 2);
        reg |= (value << 2);
        sysctl_media_clk->ai_clk_cfg = reg;

        reg &= ~(7 << 3);
        reg |= ((1 << 31) | ((denominator - 1) << 3));
        sysctl_media_clk->ai_clk_cfg = reg;
        break;
    default:
        return false;
    }
    return true;
}

void sysctl_media_clk_set_leaf_en_multi(sysctl_media_clk_node_e leaf, bool enable)
{
    static sysctl_media_clk_node_e leaf_list[] = {
        SYSCTL_CLK_AI_SRC, SYSCTL_CLK_DPU_CLK, SYSCTL_CLK_VPU_SRC};
    static uint32_t ref_count[sizeof(leaf_list) / sizeof(leaf_list[0])];
    rt_base_t level;
    int idx = -1;

    for (int i = 0; i < sizeof(leaf_list) / sizeof(leaf_list[0]); i++) {
        if (leaf_list[i] == leaf) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return;

    level = rt_hw_interrupt_disable();
    if (enable == true) {
        if (ref_count[idx] == 0)
            sysctl_media_clk_set_leaf_en(leaf, enable);
        ref_count[idx]++;
        if (ref_count[idx] == UINT32_MAX)
            rt_kprintf("error: clk %d enable too many times\n", idx);
    } else if (ref_count[idx]) {
        ref_count[idx]--;
        if (ref_count[idx] == 0)
            sysctl_media_clk_set_leaf_en(leaf, enable);
    }
    rt_hw_interrupt_enable(level);
}

int rt_hw_sysctl_media_clk_init(void)
{
    sysctl_media_clk = rt_ioremap((void*)CMU_BASE_ADDR, CMU_IO_SIZE);
    if(!sysctl_media_clk) {
        rt_kprintf("sysctl_media_clk ioremap error\n");
        return -1;
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_sysctl_media_clk_init);
