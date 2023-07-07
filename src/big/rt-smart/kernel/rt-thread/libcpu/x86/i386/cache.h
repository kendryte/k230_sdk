/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-7-19      JasonHu      The first version
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include <rtdef.h>

rt_inline rt_uint32_t rt_cpu_icache_line_size()
{
    return 0;
}

rt_inline rt_uint32_t rt_cpu_dcache_line_size()
{
    return 0;
}

void rt_hw_cpu_icache_invalidate(void *addr,int size);
void rt_hw_cpu_dcache_invalidate(void *addr,int size);
void rt_hw_cpu_dcache_clean(void *addr,int size);
void rt_hw_cpu_icache_ops(int ops,void *addr,int size);
void rt_hw_cpu_dcache_ops(int ops,void *addr,int size);
void rt_hw_cpu_dcache_flush_all();
void rt_hw_cpu_icache_invalidate_all();
rt_base_t rt_hw_cpu_icache_status();
rt_base_t rt_hw_cpu_dcache_status();

#endif  /* __CACHE_H__ */
