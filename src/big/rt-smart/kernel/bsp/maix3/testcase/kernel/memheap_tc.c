/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-16     flybreak     the first version
 */

#include <rtthread.h>
#include <stdlib.h>
#include "utest.h"

#define HEAP_SIZE        (64 * 1024)
#define HEAP_ALIGN       (8)
#define SLICE_NUM        (40)
#define TEST_TIMES       (100000)
#define HEAP_NAME        "heap1"
#define SLICE_SIZE_MAX   ((HEAP_SIZE - (2 + SLICE_NUM) * sizeof(struct rt_memheap_item)) / SLICE_NUM)

static struct rt_memheap heap1;
static rt_ubase_t ptr_start;
static void *ptr[SLICE_NUM];

static void test_memheap_init(void)
{
    /* init heap */
    ptr_start = (rt_ubase_t)rt_malloc_align(HEAP_SIZE, HEAP_ALIGN);
    uassert_not_null(ptr_start);
    rt_err_t res = rt_memheap_init(&heap1, HEAP_NAME, (void *)ptr_start, HEAP_SIZE);
    uassert_false(res);//res == 0
}

static void test_memheap_alloc(void)
{
    int i;
    /* test start ptr[] clean*/
    for (i = 0; i < SLICE_NUM; i++)
    {
        ptr[i] = 0;
    }
    /* test alloc */
    for (i = 0; i < SLICE_NUM; i++)
    {
        rt_uint32_t slice_size = rand() % SLICE_SIZE_MAX;
        ptr[i] = rt_memheap_alloc(&heap1, slice_size);
        uassert_not_null(ptr[i]);
    }

    uassert_null(rt_memheap_alloc(&heap1, HEAP_SIZE));  
}

static void test_memheap_free(void)
{
    int cnt = 0;
    /* test free */
    while (cnt < TEST_TIMES)
    {
        rt_uint32_t slice_size = rand() % (SLICE_SIZE_MAX - 1) + 1;//Eliminate the zero
        rt_uint32_t ptr_index = rand() % SLICE_NUM;
        if (ptr[ptr_index])
        {
            if (ptr[ptr_index])
            {
                rt_memheap_free(ptr[ptr_index]);
            }
            ptr[ptr_index] = rt_memheap_alloc(&heap1, slice_size);
        }
        cnt ++;
        if (cnt % (TEST_TIMES / 10) == 0)
        {
            uassert_not_null(ptr[ptr_index]);
            rt_kprintf(">");
        }
    }
}


static void test_memheap_realloc(void)
{
    int cnt = 0;
    /* test realloc */
    while (cnt < TEST_TIMES)
    {
        rt_uint32_t slice_size = rand() % (SLICE_SIZE_MAX - 1) + 1;//Eliminate the zero
        rt_uint32_t ptr_index = rand() % SLICE_NUM;
        int is_null_ptr = 0;

        if (ptr[ptr_index])
        {
            is_null_ptr = 0;
            ptr[ptr_index] = rt_memheap_realloc(&heap1, ptr[ptr_index], slice_size);
            if (!ptr[ptr_index])
            {
                is_null_ptr = 1;
            }
        }
        else
        {
            is_null_ptr = 1;
        }

        cnt ++;
        if (cnt % (TEST_TIMES / 10) == 0)
        {
            if(is_null_ptr)
                uassert_null(ptr[ptr_index]);
            else
                uassert_not_null(ptr[ptr_index]);
            rt_kprintf(">");
        }
    }
}

static void test_memheap_detach(void)
{
    /* test end */
    rt_memheap_detach(&heap1);
    rt_free_align((void *)ptr_start);
    uassert_not_null(ptr_start);
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
    UTEST_UNIT_RUN(test_memheap_init);
    UTEST_UNIT_RUN(test_memheap_alloc);
    UTEST_UNIT_RUN(test_memheap_free);
    UTEST_UNIT_RUN(test_memheap_realloc);
    UTEST_UNIT_RUN(test_memheap_detach);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.memheap_tc", utest_tc_init, utest_tc_cleanup, 10);
