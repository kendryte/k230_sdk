/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      First Version
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdef.h>
#include <rtdbg.h>

#include "cpuport.h"
#include "tss.h"
#include "segment.h"
#include "gate.h"
#include "stackframe.h"
#include "page.h"
#include "mmu.h"
#include <lwp.h>
#include <lwp_arch.h>
#include <interrupt.h>

/**
 * @brief from thread used interrupt context switch
 *
 */
volatile rt_ubase_t  rt_interrupt_from_thread = 0;
/**
 * @brief to thread used interrupt context switch
 *
 */
volatile rt_ubase_t  rt_interrupt_to_thread   = 0;
/**
 * @brief flag to indicate context switch in interrupt or not
 *
 */
volatile rt_ubase_t rt_thread_switch_interrupt_flag = 0;

extern void rt_hw_context_switch_to_real(rt_ubase_t to);
extern void rt_hw_context_switch_real(rt_ubase_t from, rt_ubase_t to);

/**
 * any thread will come here when first start
 */
static void rt_hw_thread_entry(hw_thread_func_t function, void *arg, void (*texit)())
{
    rt_hw_interrupt_enable(EFLAGS_IF);  /* enable interrupt, avoid not sched */
    function(arg);
    if (texit)
        texit();
    dbg_log(DBG_ERROR, "rt thread execute done, should never be here!");
    for (;;)
    {
    }
}

rt_uint8_t *rt_hw_stack_init(void       *tentry,
                             void       *parameter,
                             rt_uint8_t *stack_addr,
                             void       *texit)
{
    rt_uint8_t         *stk;
    stk  = stack_addr + sizeof(rt_ubase_t);
    stk  = (rt_uint8_t *)RT_ALIGN_DOWN((rt_ubase_t)stk, sizeof(rt_ubase_t));
    stk -= sizeof(struct rt_hw_stack_frame);
    stk -= sizeof(rt_hw_context_t);

    rt_hw_context_t *context = (rt_hw_context_t *)stk;
    context->eip = rt_hw_thread_entry;
    context->function = tentry;
    context->arg = parameter;
    context->texit = texit;
    context->ebp = context->ebx = context->esi = context->edi = 0;
    return stk;
}

void rt_hw_context_switch_to(rt_ubase_t to)
{
    rt_thread_t to_thread = rt_thread_sp_to_thread((void *)to);

#ifdef RT_USING_USERSPACE
    /**
     * update kernel esp0 as to thread's kernel stack, to make sure process can't
     * get the correct kernel stack from tss esp0 when interrupt occur in user mode.
     */
    rt_ubase_t stacktop = (rt_ubase_t)(to_thread->stack_addr + to_thread->stack_size);
    rt_hw_tss_set_kstacktop(stacktop);
    lwp_mmu_switch(to_thread);  /* switch mmu before switch context */
#endif /* RT_USING_USERSPACE */
    rt_hw_context_switch_to_real(to);
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to)
{
    rt_thread_t from_thread = rt_thread_sp_to_thread((void *)from);
    rt_thread_t to_thread = rt_thread_sp_to_thread((void *)to);

#ifdef RT_USING_LWP
    lwp_user_setting_save(from_thread);
#endif /* RT_USING_LWP */

#ifdef RT_USING_USERSPACE
    /**
     * update kernel esp0 as to thread's kernel stack, to make sure process can't
     * get the correct kernel stack from tss esp0 when interrupt occur in user mode.
     */
    rt_ubase_t stacktop = (rt_ubase_t)(to_thread->stack_addr + to_thread->stack_size);
    rt_hw_tss_set_kstacktop(stacktop);
    lwp_mmu_switch(to_thread);  /* switch mmu before switch context */
#endif /* RT_USING_USERSPACE */

    rt_hw_context_switch_real(from, to);

#ifdef RT_USING_LWP
    lwp_user_setting_restore(to_thread);
#endif /* RT_USING_LWP */
}

/**
 * when called rt_hw_context_switch_interrupt, just set from and to thread stack.
 * when interrupt leave, we check rt_thread_switch_interrupt_flag. if it's 1, we
 * will set rt_thread_switch_interrupt_flag as 0 then do context switch.
 * see interrupt_gcc.S on lable rt_hw_intr_thread_switch.
 */
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread, rt_thread_t to_thread)
{
    if (rt_thread_switch_interrupt_flag == 0)
        rt_interrupt_from_thread = from;

    rt_interrupt_to_thread = to;
    rt_thread_switch_interrupt_flag = 1;
    return;
}

void rt_hw_cpu_shutdown()
{
}

void rt_hw_cpu_init()
{
    rt_hw_segment_init();
    rt_hw_gate_init();
    rt_hw_tss_init();
}
