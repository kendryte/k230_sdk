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
#include <rtdebug.h>
#include "drivers/hwtimer.h"
#include "board.h"
#include "utest.h"

static struct rt_device *tmr_dev_0;
static struct rt_device *tmr_dev_1;

static rt_err_t tmr_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("hwtimer timeout callback fucntion @tick\n");
    return RT_EOK;
}

static void test_hwtimer(void)
{
    rt_hwtimerval_t timerval;
    rt_hwtimer_mode_t mode;
    rt_size_t tsize;
    rt_uint32_t freq = 25000000;               /* 计数频率 可选 12.5M 25M 50M 100M*/

    tmr_dev_0 = rt_device_find("hwtimer0");
    tmr_dev_1 = rt_device_find("hwtimer1");
    
    if (tmr_dev_0 == RT_NULL || tmr_dev_1 == RT_NULL)
    {
        rt_kprintf("hwtimer device(s) not found !\n");
    }
    else if (rt_device_open(tmr_dev_0, RT_DEVICE_OFLAG_RDWR) != RT_EOK || rt_device_open(tmr_dev_1, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("hwtimer device(s) open failed !\n");
    }
    else if (rt_device_control(tmr_dev_0, HWTIMER_CTRL_FREQ_SET, &freq))  /* timer0设置计数频率，timer1使用默认频率 */
    {
        rt_kprintf("hwtimer device(s) set freq failed !\n");
    }
    else
    {
        rt_device_set_rx_indicate(tmr_dev_0, tmr_timeout_cb);
        rt_device_set_rx_indicate(tmr_dev_1, tmr_timeout_cb);

        timerval.sec = 10;
        timerval.usec = 0;
        tsize = sizeof(timerval);
        mode = HWTIMER_MODE_ONESHOT;
        if (rt_device_control(tmr_dev_0, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
        {
            rt_kprintf("timer0 set mode failed !\n");
        }
        else if (rt_device_write(tmr_dev_0, 0, &timerval, tsize) != tsize)
        {
            rt_kprintf("timer0 start failed !\n");
        }
        else
        {
            rt_kprintf("timer0 started !\n");
        }

        timerval.sec = 5;
        timerval.usec = 0;
        tsize = sizeof(timerval);
        mode = HWTIMER_MODE_ONESHOT;
        if (rt_device_control(tmr_dev_1, HWTIMER_CTRL_MODE_SET, &mode) != RT_EOK)
        {
            rt_kprintf("timer1 set mode failed !\n");
        }
        else if (rt_device_write(tmr_dev_1, 0, &timerval, tsize) != tsize)
        {
            rt_kprintf("timer1 start failed !\n");
        }
        else
        {
            rt_kprintf("timer1 started !\n");
        }
    }

    while(1)
    {
        rt_device_read(tmr_dev_0, 0, &timerval, sizeof(timerval));
        rt_kprintf("Read timer0: Sec = %d, Usec = %d\n", timerval.sec, timerval.usec);

        rt_device_read(tmr_dev_1, 0, &timerval, sizeof(timerval));
        rt_kprintf("Read timer1: Sec = %d, Usec = %d\n", timerval.sec, timerval.usec);
        
        rt_thread_mdelay(1000);
    }

}

static void hw_timer_testcase(void)
{
    UTEST_UNIT_RUN(test_hwtimer);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(hw_timer_testcase, "testcases.kernel.hw_timer_testcase", utest_tc_init, utest_tc_cleanup, 10);
