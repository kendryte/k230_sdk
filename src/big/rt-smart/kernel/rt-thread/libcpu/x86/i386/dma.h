/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-04     JasonHu      First Version
 */

#ifndef __HW_DMA_H__
#define __HW_DMA_H__

#include <rtdef.h>

struct rt_hw_dma
{
    rt_ubase_t paddr;
    rt_ubase_t vaddr;
    rt_size_t size;
    rt_ubase_t alignment;   /* addr align */
};
typedef struct rt_hw_dma rt_hw_dma_t;

rt_err_t rt_hw_dma_alloc(rt_hw_dma_t *dma);
rt_err_t rt_hw_dma_free(rt_hw_dma_t *dma);
rt_err_t rt_hw_dma_init(rt_ubase_t start, rt_ubase_t end);

#endif  /* __HW_DMA_H__ */
