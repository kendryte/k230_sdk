/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-04     JasonHu      First Version
 */

#include <rtthread.h>
#include <rtdbg.h>
#include <rtdef.h>
#include <bitmap.h>
#include <mmu.h>
#include <dma.h>

/**
 * dma is physical addr
 */
struct rt_hw_dma_manager
{
    rt_ubase_t start;
    rt_ubase_t end;
    rt_bitmap_t bitmap;
};
typedef struct rt_hw_dma_manager rt_hw_dma_manager_t;

static rt_hw_dma_manager_t g_dma_manager;

rt_err_t rt_hw_dma_init(rt_ubase_t start, rt_ubase_t end)
{
    g_dma_manager.start = start;
    g_dma_manager.end = end;
    rt_size_t pages = (end - start) / ARCH_PAGE_SIZE;
    RT_ASSERT(pages > 0);
    rt_size_t byte_len = pages / 8;
    rt_uint8_t *bits = rt_malloc(byte_len);
    if (!bits)
    {
        return RT_ENOMEM;
    }
    rt_kprintf("dma: range:%x~%x, pages:%d, bitmap bits:%x, byte_len:0x%x\n",
               start, end, pages, bits, byte_len);
    rt_bitmap_init(&g_dma_manager.bitmap, bits, byte_len);

    return RT_EOK;
}

static rt_ubase_t dma_alloc_pages(rt_size_t npages)
{
    rt_base_t off = rt_bitmap_scan(&g_dma_manager.bitmap, npages);
    if (off < 0)
    {
        return 0;
    }
    int i;
    for (i = 0; i < npages; i++)
    {
        rt_bitmap_set(&g_dma_manager.bitmap, off + i, 1);
    }
    return (rt_ubase_t) (g_dma_manager.start + off * ARCH_PAGE_SIZE);
}

static void dma_free_pages(rt_ubase_t addr, rt_size_t npages)
{
    rt_ubase_t start_idx = (addr - g_dma_manager.start) / ARCH_PAGE_SIZE;
    int i;
    for (i = 0; i < npages; i++)
    {
        rt_bitmap_set(&g_dma_manager.bitmap, start_idx + i, 0);
    }
}

rt_err_t rt_hw_dma_alloc(rt_hw_dma_t *dma)
{
    if (!dma->size)
    {
        return RT_EINVAL;
    }
    int npages = (dma->size + (ARCH_PAGE_SIZE - 1)) / ARCH_PAGE_SIZE;
    dma->paddr = dma_alloc_pages(npages);
    if(dma->paddr == 0)
    {
        return RT_ENOMEM;
    }

    dma->vaddr =  rt_hw_phy2vir(dma->paddr);
    rt_memset((void *)dma->vaddr, 0, npages * ARCH_PAGE_SIZE);
    return RT_EOK;
}

rt_err_t rt_hw_dma_free(rt_hw_dma_t *dma)
{
    if (!dma->size || !dma->paddr || !dma->vaddr)
        return RT_EINVAL;

    dma_free_pages(dma->paddr, (dma->size + (ARCH_PAGE_SIZE - 1)) / ARCH_PAGE_SIZE);
    dma->paddr = dma->vaddr = 0;
    return RT_EOK;
}
