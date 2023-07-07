/* Copyright 2020 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <rtdef.h>
#include <stdlib.h>
#include "utest.h"
#include <math.h>

#define TS_DEV_NAME               "ts"
rt_device_t ts_dev = RT_NULL;

// #define _debug_print

#ifdef  _debug_print
#include <stdio.h>
#define DBG_BUFF_MAX_LEN          256

int float_printf(const char *fmt, ...)
{
    va_list args;
    static char rt_log_buf[DBG_BUFF_MAX_LEN] = { 0 };
    va_start(args, fmt);
    int length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    rt_kputs(rt_log_buf);
    return length;
}
#endif

static void ts_open(void)
{
    ts_dev = (rt_device_t)rt_device_find(TS_DEV_NAME);
    if (ts_dev == RT_NULL)
    {
        uassert_false(1);
        return;
    }

    if(rt_device_open(ts_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", TS_DEV_NAME);
        return;
    }

    return;
}

static void ts_close(void)
{
    rt_device_close(ts_dev);
}

static void test_ts_read(void)
{
    void *buffer = RT_NULL;
    rt_uint32_t reval;
    rt_uint32_t ts_val = 0;
    rt_uint32_t cnt;
    double code = 0, temp = 0;

    ts_open();

    for(cnt=0; cnt<5; cnt++)
    {
        reval = rt_device_read(ts_dev, 0, buffer, 4);
        if(reval <= 0)
        {
            uassert_false(1);
            return;
        }

        ts_val = *(uint32_t *)buffer;
        code = (double)(ts_val & 0xfff);
        
        rt_thread_mdelay(2600);

        if(ts_val >> 12)
        {
            temp = (1e-10 * pow(code, 4) * 1.01472 - 1e-6 * pow(code, 3) * 1.10063 + 4.36150 * 1e-3 * pow(code, 2) - 7.10128 * code + 3565.87);

        #ifdef _debug_print
            float_printf("ts_val: 0x%x, TS = %lf C\n", ts_val, temp);
        #else
            // rt_kprintf("ts_val: 0x%x, TS = %d C\n", ts_val, (int)temp);
            rt_kprintf("ts_val: 0x%x, TS = %d.%d C\n", ts_val, ((int)(temp * 1000000) / 1000000), ((int)(temp * 1000000) % 1000000));
        #endif
        }
    }

    ts_close();

    return;
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_ts_read);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.ts_tc", utest_tc_init, utest_tc_cleanup, 100);

/********************* end of file ************************/
