/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-04     JasonHu      First Version
 */

#include <bitmap.h>
#include <rtdebug.h>
#include <string.h>

void rt_bitmap_init(rt_bitmap_t *bitmap, uint8_t *bits, rt_size_t byte_len)
{
    bitmap->bits = bits;
    bitmap->byte_length = byte_len;
    memset(bitmap->bits, 0, bitmap->byte_length);
}

static rt_bool_t rt_bitmap_test(rt_bitmap_t *bitmap, rt_ubase_t index)
{
    rt_ubase_t byte_idx = index / 8;
    rt_ubase_t bit_odd  = index % 8;
    return (bitmap->bits[byte_idx] & (RT_BITMAP_MASK << bit_odd));
}

rt_base_t rt_bitmap_scan(rt_bitmap_t *bitmap, rt_size_t count)
{
    if (!bitmap || !count)
    {
        return -1;
    }

    rt_ubase_t idx_byte = 0;

    while ((0xff == bitmap->bits[idx_byte]) && (idx_byte < bitmap->byte_length))
    {
        idx_byte++;
    }

    if (idx_byte == bitmap->byte_length)    /* out of array range */
    {
        return -1;
    }

    rt_base_t idx_bit = 0;

    while ((rt_uint8_t)(RT_BITMAP_MASK << idx_bit) & bitmap->bits[idx_byte])
    {
        idx_bit++;
    }

    rt_base_t idx_start = idx_byte * 8 + idx_bit;
    if (count == 1)
    {
        return idx_start;
    }

    rt_ubase_t bit_left = (bitmap->byte_length * 8 - idx_start);
    rt_ubase_t next_bit = idx_start + 1;
    rt_ubase_t ret_count = 1;

    idx_start = -1;
    while (bit_left-- > 0)
    {
        if (!(rt_bitmap_test(bitmap, next_bit)))
        {
            ret_count++;
        }
        else
        {
            ret_count = 0;  /* no consecutive bits, reset count */
        }

        if (ret_count == count)
        {
            idx_start = next_bit - ret_count + 1;
            break;
        }
        next_bit++;
    }
    return idx_start;
}

void rt_bitmap_set(rt_bitmap_t *bitmap, rt_ubase_t index, rt_bool_t value)
{
    rt_ubase_t byte_idx = index / 8;
    rt_ubase_t bit_odd  = index % 8;

    if (value) {
        bitmap->bits[byte_idx] |= (RT_BITMAP_MASK << bit_odd);
    } else {
        bitmap->bits[byte_idx] &= ~(RT_BITMAP_MASK << bit_odd);
    }
}

rt_bool_t rt_bitmap_change(rt_bitmap_t *bitmap, rt_ubase_t index)
{
    rt_ubase_t byte_idx = index / 8;
    rt_ubase_t bit_odd  = index % 8;

    bitmap->bits[byte_idx] ^= (RT_BITMAP_MASK << bit_odd);  /* xor */
    return (bitmap->bits[byte_idx] & (RT_BITMAP_MASK << bit_odd));
}

rt_bool_t rt_bitmap_test_and_change(rt_bitmap_t *bitmap, rt_ubase_t index)
{
    rt_ubase_t byte_idx = index / 8;
    rt_ubase_t bit_odd  = index % 8;

    rt_bool_t ret = (rt_bool_t) bitmap->bits[byte_idx] & (RT_BITMAP_MASK << bit_odd);

    bitmap->bits[byte_idx] ^= (RT_BITMAP_MASK << bit_odd); /* xor */
    return ret;
}
