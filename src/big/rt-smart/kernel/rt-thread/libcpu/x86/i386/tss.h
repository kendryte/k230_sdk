/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-06     JasonHu      first version
 */

#ifndef __X86_TSS_H__
#define __X86_TSS_H__

#include <rtdef.h>
#include <rtconfig.h>

#define KERNEL_STACK_TOP_PHY 0x9f000
#define KERNEL_STACK_TOP (KERNEL_VADDR_START + KERNEL_STACK_TOP_PHY)

struct rt_hw_tss
{
    rt_uint32_t backlink;
    rt_uint32_t esp0;
    rt_uint32_t ss0;
    rt_uint32_t esp1;
    rt_uint32_t ss1;
    rt_uint32_t esp2;
    rt_uint32_t ss2;
    rt_uint32_t cr3;
    rt_uint32_t eip;
    rt_uint32_t eflags;
    rt_uint32_t eax;
    rt_uint32_t ecx;
    rt_uint32_t edx;
    rt_uint32_t ebx;
    rt_uint32_t esp;
    rt_uint32_t ebp;
    rt_uint32_t esi;
    rt_uint32_t edi;
    rt_uint32_t es;
    rt_uint32_t cs;
    rt_uint32_t ss;
    rt_uint32_t ds;
    rt_uint32_t fs;
    rt_uint32_t gs;
    rt_uint32_t ldtr;
    rt_uint32_t trap;
    rt_uint32_t iobase;
};
typedef struct rt_hw_tss rt_hw_tss_t;

void rt_hw_tss_init();
rt_hw_tss_t *rt_hw_tss_get();
void rt_hw_tss_set_kstacktop(rt_ubase_t top);

#endif  /* __X86_TSS_H__ */
