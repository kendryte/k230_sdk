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

#ifndef __C_APP_SIMPLE_READER_H__
#define __C_APP_SIMPLE_READER_H__

#include "reader.h"

class simple_reader : public reader
{
protected:
    length_field_writer lf;

    bool resize_buffer;
    bool raw_stream;
public:
    class packet_info
    {
    public:
        enum packet_write_flags
        {
            WRITE_FLAGS_NONE=0,
            WRITE_FLAGS_ADD_PREFIX=1
        };

        std::istream* file;
        uint32_t tag;
        uint64_t file_pos;
        uint32_t size;
        uint8_t prefix_size;
        bool codec_config;
        bool eos;
        bool eof;
        bool valid()
        {
            return file!=NULL;
        }
        bool is_consumed()
        {
            return size==0;
        }
        uint32_t get_remaining_size(packet_write_flags flags=WRITE_FLAGS_NONE)
        {
            bool incl_prefix = flags & WRITE_FLAGS_ADD_PREFIX;
            return (incl_prefix ? prefix_size:0) + size;
        }
        uint32_t write_to_buffer(uint8_t* buffer,uint32_t alloc_size, packet_write_flags flags=WRITE_FLAGS_NONE)
        {
            uint32_t bytes_read=0;
            bool incl_prefix = flags & packet_info::WRITE_FLAGS_ADD_PREFIX;
            if(incl_prefix)
            {
                uint32_t s=alloc_size < prefix_size ? alloc_size : prefix_size;

                file->seekg(file_pos-prefix_size);
                file->read((char*)buffer,s);
                prefix_size-=s;
                alloc_size-=s;
                buffer+=s;
                bytes_read+=s;
            }

            if(prefix_size==0 || !incl_prefix)
            {
                file->seekg(file_pos);
                uint32_t bytes_to_read = alloc_size < size ? alloc_size : size;
                file->read((char*)buffer, bytes_to_read);
                bytes_read += file->gcount();
                file_pos +=bytes_to_read;
                size -=bytes_to_read;
                //printf("r %d\n",read);
            }

            return bytes_read;
        }
        ~packet_info() {}
        packet_info()
        {
            size = 0;
            file_pos = 0;
            prefix_size=0;
            file = NULL;
            eof = false;
            eos = false;
            codec_config=false;
        }
    };

private:
    packet_info current_packet;
    void next_frame()
    {
        if(!current_packet.valid() || !current_packet.eof || current_packet.is_consumed())
        {
            do
            {
                current_packet=packet_info();
                read_packet(current_packet);
            }
            while(!current_packet.eof && !current_packet.eos);
        }
        current_packet.size=0;
    }

public:
    simple_reader(std::istream* file) : reader(file), resize_buffer(false), raw_stream(false)
    {
    }
    virtual bool seek_frames(uint32_t track, uint32_t nbr_frames)
    {
        (void)track;
        for(uint32_t f=0;f<nbr_frames && !current_packet.eos;f++)
        {
            next_frame();
        }
        return !current_packet.eos;
    }

    virtual result read(uint32_t track, uint8_t* buffer, uint32_t alloc_len, uint32_t *filled_len )
    {
        (void)track;
        if(!raw_stream || (mode & RMODE_FLAGS) || (mode & RMODE_DELIMITED))
        {
            if(!current_packet.valid() || current_packet.is_consumed())
            {
                if(current_packet.valid() && current_packet.eos)
                {
                    *filled_len = 0;
                    return RR_EOS;
                }
                current_packet = packet_info();
                read_packet(current_packet);
                if(mode & RMODE_DELIMITED)
                {
                    lf.init(get_metadata_int(RMETA_DELIMITER_LENGTH),current_packet.get_remaining_size());
                }
            }
            *filled_len = 0;
            if(mode & RMODE_DELIMITED)
            {
                uint32_t written = lf.write(buffer, alloc_len);
                *filled_len = written;
                alloc_len-=written;
                buffer+=written;
            }
            if((mode&RMODE_PACKETED) && resize_buffer && alloc_len<current_packet.get_remaining_size())
            {
                *filled_len = current_packet.get_remaining_size();
                return RR_RESIZE;
            }
            *filled_len += current_packet.write_to_buffer(buffer, alloc_len, ((mode&RMODE_PACKETED) || (mode & RMODE_DELIMITED))?packet_info::WRITE_FLAGS_NONE: packet_info::WRITE_FLAGS_ADD_PREFIX);
            if(current_packet.is_consumed())
            {
                if(current_packet.eos)
                {
                    return RR_EOS;
                }
                else if(current_packet.codec_config)
                {
                    return RR_EOP_CODEC_CONFIG;
                }
                else if(current_packet.eof)
                {
                    return RR_EOP_FRAME;
                }
                else
                {
                    return RR_EOP;
                }
            }

            return RR_OK;
        }
        return RR_ERROR;
    }

    virtual bool read_packet(packet_info& p) = 0;

    virtual void reset() = 0;

    virtual ~simple_reader() {}

    virtual void reset(uint32_t track)
    {
        (void)track;
        current_packet = packet_info();
        reset();
    }
};

#endif
