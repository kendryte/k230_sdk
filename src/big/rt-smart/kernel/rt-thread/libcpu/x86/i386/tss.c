/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-06     JasonHu      first version
 */

#include "tss.h"
#include "cpuport.h"
#include "segment.h"
#include <string.h>
#include <i386.h>
#include <rtthread.h>

static rt_hw_tss_t tss;

rt_hw_tss_t *rt_hw_tss_get()
{
    return &tss;
}

/**
 * @brief : set current process kernel stack top
 *
 * @param top : stack top
 */
void rt_hw_tss_set_kstacktop(rt_ubase_t top)
{
    // tss.esp0 is kernel statck
    tss.esp0 = top;
}

void rt_hw_tss_init()
{
    memset(&tss, 0, sizeof(rt_hw_tss_t));
    tss.esp0 = KERNEL_STACK_TOP;
    tss.ss0 = KERNEL_DATA_SEL;
    tss.iobase = sizeof(rt_hw_tss_t);
    /* load tr */
    ltr(KERNEL_TSS_SEL);
}
