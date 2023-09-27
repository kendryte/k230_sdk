/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-15     supperthomas add irq_test
 */

#include <rtthread.h>
#include "utest.h"
#include "rthw.h"

#define UTEST_NAME "irq_tc"
static volatile uint32_t irq_count = 0;
static volatile uint32_t max_get_nest_count = 0;

static void irq_callback()
{
    if(rt_interrupt_get_nest() >  max_get_nest_count)
    {
        max_get_nest_count = rt_interrupt_get_nest();
    }
    irq_count ++;
}

static void test_interrupt_enter_sethook(void)
{
    irq_count = 0;
    max_get_nest_count = 0;
    rt_interrupt_enter_sethook(irq_callback);
    rt_thread_mdelay(2);
    LOG_D("%s test irq_test! irq_count %d  max_get_nest_count %d\n", UTEST_NAME, irq_count, max_get_nest_count);
    uassert_int_not_equal(0, irq_count);
    uassert_int_not_equal(0, max_get_nest_count);
    rt_interrupt_enter_sethook(RT_NULL);
}

static void test_interrupt_leave_sethook(void)
{
    irq_count = 0;
    max_get_nest_count = 0;
    rt_interrupt_leave_sethook(irq_callback);
    rt_thread_mdelay(2);
    LOG_D("%s test irq_test! irq_count %d  max_get_nest_count %d\n", UTEST_NAME, irq_count, max_get_nest_count);
    uassert_int_not_equal(0, irq_count);
    uassert_int_equal(0, max_get_nest_count);//leave interrupt max_get_nest_count should be 0
    rt_interrupt_leave_sethook(RT_NULL);
}

static rt_err_t utest_tc_init(void)
{
    irq_count = 0;
    max_get_nest_count = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void test_interrupt_get_nest(void)
{
    max_get_nest_count = rt_interrupt_get_nest();
    uassert_int_equal(0, max_get_nest_count);
    rt_interrupt_enter_sethook(irq_callback);
    rt_thread_mdelay(2);
    LOG_D("%s test irq_test! irq_count %d  max_get_nest_count %d\n", UTEST_NAME, irq_count, max_get_nest_count);
    uassert_int_not_equal(0, max_get_nest_count);
    rt_interrupt_enter_sethook(RT_NULL);
    max_get_nest_count = 0;
    rt_interrupt_leave_sethook(irq_callback);
    rt_thread_mdelay(2);
    LOG_D("%s test irq_test! irq_count %d  max_get_nest_count %d\n", UTEST_NAME, irq_count, max_get_nest_count);
    uassert_int_equal(0, max_get_nest_count);//leave interrupt max_get_nest_count should be 0
    rt_interrupt_leave_sethook(RT_NULL);
}

static void test_hw_interrupt_enable(void)
{
    rt_base_t level;
    #define IRQ_DELAY (10000000)
    volatile uint32_t i = IRQ_DELAY;

    rt_interrupt_enter_sethook(irq_callback);

    max_get_nest_count = 0;
    while(i)i --;
    uassert_int_not_equal(0, max_get_nest_count);
    rt_kprintf("max_get_nest_count: %d\n",max_get_nest_count);

    level = rt_hw_interrupt_disable();
    max_get_nest_count = 0;
    i = IRQ_DELAY;
    while(i)i --;
    uassert_int_equal(0, max_get_nest_count);
    rt_kprintf("max_get_nest_count: %d\n",max_get_nest_count);

    rt_hw_interrupt_enable(level);
    max_get_nest_count = 0;
    i = IRQ_DELAY;
    while(i)i --;
    uassert_int_not_equal(0, max_get_nest_count);
    rt_kprintf("max_get_nest_count: %d\n",max_get_nest_count);
    rt_interrupt_enter_sethook(RT_NULL);
}

static void test_hw_interrupt_disable(void)
{
    rt_base_t level;
    volatile uint32_t i = 100000;

    rt_interrupt_enter_sethook(irq_callback);
    max_get_nest_count = 0;
    while(i)i --;
    uassert_int_not_equal(0, max_get_nest_count);
    rt_kprintf("max_get_nest_count: %d\n",max_get_nest_count);

    level = rt_hw_interrupt_disable();
    max_get_nest_count = 0;
    i = 100000;
    while(i)i --;
    uassert_int_equal(0, max_get_nest_count);
    rt_kprintf("max_get_nest_count: %d\n",max_get_nest_count);
    rt_hw_interrupt_enable(level);
    rt_interrupt_enter_sethook(RT_NULL);
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_interrupt_enter_sethook);
    UTEST_UNIT_RUN(test_interrupt_leave_sethook);
    UTEST_UNIT_RUN(test_hw_interrupt_enable);
    UTEST_UNIT_RUN(test_interrupt_get_nest);
    UTEST_UNIT_RUN(test_hw_interrupt_disable);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.irq_tc", utest_tc_init, utest_tc_cleanup, 10);
