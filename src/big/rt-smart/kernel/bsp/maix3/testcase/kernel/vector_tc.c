#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>
#include <page.h>
#include "utest.h"

static volatile int loop_exit_flag;

static void thread_buzz_entry(void *param)
{
    // rt_kprintf("%s %d thread buzz start.\n", __FUNCTION__, __LINE__);
    if (loop_exit_flag != 1)
    {
        uassert_int_equal(loop_exit_flag, 1);
    }
    while (loop_exit_flag)
    {
        rt_thread_mdelay(1);
    }
    // rt_kprintf("%s %d thread buzz exit.\n", __FUNCTION__, __LINE__);
    if (loop_exit_flag != 0)
    {
        uassert_int_equal(loop_exit_flag, 0);
    }
}

static void thread_loop_entry(void *param)
{
    void *page = rt_pages_alloc(10);
    void *page2 = rt_pages_alloc(10);
    int word = 0;
    rt_tick_t tick_start, tick_end;

    // rt_kprintf("%s %d\n", __FUNCTION__, __LINE__);
    uassert_not_null(page);
    uassert_not_null(page2);
    if((!page) || (!page2))
    {
        rt_kprintf("%s %d, rt_pages_alloc failed!\n", __FUNCTION__, __LINE__);
        goto _exit;
    }

    tick_start = rt_tick_get();
    while (1)
    {
        memset(page2, word, 0x400000); // auto V optimize.
        memcpy(page, page2, 0x400000); // auto V optimize.
        word++;

        tick_end = rt_tick_get();
        if((tick_end - tick_start) > 1000)
        {
            // rt_kprintf("%s %d thread loop %d round exit.\n", __FUNCTION__, __LINE__, word);
            uassert_true(word > 1);
            break;
        }
    }

_exit:
    if(page)
    {
        rt_pages_free(page, 10);
    }

    if(page2)
    {
        rt_pages_free(page2, 10);
    }

    rt_thread_mdelay(100);
    loop_exit_flag = 0;
}

static rt_err_t utest_tc_init(void)
{
    loop_exit_flag = 1;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void test_dynamic_thread(void)
{
    rt_thread_t los_task = RT_NULL;
    los_task = rt_thread_create("buzz", thread_buzz_entry, RT_NULL, 8096, 5, 5);
    uassert_true(los_task != RT_NULL);
    if (!los_task)
    {
        rt_kprintf("rt_thread_create error\n");
        return;
    }

    rt_thread_t task2 = RT_NULL;
    task2 = rt_thread_create("loop", thread_loop_entry, RT_NULL, 8096, 20, 5);
    uassert_true(task2 != RT_NULL);
    if (!task2)
    {
        rt_kprintf("rt_thread_create error\n");
        return;
    }

    rt_thread_startup(los_task);
    rt_thread_startup(task2);

    while (loop_exit_flag)
    {
        rt_thread_mdelay(1);
    }

}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_dynamic_thread);
}

UTEST_TC_EXPORT(testcase, "testcases.kernel.vector_tc", utest_tc_init, utest_tc_cleanup, 1000);
