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

#include "mvx_argparse.h"
#include "mvx_player.hpp"
#include <fstream>

using namespace std;
#define CFG_FILE_LINE_SIZE (6144)
static char cfg_file_line_buf[CFG_FILE_LINE_SIZE];

static bool encode_get_huffman_table(FILE* file, uint32_t type,
                           uint8_t *bits, uint8_t* values )
{
    const char *table_name_bits;
    const char *table_name_val;

    if (type == V4L2_OPT_HUFFMAN_TABLE_DC_LUMA)
    {
        table_name_bits = "DHT_luma_dc_bits";
        table_name_val  = "DHT_luma_dc_val";
    }
    else if (type == V4L2_OPT_HUFFMAN_TABLE_DC_CHROMA)
    {
        table_name_bits = "DHT_chroma_dc_bits";
        table_name_val  = "DHT_chroma_dc_val";
    }
    else if (type == V4L2_OPT_HUFFMAN_TABLE_AC_LUMA)
    {
        table_name_bits = "DHT_luma_ac_bits";
        table_name_val  = "DHT_luma_ac_val";
    }
    else
    {
        assert(type == V4L2_OPT_HUFFMAN_TABLE_AC_CHROMA);

        table_name_bits = "DHT_chroma_ac_bits";
        table_name_val  = "DHT_chroma_ac_val";
    }

    bool load_bits = false;
    bool load_val  = false;
    uint32_t temp;

    while(fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file) != NULL)
    {
        if (strstr(cfg_file_line_buf, table_name_bits) != NULL)
        {
            if (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file) != NULL)
            {
                if (strcmp(cfg_file_line_buf, "{\n") == 0)
                {
                    for (int i = 0; i < 16; i++)
                    {
                        if (fscanf(file, "0x%x, ", &temp) != 1)
                        {
                            cerr<<"Failed to load entry from huffman table file"<<endl;
                        }
                        bits[ i ] = (uint8_t)temp;
                    }

                    load_bits = true;
                }
                else
                {
                    //Not a valid table format
                    load_bits = false;
                }
            }
            else
            {
                //No table available
                load_bits = false;
            }
        }
        else if (strstr(cfg_file_line_buf, table_name_val) != NULL)
        {
            int count;
            char *sub_str;

            sub_str = strstr(cfg_file_line_buf, "[");
            sscanf(sub_str, "[%d", &count);

            if (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file) != NULL)
            {
                if (strcmp(cfg_file_line_buf, "{\n") == 0)
                {
                    for (int i = 0; i < count; i++)
                    {
                        if (fscanf(file, "0x%x, ", &temp) != 1)
                        {
                            cerr<<"Failed to load entry from huffman table file"<<endl;
                        }

                        values[ i ] = (uint8_t)temp;
                    }

                    load_val = true;
                }
                else
                {
                    //Not a valid table format
                    load_val = false;
                }
            }
            else
            {
                //No table available
                load_val = false;
            }
        }

        if (load_bits && load_val)
        {
            break;
        }
    }

    return (load_bits && load_val);
}


static void tokenize_values(const std::string &s, const char delim, std::vector<int16_t>&out)
{
    std::string::size_type beg = 0;
    for (std::string::size_type end = 0; (end = s.find(delim, end)) != std::string::npos; ++end)
    {
        out.push_back(atoi(s.substr(beg, end - beg).c_str()));
        beg = end + 1;
    }
    out.push_back(atoi(s.substr(beg).c_str()));
}


bool color_conversion_parse_coef(const char * conv_ceof_str,struct v4l2_mvx_rgb2yuv_color_conv_coef * conv_coef)
{
    std::vector<int16_t> ceof_list;

    tokenize_values(conv_ceof_str, ':', ceof_list);
    if (ceof_list.size() == 15)
    {
        conv_coef->coef[0] = ceof_list[0];
        conv_coef->coef[1] = ceof_list[1];
        conv_coef->coef[2] = ceof_list[2];

        conv_coef->coef[3] = ceof_list[3];
        conv_coef->coef[4] = ceof_list[4];
        conv_coef->coef[5] = ceof_list[5];

        conv_coef->coef[6] = ceof_list[6];
        conv_coef->coef[7] = ceof_list[7];
        conv_coef->coef[8] = ceof_list[8];


        conv_coef->luma_range[0] = ceof_list[9];
        conv_coef->luma_range[1] = ceof_list[10];
        conv_coef->chroma_range[0] = ceof_list[11];
        conv_coef->chroma_range[1] = ceof_list[12];
        conv_coef->rgb_range[0] = ceof_list[13];
        conv_coef->rgb_range[1] = ceof_list[14];
   }
   cout << "color_conversion_parse_coef" << endl;
   for(int i = 0 ; i< 10; i++){
       cout  << conv_coef->coef[i] << " ";
   }
   cout << endl;
   cout << "conv_coef->luma_range" << conv_coef->luma_range[0] <<" "<< conv_coef->luma_range[1] << endl;
   cout << "conv_coef->chroma_range" << conv_coef->chroma_range[0] << " " << conv_coef->chroma_range[1] << endl;
   cout << "conv_coef->rgb_range" << conv_coef->rgb_range[0] << " "<< conv_coef->rgb_range[1] << endl;
    return (ceof_list.size() == 15) ? true: false;
}

int main(int argc, const char *argv[])
{
    int ret;
    mvx_argparse argp;
    uint32_t inputFormat;
    uint32_t outputFormat;
    uint32_t width, height;
    const char *roi_file = NULL;
    const char *epr_file = NULL;
    struct v4l2_osd_info osd_info;
    memset(&osd_info, 0, sizeof(struct v4l2_osd_info));

    mvx_argp_construct(&argp);
    mvx_argp_add_opt(&argp, '\0', "dev", true, 1, "/dev/video0", "Device.");
    mvx_argp_add_opt(&argp, 'i', "inputformat", true, 1, "yuv420", "Pixel format.");
    mvx_argp_add_opt(&argp, 'o', "outputformat", true, 1, "h264", "Output pixel format.");
    mvx_argp_add_opt(&argp, 'f', "format", true, 1, "ivf", "Output container format. [ivf, raw]");
    mvx_argp_add_opt(&argp, 'w', "width", true, 1, "1920", "Width.");
    mvx_argp_add_opt(&argp, 'h', "height", true, 1, "1080", "Height.");
    mvx_argp_add_opt(&argp, 0, "visible_width", true, 1, "0", "Visible Width.");
    mvx_argp_add_opt(&argp, 0, "visible_height", true, 1, "0", "Visible Height.");
    mvx_argp_add_opt(&argp, 's', "strideAlign", true, 1, "1", "Stride alignment.");
    mvx_argp_add_opt(&argp, 0, "stride0", true, 1, "1", "Number of bytes of stride for the first plane.");
    mvx_argp_add_opt(&argp, 0, "stride1", true, 1, "1", "Number of bytes of stride for the second plane if have.");
    mvx_argp_add_opt(&argp, 0, "stride2", true, 1, "1", "Number of bytes of stride for the third plane if have.");
    mvx_argp_add_opt(&argp, 0, "mirror", true, 1, "0", "mirror, 1 : horizontal; 2 : vertical.");
    mvx_argp_add_opt(&argp, 'r', "rotate", true, 1, "0", "Rotation, 0 | 90 | 180 | 270");
    mvx_argp_add_opt(&argp, 0, "roi_cfg", true, 1, NULL, "ROI config file.");
    mvx_argp_add_opt(&argp, 0, "frames", true, 1, "0", "nr of frames to process");
    mvx_argp_add_opt(&argp, 0, "epr_cfg", true, 1, NULL, "Encode Parameter Records config file name");
    mvx_argp_add_opt(&argp, 0, "idr_cfg", true, 1, NULL, "Encode config file to set IDR flag on pic index");
    mvx_argp_add_opt(&argp, 0, "chr_cfg", true, 1, NULL, "Encode Change Rectangle config file name");
    mvx_argp_add_opt(&argp, 0, "rate_control", true, 1, "off", "Selects rate control type, constant/variable/off");
    mvx_argp_add_opt(&argp, 0, "target_bitrate", true, 1, "0", "If rate control is enabled, this option sets target bitrate");
    mvx_argp_add_opt(&argp, 0, "max_bitrate", true, 1, "0", "If rate control is enabled, this option sets maximum bitrate");
    mvx_argp_add_opt(&argp, 0, "rc_bit_i_mode", true, 1, "0", "0 or 1, rc_bit_i_mode for rate control (for I frames)");
    mvx_argp_add_opt(&argp, 0, "rc_bit_ratio_i", true, 1, "0", "1 ~ 100 rc_bit_ratio_i for rate control (for I frames)");
    mvx_argp_add_opt(&argp, 0, "gop", true, 1, "0", "GOP: 0 is None, 1 is Bidi, 2 is Low delay, 3 is Pyramid, 4 is Svct-3, 5 is gdr");
    mvx_argp_add_opt(&argp, 0, "pframes", true, 1, "0", "Number of P frames");
    mvx_argp_add_opt(&argp, 0, "bframes", true, 1, "0", "Number of B frames");
    mvx_argp_add_opt(&argp, 'n', "minqp", true, 1, "0", "min QP for rate control");
    mvx_argp_add_opt(&argp, 'm', "maxqp", true, 1, "51", "max QP for rate control");
    mvx_argp_add_opt(&argp, 't', "tier", true, 1, "2", "Profile.");
    mvx_argp_add_opt(&argp, 'l', "level", true, 1, "1", "Level.");
    mvx_argp_add_opt(&argp, 'v', "fps", true, 1, "24", "Frame rate.");
    mvx_argp_add_opt(&argp, 0, "ecm", true, 1, "1", "0 is CAVLC, 1 is CABAC");
    mvx_argp_add_opt(&argp, 0, "bitdepth", true, 1, "8", "Set other bitdepth,invalid value 8 | 10.used for 10bit source encode as 8bit");
    mvx_argp_add_opt(&argp, 'q', "fixedqp", true, 1, "20", "fixed QP for I P B frames. If it is combined with -x then the value will later be increased with 2.");
    mvx_argp_add_opt(&argp, 0, "qpi", true, 1, "20", "fixed QP for I frames.");
    mvx_argp_add_opt(&argp, 0, "qpb", true, 1, "20", "fixed QP for B frames.");
    mvx_argp_add_opt(&argp, 0, "qpp", true, 1, "20", "fixed QP for P frames.");
    mvx_argp_add_opt(&argp, 0, "memory", true, 1, "mmap", "support mmap and dma.");
    mvx_argp_add_opt(&argp, 0, "crop_left", true, 1, "0", "encoder SPS crop param, left offset");
    mvx_argp_add_opt(&argp, 0, "crop_right", true, 1, "0", "encoder SPS crop param, right offset");
    mvx_argp_add_opt(&argp, 0, "crop_top", true, 1, "0", "encoder SPS crop param, top offset");
    mvx_argp_add_opt(&argp, 0, "crop_bottom", true, 1, "0", "encoder SPS crop param, bottom offset");
    mvx_argp_add_opt(&argp, 0, "input_thread", true, 1, "0", "read input buffer in other thread.");

    mvx_argp_add_opt(&argp, 0, "colour_description_range", true, 1, "0", "VUI param: Colour description; range\n"
                                    "\t\tValue: 0=Unspecified, 1=Limited, 2=Full");
    mvx_argp_add_opt(&argp, 0, "colour_primaries", true, 1, "0", "VUI param: Colour description; colour primaries (0-255, see hevc spec. E.3.1)\n"
                                    "\t\tValue: 0=Unspecified, 1=BT709, 2=BT470M, 3=BT601_625, 4=T601_525, 5=GENERIC_FILM, 6=BT2020");
    mvx_argp_add_opt(&argp, 0, "transfer_characteristics", true, 1, "0", "VUI param: Colour description; transfer characteristics (0-255, see hevc spec. E.3.1)\n"
                                    "\t\tValue: 0=Unspecified, 1=LINEAR, 2=SRGB, 3=SMPTE170M, 4=GAMMA22, 5=GAMMA28, 6=ST2084, 7=HLG, 8=SMPTE240M, 9=XVYCC, 10=BT1361, 11=ST428");
    mvx_argp_add_opt(&argp, 0, "matrix_coeff", true, 1, "0", "VUI param: Colour description; matrix coefficients (0-255, see hevc spec. E.3.1)\n"
                                    "\t\tValue: 0=Unspecified, 1=BT709, 2=BT470M, 3=BT601, 4=SMPTE240M, 5=T2020, 6=BT2020Constant");
    mvx_argp_add_opt(&argp, 0, "time_scale", true, 1, "0", "VUI param: vui_time_scale");
    mvx_argp_add_opt(&argp, 0, "num_units_in_tick", true, 1, "0", "VUI param: vui_num_units_in_tick");
    mvx_argp_add_opt(&argp, 0, "aspect_ratio_idc", true, 1, "0", "VUI param: aspect_ratio_idc. [0,255]");
    mvx_argp_add_opt(&argp, 0, "sar_width", true, 1, "0", "VUI param: sar_width");
    mvx_argp_add_opt(&argp, 0, "sar_height", true, 1, "0", "VUI param: sar_height");
    mvx_argp_add_opt(&argp, 0, "video_format", true, 1, "0", "VUI param: video_format. (0-5, see hevc spec. E.3.1)\n"
                                    "\t\tValue: 0=Component, 2=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=Unspecified");

    mvx_argp_add_opt(&argp, 0, "sei_mastering_display", true, 1, "0", "SEI param : mastering display 's parameters");
    mvx_argp_add_opt(&argp, 0, "sei_content_light", true, 1, "0", "SEI param : sei_content_light");
    mvx_argp_add_opt(&argp, 0, "sei_user_data_unregistered", true, 1, "0", "SEI param : user data unregisterd");
    mvx_argp_add_opt(&argp, 0, "hrd_buffer_size", true, 1, "0", "Hypothetical Reference Decoder buffer size relative to the bitrate (in seconds) for rate control"
                                    "\t\tValue: should bigger than target_bitrate/fps on normal case");
    mvx_argp_add_opt(&argp, 0, "ltr_mode", true, 1, "0", "encoder long term reference mode,range from 1 to 8 (inclusive)\n"
                                    "\t\t1: LDP-method-1 | 2: LDP-method-2 | 3: LDB-method-1 | 4: LDB-method-2\n"
                                    "\t\t5: BiDirection-method-1 | 6: BiDirection-method-2 | 7: Pyrimid-method-1 | 8: Pyrimid-method-2\n");
    mvx_argp_add_opt(&argp, 0, "ltr_period", true, 1, "0", "encoder long term reference period, range from 1 to 255 (inclusive)");
    mvx_argp_add_pos(&argp, "input", false, 1, "", "Input file.");
    mvx_argp_add_pos(&argp, "output", false, 1, "", "Output file.");
    mvx_argp_add_opt(&argp, 0, "trystop", true, 0, "0", "Try if Encoding Stop Command exixts");
    mvx_argp_add_opt(&argp, 0, "restart_interval", true, 1, "-1", "JPEG restart interval.");
    mvx_argp_add_opt(&argp, 0, "quality", true, 1, "0", "JPEG compression quality. [1-100, 0 - default]");
    mvx_argp_add_opt(&argp, 0, "enc_stats_mode", true, 1, "0", "Only availble in H264ENC and HEVCENC, encode data statistics mode bitmask:{b0=MMS On/Off(MAD, MSE, SATD); b1=BITCOST}. Default is all off");
    mvx_argp_add_opt(&argp, 0, "enc_stats_cfg", true, 1, NULL, "enc stats cfg file, [$pic_index-$stats_mode]");
    mvx_argp_add_opt(&argp, 0, "miniframe_height", true, 1, "0", "one input frame was seperated in several input buffer");

    mvx_argp_add_opt(&argp, 0, "packet_mbs", true, 1, "0", "Number of coding units (Macroblocks/CTUs) per slice");
    mvx_argp_add_opt(&argp, 0, "constrained_intra_pred", true, 1, "0", "constrained_intra_pred");
    mvx_argp_add_opt(&argp, 0, "nQpMinI", true, 1, "0", "nQpMinI for rate control (for I frames)");
    mvx_argp_add_opt(&argp, 0, "nQpMaxI", true, 1, "0", "nQpMaxI for rate control (for I frames)");
    mvx_argp_add_opt(&argp, 0, "init_qpi", true, 1, "0", "0 ~ 51, set init_qpi by user");
    mvx_argp_add_opt(&argp, 0, "init_qpp", true, 1, "0", "0 ~ 51, set init_qpi by user");
    mvx_argp_add_opt(&argp, 0, "sao_luma_dis", true, 1, "0", "sao luma disable");
    mvx_argp_add_opt(&argp, 0, "sao_chroma_dis", true, 1, "0", "sao chroma disable");
    mvx_argp_add_opt(&argp, 0, "qp_delta_i_p", true, 1, "0", "0 ~ 51, set init_qpi by user");
    mvx_argp_add_opt(&argp, 0, "ref_rb_en", true, 1, "0", "Enable/Disable reference ring buffer feature [1:enable, 0:disable], default is disable");
    mvx_argp_add_opt(&argp, 0, "rc_qp_clip_top", true, 1, "0", "encoder : rc qp clip top, range [0, 51], but must less than --rc_qp_clip_bottom");
    mvx_argp_add_opt(&argp, 0, "rc_qp_clip_bottom", true, 1, "0", "encoder : rc qp clip top, range [0, 51], but must greater than --rc_qp_clip_top");
    mvx_argp_add_opt(&argp, 0, "qpmap_clip_top", true, 1, "0", "encoder : qpmap clip top, range [0, 51], but must less than --qpmap_clip_bottom");
    mvx_argp_add_opt(&argp, 0, "qpmap_clip_bottom", true, 1, "0", "encoder : qpmap clip top, range [0, 51], but must greater than --qpmap_clip_top");
    mvx_argp_add_opt(&argp, 0, "xsearch_range", true, 1, "0", "Search range for suitable MBs in x direction");
    mvx_argp_add_opt(&argp, 0, "ysearch_range", true, 1, "0", "Search range for suitable MBs in y direction");
    mvx_argp_add_opt(&argp, 0, "profiling", true, 1, "0", "debug purpose, disable by default");

    mvx_argp_add_opt(&argp, 0, "svct3_level1_peroid", true, 1, "0", "encoder SVCT3 level 1 peroid: must be an even number and range from 4 to 250 (inclusive)");
    mvx_argp_add_opt(&argp, 0, "intermediate_buffer_size", true, 1, "0", "intermediate_buffer_size");

    mvx_argp_add_opt(&argp, 0, "qscale", true, 1, "0", "Quantization table scaler (JPEG only)");
    mvx_argp_add_opt(&argp, 0, "qscale_luma", true, 1, "0", "Luma Quantization table scaler (JPEG only)");
    mvx_argp_add_opt(&argp, 0, "qscale_chroma", true, 1, "0", "Chroma Quantization table scaler (JPEG only)");
    mvx_argp_add_opt(&argp, 0, "huff_table", true, 1, NULL, "File to load huffman table to JPEG encoder (JPEG only)");

    mvx_argp_add_opt(&argp, 0, "reset_gop_cfg", true, 1, NULL, "Set reset gop config file, [20-5] means frame-20 reset gop and pframes use 5");
    mvx_argp_add_opt(&argp, 0, "reset_ltr_peroid_cfg", true, 1, NULL, "Set LTR peroid dynamiclly, only support LDP of H264 and HEVC now");

    mvx_argp_add_opt(&argp, 0, "gdr_number", true, 1, NULL, "The number of GDR frames per group");
    mvx_argp_add_opt(&argp, 0, "gdr_period", true, 1, NULL, "The number of normal P frames between two group of GDR frames");

    mvx_argp_add_opt(&argp, 0, "scd_enable", true, 1, NULL, "Enable scene change detection");
    mvx_argp_add_opt(&argp, 0, "scd_percent", true, 1, NULL, "0 ~ 100,The percentage of stripe are calculated for scene change detection");
    mvx_argp_add_opt(&argp, 0, "scd_threshold", true, 1, NULL, "0 ~ 2047,Threshold of scene change");
    mvx_argp_add_opt(&argp, 0, "aq_ssim_en", true, 1, NULL, "Enable SSIM driven adaptive quantizatoin.For H.264/HEVC only");
    mvx_argp_add_opt(&argp, 0, "aq_neg_ratio", true, 1, NULL, "0 ~ 63,The weight of negative delta QP.For H.264/HEVC only");
    mvx_argp_add_opt(&argp, 0, "aq_pos_ratio", true, 1, NULL, "0 ~ 63,The weight of positive delta QP.For H.264/HEVC only");
    mvx_argp_add_opt(&argp, 0, "aq_qpdelta_lmt", true, 1, NULL, "0 ~ 7,The boundary of delta QP.For H.264/HEVC only");
    mvx_argp_add_opt(&argp, 0, "aq_init_frm_avg_svar", true, 1, NULL, "0 ~ 15,Initial frame variance for aq ssim.For H.264/HEVC only");

    mvx_argp_add_opt(&argp, 0, "multi_sps_pps", true, 1, NULL, "Support multi SPS PSS for h264 and hevc");
    mvx_argp_add_opt(&argp, 0, "turn_visual", true, 1, NULL, "Pursue subjective optimization rather than BD-rate");

    mvx_argp_add_opt(&argp, 0, "forced_uv_value", true, 1, NULL, "Set forced uv valule to YUV format. The uv value represented in 10b(0~1023),recommend value:512. Supported formats:\n"
                                                                 "\tyuv444 yuv444_10 yuv420 yuv_i42010 gray gray_10");
    mvx_argp_add_opt(&argp, 0, "422to420", true, 1, NULL, "chroma format use 4:2:2 or 4:2:0,default is 420,set value 2 will use 422,only used for mjpeg 422 source to select use 420 or 422 encode");
    mvx_argp_add_opt(&argp, 0, "cust_rgb2yuv_range", true, 1, "", "customized integer coeffiecents for encoder rgb2yuv r2y:g2y:b2y:r2u:g2u:b2u:r2v:g2v:b2v:luma_offset[0]:luma_offset[1]:chroma_offste[0]:chroma_offste[1]:rgb_offset[0]:rgb_offset[1],the coefficients bit-length is 15 bits");
    mvx_argp_add_opt(&argp, 0, "rgb2yuv_color_conversion", true, 1, "0", "decoder color conversion for rgb2yuv."
                                                                "\t\tValue: 0 [default:predefined standards bt601]"
                                                                "\t\tValue: 1 [predefined standards bt601f]"
                                                                "\t\tValue: 2 [predefined standards bt709]"
                                                                "\t\tValue: 3 [predefined standards bt709f]"
                                                                "\t\tValue: 4 [predefined standards bt2020]"
                                                                "\t\tValue: 5 [predefined standards bt2020f]");
    mvx_argp_add_opt(&argp, 0, "src_crop_x", true, 1, 0, "left start x of luma in original image");
    mvx_argp_add_opt(&argp, 0, "src_crop_y", true, 1, 0, "top start y of luma in original image");
    mvx_argp_add_opt(&argp, 0, "src_crop_width", true, 1, 0, "cropped width of luma in original image");
    mvx_argp_add_opt(&argp, 0, "src_crop_height", true, 1, 0, "cropped height of luma in original image");

    mvx_argp_add_opt(&argp, 0, "adaptive_intra_block", true, 1, 0, "0: disable adaptive_intra_block function, 1: enable --scd_en and adaptive_intra_block");
    mvx_argp_add_opt(&argp, 0, "change_pos", true, 1, 0, "If rate control is enabled (with option --rate_control...), this option indicates the ratio of the max_bitrate when CVBR starts to adjust "
                                                                "QP to the maximum bitrate[50 100].");

    mvx_argp_add_opt(&argp, 0, "framebuffer_cnt", true, 1, NULL, "Number of buffers to use for yuv data");
    mvx_argp_add_opt(&argp, 0, "bitbuffer_cnt", true, 1, NULL, "Number of buffers to use for encoded data");

    mvx_argp_add_opt(&argp, 0, "osd_cfg", true, 1, NULL, "OSD config file name");
    mvx_argp_add_opt(&argp, 0, "osd_infile", true, 1, NULL, "osd in_file_1");
    mvx_argp_add_opt(&argp, 0, "osd_width", true, 1, NULL, "Width of osd 0 images in pixels");
    mvx_argp_add_opt(&argp, 0, "osd_height", true, 1, NULL, "Height of osd 0 images in pixels");
    mvx_argp_add_opt(&argp, 0, "osd_color", true, 1, "rgb1555", "Osd 0 buffer color format. Default is agrb1555. supported:argb4444, rgb565");

    mvx_argp_add_opt(&argp, 0, "osd_infile2", true, 1, NULL, "osd in_file_2");
    mvx_argp_add_opt(&argp, 0, "osd_width2", true, 1, NULL, "Width of osd 1 images in pixels");
    mvx_argp_add_opt(&argp, 0, "osd_height2", true, 1, NULL, "Height of osd 1 images in pixels");
    mvx_argp_add_opt(&argp, 0, "osd_color2", true, 1, "rgb1555", "Osd 1 buffer color format. Default is agrb1555. supported:argb4444, rgb565");

    ret = mvx_argp_parse(&argp, argc - 1, &argv[1]);
    width = mvx_argp_get_int(&argp, "width", 0);
    height = mvx_argp_get_int(&argp, "height", 0);
    if (ret != 0)
    {
        mvx_argp_help(&argp, argv[0]);
        return 1;
    }

    inputFormat = Codec::to4cc(mvx_argp_get(&argp, "inputformat", 0));
    if (inputFormat == 0)
    {
        fprintf(stderr, "Error: Illegal frame format. format=%s.\n",
                mvx_argp_get(&argp, "inputformat", 0));
        return 1;
    }

    outputFormat = Codec::to4cc(mvx_argp_get(&argp, "outputformat", 0));
    if (outputFormat == 0)
    {
        fprintf(stderr, "Error: Illegal bitstream format. format=%s.\n",
                mvx_argp_get(&argp, "outputformat", 0));
        return 1;
    }
    ifstream is(mvx_argp_get(&argp, "input", 0));
    Input *inputFile;
    ifstream *roi_stream = NULL;
    ifstream *epr_stream = NULL;
    std::string out_stats_file_base = std::string( mvx_argp_get(&argp, "output", 0)) + ".stats";
    uint32_t mini_height = 0;
    int mirror = mvx_argp_get_int(&argp,"mirror",0);
    int frames = mvx_argp_get_int(&argp,"frames",0);
    int rotation = mvx_argp_get_int(&argp,"rotate",0);
    size_t stride[VIDEO_MAX_PLANES] = {0};
    if (mvx_argp_is_set(&argp, "stride0") || mvx_argp_is_set(&argp, "stride1") ||
        mvx_argp_is_set(&argp, "stride2")) {
        stride[0] = mvx_argp_get_int(&argp, "stride0", 0);
        stride[1] = mvx_argp_get_int(&argp, "stride1", 0);
        stride[2] = mvx_argp_get_int(&argp, "stride2", 0);
    }
    if (Codec::isAFBC(inputFormat))
    {
        inputFile = new InputAFBC(is, inputFormat, mvx_argp_get_int(&argp, "width", 0),
                                  mvx_argp_get_int(&argp, "height", 0));
    }
    else
    {
        roi_file = mvx_argp_get(&argp, "roi_cfg", 0);
        if (roi_file) {
            printf("roi config filename is < %s >.\n", roi_file);
            roi_stream = new ifstream(roi_file, ios::binary);
            if(NULL == roi_stream){
                fprintf(stderr, "Error: (NULL == roi_stream).\n");
                return 1;
            }
            inputFile = new InputFileFrameWithROI(is,
                                       inputFormat,
                                       mvx_argp_get_int(&argp, "width", 0),
                                       mvx_argp_get_int(&argp, "height", 0),
                                       mvx_argp_get_int(&argp, "strideAlign", 0), *roi_stream, stride);
        } else {
            epr_file = mvx_argp_get(&argp, "epr_cfg", 0);
            if (epr_file) {
                printf("epr config filename is < %s >.\n", epr_file);
                epr_stream = new ifstream(epr_file, ios::binary);
                if(NULL == epr_stream){
                    fprintf(stderr, "Error: (NULL == epr_stream).\n");
                    return 1;
                }
                inputFile = new InputFileFrameWithEPR(is,
                                       inputFormat,
                                       mvx_argp_get_int(&argp, "width", 0),
                                       mvx_argp_get_int(&argp, "height", 0),
                                       mvx_argp_get_int(&argp, "strideAlign", 0), *epr_stream,outputFormat, stride);
            } else if (mvx_argp_is_set(&argp, "miniframe_height")){
                mini_height = mvx_argp_get_int(&argp,"miniframe_height",0);
                assert((mini_height >= 128) && (mini_height % 64 == 0) && "miniframe_height should be aligned with 64, and greater than 128");
                inputFile = new InputFileMiniFrame(is,
                                           inputFormat,
                                           mvx_argp_get_int(&argp, "width", 0),
                                           mvx_argp_get_int(&argp, "height", 0),
                                           mvx_argp_get_int(&argp, "strideAlign", 0),
                                           mini_height, stride);
            } else if (mvx_argp_is_set(&argp, "osd_cfg") && mvx_argp_is_set(&argp, "osd_infile")) {
                assert(mvx_argp_is_set(&argp, "osd_width") && mvx_argp_is_set(&argp, "osd_height"));
                inputFile = new InputFileFrameOSD(is,
                                           inputFormat,
                                           mvx_argp_get_int(&argp, "width", 0),
                                           mvx_argp_get_int(&argp, "height", 0),
                                           mvx_argp_get_int(&argp, "strideAlign", 0), stride);
                InputFileFrameOSD *input = dynamic_cast<InputFileFrameOSD*>(inputFile);
                osd_info.inputFormat_osd[0] = Codec::to4cc(mvx_argp_get(&argp, "osd_color", 0));
                osd_info.width_osd[0] = mvx_argp_get_int(&argp, "osd_width", 0);
                osd_info.height_osd[0] = mvx_argp_get_int(&argp, "osd_height", 0);
                input->osd_file_1 = new InputFileOsd(mvx_argp_get(&argp, "osd_infile", 0),
                                           osd_info.inputFormat_osd[0],
                                           osd_info.width_osd[0],
                                           osd_info.height_osd[0],
                                           1);
                if (mvx_argp_is_set(&argp, "osd_infile2")) {
                    assert(mvx_argp_is_set(&argp, "osd_width2") && mvx_argp_is_set(&argp, "osd_height2"));
                    osd_info.inputFormat_osd[1] = Codec::to4cc(mvx_argp_get(&argp, "osd_color2", 0));
                    osd_info.width_osd[1] = mvx_argp_get_int(&argp, "osd_width2", 0);
                    osd_info.height_osd[1] = mvx_argp_get_int(&argp, "osd_height2", 0);
                    input->osd_file_2 = new InputFileOsd(mvx_argp_get(&argp, "osd_infile2", 0),
                                               osd_info.inputFormat_osd[1],
                                               osd_info.width_osd[1],
                                               osd_info.height_osd[1],
                                               1);
                }
            } else {
                inputFile = new InputFileFrame(is,
                                           inputFormat,
                                           mvx_argp_get_int(&argp, "width", 0),
                                           mvx_argp_get_int(&argp, "height", 0),
                                           mvx_argp_get_int(&argp, "strideAlign", 0), stride);
            }
        }
    }

    if (mvx_argp_is_set(&argp, "chr_cfg")) {
        const char* chr_cfg = mvx_argp_get(&argp, "chr_cfg", 0);
        char *sub_buf1, *sub_buf2;
        v4l2_mvx_chr_config config;
        FILE *chr_file = fopen(chr_cfg, "r");
        if (NULL == chr_file) {
            cerr << "Could not read IDR config file!" <<endl;
            return 1;
        }
        InputFileFrame *input = dynamic_cast<InputFileFrame*>(inputFile);
        if (input->chr_list == NULL) {
            input->chr_list = new v4l2_chr_list_t();
        }
        while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, chr_file))
        {
            if (2 != sscanf(cfg_file_line_buf, "pic=%d num_chr=%d", &config.pic_index, &config.num_chr))
            {
                cerr<<"Parse CHR config file ERROR"<<endl;
            }

            if (config.num_chr > V4L2_MAX_FRAME_CHANGE_RECTANGLES)
            {
                cerr<<"Invalid n_rectangles value = "<<config.num_chr<<endl;
            }

            if (config.num_chr > 0)
            {
                int match = 0;
                uint16_t tmp_left=0;
                uint16_t tmp_right=0;
                uint16_t tmp_top=0;
                uint16_t tmp_bottom=0;

                sub_buf1 = cfg_file_line_buf;

                for (unsigned i = 0; i < config.num_chr; i++)
                {
                    sub_buf2 = strstr(sub_buf1, " chr=");
                    match = sscanf(sub_buf2, " chr={%hu,%hu,%hu,%hu}",
                        &tmp_left,
                        &tmp_right,
                        &tmp_top,
                        &tmp_bottom);
                    if (match != 4)
                    {
                        cerr<<"Error while parsing the change rectangles "<< match<<endl;
                        exit(1);
                    }
                    if (mvx_argp_is_set(&argp, "rotate") &&
                        (rotation % 360 == 90 || rotation % 360 == 270 || rotation % 360 == 180)) {
                        if(rotation % 360 == 90){
                                config.rectangle[i].x_left = tmp_top;
                                config.rectangle[i].x_right = tmp_bottom;
                                config.rectangle[i].y_top = width - tmp_right;
                                config.rectangle[i].y_bottom = width - tmp_left;
                        } else if(rotation % 360 == 270){
                                config.rectangle[i].x_left = height - tmp_bottom;
                                config.rectangle[i].x_right = height - tmp_top;
                                config.rectangle[i].y_top = tmp_left;
                                config.rectangle[i].y_bottom = tmp_right;
                        } else {
                                config.rectangle[i].x_left = width - tmp_right;
                                config.rectangle[i].x_right = width - tmp_left;
                                config.rectangle[i].y_top = height - tmp_bottom;
                                config.rectangle[i].y_bottom = height - tmp_top;
                        }
                    } else {
                        config.rectangle[i].x_left = tmp_left;
                        config.rectangle[i].x_right = tmp_right;
                        config.rectangle[i].y_top = tmp_top;
                        config.rectangle[i].y_bottom = tmp_bottom;
                    }
                    sub_buf1 = sub_buf2 + 5;
                }
            }
            input->chr_list->push_back(config);
        }
        fclose(chr_file);
    }

    if (mvx_argp_is_set(&argp, "reset_gop_cfg")) {
        const char* gop_cfg = mvx_argp_get(&argp, "reset_gop_cfg", 0);
        FILE *file = fopen(gop_cfg, "r");
        char *sub_buf;
        if (NULL == file) {
            cerr << "Could not read reset_gop_cfg file!" <<endl;
            return 1;
        }
        InputFileFrame *input = dynamic_cast<InputFileFrame*>(inputFile);
        if (input->gop_list == NULL) {
            input->gop_list = new gop_list_t();
        }
        while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file))
        {
            sub_buf = strtok(cfg_file_line_buf, ",");
            while(sub_buf != NULL)
            {
                int reset_pic;
                int reset_pframes;
                if (2 == sscanf(sub_buf, "[%d-%d]", &reset_pic, &reset_pframes))
                {
                  struct v4l2_gop_config tmp_rest_gop;
                  tmp_rest_gop.gop_pic = reset_pic;
                  tmp_rest_gop.gop_pframes = reset_pframes;
                  input->gop_list->push(tmp_rest_gop);
                } else {
                  printf("--reset_gop_cfg file format error:[%s], should be [int-int]\n", sub_buf);
                  exit(1);
                }
                sub_buf = strtok(NULL, ",");
            }
        }
        fclose(file);
    }

    if (mvx_argp_is_set(&argp, "reset_ltr_peroid_cfg")) {
        const char *ltr_cfg = mvx_argp_get(&argp, "reset_ltr_peroid_cfg", 0);
        FILE *file = fopen(ltr_cfg, "r");
        char *sub_buf;
        if (NULL == file) {
            cerr << "Could not read reset_gop_cfg file!" <<endl;
            return 1;
        }
        InputFileFrame *input = dynamic_cast<InputFileFrame*>(inputFile);
        if (input->ltr_list == NULL) {
            input->ltr_list = new ltr_list_t();
        }
        while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file))
        {
            sub_buf = strtok(cfg_file_line_buf, ",");
            while(sub_buf != NULL)
            {
                int reset_pic;
                int reset_ltr_peroid;
                if (2 == sscanf(sub_buf, "[%d-%d]", &reset_pic, &reset_ltr_peroid))
                {
                  if (reset_ltr_peroid == 0)
                        printf("--reset_ltr_peroid_cfg error: ltr new peroid must be > 0, but now get input [%s]\n", sub_buf);
                  struct v4l2_reset_ltr_peroid_config tmp_rest_gop;
                  tmp_rest_gop.reset_trigger_pic = reset_pic;
                  tmp_rest_gop.reset_ltr_peroid = reset_ltr_peroid;
                  input->ltr_list->push(tmp_rest_gop);
                } else {
                  printf("--reset_ltr_peroid_cfg file format error:[%s], should be [int-int]\n", sub_buf);
                }
                sub_buf = strtok(NULL, ",");
            }
        }
        fclose(file);
    }

    if (mvx_argp_is_set(&argp, "idr_cfg")) {
        const char* idr_cfg = mvx_argp_get(&argp, "idr_cfg", 0);
        FILE *file = fopen(idr_cfg, "r");
        char *sub_buf;
        if (NULL == file) {
            cerr << "Could not read IDR config file!" <<endl;
            return 1;
        }
        while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file))
        {
            sub_buf = strtok(cfg_file_line_buf, ",");
            while(sub_buf != NULL)
            {
                uint32_t index = atoi(sub_buf);
                inputFile->idr_list.push(index);
                sub_buf = strtok(NULL, ",");
            }
        }
        fclose(file);
    }

    if (mvx_argp_is_set(&argp, "osd_cfg") && mvx_argp_is_set(&argp, "osd_infile")) {
        const char* osd_cfg = mvx_argp_get(&argp, "osd_cfg", 0);
        FILE *file = fopen(osd_cfg, "r");
        char *sub_buf1, *sub_buf2;
        v4l2_osd_config config;
        if (NULL == file) {
            cerr << "Could not read OSD config file!" <<endl;
            return 1;
        }
        InputFileFrameOSD *input = dynamic_cast<InputFileFrameOSD*>(inputFile);
        if (input->osd_list == NULL) {
            input->osd_list = new v4l2_osd_list_t();
        }

        /* cfg_file_line_buf --- all config uses this buf */
        while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file)) {
            if (2 != sscanf(cfg_file_line_buf, "pic=%d osd_idx=%d", &config.pic_index, &config.num_osd)) {
                printf("Parse OSD config file ERROR\n");
            }

            if (config.num_osd > (1 << V4L2_MAX_FRAME_OSD_REGION) - 1) {
                printf("Invalid n_rectangles value = %d\n", config.num_osd);
            }

            int match = 0;
            int8_t osd_para_num = 8;
            sub_buf1 = cfg_file_line_buf;
            uint32_t en, alpha_en, cvt_en, alpha_val, cvt_th, csc_coef;
            uint16_t startx, starty;
            for (uint8_t i = 0; i < V4L2_MAX_FRAME_OSD_REGION; i++) {
                sub_buf2 = (i == 0) ? strstr(sub_buf1, "osd1_cfg") : strstr(sub_buf1, "osd2_cfg");
                sub_buf2 += 5;
                match = sscanf(sub_buf2, "cfg={en=%01d,alpha_en=%01d,cvt_en=%01d,alpha_val=%d,cvt_th=%d,csc_coef=%d,startx=%hu,starty=%hu}",
                    &en, &alpha_en, &cvt_en, &alpha_val, &cvt_th, &csc_coef, &startx, &starty);
                config.osd_single_cfg[i].osd_inside_enable = en;
                config.osd_single_cfg[i].osd_inside_alpha_enable = alpha_en;
                config.osd_single_cfg[i].osd_inside_convert_color_enable = cvt_en;
                config.osd_single_cfg[i].osd_inside_alpha_value = alpha_val;
                config.osd_single_cfg[i].osd_inside_convert_color_threshold = cvt_th;
                config.osd_single_cfg[i].osd_inside_rgb2yuv_mode = csc_coef;
                config.osd_single_cfg[i].osd_inside_start_x = startx;
                config.osd_single_cfg[i].osd_inside_start_y = starty;
                if (match != osd_para_num) { /* since matched, convert format to pre_defined */
                 /* when not found, set current osd para to be zero */
                    memset(&config.osd_single_cfg[i],  0, sizeof(v4l2_mvx_osd_cfg));
                }
            }
            input->osd_list->push_back(config);/* copy config info to osd_opt_list */
        }
        fclose(file);
    }

    ofstream os(mvx_argp_get(&argp, "output", 0));
    Output *outputFile;
    if (rotation % 90 != 0){
        cerr << "Unsupported rotation:"<<rotation <<endl;
        rotation = 0;
    }
    if (string(mvx_argp_get(&argp, "format", 0)).compare("ivf") == 0)
    {
        outputFile = new OutputIVF(os, outputFormat,
                                   mvx_argp_get_int(&argp, "width", 0),
                                   mvx_argp_get_int(&argp, "height", 0));
    }
    else if (string(mvx_argp_get(&argp, "format", 0)).compare("raw") == 0)
    {
        if (mvx_argp_get_int(&argp,"enc_stats_mode",0) > 0 || mvx_argp_is_set(&argp, "enc_stats_cfg")) {
            std::string out_stats_file_base = std::string( mvx_argp_get(&argp, "output", 0)) + ".stats";
            outputFile = new OutputFileFrameStats(os, outputFormat,
                            mvx_argp_get_int(&argp,"enc_stats_mode",0),
                            out_stats_file_base, mvx_argp_get_int(&argp, "width", 0),
                            mvx_argp_get_int(&argp, "height", 0));
        } else {
            outputFile = new OutputFile(os, outputFormat);
        }
        printf("%s>outputFile: %s\n", __FUNCTION__, outputFile);
    }
    else
    {
        cerr << "Error: Unsupported container format. format=" <<
        mvx_argp_get(&argp, "format", 0) << "." << endl;
        return 1;
    }

    Encoder encoder(mvx_argp_get(&argp, "dev", 0), *inputFile, *outputFile);
    printf("Encoder construct done\n");
    if (mvx_argp_is_set(&argp, "trystop"))
    {
        encoder.tryStopCmd(true);
    }
    if (mvx_argp_is_set(&argp, "restart_interval"))
    {
        encoder.setJPEGRefreshInterval(mvx_argp_get_int(&argp, "restart_interval", 0));
    }
    if (mvx_argp_is_set(&argp, "quality"))
    {
        encoder.setJPEGQuality(mvx_argp_get_int(&argp, "quality", 0));
    }
    if (mvx_argp_is_set(&argp, "osd_cfg") && mvx_argp_is_set(&argp, "osd_infile"))
    {
        encoder.setEncOSDinfo(&osd_info);
    }
    encoder.setMirror(mirror);
    encoder.setRotation(rotation);
    encoder.setStride(stride);
    encoder.setFrameCount(frames);
    encoder.setRateControl(mvx_argp_get(&argp, "rate_control", 0),
                               mvx_argp_get_int(&argp,"target_bitrate",0),
                               mvx_argp_get_int(&argp,"max_bitrate",0));
    if (mvx_argp_is_set(&argp, "change_pos")) {
        encoder.setChangePos(mvx_argp_get_int(&argp, "change_pos", 0));
    }
    if (mvx_argp_is_set(&argp, "rc_bit_i_mode")) {
        encoder.setRcBitIMode(mvx_argp_get_int(&argp, "rc_bit_i_mode", 0));
    }
    if (mvx_argp_is_set(&argp, "rc_bit_ratio_i")) {
        encoder.setRcBitRationI(mvx_argp_get_int(&argp, "rc_bit_ratio_i", 0));
    }
    if (mvx_argp_is_set(&argp, "multi_sps_pps")) {
        encoder.setMultiSPSPPS(mvx_argp_get_int(&argp, "multi_sps_pps", 0));
    }
    if (mvx_argp_is_set(&argp, "turn_visual")) {
        encoder.setEnableVisual(mvx_argp_get_int(&argp, "turn_visual", 0));
    }
    if (mvx_argp_is_set(&argp, "adaptive_intra_block")) {
        assert(!(mvx_argp_is_set(&argp, "scd_enable") && mvx_argp_get_int(&argp, "scd_enable", 0) == 0));
        encoder.setAdaptiveIntraBlock(mvx_argp_get_int(&argp, "adaptive_intra_block", 0));
    }
    if (mvx_argp_is_set(&argp, "scd_enable") || mvx_argp_is_set(&argp, "adaptive_intra_block")) {
        uint32_t scd_enable = mvx_argp_get_int(&argp, "scd_enable", 0);
        // If adaptive_intra_block is open, the scd_enable will open too.
        if (mvx_argp_is_set(&argp, "adaptive_intra_block") && mvx_argp_get_int(&argp, "adaptive_intra_block", 0)) {
            scd_enable = 1;
        }
        encoder.setEnableSCD(scd_enable);
    }
    if (mvx_argp_is_set(&argp, "scd_percent") && mvx_argp_is_set(&argp, "scd_enable")) {
        assert(mvx_argp_get_int(&argp, "scd_percent", 0) >=0 && mvx_argp_get_int(&argp, "scd_percent", 0) <= 100);
        encoder.setScdPercent(mvx_argp_get_int(&argp, "scd_percent", 0));
    }
    if (mvx_argp_is_set(&argp, "scd_threshold") && mvx_argp_is_set(&argp, "scd_enable")) {
        assert(mvx_argp_get_int(&argp, "scd_threshold", 0) >= 0 && mvx_argp_get_int(&argp, "scd_threshold", 0) <= 2047);
        encoder.setScdThreshold(mvx_argp_get_int(&argp, "scd_threshold", 0));
    }
    if (mvx_argp_is_set(&argp, "aq_ssim_en")) {
        encoder.setEnableAQSsim(mvx_argp_get_int(&argp, "aq_ssim_en", 0));
    }
    if (mvx_argp_is_set(&argp, "aq_neg_ratio") && mvx_argp_is_set(&argp, "aq_ssim_en")) {
        assert(mvx_argp_get_int(&argp, "aq_neg_ratio", 0) >= 0 && mvx_argp_get_int(&argp, "aq_neg_ratio", 0) <= 63);
        encoder.setAQNegRatio(mvx_argp_get_int(&argp, "aq_neg_ratio", 0));
    }
    if (mvx_argp_is_set(&argp, "aq_pos_ratio") && mvx_argp_is_set(&argp, "aq_ssim_en")) {
        assert(mvx_argp_get_int(&argp, "aq_pos_ratio", 0) >= 0 && mvx_argp_get_int(&argp, "aq_pos_ratio", 0) <= 63);
        encoder.setAQPosRatio(mvx_argp_get_int(&argp, "aq_pos_ratio", 0));
    }
    if (mvx_argp_is_set(&argp, "aq_qpdelta_lmt") && mvx_argp_is_set(&argp, "aq_ssim_en")) {
        assert(mvx_argp_get_int(&argp, "aq_qpdelta_lmt", 0) >= 0 && mvx_argp_get_int(&argp, "aq_qpdelta_lmt", 0) <=7);
        encoder.setAQQPDeltaLmt(mvx_argp_get_int(&argp, "aq_qpdelta_lmt", 0));
    }
    if (mvx_argp_is_set(&argp, "aq_init_frm_avg_svar") && mvx_argp_is_set(&argp, "aq_ssim_en")) {
        assert(mvx_argp_get_int(&argp, "aq_init_frm_avg_svar", 0) >= 0 && mvx_argp_get_int(&argp, "aq_init_frm_avg_svar", 0) <= 15);
        encoder.setAQInitFrmAvgSvar(mvx_argp_get_int(&argp, "aq_init_frm_avg_svar", 0));
    }
    if (mvx_argp_is_set(&argp, "huff_table")) {
        struct v4l2_mvx_huff_table huff_table;
        uint8_t bits[16] = {0};
        uint8_t vals[162] = {0};
        FILE *file = fopen(mvx_argp_get(&argp, "huff_table", 0), "r");
        if (!file) {
            cerr<<"fail to open huff table file!"<<endl;
            exit(1);
        }
        memset(&huff_table, 0, sizeof(struct v4l2_mvx_huff_table));
        if (encode_get_huffman_table(file, V4L2_OPT_HUFFMAN_TABLE_DC_LUMA, bits, vals)) {
            huff_table.type |= V4L2_OPT_HUFFMAN_TABLE_DC_LUMA;
            memcpy(&huff_table.dc_luma_code_lenght, bits, sizeof(bits));
            memcpy(&huff_table.dc_luma_table, vals, sizeof(vals));
        }
        if (encode_get_huffman_table(file, V4L2_OPT_HUFFMAN_TABLE_AC_LUMA, bits, vals)) {
            huff_table.type |= V4L2_OPT_HUFFMAN_TABLE_AC_LUMA;
            memcpy(&huff_table.ac_luma_code_lenght, bits, sizeof(bits));
            memcpy(&huff_table.ac_luma_table, vals, sizeof(vals));
        }
        if (encode_get_huffman_table(file, V4L2_OPT_HUFFMAN_TABLE_DC_CHROMA, bits, vals)) {
            huff_table.type |= V4L2_OPT_HUFFMAN_TABLE_DC_CHROMA;
            memcpy(&huff_table.dc_chroma_code_lenght, bits, sizeof(bits));
            memcpy(&huff_table.dc_chroma_table, vals, sizeof(vals));
        }
        if (encode_get_huffman_table(file, V4L2_OPT_HUFFMAN_TABLE_AC_CHROMA, bits, vals)) {
            huff_table.type |= V4L2_OPT_HUFFMAN_TABLE_AC_CHROMA;
            memcpy(&huff_table.ac_chroma_code_lenght, bits, sizeof(bits));
            memcpy(&huff_table.ac_chroma_table, vals, sizeof(vals));
        }
        encoder.setJPEGHufftable(&huff_table);
    }
    if (mini_height >= 64) {
        encoder.setMiniHeight(mini_height);
    }
    if (mvx_argp_is_set(&argp, "visible_width"))
    {
        encoder.setVisibleWidth(mvx_argp_get_int(&argp, "visible_width", 0));
    }
    if (mvx_argp_is_set(&argp, "visible_height"))
    {
        encoder.setVisibleHeight(mvx_argp_get_int(&argp, "visible_height", 0));
    }
    if (outputFormat == V4L2_PIX_FMT_JPEG && mvx_argp_is_set(&argp, "qscale"))
    {
        encoder.setJPEGQuality(mvx_argp_get_int(&argp, "qscale", 0));
    }
    if (outputFormat == V4L2_PIX_FMT_JPEG && mvx_argp_is_set(&argp, "qscale_luma"))
    {
        encoder.setJPEGQualityLuma(mvx_argp_get_int(&argp, "qscale_luma", 0));
    }
    if (outputFormat == V4L2_PIX_FMT_JPEG && mvx_argp_is_set(&argp, "qscale_chroma"))
    {
        encoder.setJPEGQualityChroma(mvx_argp_get_int(&argp, "qscale_chroma", 0));
    }
    if (mvx_argp_is_set(&argp, "pframes"))
    {
        encoder.setPFrames(mvx_argp_get_int(&argp, "pframes", 0));
    }
    if (mvx_argp_is_set(&argp, "bframes"))
    {
        encoder.setBFrames(mvx_argp_get_int(&argp, "bframes", 0));
    }
    if (mvx_argp_is_set(&argp, "packet_mbs"))
    {
        encoder.setSliceSpacing(mvx_argp_get_int(&argp, "packet_mbs", 0));
    }
    if (mvx_argp_is_set(&argp, "constrained_intra_pred"))
    {
        encoder.setConstrainedIntraPred(mvx_argp_get_int(&argp, "constrained_intra_pred", 0));
    }
    if (mvx_argp_is_set(&argp, "nQpMinI"))
    {
        encoder.setEncMinQPI(mvx_argp_get_int(&argp, "nQpMinI", 0));
    }
    if (mvx_argp_is_set(&argp, "nQpMaxI"))
    {
        encoder.setEncMaxQPI(mvx_argp_get_int(&argp, "nQpMaxI", 0));
    }
    if (mvx_argp_is_set(&argp, "init_qpi"))
    {
        encoder.setEncInitQPI(mvx_argp_get_int(&argp, "init_qpi", 0));
    }
    if (mvx_argp_is_set(&argp, "init_qpp"))
    {
        encoder.setEncInitQPP(mvx_argp_get_int(&argp, "init_qpp", 0));
    }
    if (mvx_argp_is_set(&argp, "sao_luma_dis"))
    {
        encoder.setEncSAOluma(mvx_argp_get_int(&argp, "sao_luma_dis", 0));
    }
    if (mvx_argp_is_set(&argp, "sao_chroma_dis"))
    {
        encoder.setEncSAOchroma(mvx_argp_get_int(&argp, "sao_chroma_dis", 0));
    }
    if (mvx_argp_is_set(&argp, "qp_delta_i_p"))
    {
        encoder.setEncQPDeltaIP(mvx_argp_get_int(&argp, "qp_delta_i_p", 0));
    }
    if (mvx_argp_is_set(&argp, "ref_rb_en"))
    {
        encoder.setEncRefRbEn(mvx_argp_get_int(&argp, "ref_rb_en", 0));
    }
    if (mvx_argp_is_set(&argp, "rc_qp_clip_top"))
    {
        encoder.setEncRCClipTop(mvx_argp_get_int(&argp, "rc_qp_clip_top", 0));
    }
    if (mvx_argp_is_set(&argp, "rc_qp_clip_bottom"))
    {
        encoder.setEncRCClipBot(mvx_argp_get_int(&argp, "rc_qp_clip_bottom", 0));
    }
    if (mvx_argp_is_set(&argp, "qpmap_clip_top"))
    {
        encoder.setEncQpmapClipTop(mvx_argp_get_int(&argp, "qpmap_clip_top", 0));
    }
    if (mvx_argp_is_set(&argp, "qpmap_clip_bottom"))
    {
        encoder.setEncQpmapClipBot(mvx_argp_get_int(&argp, "qpmap_clip_bottom", 0));
    }
    if (mvx_argp_is_set(&argp, "xsearch_range"))
    {
        encoder.setHorizontalMVSearchRange(mvx_argp_get_int(&argp, "xsearch_range", 0));
    }
    if (mvx_argp_is_set(&argp, "ysearch_range"))
    {
        encoder.setVerticalMVSearchRange(mvx_argp_get_int(&argp, "ysearch_range", 0));
    }
    if (mvx_argp_is_set(&argp, "profiling"))
    {
        encoder.setEncProfiling(mvx_argp_get_int(&argp, "profiling", 0));
    }
    if (mvx_argp_is_set(&argp, "maxqp"))
    {
        encoder.setEncMaxQP(mvx_argp_get_int(&argp, "maxqp", 0));
    }
    if (mvx_argp_is_set(&argp, "minqp"))
    {
        encoder.setEncMinQP(mvx_argp_get_int(&argp, "minqp", 0));
    }
    if (mvx_argp_is_set(&argp, "tier"))
    {
        encoder.setProfile(mvx_argp_get_int(&argp, "tier", 0));
    }
    if (mvx_argp_is_set(&argp, "level"))
    {
        encoder.setLevel(mvx_argp_get_int(&argp, "level", 0));
    }
    if (mvx_argp_is_set(&argp, "fps"))
    {
        encoder.setFramerate(mvx_argp_get_int(&argp, "fps", 0) << 16);
        /* encoder.setFramerate(mvx_argp_get_int(&argp, "fps", 0)); */
    }
    if (mvx_argp_is_set(&argp, "bitdepth"))
    {
        encoder.setEncBitdepth(mvx_argp_get_int(&argp, "bitdepth", 0));
    }
    if (mvx_argp_is_set(&argp, "ecm"))
    {
        encoder.setH264EntropyCodingMode(mvx_argp_get_int(&argp, "ecm", 0));
    }
    if (mvx_argp_is_set(&argp, "fixedqp"))
    {
        encoder.setEncFixedQP(mvx_argp_get_int(&argp, "fixedqp", 0));
    }
    if (mvx_argp_is_set(&argp, "qpi"))
    {
        encoder.setEncFixedQPI(mvx_argp_get_int(&argp, "qpi", 0));
    }
    if (mvx_argp_is_set(&argp, "qpb"))
    {
        encoder.setEncFixedQPB(mvx_argp_get_int(&argp, "qpb", 0));
    }
    if (mvx_argp_is_set(&argp, "qpp"))
    {
        encoder.setEncFixedQPP(mvx_argp_get_int(&argp, "qpp", 0));
    }
    if (mvx_argp_is_set(&argp, "crop_left"))
    {
        encoder.setCropLeft(mvx_argp_get_int(&argp, "crop_left", 0));
    }
        if (mvx_argp_is_set(&argp, "crop_right"))
    {
        encoder.setCropRight(mvx_argp_get_int(&argp, "crop_right", 0));
    }
    if (mvx_argp_is_set(&argp, "crop_top"))
    {
        encoder.setCropTop(mvx_argp_get_int(&argp, "crop_top", 0));
    }
    if (mvx_argp_is_set(&argp, "crop_bottom"))
    {
        encoder.setCropBottom(mvx_argp_get_int(&argp, "crop_bottom", 0));
    }
    if (mvx_argp_is_set(&argp, "input_thread"))
    {
        encoder.setInputThread(mvx_argp_get_int(&argp, "input_thread", 0));
    }
    printf("Encoder set parameters----------1\n");
    if (mvx_argp_is_set(&argp, "enc_stats_mode") || mvx_argp_is_set(&argp, "enc_stats_cfg"))
    {
        assert(!(mvx_argp_is_set(&argp, "enc_stats_mode") && mvx_argp_is_set(&argp, "enc_stats_cfg")));
        uint32_t roundup, is_hevcenc;
        uint32_t mms_buffer_size, bitcost_buffer_size, qp_buffer_size;
        if (outputFormat == V4L2_PIX_FMT_H264) {
            roundup = 16;
            is_hevcenc = 0;
        } else if (outputFormat == V4L2_PIX_FMT_HEVC) {
            roundup = 64;
            is_hevcenc = 1;
        } else {
            assert( 0 && "enc stats mode only support on h264enc/hevcenc now!");
        }
        mms_buffer_size =
                ((((width + roundup - 1)/roundup)<<is_hevcenc) * (((height+ roundup - 1)/roundup)<<is_hevcenc)) * 4;
        bitcost_buffer_size =
                ((((width + roundup - 1)/roundup)<<is_hevcenc) * (((height+ roundup - 1)/roundup)<<is_hevcenc)) * 2;
        roundup = 16;
        qp_buffer_size =
                (((width + roundup - 1)/roundup) * ((height + roundup - 1)/roundup));

        encoder.setStatsSize(mms_buffer_size, bitcost_buffer_size, qp_buffer_size);
        if (mvx_argp_is_set(&argp, "enc_stats_mode")) {
            encoder.setStatsMode(mvx_argp_get_int(&argp, "enc_stats_mode", 0));
        } else if (mvx_argp_is_set(&argp, "enc_stats_cfg")){
            char *sub_buf;
            const char *enc_stats_cfg = mvx_argp_get(&argp, "enc_stats_cfg", 0);
            FILE *file = fopen(enc_stats_cfg, "r");
            if (NULL == file)
            {
                cerr << "Could not read encoder stats cfg file!" <<endl;
                return 1;
            }

            InputFileFrame *input = dynamic_cast<InputFileFrame*>(inputFile);
            if (input->enc_stats_list == NULL) {
                input->enc_stats_list = new enc_stats_list_t();
            }

            while (fgets(cfg_file_line_buf, CFG_FILE_LINE_SIZE, file))
            {
                sub_buf = strtok(cfg_file_line_buf, ",");
                while(sub_buf != NULL)
                {
                    int reset_pic;
                    int reset_cfg;
                    if (2 == sscanf(sub_buf, "[%d-%d]", &reset_pic, &reset_cfg))
                    {
                      struct v4l2_enc_stats_cfg tmp_enc_stats;
                      tmp_enc_stats.reset_pic = reset_pic;
                      tmp_enc_stats.reset_cfg = reset_cfg;
                      input->enc_stats_list->push(tmp_enc_stats);
                    } else {
                      printf("--enc_stats_cfg file format error:[%s], should be [int-int]\n", sub_buf);
                      return 1;
                    }
                    sub_buf = strtok(NULL, ",");
                }
            }
            fclose(file);
        }
    }
    if (mvx_argp_is_set(&argp, "memory"))
    {
        const char *memory_type = mvx_argp_get(&argp, "memory", 0);
        if (strcmp(memory_type, "mmap") == 0) {
            encoder.setMemoryType(V4L2_MEMORY_MMAP);
        } else if (strcmp(memory_type, "dma") == 0) {
            encoder.setMemoryType(V4L2_MEMORY_DMABUF);
        } else {
            cerr<<"didnot support this memory type!!!"<<endl;
        }
    }
    if (mvx_argp_is_set(&argp, "rgb2yuv_color_conversion"))
    {
        int conv_mode = mvx_argp_get_int(&argp, "rgb2yuv_color_conversion", 0);
           if (conv_mode < 0 || conv_mode > 5) {
            conv_mode = 0;
        }
        encoder.setRGBToYUVMode(conv_mode);
    }
    if (mvx_argp_is_set(&argp, "cust_rgb2yuv_range"))
    {
        assert(!mvx_argp_is_set(&argp, "rgb2yuv_color_conversion"));
        struct v4l2_mvx_rgb2yuv_color_conv_coef  conv_coef;
        if(true==color_conversion_parse_coef(mvx_argp_get(&argp, "cust_rgb2yuv_range", 0),&conv_coef))
        encoder.setRGBConvertYUV(&conv_coef);
        else
        cerr << "invalid rgb2yuv coef params,pls check" << endl;
    }
    if (mvx_argp_is_set(&argp, "colour_description_range") || mvx_argp_is_set(&argp, "colour_primaries")
        || mvx_argp_is_set(&argp, "transfer_characteristics") || mvx_argp_is_set(&argp, "matrix_coeff")
        || mvx_argp_is_set(&argp, "time_scale") || mvx_argp_is_set(&argp, "num_units_in_tick")
        || mvx_argp_is_set(&argp, "aspect_ratio_idc") || mvx_argp_is_set(&argp, "sar_width")
        || mvx_argp_is_set(&argp, "sar_height") || mvx_argp_is_set(&argp, "video_format")
        || mvx_argp_is_set(&argp, "sei_mastering_display") || mvx_argp_is_set(&argp, "sei_content_light"))
    {
        struct v4l2_mvx_color_desc color_desc;
        memset(&color_desc, 0, sizeof(color_desc));
        color_desc.range = mvx_argp_get_int(&argp, "colour_description_range", 0);
        color_desc.primaries = mvx_argp_get_int(&argp, "colour_primaries", 0);
        color_desc.transfer = mvx_argp_get_int(&argp, "transfer_characteristics", 0);
        color_desc.matrix = mvx_argp_get_int(&argp, "matrix_coeff", 0);
        color_desc.time_scale = mvx_argp_get_int(&argp, "time_scale", 0);
        color_desc.num_units_in_tick = mvx_argp_get_int(&argp, "num_units_in_tick", 0);
        color_desc.aspect_ratio_idc = mvx_argp_get_int(&argp, "aspect_ratio_idc", 0);
        color_desc.sar_width = mvx_argp_get_int(&argp, "sar_width", 0);
        color_desc.sar_height = mvx_argp_get_int(&argp, "sar_height", 0);
        color_desc.video_format = mvx_argp_get_int(&argp, "video_format", 0);
        if (mvx_argp_is_set(&argp, "sei_mastering_display")) {
            uint32_t x[3];
            uint32_t y[3];
            uint32_t wp_x, wp_y;
            uint32_t max_dml, min_dml;
            if (10 == sscanf(mvx_argp_get(&argp, "sei_mastering_display", 0),
                    "SEI_MS=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                     &x[0], &y[0], &x[1], &y[1], &x[2], &y[2], &wp_x, &wp_y, &max_dml, &min_dml)) {
                color_desc.display.r.x = x[0];
                color_desc.display.r.y = y[0];
                color_desc.display.g.x = x[1];
                color_desc.display.g.y = y[1];
                color_desc.display.b.x = x[2];
                color_desc.display.b.y = y[2];
                color_desc.display.w.x = wp_x;
                color_desc.display.w.y = wp_y;
                color_desc.display.luminance_min = min_dml;
                color_desc.display.luminance_max = max_dml;
                color_desc.flags |= V4L2_BUFFER_PARAM_COLOUR_FLAG_MASTERING_DISPLAY_DATA_VALID;
            }
        }
        if (mvx_argp_is_set(&argp, "sei_content_light")) {
            uint32_t max_cl;
            uint32_t min_pal;
            if (2 == sscanf(mvx_argp_get(&argp, "sei_content_light", 0), "SEI_CL=%d,%d", &max_cl, &min_pal)) {
                color_desc.content.luminance_average = min_pal;
                color_desc.content.luminance_max = max_cl;
                color_desc.flags |= V4L2_BUFFER_PARAM_COLOUR_FLAG_CONTENT_LIGHT_DATA_VALID;
            }
        }
        encoder.setVuiColourDesc(&color_desc);
    }
    if (mvx_argp_is_set(&argp, "sei_user_data_unregistered")) {
        const char *user_data = mvx_argp_get(&argp, "sei_user_data_unregistered", 0);
        char *data_loc;
        int user_data_len;
        uint32_t uuid_l0, uuid_l1, uuid_l2, uuid_l3;
        struct v4l2_sei_user_data sei_user_data;
        user_data_len = strlen(user_data);
        if (user_data_len > (256-1)) {
            cout<<"ERROR: host input error, user_data_unregistered string info larger than 256"<<endl;;
        }
         data_loc = strstr(const_cast<char*>(user_data), "+");
        if (data_loc == NULL) {
            cout<<"ERROR: host input error, user_data_unregistered do not have seperate character : +"<<endl;
        }
        if (data_loc - user_data != 35) {
            cout<<"ERROR: host input error, user_data_unregistered's uuid is not 16 bytes, or format error"<<endl;
        }
        memset( &sei_user_data, 0, sizeof( sei_user_data ) );
        if (4 == sscanf(user_data, "%x-%x-%x-%x+", &uuid_l3, &uuid_l2, &uuid_l1, &uuid_l0)) {
            sei_user_data.uuid[0] = uuid_l3>>24;
            sei_user_data.uuid[1] = (uuid_l3>>16) & 0xff;
            sei_user_data.uuid[2] = (uuid_l3>>8) & 0xff;
            sei_user_data.uuid[3] = uuid_l3 & 0xff;
            sei_user_data.uuid[4] = uuid_l2>>24;
            sei_user_data.uuid[5] = (uuid_l2>>16) & 0xff;
            sei_user_data.uuid[6] = (uuid_l2>>8) & 0xff;
            sei_user_data.uuid[7] = uuid_l2 & 0xff;
            sei_user_data.uuid[8] = uuid_l1>>24;
            sei_user_data.uuid[9] = (uuid_l1>>16) & 0xff;
            sei_user_data.uuid[10] = (uuid_l1>>8) & 0xff;
            sei_user_data.uuid[11] = uuid_l1 & 0xff;
            sei_user_data.uuid[12] = uuid_l0>>24;
            sei_user_data.uuid[13] = (uuid_l0>>16) & 0xff;
            sei_user_data.uuid[14] = (uuid_l0>>8) & 0xff;
            sei_user_data.uuid[15] = uuid_l0 & 0xff;
        } else {
            cout<<"ERROR: invalid wrong userdata format"<<endl;
        }
        for (int i=36; i<user_data_len; i++) {
            sei_user_data.user_data[i-36] = user_data[i];
        }
        sei_user_data.user_data_len = user_data_len - 36;
        sei_user_data.flags = V4L2_BUFFER_PARAM_USER_DATA_UNREGISTERED_VALID;
        encoder.setSeiUserData(&sei_user_data);
    }
    if (mvx_argp_is_set(&argp, "hrd_buffer_size")) {
        encoder.setHRDBufferSize(mvx_argp_get_int(&argp, "hrd_buffer_size", 0));
    }
    if (mvx_argp_is_set(&argp, "ltr_mode")) {
        int ltr_mode = mvx_argp_get_int(&argp, "ltr_mode", 0);
        bool is_gop_set = mvx_argp_is_set(&argp, "gop");
        int gop = 0;
        if (is_gop_set) {
            gop = mvx_argp_get_int(&argp, "gop", 0);
        }
        if (ltr_mode >= 1 && ltr_mode <= 4) {
            if (!(is_gop_set && gop == 2)){
                cerr << "(ltr_mode >= 1 && ltr_mode <= 4), but gop_type is not MVE_OPT_GOP_TYPE_LOW_DELAY!" << endl;
                return 1;
            }
        }
        if (ltr_mode == 5 || ltr_mode == 6) {
            if (!(is_gop_set && gop == 1)){
                cerr << "(ltr_mode == 5 or 6), but gop_type is not MVE_OPT_GOP_TYPE_BIDIRECTIONAL!" << endl;
                return 1;
            }
        }
        if (ltr_mode == 7 || ltr_mode == 8) {
            if (!(is_gop_set && gop == 3)){
                cerr << "(ltr_mode == 7 or 8), but gop_type is not MVE_OPT_GOP_TYPE_PYRAMID!" << endl;
                return 1;
            }
        }
        if (mvx_argp_is_set(&argp, "ltr_period")) {
            int ltr_period = mvx_argp_get_int(&argp, "ltr_period", 0);
            switch (ltr_mode) {
                case 1: {
                    if (ltr_period < 1 || ltr_period > 255) {
                        cerr<<"ltr_period can not less than 1 or greater than 255 under long term LDP method 1!"<<endl;
                        return 1;
                    }
                    break;
                }
                case 2: {
                    if (ltr_period < 2 || ltr_period > 255) {
                        cerr<<"ltr_period can not less than 2 or greater than 255 under long term LDP method 2!"<<endl;
                        return 1;
                    }
                    break;
                }
                case 3:
                case 5: {
                    if (!mvx_argp_is_set(&argp, "bframes") || ltr_period < 1 || ltr_period > 255/(mvx_argp_get_int(&argp, "bframes", 0) + 1)) {
                        cerr<<"--ltr_period can not less than 1 or greater than 255/(options_p->bframes.val+1), and --bframes must be assigned under long term LDB method 3 or 5!"<<endl;
                        return 1;
                    }
                    break;
                }
                case 4:
                case 6: {
                    if (!mvx_argp_is_set(&argp, "bframes") || ltr_period < 2 || ltr_period > 255/(mvx_argp_get_int(&argp, "bframes", 0) + 1)) {
                        cerr<<"--ltr_period can not less than 1 or greater than 255/(options_p->bframes.val+1), and --bframes must be assigned under long term LDB method 4 or 6!"<<endl;
                        return 1;
                    }
                    break;
                }
                case 7: {
                    if (ltr_period < 1 || ltr_period > 63) {
                        cerr<<"ltr_period can not less than 1 or greater than 63 under long term LDP method 7!"<<endl;
                        return 1;
                    }
                    break;
                }
                case 8: {
                    if (ltr_period < 2 || ltr_period > 63) {
                        cerr<<"ltr_period can not less than 1 or greater than 63 under long term LDP method 8!"<<endl;
                        return 1;
                    }
                    break;
                }
                default:{
                    cerr<<"ltr_mode have been assinged with a wrong value!"<<endl;
                    return 1;
                }
            }
        }
        encoder.setLongTermRef(mvx_argp_get_int(&argp, "ltr_mode", 0), mvx_argp_get_int(&argp, "ltr_period", 0));
    } else if (mvx_argp_is_set(&argp, "ltr_period")) {
        cerr << "ltr_period have been set, but ltr_mode do not assinged!" <<endl;
        return 1;
    }
    if (mvx_argp_is_set(&argp, "gop"))
    {
        int gop_type = mvx_argp_get_int(&argp, "gop", 0);
        if (gop_type == 4)
        {
            if (!(mvx_argp_is_set(&argp, "svct3_level1_peroid") && mvx_argp_get_int(&argp, "svct3_level1_peroid", 0) >= 4 && mvx_argp_get_int(&argp, "svct3_level1_peroid", 0) <= 250))
            {
                printf("--gop_type==4 have been set, but --svct3_level1_peroid is wrong or don't set\n");
                exit(1);
            }
        }
        if (gop_type == 5)
        {
            if (!(mvx_argp_is_set(&argp, "gdr_number") && mvx_argp_is_set(&argp, "gdr_period")))
            {
                printf("--gop_type==5 have been set, but --gdr_number or --gdr_period is wrong or don't set\n");
                exit(1);
            }
        }
        encoder.setH264GOPType(gop_type);
    }
    if (mvx_argp_is_set(&argp, "gdr_number") && mvx_argp_is_set(&argp, "gdr_period")) {
        uint32_t frame_height_quad=0;
        if (mvx_argp_is_set(&argp, "rotate") && (rotation % 360 == 90 || rotation % 360 == 270 ))
        {
            frame_height_quad = (width + 31) >> 5;
        } else
        {
            frame_height_quad = (height + 31) >> 5;
        }
        uint32_t gdr_number = mvx_argp_get_int(&argp, "gdr_number", 0);
        if ( gdr_number < 2 || gdr_number > frame_height_quad){
            printf("--gdr_number must be greater than 1 and lower than frame_height_quad(current is %d)\n",frame_height_quad);
            exit(1);
        }
        if( !(mvx_argp_get_int(&argp, "gop", 0) && mvx_argp_get_int(&argp, "gop", 0) == 5)){
            printf("--gdr_number have been set, but gop_type != 5\n");
            exit(1);
        }
        encoder.setGDRnumber(gdr_number);
        encoder.setGDRperiod(mvx_argp_get_int(&argp, "gdr_period", 0));
    }

    if (mvx_argp_is_set(&argp, "svct3_level1_peroid"))
    {
        int svct3 = mvx_argp_get_int(&argp, "svct3_level1_peroid", 0);
        if ( svct3 < 4 || svct3 > 250 || svct3 % 2 ==1 )
        {
            printf("--svct3_level1_peroid must be an even number or larger than 4\n");
            exit(1);
        }
        if( !(mvx_argp_is_set(&argp, "gop") && mvx_argp_get_int(&argp, "gop", 0) == 4) )
        {
            printf("--svct3_level1_peroid have been set, but gop_type != 4\n");
            exit(1);
        }
        encoder.setSvct3Level1Period(svct3);
    }
    if (mvx_argp_is_set(&argp, "intermediate_buffer_size"))
    {
        encoder.setIntermediateBufSize(mvx_argp_get_int(&argp, "intermediate_buffer_size", 0));
    }

    if (mvx_argp_is_set(&argp, "forced_uv_value"))
    {
        assert(mvx_argp_get_int(&argp, "forced_uv_value", 0)>=0 && mvx_argp_get_int(&argp, "forced_uv_value", 0)<=1023);
        encoder.setForcedUVvalue(mvx_argp_get_int(&argp, "forced_uv_value", 0));
    }

    if(Codec::isYUV422(inputFormat) && V4L2_PIX_FMT_MJPEG == outputFormat)
    {
        if (mvx_argp_is_set(&argp, "422to420"))
        {
            encoder.setEncForceChroma(mvx_argp_get_int(&argp, "422to420", 0));
        }
    }

    if (mvx_argp_is_set(&argp, "src_crop_x") && mvx_argp_is_set(&argp, "src_crop_y")
        && mvx_argp_is_set(&argp, "src_crop_width") && mvx_argp_is_set(&argp, "src_crop_height"))
    {

        assert(rotation == 0 && mirror==0);
        assert(mvx_argp_get_int(&argp, "src_crop_x", 0) %2==0);
        assert(mvx_argp_get_int(&argp, "src_crop_y", 0) %2==0);
        assert(mvx_argp_get_int(&argp, "src_crop_width", 0) %2==0);
        assert(mvx_argp_get_int(&argp, "src_crop_height", 0) %2==0);

        struct v4l2_mvx_crop_cfg src_crop;
        src_crop.crop_en =1;
        src_crop.x = mvx_argp_get_int(&argp, "src_crop_x", 0);
        src_crop.y = mvx_argp_get_int(&argp, "src_crop_y", 0);
        src_crop.width = mvx_argp_get_int(&argp, "src_crop_width", 0);
        src_crop.height = mvx_argp_get_int(&argp, "src_crop_height", 0);
        assert(src_crop.x >=0 && src_crop.x < width);
        assert(src_crop.y >=0 && src_crop.y < height);
        assert(src_crop.width >0 && src_crop.width <= width);
        assert((src_crop.width +src_crop.x)  >0 && (src_crop.width+src_crop.x) <= width);
        assert(src_crop.height >0 && src_crop.height <= height);
        assert((src_crop.height +src_crop.y)  >0 && (src_crop.height +src_crop.y) <= height);

        //old crop need to set in the range of src crop region
        if(mvx_argp_is_set(&argp, "crop_left") && mvx_argp_is_set(&argp, "crop_right")
        && mvx_argp_is_set(&argp, "crop_top") && mvx_argp_is_set(&argp, "crop_bottom"))
        {
            assert(mvx_argp_get_int(&argp, "crop_left", 0) < src_crop.width );
            assert(mvx_argp_get_int(&argp, "crop_top", 0) < src_crop.height );
            assert(mvx_argp_get_int(&argp, "crop_right", 0) <= src_crop.width );
            assert(mvx_argp_get_int(&argp, "crop_bottom", 0) <= src_crop.height );
            assert(mvx_argp_get_int(&argp, "crop_right", 0) - mvx_argp_get_int(&argp, "crop_left", 0)  <= src_crop.width );
            assert(mvx_argp_get_int(&argp, "crop_bottom", 0) - mvx_argp_get_int(&argp, "crop_top", 0)  <= src_crop.height );
        }
        encoder.setEncSrcCrop(&src_crop);
    }

    encoder.setFrameBufCnt(mvx_argp_get_int(&argp, "framebuffer_cnt", 0));
    encoder.setBitBufCnt(mvx_argp_get_int(&argp, "bitbuffer_cnt", 0));
    printf("Encoding\n");
    return encoder.stream();
}
