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
#include <time.h>
#include <drv_rtc.h>

#include "utest.h"

#define RTC_NAME       "rtc"

static int rtc_sample(void)
{
    rt_err_t ret = RT_EOK;
    time_t now;
    uint32_t i;
    rt_device_t device = RT_NULL;
    rt_kprintf(" ============ rtc set time test ============ \n");
    device = rt_device_find(RTC_NAME);
    if (!device)
    {
      LOG_E("find %s failed!", RTC_NAME);
      return RT_ERROR;
    }

    if(rt_device_open(device, 0) != RT_EOK)
    {
      LOG_E("open %s failed!", RTC_NAME);
      return RT_ERROR;
    }

    ret = set_date(2024, 1, 1);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC date failed\n");
        return ret;
    }

    ret = set_time(1, 1, 0);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC time failed\n");
        return ret;
    }

    for (i=0; i<5; i++)
    {
        now = time(RT_NULL);
        rt_kprintf("%s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }
    rt_device_close(device);
    return ret;
}
MSH_CMD_EXPORT(rtc_sample, rtc sample);


void user_alarm_callback(void)
{
    rt_kprintf("[ user alarm callback function. ]\n");
}

static int alarm_sample(void)
{
    rt_err_t ret = RT_EOK;
    time_t now;
    uint32_t i;
    struct tm p_tm;
    rt_device_t dev = RT_NULL;
    struct rt_alarm * alarm = RT_NULL;
    struct kd_alarm_setup setup;
    rt_kprintf(" ============ rtc alarm test ============ \n");
    dev = rt_device_find(RTC_NAME);
    rt_device_open(dev, 0);
    set_date(2024, 1, 1);
    set_time(0, 0, 50);

    now = time(RT_NULL);
    gmtime_r(&now,&p_tm);
    setup.flag = RTC_INT_TICK_SECOND;
    // setup.flag = RTC_INT_ALARM_MINUTE | RTC_INT_ALARM_SECOND;
    setup.tm.tm_year = p_tm.tm_year;
    setup.tm.tm_mon = p_tm.tm_mon;
    setup.tm.tm_mday = p_tm.tm_mday;
    setup.tm.tm_wday = p_tm.tm_wday;
    setup.tm.tm_hour = p_tm.tm_hour;
    setup.tm.tm_min = p_tm.tm_min + 1;
    setup.tm.tm_sec = 0;
    rt_kprintf("get now time:  %d-%d-%d  %d:%d:%d\n", p_tm.tm_year+1900, p_tm.tm_mon+1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec);

    rt_device_control(dev, RT_DEVICE_CTRL_RTC_SET_CALLBACK, &user_alarm_callback); //set rtc intr callback

    rt_device_control(dev, RT_DEVICE_CTRL_RTC_SET_ALARM, &setup);   //set alarm time
    rt_memset(&p_tm, 0, sizeof(p_tm));
    rt_device_control(dev, RT_DEVICE_CTRL_RTC_GET_ALARM, &p_tm);   //get alarm time
    rt_kprintf("get alarm time:  %d-%d-%d  %d:%d:%d\n", p_tm.tm_year, p_tm.tm_mon+1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec);

    for (i=0; i<10; i++)
    {
        now = time(RT_NULL);
        rt_kprintf("%s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }

    rt_device_control(dev, RT_DEVICE_CTRL_RTC_STOP_TICK, RT_NULL);
    // rt_device_control(dev, RT_DEVICE_CTRL_RTC_STOP_ALARM, RT_NULL);

    while(1){
        now = time(RT_NULL);
        rt_kprintf("%s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }
    // rt_device_close(dev);
    // return ret;
}
MSH_CMD_EXPORT(alarm_sample,alarm sample);


static int rtc_interface_sample(void)
{
    rt_err_t ret = RT_EOK;
    uint32_t i;
    rt_device_t device = RT_NULL;
    time_t now;
    struct tm tm;
    rt_kprintf(" ============ rtc interface test ============ \n");
    device = rt_device_find(RTC_NAME);
    rt_kprintf(" ============ open rtc ============ \n");
    rt_device_open(device, 0);

    set_date(2024, 1, 1);

    set_time(1, 1, 0);

    for (i=0; i<5; i++)
    {
        now = time(RT_NULL);
        rt_kprintf("[sys]: %s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }

    rt_kprintf(" ============ write rtc ============ \n");
    tm.tm_year = 2023 - 1900;
    tm.tm_mon = 6 - 1;
    tm.tm_mday = 28;
    tm.tm_wday = 2;
    tm.tm_hour = 14;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    rt_device_write(device, 0, (void*)&tm, sizeof(tm));
    
    for (i=0; i<5; i++)
    {
        now = time(RT_NULL);
        rt_kprintf("[sys]:%s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }

    rt_kprintf(" ============ read rtc ============ \n");
    for (i=0; i<5; i++)
    {
        rt_device_read(device, 0, (void*)&now, sizeof(now));
        rt_kprintf("[read]: %s\n", ctime(&now));
        rt_thread_mdelay(1000);
    }
    rt_device_close(device);
    return ret;
}
MSH_CMD_EXPORT(rtc_interface_sample,alarm sample);



static void utest_rtc(void)
{
    rtc_sample();
    rtc_interface_sample();
    alarm_sample();
}

static void hw_rtc_testcase(void)
{
    UTEST_UNIT_RUN(utest_rtc);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(hw_rtc_testcase, "testcases.kernel.hw_rtc_testcase", utest_tc_init, utest_tc_cleanup, 10);
