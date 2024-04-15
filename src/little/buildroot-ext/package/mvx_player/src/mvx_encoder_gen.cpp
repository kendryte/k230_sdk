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
#include <iomanip>

using namespace std;

int main(int argc, const char *argv[])
{
    int ret;
    mvx_argparse argp;
    uint32_t inputFormat;
    uint32_t outputFormat;

    mvx_argp_construct(&argp);
    mvx_argp_add_opt(&argp, '\0', "dev", true, 1, "/dev/video0", "Device.");
    mvx_argp_add_opt(&argp, 'i', "inputformat", true, 1, "yuv420", "Pixel format.");
    mvx_argp_add_opt(&argp, 'o', "outputformat", true, 1, "h264", "Output pixel format.");
    mvx_argp_add_opt(&argp, 'f', "format", true, 1, "ivf", "Output container format. [ivf, raw]");
    mvx_argp_add_opt(&argp, 'w', "width", true, 1, "1920", "Width.");
    mvx_argp_add_opt(&argp, 'h', "height", true, 1, "1080", "Height.");
    mvx_argp_add_opt(&argp, 's', "stride", true, 1, "1", "Stride alignment.");
    mvx_argp_add_opt(&argp, 'c', "count", true, 1, "100", "Number of frames to generate.");
    mvx_argp_add_opt(&argp, 'x', "exec", true, 1, "1", "Change settings while encoding is ongoing. The main purpose is to exercise code for coverage.");
    mvx_argp_add_opt(&argp, 'v', "fps", true, 1, "24", "Frame rate.");
    mvx_argp_add_opt(&argp, 'r', "bps", true, 1, "2000000", "Bit rate. If it is combined with -x then  the value will later be 1000 and rate control deactivated.");
    mvx_argp_add_opt(&argp, 't', "tier", true, 1, "2", "Profile.");
    mvx_argp_add_opt(&argp, 'l', "level", true, 1, "1", "Level.");
    mvx_argp_add_opt(&argp, 'e', "ecm", true, 1, "1", "0 is CAVLC, 1 is CABAC");
    mvx_argp_add_opt(&argp, 'g', "gop", true, 1, "1", "GOP: 0 is None, 1 is Bidi, 2 is Low delay, 3 is Pyramid");
    mvx_argp_add_opt(&argp, 'd', "imbr", true, 1, "200", "Intra MB refresh.");
    mvx_argp_add_opt(&argp, 'u', "slice", true, 1, "1000", "Slice spacing.");
    mvx_argp_add_opt(&argp, 'a', "chroma", true, 1, "1", "Force chroma format.");
    mvx_argp_add_opt(&argp, 'j', "bitdepth", true, 1, "8", "Set other bitdepth");
    mvx_argp_add_opt(&argp, 'k', "tileCR", true, 1, "2", "Set tile columns and rows");
    mvx_argp_add_opt(&argp, 'y', "bandwidth", true, 1, "10000000", "Set bandwidth limit");
    mvx_argp_add_opt(&argp, 'n', "minqp", true, 1, "0", "H264 min QP");
    mvx_argp_add_opt(&argp, 'm', "maxqp", true, 1, "51", "H264 max QP");
    mvx_argp_add_opt(&argp, 'q', "fixedqp", true, 1, "20", "H264 fixed QP for I P B frames. If it is combined with -x then the value will later be increased with 2.");
    mvx_argp_add_opt(&argp, 'p', "pframes", true, 1, "29", "Number of P frames");
    mvx_argp_add_opt(&argp, 'b', "bframes", true, 1, "1", "Number of B frames");
    mvx_argp_add_opt(&argp, 0, "cip", true, 1, "0", "Constrained Intraprediction, 1 is on, 0 is off");
    mvx_argp_add_opt(&argp, 0, "es", true, 1, "0", "Entropy Sync 1 is on, 0 is off");
    mvx_argp_add_opt(&argp, 0, "tmvp", true, 1, "0", "TMVP 1 is on, 0 is off");
    mvx_argp_add_opt(&argp, 0, "sesc", true, 1, "16", "Stream escaping 1 is on, 0 is off");
    mvx_argp_add_opt(&argp, 0, "hmvsr", true, 1, "16", "Horisontal MV Search Range");
    mvx_argp_add_opt(&argp, 0, "vmvsr", true, 1, "16", "Vertical MV Search Range");
    mvx_argp_add_opt(&argp, 'z', "block", true, 0, "0", "Use video device in blocking mode");
    mvx_argp_add_opt(&argp, 0, "restart_interval", true, 1, "-1", "JPEG restart interval.");
    mvx_argp_add_opt(&argp, 0, "quality", true, 1, "0", "JPEG compression quality. [1-100, 0 - default]");
    mvx_argp_add_pos(&argp, "output", false, 1, "", "Output file.");

    ret = mvx_argp_parse(&argp, argc - 1, &argv[1]);
    if (ret != 0)
    {
        mvx_argp_help(&argp, argv[0]);
        return 1;
    }

    inputFormat = Codec::to4cc(mvx_argp_get(&argp, "inputformat", 0));
    if (inputFormat == 0)
    {
        cerr << "Error: Illegal frame format. format=" <<
        mvx_argp_get(&argp, "inputformat", 0) << "." << endl;
        return 1;
    }

    InputFrame inputFrame = InputFrame(
        inputFormat,
        mvx_argp_get_int(&argp, "width", 0),
        mvx_argp_get_int(&argp, "height", 0),
        mvx_argp_get_int(&argp, "stride", 0),
        mvx_argp_get_int(&argp, "count", 0));

    outputFormat = Codec::to4cc(mvx_argp_get(&argp, "outputformat", 0));
    if (outputFormat == 0)
    {
        cerr << "Error: Illegal bitstream format. format=" <<
        mvx_argp_get(&argp, "outputformat", 0) << "." << endl;
        return 1;
    }

    ofstream os(mvx_argp_get(&argp, "output", 0));
    Output *outputFile;
    if (string(mvx_argp_get(&argp, "format", 0)).compare("ivf") == 0)
    {
        outputFile = new OutputIVF(os, outputFormat,
                                   mvx_argp_get_int(&argp, "width", 0),
                                   mvx_argp_get_int(&argp, "height", 0));
    }
    else if (string(mvx_argp_get(&argp, "format", 0)).compare("raw") == 0)
    {
        outputFile = new OutputFile(os, outputFormat);
    }
    else
    {
        cerr << "Error: Unsupported container format. format=" <<
        mvx_argp_get(&argp, "format", 0) << "." << endl;
        return 1;
    }

    bool nonblock = true;
    if (mvx_argp_is_set(&argp, "block"))
    {
        nonblock = false;
    }

    Encoder encoder(mvx_argp_get(&argp, "dev", 0), inputFrame, *outputFile, nonblock);
    if (mvx_argp_is_set(&argp, "exec"))
    {
        encoder.changeSWEO(mvx_argp_get_int(&argp, "exec", 0));
    }
    if (mvx_argp_is_set(&argp, "bandwidth"))
    {
        encoder.setH264Bandwidth(mvx_argp_get_int(&argp, "bandwidth", 0));
    }
    if (mvx_argp_is_set(&argp, "chroma"))
    {
        encoder.setEncForceChroma(mvx_argp_get_int(&argp, "chroma", 0));
    }
    if (mvx_argp_is_set(&argp, "bitdepth"))
    {
        encoder.setEncBitdepth(mvx_argp_get_int(&argp, "bitdepth", 0));
    }
    if (mvx_argp_is_set(&argp, "tileCR"))
    {
        encoder.setVP9TileCR(mvx_argp_get_int(&argp, "tileCR", 0));
    }
    if (mvx_argp_is_set(&argp, "imbr"))
    {
        encoder.setH264IntraMBRefresh(mvx_argp_get_int(&argp, "imbr", 0));
    }
    if (mvx_argp_is_set(&argp, "slice"))
    {
        encoder.setSliceSpacing(mvx_argp_get_int(&argp, "slice", 0));
    }
    if (mvx_argp_is_set(&argp, "tier"))
    {
        encoder.setProfile(mvx_argp_get_int(&argp, "tier", 0));
    }
    if (mvx_argp_is_set(&argp, "level"))
    {
        encoder.setLevel(mvx_argp_get_int(&argp, "level", 0));
    }
    if (mvx_argp_is_set(&argp, "ecm"))
    {
        encoder.setH264EntropyCodingMode(mvx_argp_get_int(&argp, "ecm", 0));
    }
    if (mvx_argp_is_set(&argp, "gop"))
    {
        encoder.setH264GOPType(mvx_argp_get_int(&argp, "gop", 0));
    }
    /* Set maxQP before minQP, otherwise FW rejects */
    if (mvx_argp_is_set(&argp, "maxqp"))
    {
        encoder.setEncMaxQP(mvx_argp_get_int(&argp, "maxqp", 0));
    }
    if (mvx_argp_is_set(&argp, "minqp"))
    {
        encoder.setEncMinQP(mvx_argp_get_int(&argp, "minqp", 0));
    }
    if (mvx_argp_is_set(&argp, "fixedqp"))
    {
        encoder.setEncFixedQP(mvx_argp_get_int(&argp, "fixedqp", 0));
    }
    if (mvx_argp_is_set(&argp, "cip"))
    {
        encoder.setConstrainedIntraPred(mvx_argp_get_int(&argp, "cip", 0));
    }
    if (mvx_argp_is_set(&argp, "es"))
    {
        encoder.setHEVCEntropySync(mvx_argp_get_int(&argp, "es", 0));
    }
    if (mvx_argp_is_set(&argp, "tmvp"))
    {
        encoder.setHEVCTemporalMVP(mvx_argp_get_int(&argp, "tmvp", 0));
    }
    if (mvx_argp_is_set(&argp, "sesc"))
    {
        encoder.setStreamEscaping(mvx_argp_get_int(&argp, "sesc", 0));
    }
    if (mvx_argp_is_set(&argp, "pframes"))
    {
        encoder.setPFrames(mvx_argp_get_int(&argp, "pframes", 0));
    }
    if (mvx_argp_is_set(&argp, "bframes"))
    {
        encoder.setBFrames(mvx_argp_get_int(&argp, "bframes", 0));
    }
    if (mvx_argp_is_set(&argp, "fps"))
    {
        encoder.setFramerate(mvx_argp_get_int(&argp, "fps", 0) << 16);
        /* encoder.setFramerate(mvx_argp_get_int(&argp, "fps", 0)); */
    }
    if (mvx_argp_is_set(&argp, "bps"))
    {
        encoder.setBitrate(mvx_argp_get_int(&argp, "bps", 0));
    }
    if (mvx_argp_is_set(&argp, "hmvsr"))
    {
        encoder.setHorizontalMVSearchRange(mvx_argp_get_int(&argp, "hmvsr", 0));
    }
    if (mvx_argp_is_set(&argp, "vmvsr"))
    {
        encoder.setVerticalMVSearchRange(mvx_argp_get_int(&argp, "vmvsr", 0));
    }
    if (mvx_argp_is_set(&argp, "restart_interval"))
    {
        encoder.setJPEGRefreshInterval(mvx_argp_get_int(&argp, "restart_interval", 0));
    }
    if (mvx_argp_is_set(&argp, "quality"))
    {
        encoder.setJPEGQuality(mvx_argp_get_int(&argp, "quality", 0));
    }

    ret = encoder.stream();
    delete outputFile;

    return ret;
}
