#include <stdio.h>
#include <utest.h>

extern void ping_timeout(void *result);
extern void pong_timeout(void *result);

static void pingpong_timeout_test(void)
{
    int ping_result = -1, pong_result = -1;
    rt_thread_t tid;

    tid = rt_thread_create("pong_timeout", pong_timeout, &pong_result, 4096, 10, 10);
    if (tid)
    {
        rt_thread_startup(tid);
        /* wait so that the pong_timeout thread runs first */
        rt_thread_mdelay(100);
        ping_timeout(&ping_result);
    }

    /* wait for the ping/pong procedures to update their results */
    rt_thread_mdelay(100);
    uassert_int_equal(pong_result, 0);
    uassert_int_equal(ping_result, 0);
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
    UTEST_UNIT_RUN(pingpong_timeout_test);
}

UTEST_TC_EXPORT(testcase, "utest.pingpong_test.pingpong_timeout_tc", utest_tc_init, utest_tc_cleanup, 500);

