/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-19     WillianChan  the first version
 */

#include <rtthread.h>
#include "utest.h"

#define TIMER_TASK_STACK_SIZE 4096
#define TIMER_PERIODIC_TIMES 5

static rt_timer_t timer1 = RT_NULL, timer2 = RT_NULL;
static struct rt_timer timer3, timer4;
static volatile int timeout_flag1 = 0, timeout_flag2 = 0;


static void timeout1(void *parameter)
{
    if (timeout_flag1 ++ >= TIMER_PERIODIC_TIMES-1)
    {
        rt_timer_stop(timer1);
    }
}

static void timeout2(void *parameter)
{
    timeout_flag2 = 1;
}

static void test_timer_dynamic(void)
{
    int timeout_tick = 100;
    int timer1_start_tick = 0;
    int timer2_start_tick = 0;
    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, timeout_tick,
                             RT_TIMER_FLAG_PERIODIC);
    if (timer1 != RT_NULL)
    {
        if (rt_timer_start(timer1) != RT_EOK)
        {
            LOG_E("rt_timer_start failed");
            uassert_false(1);
            goto __exit;
        }
        timer1_start_tick = rt_tick_get();
    }
    else
    {
        LOG_E("rt_timer_create failed");
        uassert_false(timer1 == RT_NULL);
        goto __exit;
    }
    
    while (timeout_flag1 != TIMER_PERIODIC_TIMES)
    {
        if(rt_tick_get()-timer1_start_tick > (TIMER_PERIODIC_TIMES*timeout_tick))
        {
            LOG_E("timer1_start_tick  %d ,%d\n",rt_tick_get()-timer1_start_tick,TIMER_PERIODIC_TIMES*timeout_tick);
            LOG_E("timeout_flag1 rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }

    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL,  timeout_tick,
                             RT_TIMER_FLAG_ONE_SHOT);
    if (timer2 != RT_NULL)
    {
        if (rt_timer_start(timer2) != RT_EOK)
        {
            LOG_E("rt_timer_start failed");
            uassert_false(1);
            goto __exit;
        }
        timer2_start_tick = rt_tick_get();
    }
    else
    {
        LOG_E("rt_timer_create failed");
        uassert_false(timer2 == RT_NULL);
        goto __exit;
    }
    while (timeout_flag2 != 1)
    {
        if(rt_tick_get()-timer2_start_tick>(timeout_tick))
        {
            LOG_E("timer2_start_tick  %d ,%d\n",rt_tick_get()-timer2_start_tick,timeout_tick);
            LOG_E("timeout_flag2 rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }
    if (rt_timer_delete(timer1) != RT_EOK)
    {
        LOG_E("rt_timer_delete failed!");
        uassert_false(1);
        goto __exit;
    }
    timer1 = RT_NULL;
    if (rt_timer_delete(timer2) != RT_EOK)
    {
        LOG_E("rt_timer_delete failed!");
        uassert_false(1);
        goto __exit;
    }
    timer2 = RT_NULL;
    uassert_true(1);

__exit:
    if (timer1 != RT_NULL)
    {
        rt_timer_delete(timer1);
    }
    if (timer2 != RT_NULL)
    {
        rt_timer_delete(timer2);
    }
    timeout_flag1 = 0;
    timeout_flag2 = 0;
    return;
}

static void timeout3(void *parameter)
{
    if (timeout_flag1++ >= 4)
    {
        rt_timer_stop(&timer3);
    }
}

static void timeout4(void *parameter)
{
    timeout_flag2 = 1;
}

static void test_timer_static(void)
{
    rt_err_t rst_start3 = -RT_ERROR;
    rt_err_t rst_start4 = -RT_ERROR;
    int timeout_tick = 100;
    int timer3_start_tick = 0;
    int timer4_start_tick = 0;

    rt_timer_init(&timer3, "timer3",
                  timeout3, RT_NULL, timeout_tick,
                  RT_TIMER_FLAG_PERIODIC);
    rst_start3 = rt_timer_start(&timer3);
    if (rst_start3 != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        goto __exit;
    }
    timer3_start_tick = rt_tick_get();

    while (timeout_flag1 != TIMER_PERIODIC_TIMES)
    {
        if(rt_tick_get()-timer3_start_tick>(TIMER_PERIODIC_TIMES*timeout_tick))
        {
            LOG_E("timer3_start_tick  %d ,%d\n",rt_tick_get()-timer3_start_tick,TIMER_PERIODIC_TIMES*timeout_tick);
            LOG_E("rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }

    rt_timer_init(&timer4, "timer4",
                  timeout4, RT_NULL, timeout_tick,
                  RT_TIMER_FLAG_ONE_SHOT);
    rst_start4 = rt_timer_start(&timer4);
    if (rst_start4 != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        goto __exit;
    }
    timer4_start_tick = rt_tick_get();
    while (timeout_flag2 != 1)
    {
        if(rt_tick_get()-timer4_start_tick>(timeout_tick))
        {
            LOG_E("timer4_start_tick  %d ,%d\n",rt_tick_get()-timer4_start_tick,timeout_tick);
            LOG_E("rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }

    if (rt_timer_detach(&timer3) != RT_EOK)
    {
        LOG_E("rt_timer_detach failed!");
        uassert_false(1);
        goto __exit;
    }
    if (rt_timer_detach(&timer4) != RT_EOK)
    {
        LOG_E("rt_timer_detach failed!");
        uassert_false(1);
        goto __exit;
    }
    uassert_true(1);


__exit:
    if (rst_start3 != RT_EOK)
    {
        rt_timer_detach(&timer3);
    }
    if (rst_start4 != RT_EOK)
    {
        rt_timer_detach(&timer4);
    }
    timeout_flag1 = 0;
    timeout_flag2 = 0;
    return;
}

static void test_timer_control(void)
{
    int timeout_tick = 10;
    rt_tick_t timeout_get = 0;
    rt_tick_t timeout_set = 50;
    rt_err_t rst = -RT_ERROR;
    rt_uint32_t timeout_tick_before = 0, timeout_tick_after = 0;
    int timer_start_tick = 0;
    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL, timeout_tick,
                             RT_TIMER_FLAG_ONE_SHOT);
    if (rt_timer_start(timer2) != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        uassert_false(1);
        goto __exit;
    }
    timer_start_tick = rt_tick_get();
    rst = rt_timer_control(timer2, RT_TIMER_CTRL_GET_TIME, (void *)&timeout_get);
    if (rst != RT_EOK)
    {
        LOG_E("rt_timer_control failed");
        uassert_false(1);
        goto __exit;
    }
    if (timeout_get != timeout_tick || rst != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        uassert_false(1);
        goto __exit;
    }
    while (timeout_flag2 != 1)
    {
        if(rt_tick_get()-timer_start_tick>(timeout_tick))
        {
            LOG_E("timer2_start_tick  %d ,%d\n",rt_tick_get()-timer_start_tick,timeout_tick);
            LOG_E("rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }
    timeout_flag2 = 0;
    timeout_tick_before = timer2->timeout_tick;

    rt_timer_control(timer2, RT_TIMER_CTRL_SET_TIME, (void *)&timeout_set);
    rt_timer_start(timer2);
    timer_start_tick = rt_tick_get();
    while (timeout_flag2 != 1)
    {
        if(rt_tick_get()-timer_start_tick>(timeout_set))
        {
            LOG_E("timer2_start_tick  %d ,%d\n",rt_tick_get()-timer_start_tick,timeout_tick);
            LOG_E("rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }
    timeout_flag2 = 0;
    timeout_tick_after = timer2->timeout_tick;

    rt_timer_control(timer2, RT_TIMER_CTRL_GET_TIME, (void *)&timeout_get);
    rt_timer_start(timer2);
    timer_start_tick = rt_tick_get();
    while (timeout_flag2 != 1)
    {
        if(rt_tick_get()-timer_start_tick>(timeout_get))
        {
            LOG_E("timer2_start_tick  %d ,%d\n",rt_tick_get()-timer_start_tick,timeout_tick);
            LOG_E("rt_timer_stop timeout!");
            uassert_false(1);
            goto __exit;
        }
    }
    int ret_def = timeout_tick_after - timeout_tick_before;
    LOG_I("timeout_get %d ret_def %d",timeout_get,ret_def);
    uassert_true(timeout_get == 50 && ret_def == 50);

__exit:
    if (timer2 != RT_NULL)
    {
        rt_timer_delete(timer2);
    }
    return;
}

static rt_err_t utest_tc_init(void)
{
    timeout_flag1 = 0;
    timeout_flag2 = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    LOG_I("in testcase func...");

    rt_system_timer_init(TIMER_TASK_STACK_SIZE);
    UTEST_UNIT_RUN(test_timer_dynamic);
    UTEST_UNIT_RUN(test_timer_static);
    UTEST_UNIT_RUN(test_timer_control);
}
UTEST_TC_EXPORT(testcase, "utest.timer.timer_tc", utest_tc_init, utest_tc_cleanup, 60); 
