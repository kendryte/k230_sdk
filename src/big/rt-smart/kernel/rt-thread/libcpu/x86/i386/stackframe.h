/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-16     JasonHu      first version
 */

#ifndef __STACK_FRAME_H__
#define __STACK_FRAME_H__

#include <rtdef.h>

struct rt_hw_stack_frame
{
    rt_uint32_t vec_no;

    rt_uint32_t edi;
    rt_uint32_t esi;
    rt_uint32_t ebp;
    rt_uint32_t esp_dummy; /* esp_dummy not used, only use a position */
    rt_uint32_t ebx;
    rt_uint32_t edx;
    rt_uint32_t ecx;
    rt_uint32_t eax;

    rt_uint32_t gs;
    rt_uint32_t fs;
    rt_uint32_t es;
    rt_uint32_t ds;
    rt_uint32_t error_code; /* error code will push into stack if exception has it, or not push 0 by ourself */
    rt_uint32_t eip;
    rt_uint32_t cs;
    rt_uint32_t eflags;
    /* 
     * below will push into stack when from user privilege level enter
     * kernel privilege level (syscall/excption/interrupt) 
     */
    rt_uint32_t esp;
    rt_uint32_t ss;
} __attribute__((packed));

typedef struct rt_hw_stack_frame rt_hw_stack_frame_t;

typedef void (*hw_thread_func_t)(void *);

/* we use ebp, ebx, edi, esi, eip as context for fork/clone */
#define HW_CONTEXT_MEMBER_NR    5

struct rt_hw_context
{
    rt_uint32_t ebp;
    rt_uint32_t ebx;
    rt_uint32_t edi;
    rt_uint32_t esi;

    /* first run point to func, other time point to the ret addr of switch_to */
    void (*eip) (hw_thread_func_t func, void *arg, void (*texit)());

    rt_uint32_t unused;
    hw_thread_func_t function;
    void *arg;
    void *texit; /* thread exit call */
};
typedef struct rt_hw_context rt_hw_context_t;

void rt_hw_stack_frame_dump(rt_hw_stack_frame_t *frame);

#endif  /* __STACK_FRAME_H__ */
