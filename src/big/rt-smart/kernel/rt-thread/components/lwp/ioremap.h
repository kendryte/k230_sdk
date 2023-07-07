/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-06     Jesven       first version
 * 2023-03-15     RT-Thread    Support rt_ioremap_wt
 */
#ifndef  __IOREMAP_H__
#define  __IOREMAP_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IOREMAP family
 * `rt_ioremap` default to map physical memory in MMIO region as DEVICE memory
 * to kernel space. And there are 3 variants currently supported.
 *
 * name               | attribution
 * ------------------ | -----------
 * rt_ioremap_nocache | Device (MMU_MAP_K_DEVICE)
 * rt_ioremap_cache   | Normal memory (MMU_MAP_K_RWCB)
 * rt_ioremap_wt      | Normal memory but guarantee that
 *                    | Each write access should go to system memory directly
 *                    | Currently as non-cacheable
 */

void *rt_ioremap(void *paddr, size_t size);
void *rt_ioremap_nocache(void *paddr, size_t size);
void *rt_ioremap_cached(void *paddr, size_t size);
void *rt_ioremap_wt(void *paddr, size_t size);
void rt_iounmap(volatile void *addr);

#ifdef __cplusplus
}
#endif

#endif  /*__LWP_IOREMAP_H__*/
