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

#ifndef __C_APP_PACKET_PARSER_H__
#define __C_APP_PACKET_PARSER_H__

#include "simple_reader.h"

class parser
{
public:
    class info
    {
    public:
        bool new_frame;
        bool config;
        info()
        {
            new_frame = false;
            config = false;
        }
    };
    virtual void reset(){}
    virtual bool parse(simple_reader::packet_info packet, info& inf)=0;
    virtual ~parser(){}
};


class mpeg4_parser: public parser
{
public:
    mpeg4_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        uint8_t buf[4];
        packet.write_to_buffer(buf,4);
        //printf(" %x\n",buf[3]);
        if(buf[3]==0xb6)
        {
            inf.new_frame = true;
        }
        return true;
    }
};

class mpeg2_parser: public parser
{
public:
    mpeg2_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        uint8_t buf[1];
        packet.write_to_buffer(buf,1);
        if(buf[0]==0x0)
        {
            inf.new_frame = true;
        }
        return true;
    }
};
class h263_parser: public parser
{

public:
    h263_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        (void)packet;
        inf.new_frame=true;
        return true;
    }
};
class jpeg_parser: public parser
{

public:
    jpeg_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        (void)packet;
        inf.new_frame=true;
        return true;
    }
};
class rv_parser: public parser
{
public:
    rv_parser()
    {
    }
    bool parse(simple_reader::packet_info packet, info& inf)
    {
        (void)packet;
        inf.new_frame=true;//hmm
        return true;
    }
};

class vc1_advanced_parser: public parser
{
public:
    vc1_advanced_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        uint8_t buf[1];
        packet.write_to_buffer(buf,1);
        if(buf[0]==0x0D || buf[0]==0x0C)
        {
            inf.new_frame = true;
        }
        return true;
    }
};

class hevc_parser: public parser
{
public:
    hevc_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        uint8_t buf[32];
        uint32_t r = packet.write_to_buffer(buf,32);
        bitreader b(buf,r);

        int forbidden_zero_bit = b.read_bits(1);
        int nal_unit_type = b.read_bits(6);
        int nuh_layer_id = b.read_bits(6);
        int nuh_temporal_id = b.read_bits(3);

        (void)forbidden_zero_bit;
        (void)nuh_layer_id;
        (void)nuh_temporal_id;

        //printf("%d\n",nal_unit_type);
        switch(nal_unit_type)
        {
            case HEVC_NAL_TRAIL_N:
            case HEVC_NAL_TRAIL_R:
            case HEVC_NAL_TSA_N:
            case HEVC_NAL_TLA_R:
            case HEVC_NAL_STSA_N:
            case HEVC_NAL_STSA_R:
            case HEVC_NAL_RADL_N:
            case HEVC_NAL_RADL_R:
            case HEVC_NAL_RASL_N:
            case HEVC_NAL_RASL_R:
            case HEVC_NAL_BLA_W_LP:
            case HEVC_NAL_BLA_W_RADL:
            case HEVC_NAL_BLA_N_LP:
            case HEVC_NAL_IDR_W_RADL:
            case HEVC_NAL_IDR_N_LP:
            case HEVC_NAL_CRA:
            {
                //printf("Slice\n");
                uint32_t first_slice_segment_in_pic = b.read_bits(1);
                if(first_slice_segment_in_pic)
                {
                    inf.new_frame = true;
                }
                break;
            }
            case HEVC_NAL_VPS:
                //printf("VPS\n");
                inf.config = true;
                break;
            case HEVC_NAL_SPS:
                //printf("SPS\n");
                inf.config = true;
                break;
            case HEVC_NAL_PPS:
                //printf("PPS\n");
                inf.config = true;
                break;
            default:
                //printf("NAL %d\n",nal_unit_type);
                break;
        }
        return true;
    }
    enum hevc_nal_unit_type
    {
        HEVC_NAL_TRAIL_N = 0,  // 0
        HEVC_NAL_TRAIL_R,      // 1
        HEVC_NAL_TSA_N,        // 2
        HEVC_NAL_TLA_R,        // 3
        HEVC_NAL_STSA_N,       // 4
        HEVC_NAL_STSA_R,       // 5
        HEVC_NAL_RADL_N,       // 6
        HEVC_NAL_RADL_R,       // 7
        HEVC_NAL_RASL_N,       // 8
        HEVC_NAL_RASL_R,       // 9

        HEVC_NAL_RSV_VCL_N10,
        HEVC_NAL_RSV_VCL_R11,
        HEVC_NAL_RSV_VCL_N12,
        HEVC_NAL_RSV_VCL_R13,
        HEVC_NAL_RSV_VCL_N14,
        HEVC_NAL_RSV_VCL_R15,

        HEVC_NAL_BLA_W_LP = 16,// 16
        HEVC_NAL_BLA_W_RADL,   // 17
        HEVC_NAL_BLA_N_LP,     // 18
        HEVC_NAL_IDR_W_RADL,   // 19
        HEVC_NAL_IDR_N_LP,     // 20
        HEVC_NAL_CRA,          // 21

        HEVC_NAL_RSV_IRAP_VCL22,
        HEVC_NAL_RSV_IRAP_VCL23,
        HEVC_NAL_RSV_VCL24,
        HEVC_NAL_RSV_VCL25,
        HEVC_NAL_RSV_VCL26,
        HEVC_NAL_RSV_VCL27,
        HEVC_NAL_RSV_VCL28,
        HEVC_NAL_RSV_VCL29,
        HEVC_NAL_RSV_VCL30,
        HEVC_NAL_RSV_VCL31,

        HEVC_NAL_VPS = 32,     // 32
        HEVC_NAL_SPS,          // 33
        HEVC_NAL_PPS,          // 34
        HEVC_NAL_AUD,          // 35
        HEVC_NAL_EOS,          // 36
        HEVC_NAL_EOB,          // 37
        HEVC_NAL_FILLER_DATA,  // 38
        HEVC_NAL_PREFIX_SEI,   // 39
        HEVC_NAL_SUFFIX_SEI,   // 40
        HEVC_NAL_RSV_NVCL41,
        HEVC_NAL_RSV_NVCL42,
        HEVC_NAL_RSV_NVCL43,
        HEVC_NAL_RSV_NVCL44,
        HEVC_NAL_RSV_NVCL45,
        HEVC_NAL_RSV_NVCL46,
        HEVC_NAL_RSV_NVCL47,
        HEVC_NAL_INVALID = 64,
    };
};


class avs2_parser: public parser
{
public:
    avs2_parser()
    {
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        uint8_t buf[32];
        uint32_t r = packet.write_to_buffer(buf,32);
        bitreader b(buf,r);

        int nal_unit_type = b.read_bits(8);

        //printf("[avs2_parser] %x\n",nal_unit_type);
        if ((nal_unit_type >= AVS2_NAL_SLICE) && (nal_unit_type < AVS2_NAL_SEQUENCE_START))
        {
            //printf("AVS2-Slice\n");
            if(nal_unit_type == 0)
            {
                inf.new_frame = true;
            }
        }
        else
        {
            switch(nal_unit_type)
            {
                case AVS2_NAL_SEQUENCE_START:
                    //printf("AVS2-SPS\n");
                    inf.config = true;
                    break;
                case AVS2_NAL_I_PICTURE:
                    //printf("AVS2-I-Pic\n");
                    inf.config = true;
                    break;
                case AVS2_NAL_PB_PICTURE:
                    //printf("AVS2-PB-Pic\n");
                    inf.config = true;
                    break;
                case AVS2_NAL_SEQUENCE_END:
                case AVS2_NAL_VIDEO_EDIT:
                    break;
                default:
                    //printf("AVS2-NAL %x\n", nal_unit_type);
                    break;
            }
        }
        return true;
    }
    enum avs2_nal_unit_type
    {
        AVS2_NAL_SLICE = 0,
        AVS2_NAL_SEQUENCE_START = 0xb0,
        AVS2_NAL_SEQUENCE_END = 0xb1,
        AVS2_NAL_USER_DATA = 0xb2,
        AVS2_NAL_I_PICTURE = 0xb3,
        AVS2_NAL_RESERVED0 = 0xb4,
        AVS2_NAL_EXT = 0xb5,
        AVS2_NAL_PB_PICTURE = 0xb6,
        AVS2_NAL_VIDEO_EDIT = 0xb7,
        AVS2_NAL_RESERVED1 = 0xb8,
        AVS2_NAL_SYSTEM = 0xb9,
        AVS2_NAL_INVALID = 64,
    };
};

class h264_parser: public parser
{
    struct sps_data
    {
        int log2_max_frame_num;
        bool frame_mbs_only_flag;
        int pic_order_cnt_type;
        int log2_max_pic_order_cnt_lsb;
        bool delta_pic_order_always_zero_flag;
        sps_data()
        {
            log2_max_frame_num = 0;
            frame_mbs_only_flag = false;
            pic_order_cnt_type = 0;
            log2_max_pic_order_cnt_lsb = 0;
            delta_pic_order_always_zero_flag = false;
        }
    };
    struct pps_data
    {
        int sps_id;
        bool pic_order_present_flag;
        pps_data()
        {
            sps_id = 0;
            pic_order_present_flag = false;
        }
    };

    sps_data sps[32];
    pps_data pps[256];
    int last_frame_num;
    int last_field_mode;
    int last_idr_pic_flag;
    int last_idr_pic_id;
    int last_nal_ref_idc;
    int last_pps_id;
    int last_pic_order_cnt_0;
    int last_pic_order_cnt_1;
public:

    h264_parser()
    {
        last_frame_num = -1;
        last_field_mode = -1;
        last_idr_pic_flag = -1;
        last_idr_pic_id = -1;
        last_nal_ref_idc = -1;
        last_pps_id = -1;
        last_pic_order_cnt_0 = -1;
        last_pic_order_cnt_1 = -1;
    }

    bool parse(simple_reader::packet_info packet, info& inf)
    {
        const int buf_size = 256;
        uint8_t buf[buf_size];
        uint32_t bytes = packet.write_to_buffer(buf,32);
        bitreader b(buf,bytes);

        b.read_bits(1); // forbidden_zero_bit
        int nal_ref_idc=b.read_bits(2);
        int nal_unit_type=b.read_bits(5);

        //printf("nal_unit_type %d\n",nal_unit_type);
        switch(nal_unit_type)
        {
            case 1:
            case 5: // slice
            {
                ue(b); // first_mb_in_slice
                ue(b); // slice_type
                int pps_id = ue(b);
                pps_data& p = pps[pps_id];
                sps_data& s = sps[p.sps_id];
                int log2_max_frame_num = s.log2_max_frame_num;

                int frame_num = b.read_bits(log2_max_frame_num);

                int field_mode = 0;
                if (!s.frame_mbs_only_flag)
                {
                    if (b.read_bits(1))//field_pic_flag
                    {
                        field_mode = 1;
                        field_mode += b.read_bits(1);// bottom_field_flag
                    }
                }
                int idr_pic_flag = 0;
                int idr_pic_id = 0;
                if ( nal_unit_type == 5 )
                {
                    idr_pic_flag = 1;
                    idr_pic_id   = ue(b);
                }
                int pic_order_cnt_type = s.pic_order_cnt_type;
                int pic_order_cnt_0 = 0;
                int pic_order_cnt_1 = 0;
                if ( pic_order_cnt_type ==  0 )
                {
                    pic_order_cnt_0 = b.read_bits(s.log2_max_pic_order_cnt_lsb);
                    if( p.pic_order_present_flag && field_mode==0 )
                    {
                        pic_order_cnt_1 = se(b);
                    }
                }
                else if( pic_order_cnt_type == 1 && !s.delta_pic_order_always_zero_flag )
                {
                    pic_order_cnt_0 = se(b);
                    if( p.pic_order_present_flag && field_mode==0 )
                    {
                        pic_order_cnt_1 = se(b);
                    }
                }
                if (pps_id       != last_pps_id       ||
                    frame_num    != last_frame_num    ||
                    field_mode   != last_field_mode   ||
                    idr_pic_flag != last_idr_pic_flag ||
                    idr_pic_id   != last_idr_pic_id   ||
                    ((nal_ref_idc!=0) && (last_nal_ref_idc==0)) ||
                    ((nal_ref_idc==0) && (last_nal_ref_idc!=0)) ||
                    pic_order_cnt_0 != last_pic_order_cnt_0 ||
                    pic_order_cnt_1 != last_pic_order_cnt_1 )
                {
                     inf.new_frame = true;
                     //printf("new frame\n");
                }

                last_field_mode = field_mode;
                last_idr_pic_flag = idr_pic_flag;
                last_idr_pic_id = idr_pic_id;
                last_nal_ref_idc = nal_ref_idc;
                last_pps_id= pps_id;
                last_frame_num = frame_num;
                last_pic_order_cnt_0 = pic_order_cnt_0;
                last_pic_order_cnt_1 = pic_order_cnt_1;

                //printf("first_mb_in_slice %d, slice_type %d, pps_id %d, frame_num %d pic_order_cnt_type %d pic_order_cnt_0 %d pic_order_cnt_1 %d log2_max_pic_order_cnt_lsb %d \n",first_mb_in_slice,slice_type ,pps_id,frame_num, pic_order_cnt_type, pic_order_cnt_0,pic_order_cnt_1,s.log2_max_pic_order_cnt_lsb);
                break;
            }
            case 9:
            {
                /*access unit delimiter*/
                break;
            }
            case 6:
            {
                /*SEI NAL unit*/
                break;
            }
            case 8: //PPS
            {
                inf.config = true;
                int pic_parameter_set_id = ue(b);
                int seq_parameter_set_id = ue(b);
                b.read_bits(1); // entropy_coding_mode_flag

                /* bottom_field_pic_order_in_frame_present_flag */
                bool pic_order_present_flag = b.read_bits(1);

                pps[pic_parameter_set_id].sps_id = seq_parameter_set_id;
                pps[pic_parameter_set_id].pic_order_present_flag = pic_order_present_flag;
               // printf("pic_parameter_set_id %d, pic_parameter_set_id %d\n",pic_parameter_set_id,pic_parameter_set_id);
                break;
            }
            case 7: //SPS
            {
                //printf("sps\n");
                inf.config = true;

                bytes += packet.write_to_buffer(buf+32,buf_size-32);
                b = bitreader(buf+1,bytes-1);

                int profile_idc=b.read_bits(8);
                b.read_bits(8); // constra_and_res
                b.read_bits(8); // level_idc
                int sps_id = ue(b);

                // printf("profile_idc %d, cc %d, level %d, id %d\n",profile_idc,constra_and_res,level_idc,sps_id);

                if( profile_idc == 100 || profile_idc == 110 ||
                    profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
                    profile_idc == 83 || profile_idc == 86 || profile_idc == 118 || profile_idc == 128 )
                {
                    int chroma_format_idc=ue(b);

                    if( chroma_format_idc == 3 )
                    {
                        b.read_bits(1);//separate_colour_plane_flag
                    }
                    ue(b); // bit_depth_luma_minus8
                    ue(b); // bit_depth_chroma_minus8
                    b.read_bits(1); // qpprime_y_zero_transform_bypass_flag
                    int seq_scaling_matrix_present_flag = b.read_bits(1);

                    if( seq_scaling_matrix_present_flag )
                    {
                        for(int  i = 0; i < ( ( chroma_format_idc != 3 ) ? 8 : 12 ) && !b.eos; i++ )
                        {
                            bool seq_scaling_list_present_flag = b.read_bits(1);
                            if( seq_scaling_list_present_flag)
                            {
                                if( i < 6 )
                                {
                                    scaling_list(b,16);
                                }
                                else
                                {
                                    scaling_list(b,64);
                                }
                            }
                        }
                    }
                }

                int log2_max_frame_num = ue(b) + 4;
                //printf("log2_max_frame_num %d\n",log2_max_frame_num);

                sps_data* s=&sps[sps_id];
                s->log2_max_frame_num = log2_max_frame_num;

                int pic_order_cnt_type = ue(b);

                if( pic_order_cnt_type == 0 )
                {
                    int log2_max_pic_order_cnt_lsb_minus4 = ue(b);
                    s->log2_max_pic_order_cnt_lsb = log2_max_pic_order_cnt_lsb_minus4+4;
                }
                else if( pic_order_cnt_type == 1 )
                {
                    bool delta_pic_order_always_zero_flag = b.read_bits(1);
                    s->delta_pic_order_always_zero_flag=delta_pic_order_always_zero_flag;
                    se(b); // offset_for_non_ref_pic
                    se(b); // offset_for_top_to_bottom_field
                    int num_ref_frames_in_pic_order_cnt_cycle = ue(b);
                    for(int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle && !b.eos; i++ )
                    {
                         se(b); // offset_for_ref_frame[i]
                    }
                }
                ue(b); // ax_num_ref_frames
                b.read_bits(1); // gaps_in_frame_num_value_allowed_flag

                ue(b); // pic_width_in_mbs_minus1
                ue(b); // pic_height_in_map_units_minus1
                s->frame_mbs_only_flag = b.read_bits(1);

                s->pic_order_cnt_type=pic_order_cnt_type;
                //printf("s->frame_mbs_only_flag %d\n",s->frame_mbs_only_flag);

                break;
            }
            default:
            {
                break;
            }
        }
        return true;
    }

private:

    uint32_t ue(bitreader &b)
    {
        return b.read_exp_golomb();
    }

    int32_t se(bitreader &b)
    {
        uint32_t c = ue(b);
        if((c&1)==0)
        {
            return -(c>>1);
        }
        else
        {
            return 1+(c>>1);
        }
    }

    void scaling_list(bitreader &b, int sizeOfScalingList)
    {

        int lastScale = 8;
        int nextScale = 8;
        for(int j = 0; j < sizeOfScalingList && !b.eos; j++ )
        {
            if( nextScale != 0 )
            {
                int delta_scale = se(b);
                nextScale = ( lastScale + delta_scale + 256 ) % 256;
            }
            int s = ( nextScale == 0 ) ? lastScale : nextScale;
            lastScale = s;
        }
    }

};

#endif

