/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-05     JasonHu      The first version
 */

#ifndef CACHE_H__
#define CACHE_H__

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#include <opcode.h>

void rt_hw_cpu_dcache_clean(void *addr,int size);
void rt_hw_cpu_icache_invalidate(void *addr,int size);
void rt_hw_cpu_dcache_invalidate(void *addr,int size);

void rt_hw_cpu_dcache_clean_flush(void *addr,int size);
void rt_hw_cpu_dcache_clean_all(void);
void rt_hw_cpu_dcache_invalidate_all(void);
void rt_hw_cpu_icache_invalidate_all(void);

ALWAYS_INLINE void rt_hw_cpu_sync(void)
{
    asm volatile(OPC_SYNC::
                     : "memory");
}

ALWAYS_INLINE void rt_hw_cpu_sync_i(void)
{
    asm volatile(OPC_SYNC_I::
                     : "memory");
}

ALWAYS_INLINE void rt_hw_cpu_sync_s(void)
{
    asm volatile(OPC_SYNC_S::
                     : "memory");
}

ALWAYS_INLINE void rt_hw_cpu_sync_is(void)
{
    asm volatile(OPC_SYNC_IS::
                     : "memory");
}

#endif /* CACHE_H__ */
