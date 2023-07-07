/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-12     luckyzjq     the first version
 */

#include <rtthread.h>
#include <stdlib.h>
#include "utest.h"

static rt_uint8_t timer_flag_oneshot[] = {
    RT_TIMER_FLAG_ONE_SHOT,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_HARD_TIMER,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER,
};

static rt_uint8_t timer_flag_periodic[] = {
    RT_TIMER_FLAG_PERIODIC,
    RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_HARD_TIMER,
    RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER,
};

typedef struct test_timer_struct
{
    struct rt_timer timer; /* static timer handler */
    rt_timer_t dynamic_timer;     /* dynamic timer pointer */
    rt_tick_t expect_tick;        /* expect tick */
    rt_uint8_t test_flag;         /* timer callback done flag */
    rt_uint8_t periodic_count;
} timer_struct;

static timer_struct timer;
static timer_struct _multi_timer[5];

#define TIMER_PERIODIC_TEST_TIMES   5
#define TIMER_PERIODIC_TICK 5

#define test_static_timer_start test_static_timer_init
#define test_static_timer_stop test_static_timer_init
#define test_static_timer_detach test_static_timer_init

static void static_timer_oneshot(void *param)
{
    timer_struct *timer_call;

    timer_call = (timer_struct *)param;
    timer_call->test_flag = RT_TRUE;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
    }

    return;
}

static void static_timer_periodic(void *param)
{
    rt_err_t result;
    timer_struct *timer_call;

    timer_call = (timer_struct *)param;
    timer_call->test_flag = RT_TRUE;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
    }

    /* periodic timer can stop */
    result = rt_timer_stop(&timer_call->timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    return;
}

static void _timer_periodic_callback(void *param)
{
    rt_err_t result;
    rt_tick_t get_tick;
    timer_struct *timer_call;

    timer_call = (timer_struct *)param;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
         /* periodic timer can stop */
        result = rt_timer_stop(&timer_call->timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->test_flag = RT_TRUE;
    }

    timer_call->periodic_count++;
    if (timer_call->periodic_count == TIMER_PERIODIC_TEST_TIMES)
    {
        /* periodic timer can stop */
        result = rt_timer_stop(&timer_call->timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->test_flag = RT_TRUE;
    }
    else
    {
        result = rt_timer_control(&timer_call->timer, RT_TIMER_CTRL_GET_TIME, &get_tick);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->expect_tick = rt_tick_get() + get_tick;
    }
}

static void test_static_timer_init(void)
{
    rt_err_t result;
    int rand_num = (rand() % 10) + 1; //Eliminate the zero
    /* one shot timer test */
    for (int time_out = 0; time_out < rand_num; time_out++)
    {
        for (int i = 0; i < sizeof(timer_flag_oneshot); i++)
        {
            rt_timer_init(&timer.timer,
                          "static_timer",
                          static_timer_oneshot,
                          &timer,
                          time_out,
                          timer_flag_oneshot[i]);

            /* calc expect tick */
            timer.expect_tick = rt_tick_get() + time_out;

            /* start timer */
            result = rt_timer_start(&timer.timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            /* wait for timerout
            * should timeout only 1 times
            */
            rt_thread_delay(5 * time_out + 1);

            /* detach timer */
            result = rt_timer_detach(&timer.timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            if (timer.test_flag != RT_TRUE)
            {
                uassert_true(RT_FALSE);
                return;
            }
        }
    }

    /* periodic timer test */
    for (int time_out = 0; time_out < rand_num; time_out++)
    {
        for (int i = 0; i < sizeof(timer_flag_periodic); i++)
        {
            rt_timer_init(&timer.timer,
                          "static_timer",
                          static_timer_periodic,
                          &timer,
                          time_out,
                          timer_flag_periodic[i]);

            /* calc expect tick */
            timer.expect_tick = rt_tick_get() + time_out;

            /* start timer */
            result = rt_timer_start(&timer.timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            /* wait for timeout */
            rt_thread_delay(time_out + 1);

            /* detach timer */
            result = rt_timer_detach(&timer.timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            if (timer.test_flag != RT_TRUE)
            {
                uassert_true(RT_FALSE);
                return;
            }
        }
    }

    timer.test_flag = RT_FALSE;
    uassert_true(RT_TRUE);

    /* install periodic timer run */
    rt_timer_init(&timer.timer,
                "static_timer",
                _timer_periodic_callback,
                &timer,
                TIMER_PERIODIC_TICK,
                RT_TIMER_FLAG_PERIODIC);

    /* calc expect tick */
    timer.expect_tick = rt_tick_get() + TIMER_PERIODIC_TICK;
    timer.periodic_count = 0;

    /* start timer */
    result = rt_timer_start(&timer.timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    /* wait timer test run done */
    while (timer.test_flag == RT_FALSE)
    {
        rt_thread_delay(TIMER_PERIODIC_TICK);
    }

    timer.test_flag = RT_FALSE;
    uassert_int_equal(timer.periodic_count, TIMER_PERIODIC_TEST_TIMES);

    /* detach timer */
    result = rt_timer_detach(&timer.timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    return;
}

static void test_static_timer_control(void)
{
    rt_err_t result;
    int rand_num = (rand() % 10) + 1; // Eliminate the zero
    int set_data;
    int get_data;

    rt_timer_init(&timer.timer,
                  "static_timer",
                  _timer_periodic_callback,
                  &timer,
                  5,
                  RT_TIMER_FLAG_PERIODIC);

    /* test set data */
    set_data = rand_num;
    result = rt_timer_control(&timer.timer, RT_TIMER_CTRL_SET_TIME, &set_data);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    /* test get data */
    result = rt_timer_control(&timer.timer, RT_TIMER_CTRL_GET_TIME, &get_data);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    /* a set of test */
    if (set_data != get_data)
    {
        uassert_true(RT_FALSE);
    }

    /* calc expect tick */
    timer.expect_tick = rt_tick_get() + set_data;
    timer.periodic_count = 0;
    timer.test_flag = RT_FALSE;

    /* start timer */
    result = rt_timer_start(&timer.timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    /* wait timer test run done */
    while (timer.test_flag == RT_FALSE)
    {
        rt_thread_delay(set_data);
    }

    /* control change to oneshot mode */
    rt_timer_control(&timer.timer, RT_TIMER_CTRL_SET_FUNC, static_timer_oneshot);
    rt_timer_control(&timer.timer, RT_TIMER_CTRL_SET_ONESHOT, RT_NULL);
    timer.expect_tick = rt_tick_get() + set_data;
    timer.periodic_count = 0;
    timer.test_flag = RT_FALSE;

    /* start timer */
    result = rt_timer_start(&timer.timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    rt_thread_delay(5 * set_data + 1);

    /* detach timer */
    result = rt_timer_detach(&timer.timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    if (timer.test_flag != RT_TRUE)
    {
        uassert_true(RT_FALSE);
        return;
    }

    timer.test_flag = RT_FALSE;
    uassert_true(RT_TRUE);
}

static void _multi_timer_periodic(void *param)
{
    rt_err_t result;
    rt_tick_t get_tick;
    timer_struct *timer_call;

    timer_call = (timer_struct *)param;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
         /* periodic timer can stop */
        result = rt_timer_stop(&timer_call->timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->test_flag = RT_TRUE;
    }

    timer_call->periodic_count++;
    if (timer_call->periodic_count == TIMER_PERIODIC_TEST_TIMES)
    {
        /* periodic timer can stop */
        result = rt_timer_stop(&timer_call->timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->test_flag = RT_TRUE;
    }
    else
    {
        result = rt_timer_control(&timer_call->timer, RT_TIMER_CTRL_GET_TIME, &get_tick);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
        timer_call->expect_tick = rt_tick_get() + get_tick;
    }
}

static void test_static_multi_timer_accuracy(void)
{
    rt_err_t result;
    int i;

    for (i = 0; i < sizeof(_multi_timer) / sizeof(_multi_timer[0]); i++)
    {
        rt_timer_init(&_multi_timer[i].timer,
                    "static_timer",
                    _multi_timer_periodic,
                    &_multi_timer[i],
                    TIMER_PERIODIC_TICK + i,
                    RT_TIMER_FLAG_PERIODIC);

        /* calc expect tick */
        _multi_timer[i].expect_tick = rt_tick_get() + TIMER_PERIODIC_TICK + i;

        _multi_timer[i].periodic_count = 0;
        _multi_timer[i].test_flag = RT_FALSE;
        /* start timer */
        result = rt_timer_start(&_multi_timer[i].timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
            return;
        }
    }

    for (i = 0; i < sizeof(_multi_timer) / sizeof(_multi_timer[0]); i++)
    {
        /* wait timer test run done */
        while (_multi_timer[i].test_flag == RT_FALSE)
        {
            rt_thread_delay(TIMER_PERIODIC_TICK);
        }

        _multi_timer[i].test_flag = RT_FALSE;
        uassert_int_equal(_multi_timer[i].periodic_count, TIMER_PERIODIC_TEST_TIMES);

        /* detach timer */
        result = rt_timer_detach(&_multi_timer[i].timer);
        if (RT_EOK != result)
        {
            uassert_true(RT_FALSE);
        }
    }
}

#ifdef RT_USING_HEAP

#define test_dynamic_timer_start test_dynamic_timer_create
#define test_dynamic_timer_stop test_dynamic_timer_create
#define test_dynamic_timer_delete test_dynamic_timer_create

static void dynamic_timer_oneshot(void *param)
{
    timer_struct *timer_call;
    timer_call = (timer_struct *)param;
    timer_call->test_flag = RT_TRUE;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
    }

    return;
}
static void dynamic_timer_periodic(void *param)
{
    rt_err_t result;
    timer_struct *timer_call;
    timer_call = (timer_struct *)param;
    timer_call->test_flag = RT_TRUE;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
    }

    /* periodic timer can stop */
    result = rt_timer_stop(timer_call->dynamic_timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    return;
}

static void test_dynamic_timer_create(void)
{
    rt_err_t result;
    int rand_num = (rand() % 10) + 1;// Eliminate the zero
    /* one shot timer test */
    for (int time_out = 1; time_out <= rand_num; time_out++)
    {
        for (int i = 0; i < sizeof(timer_flag_oneshot); i++)
        {
            timer.dynamic_timer = rt_timer_create("dynamic_timer",
                                                  dynamic_timer_oneshot,
                                                  &timer,
                                                  time_out,
                                                  timer_flag_oneshot[i]);

            /* calc expect tick */
            timer.expect_tick = rt_tick_get() + time_out;

            /* start timer */
            result = rt_timer_start(timer.dynamic_timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            /* wait for timerout */
            rt_thread_delay(time_out + 1);

            /* detach timer */
            result = rt_timer_delete(timer.dynamic_timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            if (timer.test_flag != RT_TRUE)
            {
                uassert_true(RT_FALSE);
                return;
            }
        }
    }

    /* periodic timer test */
    for (int time_out = 1; time_out <= rand_num; time_out++)
    {
        for (int i = 0; i < sizeof(timer_flag_periodic); i++)
        {
            timer.dynamic_timer = rt_timer_create("dynamic_timer",
                                                  dynamic_timer_periodic,
                                                  &timer,
                                                  time_out,
                                                  timer_flag_periodic[i]);

            /* calc expect tick */
            timer.expect_tick = rt_tick_get() + time_out;

            /* start timer */
            result = rt_timer_start(timer.dynamic_timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            /* wait for timerout */
            rt_thread_delay(time_out + 1);

            /* detach timer */
            result = rt_timer_delete(timer.dynamic_timer);
            if (RT_EOK != result)
            {
                uassert_true(RT_FALSE);
                return;
            }

            if (timer.test_flag != RT_TRUE)
            {
                uassert_true(RT_FALSE);
                return;
            }
        }
    }

    timer.test_flag = RT_FALSE;
    uassert_true(RT_TRUE);

    return;
}

static void dynamic_timer_control(void *param)
{
    rt_err_t result;
    timer_struct *timer_call;
    timer_call = (timer_struct *)param;
    timer_call->test_flag = RT_TRUE;

    /* check expect tick */
    if (rt_tick_get() - timer_call->expect_tick > 1)
    {
        uassert_true(RT_FALSE);
    }

    /* periodic timer can stop */
    result = rt_timer_stop(timer_call->dynamic_timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    return;
}

static void test_dynamic_timer_control(void)
{
    rt_err_t result;
    int rand_num = (rand() % 10) + 1;// Eliminate the zero
    int set_data;
    int get_data;

    timer.dynamic_timer = rt_timer_create("dynamic_timer",
                                          dynamic_timer_control,
                                          &timer,
                                          5,
                                          RT_TIMER_FLAG_PERIODIC);

    /* test set data */
    set_data = rand_num;
    result = rt_timer_control(timer.dynamic_timer, RT_TIMER_CTRL_SET_TIME, &set_data);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    /* test get data */
    result = rt_timer_control(timer.dynamic_timer, RT_TIMER_CTRL_GET_TIME, &get_data);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    /* a set of test */
    if (set_data != get_data)
    {
        uassert_true(RT_FALSE);
    }

    /* calc expect tick */
    timer.expect_tick = rt_tick_get() + set_data;

    /* start timer */
    result = rt_timer_start(timer.dynamic_timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    rt_thread_delay(set_data + 1);

    /* detach timer */
    result = rt_timer_delete(timer.dynamic_timer);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    if (timer.test_flag != RT_TRUE)
    {
        uassert_true(RT_FALSE);
        return;
    }

    timer.test_flag = RT_FALSE;
    uassert_true(RT_TRUE);
}

#endif /* RT_USING_HEAP */

static rt_err_t utest_tc_init(void)
{
    timer.dynamic_timer = RT_NULL;
    timer.test_flag = RT_FALSE;

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    timer.dynamic_timer = RT_NULL;
    timer.test_flag = RT_FALSE;

    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_static_timer_init);
    UTEST_UNIT_RUN(test_static_timer_start);
    UTEST_UNIT_RUN(test_static_timer_stop);
    UTEST_UNIT_RUN(test_static_timer_detach);
    UTEST_UNIT_RUN(test_static_timer_control);
    UTEST_UNIT_RUN(test_static_multi_timer_accuracy);
#ifdef RT_USING_HEAP
    UTEST_UNIT_RUN(test_dynamic_timer_create);
    UTEST_UNIT_RUN(test_dynamic_timer_start);
    UTEST_UNIT_RUN(test_dynamic_timer_stop);
    UTEST_UNIT_RUN(test_dynamic_timer_delete);
    UTEST_UNIT_RUN(test_dynamic_timer_control);
#endif /* RT_USING_HEAP */
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.timer_tc", utest_tc_init, utest_tc_cleanup, 1000);

/*********************** end of file ****************************/
