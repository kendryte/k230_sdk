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
#include <stdlib.h>
#include "utest.h"

#define ADC_DEV_NAME            "adc"
#define ADC_MAX_CHANNEL         5       /* On the EVB board, the 6 channel is connected to GND */

static void test_probe(void)
{
    rt_adc_device_t adc_dev;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    uassert_not_null(adc_dev);

    return;
}

static void test_read(void)
{
    int i;
    rt_err_t ret = RT_EOK;
    rt_uint32_t reg_value = 0;
    rt_adc_device_t adc_dev;

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        uassert_false(1);
        return;
    }

    ret = rt_adc_enable(adc_dev, 0);
    uassert_int_equal(ret, RT_EOK);

    for (i = 0; i < 1; i++)
    {
        ret = rt_adc_enable(adc_dev, i);
        uassert_int_equal(ret, RT_EOK);

        reg_value = rt_adc_read(adc_dev, i);
        uassert_in_range(reg_value, 0xFF0, 0x1000);
    }

    for (i = 0; i < ADC_MAX_CHANNEL; i++)
    {
        ret = rt_adc_disable(adc_dev, i);
        uassert_int_equal(ret, RT_EOK);

        reg_value = rt_adc_read(adc_dev, i);
        uassert_int_equal(reg_value, 0);
    }

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
    UTEST_UNIT_RUN(test_probe);
    UTEST_UNIT_RUN(test_read);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.adc_tc", utest_tc_init, utest_tc_cleanup, 100);

/********************* end of file ************************/
