/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-06     JasonHu      first version
 */

#include "segment.h"
#include "tss.h"
#include "cpuport.h"

#include <i386.h>
#include <rtthread.h>

struct rt_hw_segment
{
    rt_uint16_t limit_low, base_low;
    rt_uint8_t base_mid, access_right;
    rt_uint8_t limit_high, base_high;
};
typedef struct rt_hw_segment rt_hw_segment_t;

static void segment_set(rt_hw_segment_t *seg, rt_ubase_t limit,
                        rt_ubase_t base, rt_ubase_t attributes)
{
    seg->limit_low    = limit & 0xffff;
    seg->base_low     = base & 0xffff;
    seg->base_mid     = (base >> 16) & 0xff;
    seg->access_right = attributes & 0xff;
    seg->limit_high   = ((limit >> 16) & 0x0f) | ((attributes >> 8) & 0xf0);
    seg->base_high    = (base >> 24) & 0xff;
}

/**
 * in x86, we can use fs/gs segment to save thread info,
 * set thread info base addr, os can use gs:0 to get the first
 * data on [base]
 */
void rt_hw_seg_tls_set(rt_ubase_t base)
{
    rt_hw_segment_t *seg = GDT_OFF2PTR(((rt_hw_segment_t *) GDT_VADDR), INDEX_USER_TLS);
    seg->base_low     = base & 0xffff;
    seg->base_mid     = (base >> 16) & 0xff;
    seg->base_high    = (base >> 24) & 0xff;
}

rt_ubase_t rt_hw_seg_tls_get()
{
    rt_hw_segment_t *seg = GDT_OFF2PTR(((rt_hw_segment_t *) GDT_VADDR), INDEX_USER_TLS);
    return (seg->base_low & 0xffff) | ((seg->base_mid & 0xff) << 16) | ((seg->base_high & 0xff) << 24);
}

void rt_hw_segment_init(void)
{
    /* Global segment table */
    rt_hw_segment_t *gdt = (rt_hw_segment_t *) GDT_VADDR;

    int i;
    for (i = 0; i <= GDT_LIMIT/8; i++)
    {
        segment_set(GDT_OFF2PTR(gdt, i), 0, 0, 0);
    }

    segment_set(GDT_OFF2PTR(gdt, INDEX_KERNEL_CODE), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_KERNEL_CODE_ATTR);
    segment_set(GDT_OFF2PTR(gdt, INDEX_KERNEL_DATA), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_KERNEL_DATA_ATTR);

    rt_hw_tss_t *tss = rt_hw_tss_get();
    segment_set(GDT_OFF2PTR(gdt, INDEX_TSS), sizeof(rt_hw_tss_t) - 1, (rt_ubase_t )tss, GDT_TSS_ATTR);

    segment_set(GDT_OFF2PTR(gdt, INDEX_USER_CODE), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_CODE_ATTR);
    segment_set(GDT_OFF2PTR(gdt, INDEX_USER_DATA), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_DATA_ATTR);

    segment_set(GDT_OFF2PTR(gdt, INDEX_USER_TLS), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_TLS_ATTR);

    extern void load_new_gdt(rt_ubase_t size, rt_ubase_t gdtr);
    load_new_gdt(GDT_LIMIT, GDT_VADDR);
}
