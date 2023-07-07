/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-04     JasonHu      first version
 */

#ifndef __RT_BITMAP_H__
#define __RT_BITMAP_H__

#include <rtdef.h>

#define RT_BITMAP_MASK 1UL

/**
 * rt_bitmap structure
 */
struct rt_bitmap
{
   rt_size_t byte_length;   /**< rt_bitmap size in byte. */
   rt_uint8_t *bits;        /**< rt_bitmap bits base addr. */
};
typedef struct rt_bitmap rt_bitmap_t;

void rt_bitmap_init(rt_bitmap_t *bitmap, uint8_t *bits, rt_size_t byte_len);
rt_base_t rt_bitmap_scan(rt_bitmap_t *bitmap, rt_size_t count);
void rt_bitmap_set(rt_bitmap_t *bitmap, rt_ubase_t index, rt_bool_t value);
rt_bool_t rt_bitmap_change(rt_bitmap_t *bitmap, rt_ubase_t index);
rt_bool_t rt_bitmap_test_and_change(rt_bitmap_t *bitmap, rt_ubase_t index);

#endif  /* __RT_BITMAP_H__ */
