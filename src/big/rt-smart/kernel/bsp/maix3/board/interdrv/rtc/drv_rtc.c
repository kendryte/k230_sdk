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
#include <rthw.h>
#include "riscv_io.h"
#include <time.h>
#include "board.h"
#include "ioremap.h"
#include "drv_rtc.h"

struct rt_rtc_device
{
    struct rt_device device;
};

static struct rt_rtc_device rtc_device;
static volatile void *addr_base; 
typedef void (*rtc_callback_t)(void);
static rtc_callback_t callback;

static void pmu_isolation_rtc(void)
{
    volatile void *reg_pmu_pwr = rt_ioremap((void *)PWR_BASE_ADDR, PWR_IO_SIZE);
    uint32_t *addr = (uint32_t*)(reg_pmu_pwr + 0x158);
    uint32_t data;

    data = *addr;
    data &= ~0x20;
    *addr = data;
    rt_iounmap(reg_pmu_pwr);

    volatile void *reg_pmu = rt_ioremap((void*)PMU_BASE_ADDR, PMU_IO_SIZE);
    addr = (uint32_t*)(reg_pmu + 0x48);
    data = *addr;
    data |= 0x06;
    *addr = data;

    addr = (uint32_t*)(reg_pmu + 0x4c);
    data = *addr;
    data |= 0x06;
    *addr = data;
    rt_iounmap(reg_pmu);
}

static int rtc_year_is_leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static void register_set_success(void)
{
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);
    interrupt_ctrl->timer_w_en = 1;
    interrupt_ctrl->timer_w_en = 0;
    interrupt_ctrl->timer_r_en = 1;
}


static int rtc_timer_set_clock_count_value(unsigned int count)
{
    volatile rtc_t *const rtc = (volatile rtc_t *)addr_base;
    rtc_count_t current_count;
    current_count.curr_count = count;
    current_count.sum_count = 32767;
    rtc->count = current_count;
    rt_thread_mdelay(1);
    return 0;
}

static void rtc_interrupt_ctrl_set(rtc_interrupt_mode_t mode)
{
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);

    if (mode < RTC_INT_TICK_YEAR)
    {
        interrupt_ctrl->year_cmp = 0;
        interrupt_ctrl->month_cmp = 0;
        interrupt_ctrl->day_cmp = 0;
        interrupt_ctrl->week_cmp = 0;
        interrupt_ctrl->hour_cmp = 0;
        interrupt_ctrl->minute_cmp = 0;
        interrupt_ctrl->second_cmp = 0;

        if (mode & RTC_INT_ALARM_YEAR)
        {
            interrupt_ctrl->year_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_MONTH)
        {
            interrupt_ctrl->month_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_DAY)
        {
            interrupt_ctrl->day_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_WEEK)
        {
            interrupt_ctrl->week_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_HOUR)
        {
            interrupt_ctrl->hour_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_MINUTE)
        {
            interrupt_ctrl->minute_cmp = 1;
        }
        if (mode & RTC_INT_ALARM_SECOND)
        {
            interrupt_ctrl->second_cmp = 1;
        }

        interrupt_ctrl->alarm_en = 1;
        return;
    } else {
        switch(mode)
        {
            case RTC_INT_TICK_YEAR:
                interrupt_ctrl->tick_sel = 0x8;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_MONTH:
                interrupt_ctrl->tick_sel = 0x7;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_DAY:
                interrupt_ctrl->tick_sel = 0x6;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_WEEK:
                interrupt_ctrl->tick_sel = 0x5;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_HOUR:
                interrupt_ctrl->tick_sel = 0x4;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_MINUTE:
                interrupt_ctrl->tick_sel = 0x3;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_SECOND:
                interrupt_ctrl->tick_sel = 0x2;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_S8:
                interrupt_ctrl->tick_sel = 0x1;
                interrupt_ctrl->tick_en = 1;
                break;
            case RTC_INT_TICK_S64:
                interrupt_ctrl->tick_sel = 0x0;
                interrupt_ctrl->tick_en = 1;
                break;
            default :
                break;
        }
        return;
    }
}

static void alarm_stop(void)
{
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);
    interrupt_ctrl->alarm_en = 0;
}

static void tick_stop(void)
{
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);
    interrupt_ctrl->tick_en = 0;
}

static int rtc_set_interrupt(int enable, rtc_interrupt_mode_t mode)
{
    if(enable)
        rtc_interrupt_ctrl_set(mode);
    return 0;
}

static void rtc_stop_interrupt(void)
{
    rt_hw_interrupt_mask(IRQN_PMU_INTERRUPT);
}

static int rtc_alarm_clear_interrupt(void)
{
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t *)((void*)addr_base + 0x14);
    interrupt_ctrl->alarm_clr = 0x1;
    return 0;
}

static void rtc_irq(int vector, void *param)
{
    rtc_alarm_clear_interrupt();
    if (callback != RT_NULL)
        callback();
}

static int rtc_date_time_set(int year, int month, int day, int hour, int minute, int second, int week)
{
    volatile rtc_t *const rtc = (volatile rtc_t *)addr_base;
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);

    rtc_date_t date;
    rtc_time_t time;

    int val = year % 100;
    int year_l, year_h;
    if(val == 0)
    {
        year_l = 100;
        year_h = year / 100 - 1;
    } else {
        year_l = val;
        year_h = (year - val) / 100;
    }

    interrupt_ctrl->timer_w_en = 1;
    
    date.year_h = year_h;
    date.year_l = year_l;
    date.month = month;
    date.day = day;
    date.leap_year = rtc_year_is_leap(year);
    time.week = week;
    time.hour = hour;
    time.minute = minute;
    time.second = second;

    rtc->date = date;
    rtc->time = time;

    rtc_count_t current_count;
    current_count.curr_count = 0;
    current_count.sum_count = 32767;
    rtc->count = current_count;
    rt_thread_mdelay(10);

    return 0;
}


static void rtc_timer_get(time_t *t)
{
    volatile rtc_t *const rtc = (volatile rtc_t *)addr_base;
    rtc_interrupt_ctrl_t *interrupt_ctrl = (rtc_interrupt_ctrl_t*)((void*)addr_base + 0x14);

    if (interrupt_ctrl->timer_r_en == 0)
        interrupt_ctrl->timer_r_en = 1;

    struct tm *tm;
    rtc_date_t date = rtc->date;
    rtc_time_t time = rtc->time;

    tm->tm_sec = time.second;
    tm->tm_min = time.minute;
    tm->tm_hour = time.hour;
    tm->tm_mday = date.day;
    tm->tm_mon = date.month - 1;
    tm->tm_year = (date.year_h * 100 + date.year_l) - 1900;
    tm->tm_wday = time.week;

    *t = mktime(tm);
}

static void rtc_timer_set(time_t *t)
{
    struct tm *p_tm;
    gmtime_r(t, p_tm);

    rtc_date_time_set((p_tm->tm_year + 1900), p_tm->tm_mon + 1, p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, p_tm->tm_wday);

    rtc_timer_set_clock_count_value(0);
    register_set_success();
}

static rt_err_t kd_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t kd_rtc_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t kd_rtc_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    time_t t;
    rtc_timer_get(&t);
    rt_memcpy(buffer, (void*)&t, sizeof(t));
    return size;
}

static rt_size_t kd_rtc_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct tm *tm = (struct tm*)buffer;
    time_t t = mktime(tm);
    rtc_timer_set(&t);
    return size;
}

static rt_err_t rtc_alarm_get(void *args)
{
    struct tm *tm = (struct tm*)args;
    
    volatile rtc_t *const rtc = (volatile rtc_t *)addr_base;
    rtc_alarm_date_t alarm_date = rtc->alarm_date;
    rtc_alarm_time_t alarm_time = rtc->alarm_time;
    tm->tm_year = alarm_date.alarm_year_h * 100 + alarm_date.alarm_year_l;
    tm->tm_mon = alarm_date.alarm_month;
    tm->tm_mday = alarm_date.alarm_day;
    tm->tm_hour = alarm_time.alarm_hour;
    tm->tm_min = alarm_time.alarm_minute;
    tm->tm_sec = alarm_time.alarm_second;
    return RT_EOK;
}

static rt_err_t rtc_alarm_set(void *args)
{
    struct kd_alarm_setup *setup = (struct kd_alarm_setup*)args;
    struct tm tm = setup->tm;
    volatile rtc_t *const rtc = (volatile rtc_t *)addr_base;
    rtc_alarm_time_t alarm_time;
    rtc_alarm_date_t alarm_date;
    rtc_date_t date = rtc->date;

    int year = tm.tm_year + 1900;
    int val = year % 100;
    int year_l,year_h;
    if(val == 0)
    {
        year_l = 100;
        year_h = year / 100 - 1;
    } else {
        year_l = val;
        year_h = (year - val) / 100;
    }

    alarm_date.alarm_year_h = year_h;
    alarm_date.alarm_year_l = year_l;
    alarm_date.alarm_month = tm.tm_mon;
    alarm_date.alarm_day = tm.tm_mday;
    alarm_time.alarm_hour = tm.tm_hour;
    alarm_time.alarm_minute = tm.tm_min;
    alarm_time.alarm_second = tm.tm_sec;

    rtc->alarm_date = alarm_date;
    rtc->alarm_time = alarm_time;

    rtc_alarm_clear_interrupt();
    rtc_set_interrupt(0, setup->flag);
    rt_hw_interrupt_install(IRQN_PMU_INTERRUPT, rtc_irq, NULL, "rtc");
    rt_hw_interrupt_umask(IRQN_PMU_INTERRUPT);
    rtc_set_interrupt(1, setup->flag);
}

static rt_err_t kd_rtc_control(rt_device_t dev, int cmd, void *args)
{
    time_t time;
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        rtc_timer_get((time_t*)args);
        break;
    case RT_DEVICE_CTRL_RTC_SET_TIME:
        rtc_timer_set((time_t *)args);
        break;
    case RT_DEVICE_CTRL_RTC_GET_ALARM:
        rtc_alarm_get(args);
        break;
    case RT_DEVICE_CTRL_RTC_SET_ALARM:
        rtc_alarm_set(args);
        break;
    case RT_DEVICE_CTRL_RTC_STOP_ALARM:
        alarm_stop();
        break;
    case RT_DEVICE_CTRL_RTC_STOP_TICK:
        tick_stop();
        break;
    case RT_DEVICE_CTRL_RTC_SET_CALLBACK:
        callback = args;
        break;
    default:
        return RT_EINVAL;
    }
    return RT_EOK;
}


const static struct rt_device_ops kd_rtc_ops =
{
    .init = RT_NULL,
    .open = kd_rtc_open,
    .close = kd_rtc_close,
    .read = kd_rtc_read,
    .write = kd_rtc_write,
    .control = kd_rtc_control
};

static int rt_hw_rtc_init(void)
{
    struct tm *tm;
    time_t t;
    pmu_isolation_rtc();
    rt_memset(&rtc_device, 0, sizeof(rtc_device));
    addr_base = (void*)rt_ioremap((void *)RTC_BASE_ADDR, RTC_IO_SIZE);

    rtc_device.device.type        = RT_Device_Class_RTC;
    rtc_device.device.rx_indicate = RT_NULL;
    rtc_device.device.tx_complete = RT_NULL;
    rtc_device.device.ops         = &kd_rtc_ops;
    rtc_device.device.user_data   = &addr_base;
    rt_device_register(&rtc_device.device, "rtc", RT_DEVICE_FLAG_RDWR);
#ifndef RT_FASTBOOT
    rt_kprintf("rtc driver register OK\n");
#endif
    tm->tm_year = 1970 - 1900;
    tm->tm_mon = 1 - 1;
    tm->tm_mday = 1;
    tm->tm_wday = 5;
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    t = mktime(tm);
    rtc_timer_set(&t);
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_rtc_init);
