/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/10/28     Bernard      The unify RISC-V porting code.
 */

#include <rthw.h>
#include <rtthread.h>

#include <encoding.h>
#include "sbi.h"
#include "tick.h"
#include "clint.h"

static volatile uint64_t time_elapsed = 0;
static volatile unsigned long tick_cycles = 0;

static unsigned long tick_delta = TIMER_CLK_FREQ / RT_TICK_PER_SECOND;

static uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

int tick_isr(void)
{
#ifdef TICK_USE_SBI
    rt_tick_increase();
    sbi_set_timer(get_ticks() + tick_delta);
#else
    clint_timer_cmp_set_val(tick_delta);
    rt_tick_increase();
#endif
    return 0;
}

/* Sets and enable the timer interrupt */
int rt_hw_tick_init(void)
{
#ifdef TICK_USE_SBI
    /* Clear the Supervisor-Timer bit in SIE */
    clear_csr(sie, SIP_STIP);

    /* Set timer */
    sbi_set_timer(get_ticks() + tick_delta);

    /* Enable the Supervisor-Timer bit in SIE */
    set_csr(sie, SIP_STIP);
#else
    clint_timer_init();
#endif
    return 0;
}
