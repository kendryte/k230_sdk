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

#define OTP_DEV_NAME            "otp"

rt_device_t otp_dev = RT_NULL;

static int otp_write(rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_size_t reval = -1;

    reval = rt_device_write(otp_dev, pos, buffer, size);
    if(reval <= 0)
    {
        uassert_false(1);
        return reval;
    }
    else
    {
        rt_kprintf("WRITE OTP SUCCESS!\n");
    }

    return reval;
}

static int otp_read(rt_off_t pos, void *buffer, rt_size_t size)
{
    int reval = -1;
    rt_uint32_t *buf = (rt_uint32_t *)buffer;

    reval = rt_device_read(otp_dev, pos, (void*)buf, size);
    if(reval <= 0)
    {
        uassert_false(1);
        return reval;
    }
    else
    {
        rt_kprintf("READ OTP SUCCESS!\n");
        for(int i=0; i<reval/4; i++)
        {
            rt_kprintf("\n\r%08x    0x%08x", pos+i*4, buf[i]);
        }
        rt_kprintf("\n");
    }

    return reval;
}

static void otp_open(void)
{
    otp_dev = (rt_device_t)rt_device_find(OTP_DEV_NAME);
    if (otp_dev == RT_NULL)
    {
        uassert_false(1);
        return;
    }

    if(rt_device_open(otp_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", OTP_DEV_NAME);
        return;
    }

    return;
}

static void otp_close(void)
{
    rt_device_close(otp_dev);
}

static void test_otp_read(void)
{
    rt_off_t pos;
    rt_size_t size;
    rt_uint32_t buffer[256] = {0};
    rt_uint32_t ret;

    // initilize param
    pos = 0x0;
    size = 0x300;

    otp_open();
    ret = otp_read(pos, (void *)buffer, size);
    if(ret < 0)
    {
        rt_kprintf("read test failed!\n");
        return;
    }
    otp_close();
    return;
}

static void test_otp_write(void)
{
    rt_off_t pos;
    rt_size_t size;
    rt_uint32_t buffer[256] = {0};
    rt_uint32_t ret;

    // initilize param
    pos = 0xc;
    size = 0x4;
    buffer[0] = 0xff11ff11;

    otp_open();
    ret = otp_write(pos, (void *)buffer, size);
    if(ret < 0)
    {
        rt_kprintf("write test failed!\n");
        return;
    }
    otp_close();
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
    UTEST_UNIT_RUN(test_otp_read);
    UTEST_UNIT_RUN(test_otp_write);
    UTEST_UNIT_RUN(test_otp_read);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.otp_tc", utest_tc_init, utest_tc_cleanup, 100);

/********************* end of file ************************/
