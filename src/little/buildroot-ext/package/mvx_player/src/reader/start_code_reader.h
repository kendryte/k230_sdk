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

#ifndef __C_APP_START_CODE_H__
#define __C_APP_START_CODE_H__

#include <string>
#include "parser.h"
#include "read_util.h"
#include "simple_reader.h"

class start_code_reader : public simple_reader
{
protected:
    int last_start_code_len;
    uint64_t prev_offset;
    bool include_start_codes_in_packet;
    bool allow_start_codes_len_4;
    parser *dec;
    uint32_t start_code;
    uint32_t start_code_mask;
    packet_info next_packet;
    bool seen_new_frame;
    bool has_reset;
    codec codec_id;
public:

    start_code_reader(uint32_t name, std::istream*file) : simple_reader(file), prev_offset(0),dec(NULL)
    {
        start_code = 0x00000001;
        start_code_mask = 0x00ffffff;
        allow_start_codes_len_4 = false;
        include_start_codes_in_packet = false;
        seen_new_frame = false;
        raw_stream = true;
        simple_reader::reset(0);
        switch(name)
        {
            /*case RCODEC_VC1:
                set_metadata(RMETA_CODEC, RCODEC_VC1_ADVANCED_PROFILE);
                dec = new vc1_advanced_parser();
                break;
            case RCODEC_MPEG2:
                dec = new mpeg2_parser();
                break;
            case RCODEC_MPEG4:
                include_start_codes_in_packet = true;
                dec = new mpeg4_parser();
                break;*/
            case V4L2_PIX_FMT_HEVC:
                allow_start_codes_len_4 = true;
                dec = new hevc_parser();
                break;
            case V4L2_PIX_FMT_H264:
                allow_start_codes_len_4 = true;
                dec = new h264_parser();
                break;
            /*case RCODEC_REAL_VIDEO:
                dec = new rv_parser();
                break;
            case RCODEC_AVS2:
                allow_start_codes_len_4 = true;
                dec = new avs2_parser();
                break;*/
            default:
                break;
        }
    }
    virtual ~start_code_reader()
    {
        if(dec!=NULL)
        {
            delete dec;
        }
    }
    void reset()
    {
        has_reset = true;
        prev_offset = 0;
        seen_new_frame = false;
        file->seekg(0);
    }

    virtual bool read_packet(packet_info& packet)
    {
        if(has_reset)
        {
            read_next_packet(next_packet);
            if(dec!=NULL)
            {
                parser::info info;
                dec->parse(next_packet,info);
                next_packet.codec_config = info.config;
                seen_new_frame |= info.new_frame;
            }
            has_reset = false;
        }
        //printf("read_packet cc %d  pos %d size %d\n",packet.codec_config,packet.file_pos,packet.size );
        packet=next_packet;
        // read ahead to find end of frame
        if(!next_packet.eos)
        {
            read_next_packet(next_packet);

            if(dec!=NULL)
            {
                parser::info info;
                dec->parse(next_packet,info);
                //printf("config %d seen_new_frame %d, new frame %d\n",info.config,seen_new_frame,info.new_frame);
                if(seen_new_frame && (info.new_frame || info.config))
                {
                    packet.eof = true;
                    seen_new_frame = false;
                }
                seen_new_frame |= info.new_frame;
                next_packet.codec_config = info.config;
            }
        }
        else
        {
            packet.eof=true;
        }
        packet.file = next_packet.file;
        return true;
    }
private:

    bool read_next_packet(packet_info& packet)
    {

        if(prev_offset==0)
        {
            find_first_start_code();
        }

        uint64_t next, prev = prev_offset;
        uint32_t prefix;

        file->seekg(prev);
        //printf("prev %x\n",prev);
        next = seek_prefix(start_code_mask, start_code, prefix);

        int start_code_len = get_start_code_len(prefix);

        packet.prefix_size = last_start_code_len;
        uint32_t size  = (uint32_t)(next - prev);
        if(include_start_codes_in_packet)
        {
            prev-=packet.prefix_size;
            size+=packet.prefix_size;
            packet.prefix_size=0;
        }

        packet.file_pos = prev;
        packet.eos=file->peek() == EOF;
        packet.size = size - (!packet.eos?start_code_len:0);
        packet.file = file;
        prev_offset = next;
        last_start_code_len = start_code_len;
        //file->seekg(packet.file_pos);
        return true;

    }


    void find_first_start_code()
    {
        file->seekg(0);
        uint32_t prefix;
        prev_offset = seek_prefix(start_code_mask, start_code, prefix);
        last_start_code_len = get_start_code_len(prefix);
    }

    virtual int get_start_code_len(uint32_t prefix)
    {
        if(allow_start_codes_len_4 && (prefix&0xff000000)==0)
        {
            return 4;
        }
        else
        {
            return 3;
        }
    }

    bool seek_prefix_in_buffer(char* buffer, uint32_t buffer_size, uint32_t mask, uint32_t match,uint64_t &found_pos, uint32_t &check )
    {
        for(uint32_t i=0;i<buffer_size;i++)
        {
            uint8_t temp = buffer[i];

            check = (check << 8) | temp;

             //printf("%d: x c %x\n",i,temp, check);
            if ((check & mask) == match)
            {
                found_pos = i+1;
                return true;
            }
        }
        return false;
    }
    uint64_t seek_prefix(uint32_t mask, uint32_t match, uint32_t& outprefix)
    {
        #define SEEK_PREFIX_BUF_SIZE 4096
        static char buf[SEEK_PREFIX_BUF_SIZE];
        uint32_t check = ~0u;

        //printf("pos %d \n",position());
        while(true)
        {
            file->read(static_cast<char*>(buf),SEEK_PREFIX_BUF_SIZE);
            uint32_t bytes = file->gcount();

            if( bytes == 0 )
            {
                break;
            }
            uint64_t pos;
            if(seek_prefix_in_buffer(buf,bytes,mask,match,pos,check))
            {
                if(bytes < SEEK_PREFIX_BUF_SIZE) {
                    file->clear();
                }
                file->seekg( pos-bytes, std::ios::cur );
                break;
            } else if(bytes < SEEK_PREFIX_BUF_SIZE) {
                file->clear();
                break;
            }
        }
        outprefix = check;
        return file->tellg();
    }
};

#endif
