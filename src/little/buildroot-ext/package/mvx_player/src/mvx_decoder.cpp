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


bool color_conversion_parse_coef(const char * conv_ceof_str,struct v4l2_mvx_color_conv_coef * conv_coef)
{
    std::vector<int16_t> ceof_list;

    tokenize_values(conv_ceof_str, ':', ceof_list);
    if (ceof_list.size() == 12)
    {
        conv_coef->coef[0][0] = ceof_list[0];
        conv_coef->coef[0][1] = ceof_list[1];
        conv_coef->coef[0][2] = ceof_list[2];

        conv_coef->coef[1][0] = ceof_list[3];
        conv_coef->coef[1][1] = ceof_list[4];
        conv_coef->coef[1][2] = ceof_list[5];

        conv_coef->coef[2][0] = ceof_list[6];
        conv_coef->coef[2][1] = ceof_list[7];
        conv_coef->coef[2][2] = ceof_list[8];


        conv_coef->offset[0] = ceof_list[9];
        conv_coef->offset[1] = ceof_list[10];
        conv_coef->offset[2] = ceof_list[11];

   }
   cout << "color_conversion_parse_coef coef[0][0]( " << conv_coef->coef[0][0]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[0][1]( " << conv_coef->coef[0][1]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[0][2]( " << conv_coef->coef[0][2]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[1][0]( " << conv_coef->coef[1][0]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[1][1]( " << conv_coef->coef[1][1]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[1][2]( " << conv_coef->coef[1][2]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[2][0]( " << conv_coef->coef[2][0]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[2][1]( " << conv_coef->coef[2][1]<< ")" << endl;
   cout << "color_conversion_parse_coef coef[2][2]( " << conv_coef->coef[2][2]<< ")" << endl;
   cout << "color_conversion_parse_coef offset[0] ( " << conv_coef->offset[0]<< ")" << endl;
   cout << "color_conversion_parse_coef offset[1] ( " << conv_coef->offset[1]<< ")" << endl;
   cout << "color_conversion_parse_coef offset[2] ( " << conv_coef->offset[2]<< ")" << endl;

   if (    ( conv_coef->offset[0]<0 || conv_coef->offset[0]>255)
        || ( conv_coef->offset[1]<0 || conv_coef->offset[1]>255)
        || ( conv_coef->offset[2]<0 || conv_coef->offset[2]>255))
   {
        return false;
   }
    return (ceof_list.size() == 12) ? true: false;
}
int main(int argc, const char *argv[])
{
    int ret;
    mvx_argparse argp;
    uint32_t inputFormat;
    uint32_t outputFormat;
    const char *md5_filename = NULL;
    ofstream *md5_os = NULL;

    mvx_argp_construct(&argp);
    mvx_argp_add_opt(&argp, '\0', "dev", true, 1, "/dev/video0", "Device.");
    mvx_argp_add_opt(&argp, 'i', "inputformat", true, 1, "h264", "Pixel format.");
    mvx_argp_add_opt(&argp, 'o', "outputformat", true, 1, "yuv420", "Output pixel format.");
    mvx_argp_add_opt(&argp, 'f', "format", true, 1, "ivf", "Input container format. [ivf, rcv, raw, rv]\n\t\tFor ivf input format will be taken from IVF header.");
    mvx_argp_add_opt(&argp, 's', "strideAlign", true, 1, "1", "Stride alignment.");
    mvx_argp_add_opt(&argp, 0, "stride0", true, 1, "1", "Number of bytes of stride for the first plane.");
    mvx_argp_add_opt(&argp, 0, "stride1", true, 1, "1", "Number of bytes of stride for the second plane if have.");
    mvx_argp_add_opt(&argp, 0, "stride2", true, 1, "1", "Number of bytes of stride for the third plane if have.");
    mvx_argp_add_opt(&argp, 'y', "intbuf", true, 1, "1000000", "Limit of intermediate buffer size");
    mvx_argp_add_opt(&argp, 'm', "md5", true, 1, NULL, "Output md5 file");
    mvx_argp_add_opt(&argp, 'u', "nalu", true, 1, "0", "Nalu format, START_CODES (0) and ONE_NALU_PER_BUFFER (1), ONE_BYTE_LENGTH_FIELD (2), TWO_BYTE_LENGTH_FIELD (3), FOUR_BYTE_LENGTH_FIELD (4).");
    mvx_argp_add_opt(&argp, 'r', "rotate", true, 1, "0", "Rotation, 0 | 90 | 180 | 270");
    mvx_argp_add_opt(&argp, 'd', "downscale", true, 1, "1", "Down Scale, 1 | 2 | 4");
    mvx_argp_add_opt(&argp, 0, "memory", true, 1, "mmap", "support mmap and dma.");
    mvx_argp_add_opt(&argp, 0, "input_thread", true, 1, "0", "read input buffer in other thread.");
    mvx_argp_add_opt(&argp, 0, "dsl_ratio_hor", true, 1, "0", "Horizontal downscale ratio, [1, 256]");
    mvx_argp_add_opt(&argp, 0, "dsl_ratio_ver", true, 1, "0", "Vertical downscale ratio, [1, 128]");
    mvx_argp_add_opt(&argp, 0, "dsl_frame_width", true, 1, "0", "Downscaled frame width in pixels");
    mvx_argp_add_opt(&argp, 0, "dsl_frame_height", true, 1, "0", "Downscaled frame height in pixels");
    mvx_argp_add_opt(&argp, 0, "dsl_pos_mode", true, 1, "0", "Flexible Downscaled original position mode [0, 2], only availble in high precision mode."
                                                                "\t\tValue: 0 [default:x_original=(x_resized + 0.5)/scale - 0.5]"
                                                                "\t\tValue: 1 [x_original=x_reized/scale]"
                                                                "\t\tValue: 2 [x_original=(x_resized+0.5)/scale]");
    mvx_argp_add_opt(&argp, 0, "dsl_nearest_mode", true, 1, "0", "Downscaling Interpolation mode: 0: Bilinear(default), 1: Nearest");
    mvx_argp_add_opt(&argp, 0, "frames", true, 1, "0", "nr of frames to process");
    mvx_argp_add_opt(&argp, 0, "fro", true, 1, "1", "Frame reordering 1 is on (default), 0 is off");
    mvx_argp_add_opt(&argp, 0, "ish", true, 1, "0", "Ignore Stream Headers 1 is on, 0 is off (default)");
    mvx_argp_add_opt(&argp, 0, "trystop", true, 0, "0", "Try if Decoding Stop Command exixts");
    mvx_argp_add_opt(&argp, 0, "one_frame_per_packet", true, 0, "0", "Each input buffer contains one frame.");
    mvx_argp_add_opt(&argp, 0, "framebuffer_cnt", true, 1, NULL, "Number of buffers to use for yuv data");
    mvx_argp_add_opt(&argp, 0, "bitbuffer_cnt", true, 1, NULL, "Number of buffers to use for bitstream data");
    mvx_argp_add_opt(&argp, 0, "interlaced", true, 0, "0", "Frames are interlaced");
    mvx_argp_add_opt(&argp, 0, "color_conversion", true, 1, "0", "decoder color conversion for ycbcr2rgb."
                                                                "\t\tValue: 0 [default:predefined standards bt601]"
                                                                "\t\tValue: 1 [predefined standards bt601f]"
                                                                "\t\tValue: 2 [predefined standards bt709]"
                                                                "\t\tValue: 3 [predefined standards bt709f]"
                                                                "\t\tValue: 4 [predefined standards bt2020]"
                                                                "\t\tValue: 5 [predefined standards bt2020f]");
    mvx_argp_add_opt(&argp, 0, "cust_yuv2rgb_coef", true, 1, "", "customized integer coeffiecents for decoder ycbcr2rgb y2r:u2r:v2r:y2g:u2g:v2g:y2b:u2b:v2b:yoffset:cboffste:croffset");
    mvx_argp_add_opt(&argp, 0, "disable_features", true, 1, 0, "Disable features bitmask:"
                                                                "\t\tb0=AFBC compression, b1=REF caching, b2=Deblock, b3=SAO,b5=Picture Output Removal, "
                                                                "\t\tb6=Pipe, b7=Sleep b8=LegacyAFBC, b9=FilmGrain b12=REFSZ limit");

    mvx_argp_add_opt(&argp, 0, "dst_crop_x", true, 1, 0, "left start x of luma in output image");
    mvx_argp_add_opt(&argp, 0, "dst_crop_y", true, 1, 0, "top start y of luma in output image");
    mvx_argp_add_opt(&argp, 0, "dst_crop_width", true, 1, 0, "cropped width of luma in output image");
    mvx_argp_add_opt(&argp, 0, "dst_crop_height", true, 1, 0, "cropped height of luma in output image");

    mvx_argp_add_opt(&argp, 0, "ad_stats", true, 1, "0", "Enable the output of Assertive Display (AD) statistics for decode (<outfile>.adatst = frame level statistics, <outfile>.thumbnail = luma thumbnail)");

    mvx_argp_add_opt(&argp, 0, "tiled", true, 0, "disabled", "Use tiles for AFBC formats.");
    mvx_argp_add_pos(&argp, "input", false, 1, "", "Input file.");
    mvx_argp_add_pos(&argp, "output", false, 1, "", "Output file.");
    mvx_argp_add_opt(&argp, 0, "seamless", true, 1, "", "seamless mode for mjepg. 0:disable 1:fixed buffer w/h fhd,1:fixed buffer uhd");

    ret = mvx_argp_parse(&argp, argc - 1, &argv[1]);
    if (ret != 0)
    {
        mvx_argp_help(&argp, argv[0]);
        return 1;
    }

    inputFormat = Codec::to4cc(mvx_argp_get(&argp, "inputformat", 0));
    if (inputFormat == 0)
    {
        fprintf(stderr, "Error: Illegal bitstream format. format=%s.\n",
                mvx_argp_get(&argp, "inputformat", 0));
        return 1;
    }

    ifstream is(mvx_argp_get(&argp, "input", 0));
    Input *inputFile;
    if (string(mvx_argp_get(&argp, "format", 0)).compare("ivf") == 0)
    {
        inputFile = new InputIVF(is, inputFormat);
    }
    else if (string(mvx_argp_get(&argp, "format", 0)).compare("rcv") == 0)
    {
        inputFile = new InputRCV(is);
    }
    else if (string(mvx_argp_get(&argp, "format", 0)).compare("rv") == 0)
    {
        inputFile = new InputRV(is);
    }
    else if (string(mvx_argp_get(&argp, "format", 0)).compare("raw") == 0)
    {
        inputFile = new InputFile(is, inputFormat);
    }
    else
    {
        cerr << "Error: Unsupported container format. format=" <<
        mvx_argp_get(&argp, "format", 0) << "." << endl;
        return 1;
    }
    int nalu_format = mvx_argp_get_int(&argp,"nalu",0);
    int rotation = mvx_argp_get_int(&argp,"rotate",0);
    int scale = mvx_argp_get_int(&argp,"downscale",0);
    int frames = mvx_argp_get_int(&argp,"frames",0);
    if (rotation % 90 != 0){
        cerr << "Unsupported rotation:"<<rotation <<endl;
        rotation = 0;
    }
    outputFormat = Codec::to4cc(mvx_argp_get(&argp, "outputformat", 0));
    if (outputFormat == 0)
    {
        fprintf(stderr, "Error: Illegal frame format. format=%s.\n",
                mvx_argp_get(&argp, "outputformat", 0));
        return 1;
    }

    bool interlaced = mvx_argp_is_set(&argp, "interlaced");
    bool tiled = mvx_argp_is_set(&argp, "tiled");
    ofstream os(mvx_argp_get(&argp, "output", 0), ios::binary);
    Output *output;
    if (Codec::isAFBC(outputFormat))
    {
        output = (interlaced) ?
                 new OutputAFBCInterlaced(os, outputFormat, tiled) :
                 new OutputAFBC(os, outputFormat, tiled);
    }
    else
    {
        md5_filename = mvx_argp_get(&argp, "md5", 0);
        if(md5_filename){
            printf("md5_filename is < %s >.\n", md5_filename);
            md5_os = new ofstream(md5_filename, ios::binary);
            if(NULL == md5_os){
                fprintf(stderr, "Error: (NULL == md5_os).\n");
                return 1;
            }
            output = new OutputFileWithMD5(os, outputFormat, *md5_os, inputFormat == V4L2_PIX_FMT_AV1);
        }
        else if( mvx_argp_is_set(&argp, "ad_stats") && mvx_argp_get_int(&argp, "ad_stats", 0) !=0
                && mvx_argp_is_set(&argp, "seamless") && mvx_argp_get_int(&argp, "seamless", 0) !=0)
        {
            std::string thumbnail_file = std::string( mvx_argp_get(&argp, "output", 0)) + ".thumbnail";
            std::string ad_stats_file = std::string( mvx_argp_get(&argp, "output", 0)) + ".stats";
            output = new OutputFileWithADStats(os, outputFormat,ad_stats_file,thumbnail_file,inputFormat == V4L2_PIX_FMT_AV1);
        }
        else {
            output = new OutputFile(os, outputFormat, inputFormat == V4L2_PIX_FMT_AV1);
        }
    }

    Decoder decoder(mvx_argp_get(&argp, "dev", 0), *inputFile, *output);
    if (mvx_argp_is_set(&argp, "intbuf"))
    {
        decoder.setH264IntBufSize(mvx_argp_get_int(&argp, "intbuf", 0));
    }
    if (mvx_argp_is_set(&argp, "fro"))
    {
        decoder.setFrameReOrdering(mvx_argp_get_int(&argp, "fro", 0));
    }
    if (mvx_argp_is_set(&argp, "ish"))
    {
        decoder.setIgnoreStreamHeaders(mvx_argp_get_int(&argp, "ish", 0));
    }
    if (mvx_argp_is_set(&argp, "trystop"))
    {
        decoder.tryStopCmd(true);
    }
    if (mvx_argp_is_set(&argp, "one_frame_per_packet"))
    {
        decoder.setNaluFormat(V4L2_OPT_NALU_FORMAT_ONE_FRAME_PER_BUFFER);
    } else {
        decoder.setNaluFormat(nalu_format);
    }
    if (mvx_argp_is_set(&argp, "input_thread"))
    {
        decoder.setInputThread(mvx_argp_get_int(&argp, "input_thread", 0));
    }
    if (mvx_argp_is_set(&argp, "memory"))
    {
        const char *memory_type = mvx_argp_get(&argp, "memory", 0);
        if (strcmp(memory_type, "mmap") == 0) {
            decoder.setMemoryType(V4L2_MEMORY_MMAP);
        } else if (strcmp(memory_type, "dma") == 0) {
            decoder.setMemoryType(V4L2_MEMORY_DMABUF);
        } else {
            cerr<<"didnot support this memory type!!!"<<endl;
        }
    }

    if (mvx_argp_is_set(&argp, "dsl_frame_width") && mvx_argp_is_set(&argp, "dsl_frame_height")){
        assert(!mvx_argp_is_set(&argp, "dsl_ratio_hor") && !mvx_argp_is_set(&argp, "dsl_ratio_ver"));
        int width = mvx_argp_get_int(&argp, "dsl_frame_width", 0);
        int height = mvx_argp_get_int(&argp, "dsl_frame_height", 0);
        assert(2 <= width && 2 <= height);
        decoder.setDSLFrame(width,height);
    } else if (mvx_argp_is_set(&argp, "dsl_frame_width") || mvx_argp_is_set(&argp, "dsl_frame_height")){
        cerr << "Downscale frame width and height shoule be set together!"<<endl;
    }

    if (mvx_argp_is_set(&argp, "dsl_ratio_hor") || mvx_argp_is_set(&argp, "dsl_ratio_ver")){
        assert(!mvx_argp_is_set(&argp, "dsl_frame_width") && !mvx_argp_is_set(&argp, "dsl_frame_height"));
        int hor = mvx_argp_is_set(&argp, "dsl_ratio_hor")? mvx_argp_get_int(&argp, "dsl_ratio_hor", 0): 1;
        int ver = mvx_argp_is_set(&argp, "dsl_ratio_hor")? mvx_argp_get_int(&argp, "dsl_ratio_ver", 0): 1;
        decoder.setDSLRatio(hor,ver);
    }

    if (mvx_argp_is_set(&argp, "dsl_pos_mode")) {
        assert(mvx_argp_is_set(&argp, "dsl_ratio_hor") || mvx_argp_is_set(&argp, "dsl_ratio_ver") ||
                mvx_argp_is_set(&argp, "dsl_frame_width") || mvx_argp_is_set(&argp, "dsl_frame_height"));
        int mode = mvx_argp_get_int(&argp, "dsl_pos_mode", 0);
        if (mode < 0 || mode > 2) {
            mode = 0;
        }
        decoder.setDSLMode(mode);
    }

    if (mvx_argp_is_set(&argp, "dsl_nearest_mode")) {
      assert(mvx_argp_is_set(&argp, "dsl_ratio_hor") || mvx_argp_is_set(&argp, "dsl_ratio_ver") ||
                mvx_argp_is_set(&argp, "dsl_frame_width") || mvx_argp_is_set(&argp, "dsl_frame_height"));
           int mode = mvx_argp_get_int(&argp, "dsl_nearest_mode", 0);
           if (mode < 0 || mode > 1) {
               mode = 0;
           }
           decoder.setDSLInterpMode(mode);
    }

    if (mvx_argp_is_set(&argp, "disable_features")) {
             decoder.setDisabledFeatures(mvx_argp_get_int(&argp, "disable_features", 0));
    }

    if (mvx_argp_is_set(&argp, "color_conversion"))
    {
        int conv_mode = mvx_argp_get_int(&argp, "color_conversion", 0);
           if (conv_mode < 0 || conv_mode > 5) {
            conv_mode = 0;
        }
        decoder.setColorConversion(conv_mode);
    }

    if (mvx_argp_is_set(&argp, "cust_yuv2rgb_coef"))
    {
        assert(!mvx_argp_is_set(&argp, "color_conversion"));
        struct v4l2_mvx_color_conv_coef  conv_coef;
        if(true==color_conversion_parse_coef(mvx_argp_get(&argp, "cust_yuv2rgb_coef", 0),&conv_coef))
        decoder.setCustColorConvCoef(&conv_coef);
        else
        cerr << "invalid  yuv2rgb csd coef params,pls check " << endl;
    }


    if (mvx_argp_is_set(&argp, "dst_crop_x") && mvx_argp_is_set(&argp, "dst_crop_y")
    && mvx_argp_is_set(&argp, "dst_crop_width") && mvx_argp_is_set(&argp, "dst_crop_height"))
    {
        assert(!mvx_argp_is_set(&argp, "dsl_ratio_hor") && !mvx_argp_is_set(&argp, "dsl_ratio_ver"));
        assert(!mvx_argp_is_set(&argp, "dsl_frame_width") && !mvx_argp_is_set(&argp, "dsl_frame_height"));
        assert(rotation == 0);
        assert(mvx_argp_get_int(&argp, "dst_crop_x", 0) %4==0);
        assert(mvx_argp_get_int(&argp, "dst_crop_y", 0) %4==0);
        assert(mvx_argp_get_int(&argp, "dst_crop_width", 0) %4==0);
        assert(mvx_argp_get_int(&argp, "dst_crop_height", 0) %4==0);
        assert(mvx_argp_get_int(&argp, "dst_crop_width", 0) >0);
        assert(mvx_argp_get_int(&argp, "dst_crop_height", 0) >0);

        struct v4l2_mvx_crop_cfg dst_crop;
        dst_crop.crop_en =1;
        dst_crop.x = mvx_argp_get_int(&argp, "dst_crop_x", 0);
        dst_crop.y = mvx_argp_get_int(&argp, "dst_crop_y", 0);
        dst_crop.width = mvx_argp_get_int(&argp, "dst_crop_width", 0);
        dst_crop.height = mvx_argp_get_int(&argp, "dst_crop_height", 0);
        decoder.setDecDstCrop(&dst_crop);
    }

    if (mvx_argp_is_set(&argp, "stride0") || mvx_argp_is_set(&argp, "stride1") ||
        mvx_argp_is_set(&argp, "stride2")) {
        size_t stride[VIDEO_MAX_PLANES] = {0};
        stride[0] = mvx_argp_get_int(&argp, "stride0", 0);
        stride[1] = mvx_argp_get_int(&argp, "stride1", 0);
        stride[2] = mvx_argp_get_int(&argp, "stride2", 0);
        decoder.setStride(stride);
    }
    if(mvx_argp_is_set(&argp, "seamless") && mvx_argp_get_int(&argp, "seamless", 0) !=0)
    {
        unsigned int target_width = 1920;
        unsigned int target_height =1088;
        struct v4l2_mvx_seamless_target seamless;
        seamless.seamless_mode = mvx_argp_get_int(&argp, "seamless", 0);

        if(mvx_argp_get_int(&argp, "seamless", 0) == 1)
        {
            target_width = 1920;
            target_height =1088;
        }
        else if(mvx_argp_get_int(&argp, "seamless", 0) == 2)
        {
            target_width = 3840;
            target_height =2160;
        }
        else if(mvx_argp_get_int(&argp, "seamless", 0) == 3)
        {
            target_width = 4096;
            target_height =2304;
        }

        seamless.target_width     = target_width;
        seamless.target_height    = target_height;
        decoder.setSeamlessTarget(outputFormat,&seamless);

        if(mvx_argp_is_set(&argp, "ad_stats"))
        {
            decoder.setAdStats(mvx_argp_get_int(&argp, "ad_stats", 0));
        }

    }



    decoder.setInterlaced(interlaced);
    decoder.setRotation(rotation);
    decoder.setDownScale(scale);
    decoder.setFrameCount(frames);
    decoder.setFrameBufCnt(mvx_argp_get_int(&argp, "framebuffer_cnt", 0));
    decoder.setBitBufCnt(mvx_argp_get_int(&argp, "bitbuffer_cnt", 0));
    ret = decoder.stream();

    delete inputFile;
    delete output;
    if(md5_os){
        delete md5_os;
    }

    return ret;
}
