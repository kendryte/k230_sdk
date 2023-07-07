/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-29     lizhirui     first version
 * 2021-11-05     JasonHu      add c906 cache inst
 * 2022-11-09     WangXiaoyao  Support cache coherence operations;
 *                             Porting to C908
 */

#include <rthw.h>
#include <rtdef.h>
#include <board.h>
#include <riscv.h>

#include "opcode.h"

#define L1_CACHE_BYTES (64)

/**
 * GCC version not support t-head cache flush, so we use fixed code to achieve.
 * The following function cannot be optimized.
 */
static void dcache_wb_range(unsigned long start, unsigned long end) __attribute__((optimize("O0")));
static void dcache_inv_range(unsigned long start, unsigned long end) __attribute__((optimize("O0")));
static void dcache_wbinv_range(unsigned long start, unsigned long end) __attribute__((optimize("O0")));
static void icache_inv_range(unsigned long start, unsigned long end) __attribute__((optimize("O0")));

#define CACHE_OP_RANGE(instr)                            \
    {                                                    \
        unsigned long i = start & ~(L1_CACHE_BYTES - 1); \
        for (; i < end; i += L1_CACHE_BYTES)             \
        {                                                \
            __asm__ volatile(instr ::"r"(i)              \
                             : "memory");                \
        }                                                \
    }

static void dcache_wb_range(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        /* asm volatile("dcache.cva %0\n"::"r"(i):"memory"); */
        /*
         * compiler always use a5 = i.
         * a6 not used, so we use a6 here.
         */
        asm volatile("mv a6, %0\n" ::"r"(i)
                     : "memory");         /* a6 = a5(i) */
        asm volatile(".long 0x0257800b"); /* dcache.cva a6 */
    }
}

static void dcache_inv_range(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        /* asm volatile("dcache.iva %0\n"::"r"(i):"memory"); */
        asm volatile("mv a6, %0\n" ::"r"(i)
                     : "memory");         /* a6 = a5(i) */
        asm volatile(".long 0x0268000b"); /* dcache.iva a6 */
    }
}

static void dcache_wbinv_range(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        /* asm volatile("dcache.civa %0\n"::"r"(i):"memory"); */
        asm volatile("mv a6, %0\n" ::"r"(i)
                     : "memory");         /* a6 = a5(i) */
        asm volatile(".long 0x0278000b"); /* dcache.civa a6 */
    }
}

static void icache_inv_range(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        /* asm volatile("icache.iva %0\n"::"r"(i):"memory"); */
        asm volatile("mv a6, %0\n" ::"r"(i)
                     : "memory");         /* a6 = a5(i) */
        asm volatile(".long 0x0308000b"); /* icache.iva a6 */
    }
}

rt_inline rt_uint32_t rt_cpu_icache_line_size(void)
{
    return L1_CACHE_BYTES;
}

rt_inline rt_uint32_t rt_cpu_dcache_line_size(void)
{
    return L1_CACHE_BYTES;
}

void rt_hw_cpu_icache_invalidate(void *addr, int size)
{
    icache_inv_range((unsigned long)addr, (unsigned long)((unsigned char *)addr + size));
    asm volatile(OPC_SYNC_IS::
                     : "memory");
}

void rt_hw_cpu_dcache_invalidate(void *addr, int size)
{
    dcache_inv_range((unsigned long)addr, (unsigned long)((unsigned char *)addr + size));
    asm volatile(OPC_SYNC_S::
                     : "memory");
}

void rt_hw_cpu_dcache_clean(void *addr, int size)
{
    dcache_wb_range((unsigned long)addr, (unsigned long)((unsigned char *)addr + size));
    asm volatile(OPC_SYNC_S::
                     : "memory");
}

void rt_hw_cpu_dcache_clean_flush(void *addr, int size)
{
    dcache_wbinv_range((unsigned long)addr, (unsigned long)((unsigned char *)addr + size));
    asm volatile(OPC_SYNC_S::
                     : "memory");
}

void rt_hw_cpu_dcachel1_clean_local(void *addr, int size)
{
    __asm__ volatile(OPC_DCACHE_CVAL1(a0)::
                         : "memory");
}

void rt_hw_cpu_icachel1_invalid_local(void *addr, int size)
{
    __asm__ volatile(OPC_DCACHE_CVAL1(a0)::
                         : "memory");
}

void rt_hw_cpu_icache_ops(int ops, void *addr, int size)
{
    if (ops == RT_HW_CACHE_INVALIDATE)
    {
        rt_hw_cpu_icache_invalidate(addr, size);
    }
}

void rt_hw_cpu_dcache_ops(int ops, void *addr, int size)
{
    if (ops == RT_HW_CACHE_FLUSH)
    {
        rt_hw_cpu_dcache_clean(addr, size);
    }
    else
    {
        rt_hw_cpu_dcache_invalidate(addr, size);
    }
}

void rt_hw_cpu_dcache_clean_all(void)
{
    /* asm volatile("dcache.call\n":::"memory"); */
    asm volatile(".long 0x0010000b\n" ::
                     : "memory");
}

void rt_hw_cpu_dcache_invalidate_all(void)
{
    /* asm volatile("dcache.ciall\n":::"memory"); */
    asm volatile(".long 0x0030000b\n" ::
                     : "memory");
}

void rt_hw_cpu_icache_invalidate_all(void)
{
    /* asm volatile("icache.iall\n":::"memory"); */
    asm volatile(".long 0x0100000b\n" ::
                     : "memory");
}

int sys_cacheflush(void *addr, int size, int cache)
{
    return 0;
}

void rt_hw_cpu_sync(void)
{
    asm volatile(OPC_SYNC::
                     : "memory");
}

void rt_hw_cpu_sync_i(void)
{
    asm volatile(OPC_SYNC_I::
                     : "memory");
}

void rt_hw_sync_cache_local(void *addr, int size)
{
    /**
     * TODO following APIs should be completed later
     */
    // rt_hw_cpu_dcachel1_clean_local(addr, size);
    // rt_hw_cpu_icachel1_invalidate_local(addr, size);

    /** not a perfect implementation */
    rt_hw_cpu_dcache_clean_all();
    rt_hw_cpu_sync();
    rt_hw_cpu_icache_invalidate_all();
    rt_hw_cpu_sync_i();
}
