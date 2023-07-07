/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include <mmu.h>
#include <ioremap.h>
#include <page.h>
#include "utest.h"

static void *cache_pv;
static void *nocache_pv;
void *phy;

extern rt_mmu_info mmu_info;

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static void test_pages_alloc(void)
{
    cache_pv = rt_pages_alloc(1);
    uassert_true(cache_pv != NULL);
}

static void test_hw_mmu_v2p(void)
{
    phy = rt_hw_mmu_v2p(&mmu_info, cache_pv);
    uassert_true(phy != NULL);
}

static void test_ioremap_nocache(void)
{
    nocache_pv = rt_ioremap_nocache(phy, 4096);
    uassert_true(nocache_pv != NULL);
}

static void test_hw_cpu_dcache_ops(void)
{
    int r_val;
    rt_tick_t tval;

    tval = rt_tick_get();

    *(int*)cache_pv = tval;
    r_val = *(volatile int*)nocache_pv;

    uassert_int_not_equal(tval, r_val);
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, cache_pv, sizeof(int));

    r_val = *(volatile int*)nocache_pv;
    uassert_int_equal(tval, r_val);
}

static rt_err_t utest_tc_cleanup(void)
{
    rt_pages_free(cache_pv, 1);
    rt_iounmap(nocache_pv);
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_pages_alloc);
    UTEST_UNIT_RUN(test_hw_mmu_v2p);
    UTEST_UNIT_RUN(test_ioremap_nocache);
    UTEST_UNIT_RUN(test_hw_cpu_dcache_ops);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.mmu_tc", utest_tc_init, utest_tc_cleanup, 1000);