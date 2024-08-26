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
#include "riscv_io.h"
#include "board.h"
#include "ioremap.h"
#include <rtdbg.h>
#include <stdbool.h>
#include "sysctl_clk.h"
#include "drv_pwm.h"
static struct rt_device_pwm kd_pwm;
kd_pwm_t *reg_pwm;

static int check_channel(int channel)
{
    if (channel < 0 || channel > 5)
    {
        LOG_E("channel %d is not valid\n", channel);
        return -RT_ERROR;
    }
    return channel;
}

static int pwm_start(kd_pwm_t *reg, int channel)
{
    int ret;
    ret = check_channel(channel);
    if (ret < 0)
        return ret;

    if (channel > 2)
    {
        reg = (kd_pwm_t *)((void*)reg + 0x40);
    }
    reg->pwmcfg |= (1 << 12);  //default always mode
    return ret;
}

static int pwm_stop(kd_pwm_t *reg, int channel)
{
    int ret;
    ret = check_channel(channel);
    if (ret < 0)
        return ret;

    if (channel > 2)
    {
        reg = (kd_pwm_t *)((void*)reg + 0x40);
    }
    reg->pwmcfg &= ~(1 << 12);

    return ret;
}

static rt_err_t kd_pwm_get(kd_pwm_t *reg, rt_uint8_t channel, struct rt_pwm_configuration *configuration)
{
    int ret;
    uint64_t pulse, period;
    uint32_t pwm_pclock, pwmscale;

    ret = check_channel(channel);
    if (ret < 0)
        return ret;

    pwm_pclock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_PWM_PCLK_GATE);

    if (channel > 2)
        reg = (kd_pwm_t *)((void*)reg + 0x40);

    pwmscale = reg->pwmcfg & 0xf;
    pwm_pclock >>= pwmscale;
    period = reg->pwmcmp0;
    period = period * NSEC_PER_SEC / pwm_pclock;
    pulse = *((&reg->pwmcmp1) + (channel % 3));
    pulse = pulse * NSEC_PER_SEC / pwm_pclock;

    configuration->period = period;
    configuration->pulse = pulse;

    return RT_EOK;
}

static int kd_pwm_set(kd_pwm_t *reg, int channel, struct rt_pwm_configuration *configuration)
{
    int ret;
    uint64_t pulse, period, pwmcmpx_max;
    uint32_t pwm_pclock, pwmscale = 0;

    ret = check_channel(channel);
    if (ret < 0)
        return ret;

    pwm_pclock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_PWM_PCLK_GATE);
    pulse = (uint64_t)configuration->pulse * pwm_pclock / NSEC_PER_SEC;
    period = (uint64_t)configuration->period * pwm_pclock / NSEC_PER_SEC;
    if (pulse > period)
        return -RT_EINVAL; 

    if (channel > 2)
        reg = (kd_pwm_t *)((void*)reg + 0x40);

    /* 计算占空比 */
    pwmcmpx_max = (1 << 16) - 1;
    if (period > ((1 << (15 + 16)) - 1LL))
        return -RT_EINVAL; 

    while ((period >> pwmscale) > pwmcmpx_max)
        pwmscale++;
    if (pwmscale > 0xf)
        return -RT_EINVAL;

    reg->pwmcfg |= (1 << 9);  //default always mode
    reg->pwmcfg &= (~0xf);
    reg->pwmcfg |= pwmscale;  //scale
    reg->pwmcmp0 = (period >> pwmscale);
    *((&reg->pwmcmp1) + (channel % 3)) = reg->pwmcmp0 - (pulse >> pwmscale);

    return RT_EOK;
}

static rt_err_t kd_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    struct rt_pwm_configuration *configuration = (struct rt_pwm_configuration *)arg;
    rt_uint32_t channel = 0;
    int ret;

    kd_pwm_t *reg = (kd_pwm_t *)(device->parent.user_data);
    channel = configuration->channel;

    switch (cmd)
    {
    case PWM_CMD_ENABLE:
    case KD_PWM_CMD_ENABLE:
        ret = pwm_start(reg, channel);
        if (ret < 0)
            return -RT_ERROR;
        else
            return RT_EOK;
    case PWM_CMD_DISABLE:
    case KD_PWM_CMD_DISABLE:
        ret = pwm_stop(reg, channel);
        if (ret < 0)
            return -RT_ERROR;
        else
            return RT_EOK;
    case PWM_CMD_SET:
    case KD_PWM_CMD_SET:
        kd_pwm_set(reg, channel, configuration);
        break;
    case PWM_CMD_GET:
    case KD_PWM_CMD_GET:
        kd_pwm_get(reg, channel, configuration);
        break;
    default:
        return -RT_EINVAL;
    }

    return RT_EOK;
}

static struct rt_pwm_ops drv_ops =
{
    kd_pwm_control
};

int rt_hw_pwm_init(void)
{
    reg_pwm = (kd_pwm_t *)rt_ioremap((void *)PWM_BASE_ADDR, PWM_IO_SIZE);
    kd_pwm.ops = &drv_ops;
    rt_device_pwm_register(&kd_pwm, "pwm", &drv_ops, (void *)reg_pwm);
#ifndef RT_FASTBOOT
    rt_kprintf("pwm driver register OK\n");
#endif
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);
