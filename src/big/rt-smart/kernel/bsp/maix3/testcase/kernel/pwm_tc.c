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

#include <unistd.h>
#include <stdio.h>
#include <drivers/rt_drv_pwm.h>
#include "drv_pwm.h"
#include <rtdbg.h>
#include "utest.h"

#define PWM_DEV_NAME       "pwm"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL_2     2       /* PWM通道 */
#define PWM_DEV_CHANNEL_3     3
#define PWM_DEV_CHANNEL_4     4
struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */
rt_uint32_t period, pulse;

/**
 * @brief 
 * io52  pwm4
 * io7   pwm2 测量点R77
 * io8   pwm3 测量点R79
 * 
 * pwm0-
 *      |_channel0
 *      |_channel1
 *      |_channel2
 * 
 * pwm1-
 *      |_channel3
 *      |_channel4
 *      |_channel5
 */


void pwm_demo(void)
{
    rt_kprintf("pwm test start.\n");
    /* 查找设备 */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    /* 设置PWM周期和脉冲宽度 */
    period = 100000;         /* 周期, 单位为纳秒ns */
    pulse = 50000;          /* PWM脉冲宽度值, 单位为纳秒ns */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_2, period, pulse);
    period = 100000;
    pulse = 25000;
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_3, period, pulse);
    period = 100000;
    pulse = 75000;
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_4, period, pulse);
    /* 使能设备，同一个pwm的不同channel仅需任意channel使能一次 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL_2);
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL_4);
    rt_kprintf("pwm-%d start OK.\n", PWM_DEV_CHANNEL_2);
    rt_kprintf("pwm-%d start OK.\n", PWM_DEV_CHANNEL_3);
    rt_kprintf("pwm-%d start OK.\n", PWM_DEV_CHANNEL_4);
    sleep(10);
    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL_2);
    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL_4);
}


static void pwm_testcase(void)
{
    UTEST_UNIT_RUN(pwm_demo);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(pwm_testcase, "testcases.kernel.pwm_testcase", utest_tc_init, utest_tc_cleanup, 10);
