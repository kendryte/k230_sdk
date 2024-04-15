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
#include <drivers/mmcsd_core.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_adc.h"
#include "riscv_io.h"
#include <string.h>
#include <ioremap.h>
#include "sysctl_rst.h"
#include "sysctl_clk.h"
#include "tick.h"

#ifdef RT_USING_ADC

static struct rt_adc_device k230_adc;
k_adc_dev_t adc_dev;

static uint64_t adc_perf_get_times(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(cnt));
    return cnt;
}
static void adc_delay_us(uint64_t us)
{
    uint64_t delay = (TIMER_CLK_FREQ / 1000000) * us;
    volatile uint64_t cur_time = adc_perf_get_times();
    while(1)
    {
        if((adc_perf_get_times() - cur_time ) >= delay)
            break;
    }
}

static int k_adc_drv_hw_init(k_adc_regs_t *adc_regs)
{
    rt_uint32_t reg;

    reg = readl(&adc_regs->trim_reg);
    reg &= (~(0x1));
    writel(reg, &adc_regs->trim_reg);

    reg = readl(&adc_regs->trim_reg);
    reg |= 0x1;
    writel(reg, &adc_regs->trim_reg);

    reg = readl(&adc_regs->trim_reg);
    reg |= (0x1 << 20);
    writel(reg, &adc_regs->trim_reg);

    adc_delay_us(150);

    reg &= ~(0x1 << 20);
    writel(reg, &adc_regs->trim_reg);

    writel(0x0, &adc_regs->mode_reg);

    return RT_EOK;
}

static int k_adc_drv_init()
{
    int i;

    adc_dev.dev_num = 0;
    adc_dev.adc_regs = (k_adc_regs_t*)rt_ioremap((void *)ADC_BASE_ADDR, ADC_IO_SIZE);
    adc_dev.use_num = 0;

    for (i = 0; i < ADC_MAX_DMA_CHN; i++)
    {
        adc_dev.chn[i].dev_num = adc_dev.dev_num;
        adc_dev.chn[i].chn_num = i;
        adc_dev.chn[i].enabled = 0;
    }

    k_adc_drv_hw_init(adc_dev.adc_regs);

    return RT_EOK;
}

static int k_adc_drv_enabled(k_adc_regs_t *adc_regs)
{
    rt_uint32_t reg;

    reg = readl(&adc_regs->trim_reg);
    reg |= 0x1;
    writel(reg, &adc_regs->trim_reg);

    return RT_EOK;
}

static int k_adc_drv_disabled(k_adc_regs_t *adc_regs)
{
    rt_uint32_t reg;

    reg = readl(&adc_regs->trim_reg);
    reg = reg & (~(0x1));
    writel(reg, &adc_regs->trim_reg);

    return RT_EOK;
}

rt_err_t k230_adc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    if (channel >= ADC_MAX_CHANNEL)
    {
        return RT_ERROR;
    }

    if (enabled)
    {
        if (adc_dev.chn[channel].enabled)
        {
            return RT_EOK;
        }
        else
        {
            adc_dev.chn[channel].enabled = 1;
            adc_dev.use_num++;
        }
        // if (adc_dev.use_num == 1)
        // {
        //     k_adc_drv_enabled(adc_dev.adc_regs);
        // }
    }
    else
    {
        if (!adc_dev.chn[channel].enabled)
        {
            return RT_EOK;
        }
        else
        {
            adc_dev.chn[channel].enabled = 0;
            adc_dev.use_num--;
        }
        // if (adc_dev.use_num == 0)
        // {
        //     k_adc_drv_disabled(adc_dev.adc_regs);
        // }
    }

    return RT_EOK;
}

rt_err_t k230_get_adc_value(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    if (channel >= ADC_MAX_CHANNEL)
        return RT_ERROR;
    
    if (!adc_dev.chn[channel].enabled){
        return RT_ERROR;
    }

    writel(channel | 0x10, &adc_dev.adc_regs->cfg_reg);
    while ((readl(&adc_dev.adc_regs->cfg_reg) & 0x10000) == 0);
    *value = readl(&adc_dev.adc_regs->data_reg[channel]);

    return RT_EOK;
}

static const struct rt_adc_ops k230_adc_ops =
{
    k230_adc_enabled,
    k230_get_adc_value,
};

int rt_hw_adc_init(void)
{
    k_adc_drv_init();

    rt_hw_adc_register(&k230_adc, K230_ADC_NAME, &k230_adc_ops, NULL);

    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_hw_adc_init);

#endif /* defined(RT_USING_ADC) */