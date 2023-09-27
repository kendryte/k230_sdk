/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-19     MurphyZhao   the first version
 */

#include <rtthread.h>
#include <string.h>
#include <stdlib.h>

#include "utest.h"
#include <utest_log.h>
#include <rtconfig.h>

#undef DBG_TAG
#undef DBG_LVL

#define DBG_TAG          "utest"
#ifdef UTEST_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_INFO
#endif
#include <rtdbg.h>

#if RT_CONSOLEBUF_SIZE < 256
#error "RT_CONSOLEBUF_SIZE is less than 256!"
#endif

#ifdef UTEST_THR_STACK_SIZE
#define UTEST_THREAD_STACK_SIZE UTEST_THR_STACK_SIZE
#else
#define UTEST_THREAD_STACK_SIZE (4096)
#endif

#ifdef UTEST_THR_PRIORITY
#define UTEST_THREAD_PRIORITY   UTEST_THR_PRIORITY
#else
#define UTEST_THREAD_PRIORITY   FINSH_THREAD_PRIORITY
#endif

static rt_uint8_t utest_log_lv = UTEST_LOG_ALL;

/* the table and the number of all the available testcases */
static utest_tc_export_t tc_table = RT_NULL;
static rt_size_t tc_num;
static rt_uint32_t tc_loop = 1;     /* how many times all the testcases have to run */

static struct utest local_utest = {UTEST_PASSED, 0, 0};

#if defined(__ICCARM__) || defined(__ICCRX__)         /* for IAR compiler */
#pragma section="UtestTcTab"
#endif

void utest_log_lv_set(rt_uint8_t lv)
{
    if (lv == UTEST_LOG_ALL || lv == UTEST_LOG_ASSERT)
    {
        utest_log_lv = lv;
    }
}

/*
 * The structures recording all the testcases are compiled into the
 * 'UtestTcTab/__rt_utest_tc_tab' section. The initialiation procedure counts
 * the quantity of the testcases.
 */
int utest_init(void)
{
    /* initialize the utest commands table.*/
#if defined(__CC_ARM) || defined(__CLANG_ARM)       /* ARM C Compiler */
    extern const int UtestTcTab$$Base;
    extern const int UtestTcTab$$Limit;
    tc_table = (utest_tc_export_t)&UtestTcTab$$Base;
    tc_num = (utest_tc_export_t)&UtestTcTab$$Limit - tc_table;
#elif defined (__ICCARM__) || defined(__ICCRX__)    /* for IAR Compiler */
    tc_table = (utest_tc_export_t)__section_begin("UtestTcTab");
    tc_num = (utest_tc_export_t)__section_end("UtestTcTab") - tc_table;
#elif defined (__GNUC__)                            /* for GCC Compiler */
    extern const int __start_UtestTcTab;
    extern const int __stop_UtestTcTab;
    tc_table = (utest_tc_export_t)&__start_UtestTcTab;
    tc_num = (utest_tc_export_t) &__stop_UtestTcTab - tc_table;
#endif

    LOG_I("Initializing utest successfully!");
    LOG_I("The total number of testcases is %d", tc_num);
    return tc_num;
}

/* List the name and the maximum runtime limit of each testcase. */
void utest_tc_list(void)
{
    rt_size_t i = 0;

    LOG_I("Commands list : ");

    for (i = 0; i < tc_num; i++)
    {
        LOG_I("[testcase name]:%s; [run timeout]:%d", tc_table[i].name, tc_table[i].run_timeout);
    }
}

/* Remove the directory part of a path. */
static const char *file_basename(const char *file)
{
    char *end_ptr = RT_NULL;
    char *rst = RT_NULL;

    if (!((end_ptr = strrchr(file, '\\')) != RT_NULL || \
        (end_ptr = strrchr(file, '/')) != RT_NULL) || \
        (rt_strlen(file) < 2))
    {
        rst = (char *)file;
    }
    else
    {
        rst = (char *)(end_ptr + 1);
    }
    return (const char *)rst;
}

/**
 * Run the testcase that match 'utest_name'. If the specified name is null, run
 * all the testcases.
 */
void utest_run(const char *utest_name)
{
    rt_size_t i;
    rt_uint32_t loop;
    rt_bool_t is_find;

    rt_thread_mdelay(1000);

    for (loop = 0; loop < tc_loop; loop ++)
    {
        i = 0;
        is_find = RT_FALSE;
        LOG_I("[==========] [ utest    ] loop %d/%d", loop + 1, tc_loop);
        LOG_I("[==========] [ utest    ] started");

        /* traverse all the testcases */
        while(i < tc_num)
        {
            if (utest_name)
            {
                int len = strlen(utest_name);
                if (utest_name[len - 1] == '*') /* run all the sub-testcases */
                {
                    len -= 1;
                }

                /* not matched */
                if (rt_memcmp(tc_table[i].name, utest_name, len) != 0)
                {
                    i++;
                    continue;
                }
            }
            is_find = RT_TRUE;

            /* initialization */
            LOG_I("[----------] [ testcase ] (%s) started", tc_table[i].name);
            if (tc_table[i].init != RT_NULL)
            {
                if (tc_table[i].init() != RT_EOK)
                {
                    LOG_E("[  FAILED  ] [ result   ] testcase (%s): initialization failed.", tc_table[i].name);
                    goto __tc_continue;
                }
            }

            /* testcase */
            if (tc_table[i].tc != RT_NULL)
            {
                tc_table[i].tc();
                if (local_utest.failed_num == 0)
                {
                    LOG_I("[  PASSED  ] [ result   ] testcase (%s)", tc_table[i].name);
                }
                else
                {
                    LOG_E("[  FAILED  ] [ result   ] testcase (%s)", tc_table[i].name);
                }
            }
            else
            {
                LOG_E("[  FAILED  ] [ result   ] testcase (%s): lack of testcase function", tc_table[i].name);
            }

            /* cleanup */
            if (tc_table[i].cleanup != RT_NULL)
            {
                if (tc_table[i].cleanup() != RT_EOK)
                {
                    LOG_E("[  FAILED  ] [ result   ] testcase (%s): cleanup failed.", tc_table[i].name);
                    goto __tc_continue;
                }
            }

    __tc_continue:
            LOG_I("[----------] [ testcase ] (%s) finished", tc_table[i].name);

            i++;
        }

        if (i == tc_num && is_find == RT_FALSE && utest_name != RT_NULL)
        {
            LOG_I("[==========] [ utest    ] Not find (%s)", utest_name);
            LOG_I("[==========] [ utest    ] finished");
            break;
        }

        LOG_I("[==========] [ utest    ] finished");
    }
}

/*
 * The following 2 interfaces are used to invoke the testcase functions and
 * obtain the statistics.
 */
void utest_unit_run(test_unit_func func, const char *unit_func_name)
{
     LOG_I("[==========] utest unit name: (%s)", unit_func_name);
    local_utest.error = UTEST_PASSED;
    local_utest.passed_num = 0;
    local_utest.failed_num = 0;

    if (func != RT_NULL)
    {
        func();
    }
}

utest_t utest_handle_get(void)
{
    return (utest_t)&local_utest;
}

/**
 * The following 3 interfaces must be used to check the testcase result, like:
 *      void testcase_func(void)
 *      {
 *          ...
 *          result = test_some_aspect(...);
 *          utest_assert(result, ...);
 *          ...
 *      }
 */

/* Check whether the valud is zero. Update the statistics and log the result. */
void utest_assert(int value, const char *file, int line, const char *func, const char *msg)
{
    if (value == 0)
    {
        local_utest.failed_num++;
        local_utest.error = UTEST_FAILED;

        LOG_E("[  ASSERT  ] [ unit     ] at (%s); func: (%s:%d); msg: (%s)", file_basename(file), func, line, msg);
    }
    else
    {
        local_utest.passed_num++;

        if (utest_log_lv == UTEST_LOG_ALL)
        {
            LOG_D("[    OK    ] [ unit     ] (%s:%d) is passed", func, line);
        }
    }
}

/*
 * Check whether the 2 strings are equal or not. Update the statistics and log
 * the result.
 */
void utest_assert_string(const char *a, const char *b, rt_bool_t equal, const char *file, int line, const char *func, const char *msg)
{
    if (a == RT_NULL || b == RT_NULL)
    {
        utest_assert(0, file, line, func, msg);
        return ;
    }

    if ((!equal) ^ (rt_strcmp(a, b) == 0))
        utest_assert(1, file, line, func, msg);
    else
        utest_assert(0, file, line, func, msg);
}

/*
 * Check whether the 2 memory areas contain the same content or not. Update the
 * statistics and log the result.
 */
void utest_assert_buf(const char *a, const char *b, rt_size_t sz, rt_bool_t equal, const char *file, int line, const char *func, const char *msg)
{
    if (a == RT_NULL || b == RT_NULL)
    {
        utest_assert(0, file, line, func, msg);
        return ;
    }

    if ((!equal) ^ (rt_memcmp(a, b, sz) == 0))
        utest_assert(1, file, line, func, msg);
    else
        utest_assert(0, file, line, func, msg);
}
