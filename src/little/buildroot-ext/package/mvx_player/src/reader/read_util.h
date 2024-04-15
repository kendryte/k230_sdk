/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __C_APP_READ_UTIL_H__
#define __C_APP_READ_UTIL_H__
#include <cassert>
#include <inttypes.h>
#include <string.h>
#include <memory.h>

// Endian conversion helper functions
static inline uint16_t bswap_16(uint16_t val)
{
    return (val >> 8) | (val << 8);
}

static inline uint32_t bswap_32(uint32_t val)
{
    return (val >> 24) | ((val & 0xFF0000) >> 8) | ((val & 0xFF00) << 8) | (val << 24);
}

static inline uint64_t bswap_64(uint64_t val)
{
    return (uint64_t)bswap_32((uint32_t)val) << 32 | bswap_32((uint32_t)(val >> 32));
}

class bitreader
{
    uint8_t* data;
    uint32_t size;
    uint64_t t;
    uint32_t e;
    uint32_t off;
public:
    bool eos;
    bitreader(uint8_t* _data,uint32_t _size)
    {
        this->data=_data;
        this->size=_size;
        e=0;
        t=0;
        off=0;
        eos=false;
    }

    uint32_t read_bits(uint32_t n)
    {
        uint32_t val = peek_bits(n);
        t <<=n;
        e-=n;
        return val;
    }
    uint32_t peek_bits(uint32_t n)
    {
        while(e<n)
        {
            if(off>=size)
            {
                eos = true;
                return 0;
            }
            uint64_t a=data[off++];
            t |= a<<(56-e);
            e+=8;
        }
        uint32_t val = (uint64_t)(t>>(64-n));
        return val;
    }
    uint32_t read_exp_golomb()
    {
        if(eos)
        {
            return 0;
        }
        int leadingZeroBits = -1;
        for(bool b = 0; !b && !eos; leadingZeroBits++ )
        {
            b = read_bits( 1 ) ;
        }
        return (1<<leadingZeroBits) -1 + read_bits(leadingZeroBits) ;
    }
};

class buf
{
public:
    uint32_t size;
    uint32_t offset;
    uint8_t* data;
    buf()
    {
        offset = 0;
    }
    buf(uint8_t *dat,uint32_t siz)
    {
        data = dat;
        size = siz;
        offset = 0;
    }
};

template<int SIZE>
class mem_buf_writer
{

public:
    uint32_t offset;
    uint8_t data[SIZE];
    uint32_t size;
    mem_buf_writer()
    {
        size = 0;
        offset = 0;
    }

    uint32_t write(uint8_t* buf,uint32_t alloc_size)
    {
        uint32_t size_left = size-offset;
        if(size_left == 0)
        {
            return 0;
        }
        uint32_t s=alloc_size < size_left ? alloc_size : size_left;
        memcpy(buf, data+offset ,s);
        offset+=s;
        return s;
    }
    uint32_t write(buf& buf)
    {
        uint32_t alloc_size = buf.size-buf.offset;
        uint32_t size_left = size-offset;
        if(size_left == 0)
        {
            return 0;
        }
        uint32_t s=alloc_size < size_left ? alloc_size : size_left;
        memcpy(buf.data+buf.offset, data+offset ,s);
        offset+=s;
        buf.offset+=s;
        return s;
    }
};

class length_field_writer : public mem_buf_writer<4>
{
public:

    length_field_writer(uint32_t length_field_bytes, uint32_t nal_size)
    {
        init(length_field_bytes,nal_size);
    }
    length_field_writer()
    {

    }
    void init(uint32_t length_field_bytes, uint32_t nal_size)
    {
        assert(length_field_bytes > 0 && length_field_bytes <= 4);
        uint8_t* buffer = data;
        if (length_field_bytes == 1)
        {
            *buffer++ = nal_size & 0xff;
            assert( (nal_size >> 8) == 0 );
        }
        else if (length_field_bytes == 2)
        {
            *buffer++ = nal_size & 0xff;
            *buffer++ = (nal_size >> 8) & 0xff;
            assert( (nal_size >> 16) == 0 );
        }
        else if (length_field_bytes == 4)
        {
            *buffer++ = nal_size & 0xff;
            *buffer++ = (nal_size >> 8) & 0xff;
            *buffer++ = (nal_size >> 16) & 0xff;
            *buffer++ = (nal_size >> 24) & 0xff;
        }
        size = length_field_bytes;
        offset = 0;
    }

};

#endif /*__C_APP_READ_UTIL_H__*/

