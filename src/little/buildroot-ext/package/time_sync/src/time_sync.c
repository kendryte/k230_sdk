/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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

/********************************************************************************
* @程序功能: 实现大核同步小核的时间，小核一般同步ntp网络时间。
* @程序实现: 通过SOC的RTC模块实现同步。小核获取时间后写RTC模块。大核读RTC模块。
********************************************************************************/

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

typedef struct _rtc_time
{
    uint32_t second : 6;
    uint32_t resv0 : 2;
    uint32_t minute : 6;
    uint32_t resv1 : 2;
    uint32_t hour : 5;
    uint32_t resv2 : 3;
    uint32_t week : 3;
    uint32_t resv3 : 5;
} __attribute__((packed, aligned(4))) rtc_time_t;

typedef struct _rtc_date
{
    uint32_t day : 5;
    uint32_t resv0 : 3;
    uint32_t month : 4;
    uint32_t resv1 : 4;
    uint32_t year_l : 7;
    uint32_t leap_year : 1;
    uint32_t year_h : 7;
    uint32_t resv2 : 1;
} __attribute__((packed, aligned(4))) rtc_date_t;

typedef struct _rtc_interrupt_ctrl
{
    uint32_t timer_w_en : 1;
    uint32_t timer_r_en : 1;
    uint32_t resv0 : 6;
    uint32_t tick_en : 1;
    uint32_t tick_sel : 4;
    uint32_t resv1 : 3;
    uint32_t alarm_en : 1;
    uint32_t alarm_clr : 1;
    uint32_t resv2 : 6;
    uint32_t second_cmp : 1;
    uint32_t minute_cmp : 1;
    uint32_t hour_cmp : 1;
    uint32_t week_cmp : 1;
    uint32_t day_cmp : 1;
    uint32_t month_cmp : 1;
    uint32_t year_cmp : 1;
    uint32_t resv3 : 1;
} __attribute__((packed, aligned(4))) rtc_interrupt_ctrl_t;

typedef struct _rtc_initial_count
{
    uint32_t curr_count : 15; /*!< RTC counter currunt value */
    uint32_t resv0 : 1;
    uint32_t sum_count : 15;   /*!< RTC counter max value */
    uint32_t resv1 : 1;
} __attribute__((packed, aligned(4))) rtc_count_t;

typedef struct _rtc
{
    rtc_date_t date;
    rtc_time_t time;
    uint32_t resv0;
    uint32_t resv1;
    rtc_count_t count;
    rtc_interrupt_ctrl_t int_ctrl;
} __attribute__((packed, aligned(4))) rtc_t;

#define PMU_BASEADDR 0x91000000UL

static int rtc_year_is_leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int rtc_date_time_set(int year, int month, int day, int hour, int minute, int second, int week)
{
    int fd;
    unsigned int *pmu_base = NULL;
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd == -1)
    {
        printf("open fail \n");
        return (-1);
    }

    pmu_base = (unsigned int *)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PMU_BASEADDR);
    if (pmu_base == MAP_FAILED) 
    {
        printf("mmap fail \n");
        close(fd);
        return -2;
    }
    volatile rtc_t *const rtc = (volatile rtc_t *)((unsigned long)pmu_base + 0xc00);
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

    date.year_h = year_h;
    date.year_l = year_l;
    date.month = month;
    date.day = day;
    date.leap_year = rtc_year_is_leap(year);
    time.week = week;
    time.hour = hour;
    time.minute = minute;
    time.second = second;

    rtc->int_ctrl.timer_w_en = 1;
    rtc->date = date;
    rtc->time = time;

    rtc_count_t current_count;
    current_count.curr_count = 0;
    current_count.sum_count = 32767;
    rtc->count = current_count;
    
    rtc->int_ctrl.timer_w_en = 1;
    rtc->int_ctrl.timer_w_en = 0;
    rtc->int_ctrl.timer_r_en = 1;

    munmap(pmu_base, 0x1000);
    return 0;
}

static int rtc_date_time_get(struct tm *local)
{
    int fd;
    unsigned int *pmu_base = NULL;
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd == -1)
    {
        printf("open fail \n");
        return (-1);
    }

    pmu_base = (unsigned int *)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, PMU_BASEADDR);
    if (pmu_base == MAP_FAILED) 
    {
        printf("mmap fail \n");
        close(fd);
        return -2;
    }
    volatile rtc_t *const rtc = (volatile rtc_t *)((unsigned long)pmu_base + 0xc00);

    if (rtc->int_ctrl.timer_r_en == 0)
        rtc->int_ctrl.timer_r_en = 1;
    rtc_date_t date = rtc->date;
    rtc_time_t time = rtc->time;

    local->tm_sec = time.second;
    local->tm_min = time.minute;
    local->tm_hour = time.hour;
    local->tm_mday = date.day;
    local->tm_mon = date.month - 1;
    local->tm_year = (date.year_h * 100 + date.year_l) - 1900;
    local->tm_wday = time.week;

    munmap(pmu_base, 0x1000);
    return 0;
}

int main(int argc, char *argv[]) {
    time_t now;
    struct tm *local;
    time(&now);
    local = localtime(&now);
    
    if (argc == 1)
    {
        printf("\n%s\n",asctime(local));
    }
    else if (argc == 2)
    {
        if(!strncmp(argv[1], "g", 1)) {
            if(rtc_date_time_get(local)<0)
                return -1;
            printf("\n%s\n",asctime(local));
        }
        else {
            if(rtc_date_time_set(
                local->tm_year + 1900,
                local->tm_mon + 1,
                local->tm_mday,
                local->tm_hour,
                local->tm_min,
                local->tm_sec,
                local->tm_wday)<0)
                return -1;
        }
    }
    else {
        printf("time_sync \n"
                "time_sync get\n"
                "time_sync set\n"
            );
    }

    return 0;
}
