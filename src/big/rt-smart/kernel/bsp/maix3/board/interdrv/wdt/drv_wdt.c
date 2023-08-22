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
#include <rtdevice.h>
#include <riscv_io.h>
#include "board.h"
#include "ioremap.h"
#include "sysctl_clk.h"
#include "drv_wdt.h"
#include <rtdbg.h>
static rt_watchdog_t kd_watchdog;
static kd_wdt_t *kd_wdt_reg;

/* There are sixteen TOPs (timeout periods) that can be set in the watchdog. */

static const rt_uint32_t *tops;

static const rt_uint32_t kd_wdt_fix_tops[KD_WDT_NUM_TOPS] = {
	KD_WDT_FIX_TOP(0), KD_WDT_FIX_TOP(1), KD_WDT_FIX_TOP(2),
	KD_WDT_FIX_TOP(3), KD_WDT_FIX_TOP(4), KD_WDT_FIX_TOP(5),
	KD_WDT_FIX_TOP(6), KD_WDT_FIX_TOP(7), KD_WDT_FIX_TOP(8),
	KD_WDT_FIX_TOP(9), KD_WDT_FIX_TOP(10), KD_WDT_FIX_TOP(11),
	KD_WDT_FIX_TOP(12), KD_WDT_FIX_TOP(13), KD_WDT_FIX_TOP(14),
	KD_WDT_FIX_TOP(15)
};

static struct kd_wdt_timeout timeouts[KD_WDT_NUM_TOPS];
static char rmod;   //wdt reset mode,

static void kd_wdt_timeouts_init(void)
{
    rt_uint32_t wdt_clk;
    rt_uint32_t time_value;
    rt_uint32_t i, t;
    rt_uint64_t msec;
	struct kd_wdt_timeout tout, *dst;
    /* caculate timeout value */
    wdt_clk = sysctl_clk_get_leaf_freq(SYSCTL_CLK_WDT_1_CLK);

    for (i = 0; i < KD_WDT_NUM_TOPS; ++i)
    {
        tout.top_val = i;
        tout.sec = tops[i] / wdt_clk;
        msec = (rt_uint64_t)tops[i] * (rt_uint64_t)1000L;
        msec = msec / wdt_clk;
        tout.msec = msec - (rt_uint64_t)tout.sec * (rt_uint64_t)1000L;

        for (t = 0; t < i; ++t)
        {
            dst = &timeouts[t];
			if (tout.sec > dst->sec || (tout.sec == dst->sec &&
			    tout.msec >= dst->msec))
				continue;
			else
				swap(*dst, tout);
        }

        timeouts[i] = tout;
    }
    rt_kprintf("watchdog timeout table init OK!\n");
}

static rt_err_t kd_wdt_feed(rt_watchdog_t *wdt)
{
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    reg->crr = 0x76;

    return RT_EOK;
}

static rt_err_t kd_wdt_enable(rt_watchdog_t *wdt)
{
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    reg->crr = 0x76;
    reg->cr |= 0x1;

    return RT_EOK;
}

static rt_err_t kd_wdt_disable(rt_watchdog_t *wdt)
{
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    reg->crr = 0x76;
    reg->cr &= ~0x1;

    return RT_EOK;
}

static rt_err_t kd_wdt_set_timeout(rt_watchdog_t *wdt, rt_uint64_t timeout)
{
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    rt_uint32_t top_val;
    rt_uint32_t i;
    rt_uint32_t time = (timeout + rmod -1) / rmod;

    for (i = 0; i < KD_WDT_NUM_TOPS; ++i)
    {
        if (timeouts[i].sec >= time)
            break;
    }

	if (i == KD_WDT_NUM_TOPS)
		--i;

	top_val = timeouts[i].top_val;

    reg->torr = (top_val << 4) | (top_val << 0);

    return RT_EOK;
}

static rt_err_t kd_wdt_get_timeout(rt_watchdog_t *wdt, void *timeout)
{
    rt_uint64_t top_val;
    rt_uint32_t i;
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    top_val = ((reg->torr) & 0xf);

    for (i = 0; i < KD_WDT_NUM_TOPS; ++i)
    {
		if (timeouts[i].top_val == top_val)
			break;
    }

    *((rt_uint64_t *)timeout) = timeouts[i].sec * rmod;

    return RT_EOK;
}

static rt_err_t kd_wdt_init(rt_watchdog_t *wdt)
{
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;
    reg->cr &= ~(0x01 << 1);
    reg->cr |= (0x0 << 1);      //default set wdt reset mode
    rmod = KD_WDT_RMOD_RESET;

    if (reg->comp_param_1 & (1 << 6))
    {
        tops = kd_wdt_fix_tops;
    }

    kd_wdt_timeouts_init();

    if (!timeouts[KD_WDT_NUM_TOPS - 1].sec)
    {
        rt_kprintf("No any valid Timeout period detected\n");
        return -RT_EINVAL;
    }

    return RT_EOK;
}

static rt_err_t kd_wdt_control(rt_watchdog_t *wdt, int cmd, void *args)
{
    RT_ASSERT(wdt != NULL);
    kd_wdt_t *reg = (kd_wdt_t *)wdt->parent.user_data;

    switch(cmd)
    {
        case KD_DEVICE_CTRL_WDT_GET_TIMEOUT:
            kd_wdt_get_timeout(wdt, args);
        break;
        case KD_DEVICE_CTRL_WDT_SET_TIMEOUT:
            kd_wdt_set_timeout(wdt, *((rt_uint32_t*)args));
        break;
        case KD_DEVICE_CTRL_WDT_KEEPALIVE:
            kd_wdt_feed(wdt);
        break;
        case KD_DEVICE_CTRL_WDT_START:
            kd_wdt_enable(wdt);
        break;
        case RT_DEVICE_CTRL_WDT_STOP:
        case KD_DEVICE_CTRL_WDT_STOP:
            kd_wdt_disable(wdt);
        break;
        default:
            return -RT_EINVAL;
    }

    return RT_EOK;
}

static struct rt_watchdog_ops kd_wdt_ops =
{
    .init = kd_wdt_init,
    .control = kd_wdt_control,
};

int rt_hw_wdt_init(void)
{
    rt_err_t ret = RT_EOK;
    kd_wdt_reg = (kd_wdt_t *)rt_ioremap((void *)WDT1_BASE_ADDR, WDT1_IO_SIZE);

    kd_watchdog.ops = &kd_wdt_ops;

    ret = rt_hw_watchdog_register(&kd_watchdog, "watchdog1", RT_DEVICE_FLAG_RDWR, kd_wdt_reg);

    if (ret != RT_EOK)
    {
        LOG_E("rt device register failed %d\n", ret);
    }
#ifndef RT_FASTBOOT
    rt_kprintf("watchdog register OK!\n");
#endif
    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_wdt_init);