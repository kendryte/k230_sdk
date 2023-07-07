/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-7-14      JasonHu      The first version
 */

#ifndef __CPUPORT_H__
#define __CPUPORT_H__

#include <rtconfig.h>
#include <rtdef.h>

#ifndef __ASSEMBLY__

/* write memory  */
rt_inline void rt_hw_dsb(void)
{
    asm volatile ("sfence": : :"memory");
}

/* read memory */
rt_inline void rt_hw_dmb(void)
{
    asm volatile ("lfence": : :"memory");
}

/* instruction */
rt_inline void rt_hw_isb(void)
{
    asm volatile ("": : :"memory");
}

#endif  /* __ASSEMBLY__ */

void rt_hw_cpu_init();

#endif  /* __CPUPORT_H__ */
