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

#ifndef __C_APP_READER_H__
#define __C_APP_READER_H__

#include <cassert>
#include <string>
#include <memory.h>
#include <string.h>
#include "read_util.h"

class reader
{
public:
    enum result
    {
        RR_OK,              // Read operation OK (streamed data)
        RR_EOS,             // Read to end of file (returned data still valid)
        RR_EOP,             // Read to end of packet
        RR_EOP_FRAME,       // Read to end of frame
        RR_EOP_CODEC_CONFIG,// Read to end of codec config
        RR_RESIZE,          //
        RR_ERROR,
    };

    enum codec
    {
        RCODEC_UNKNOWN,
        RCODEC_VC1,
        RCODEC_VC1_SIMPLE_PROFILE,
        RCODEC_VC1_MAIN_PROFILE,
        RCODEC_VC1_ADVANCED_PROFILE,
        RCODEC_REAL_VIDEO,
        RCODEC_REAL_MEDIA_FORMAT,
        RCODEC_MPEG2,
        RCODEC_MPEG4,
        RCODEC_H263,
        RCODEC_H264,
        RCODEC_HEVC,
        RCODEC_VP8,
        RCODEC_VP9,
        RCODEC_JPEG,
        RCODEC_MBINFO,
        RCODEC_AVS,
        RCODEC_AVS2,
    };
    enum reader_mode
    {
        RMODE_FLAGS=1,
        RMODE_PACKETED=2,
        RMODE_DELIMITED=4
    };

    // Metadata published or consumed by the reader
    enum reader_metadata
    {
        RMETA_CODEC = 0,         // Detected codec type index
        RMETA_WIDTH,             // Video frame width
        RMETA_HEIGHT,            // Video frame height
        RMETA_VIDEO_TRACK,       // Video track index
        RMETA_AUDIO_TRACK,       // Audio track index
        RMETA_YUV_FORMAT,        // YUV color format
        RMETA_DELIMITER_LENGTH,
        RMETA_LAST,
    };

protected:
    union meta_value
    {
        bool    b;
        int32_t i;
        float   f;
    };

    std::istream *file;
    meta_value metadata[RMETA_LAST];
    int mode;
    uint64_t file_length;
public:

    reader(std::istream * file): file(file)
    {
        mode = 0;
    }

    virtual bool seek_frames(uint32_t track, uint32_t nbr_frames) = 0;

    virtual result read(uint32_t track, uint8_t* buffer, uint32_t alloc_len, uint32_t *filled_len ) = 0;

    virtual void set_mode(int new_mode)     { this->mode=new_mode;}
    virtual int  get_mode()                 { return mode;}
    void setFileLength(uint64_t length){file_length = length;}
    void set_metadata(int name,int value)   { metadata[name].i=value;}
    int  get_metadata_int(int name)         { return metadata[name].i; }

    virtual ~reader() {}
};

#endif
