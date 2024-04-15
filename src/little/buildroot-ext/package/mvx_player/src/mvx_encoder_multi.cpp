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
#include <pthread.h>
#include <fstream>
#include <sstream>

using namespace std;

struct job
{
    job(const char *dev,
        const string &inputFile, const uint32_t inputFormat,
        const string &outputFile, const uint32_t outputFormat,
        const size_t outputStride, const uint32_t w, const uint32_t h, const uint32_t f) :
        dev(dev),
        inputFile(inputFile),
        inputFormat(inputFormat),
        outputFile(outputFile),
        outputFormat(outputFormat),
        outputStride(outputStride),
        width(w),
        height(h),
        frames(f),
        ret(0)
    {}

    const char *dev;
    string inputFile;
    uint32_t inputFormat;
    string outputFile;
    uint32_t outputFormat;
    size_t outputStride;
    uint32_t width;
    uint32_t height;
    uint32_t frames;
    int ret;
};

void *encodeThread(void *arg)
{
    job *j = static_cast<job *>(arg);

    ifstream is(j->inputFile.c_str());
    ofstream os(j->outputFile.c_str());
    string logf = j->outputFile + ".log";
    ofstream log(logf.c_str());
    size_t stride[VIDEO_MAX_PLANES] = {0};
    InputFileFrame inputFile = InputFileFrame(is, j->inputFormat, j->width, j->height, j->outputStride, stride);
    OutputFile outputFile = OutputFile(os, j->outputFormat);

    Encoder encoder(j->dev, inputFile, outputFile, true, log);
    if (j->frames > 0) {
        encoder.setFrameCount(j->frames);
    }
    j->ret = encoder.stream();

    return j;
}

int main(int argc, const char *argv[])
{
    int ret;
    mvx_argparse argp;
    uint32_t inputFormat;
    uint32_t outputFormat;
    int nsessions;

    mvx_argp_construct(&argp);
    mvx_argp_add_opt(&argp, '\0', "dev", true, 1, "/dev/video0", "Device.");
    mvx_argp_add_opt(&argp, 'i', "inputformat", true, 1, "yuv420", "Pixel format.");
    mvx_argp_add_opt(&argp, 'o', "outputformat", true, 1, "h264", "Output pixel format.");
    mvx_argp_add_opt(&argp, 's', "stride", true, 1, "1", "Stride alignment.");
    mvx_argp_add_opt(&argp, 'w', "width", true, 1, "1280", "input yuv width.");
    mvx_argp_add_opt(&argp, 'h', "height", true, 1, "720", "input yuv height.");
    mvx_argp_add_opt(&argp, 'f', "frames", true, 1, "0", "Specfied frame count to be processed.");
    mvx_argp_add_opt(&argp, 'n', "nsessions", true, 1, "1", "Number of sessions.");
    mvx_argp_add_pos(&argp, "input", false, 1, "", "Input file.");
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
        fprintf(stderr, "Error: Illegal bitstream format. format=%s.\n",
                mvx_argp_get(&argp, "inputformat", 0));
        return 1;
    }

    outputFormat = Codec::to4cc(mvx_argp_get(&argp, "outputformat", 0));
    if (outputFormat == 0)
    {
        fprintf(stderr, "Error: Illegal frame format. format=%s.\n",
                mvx_argp_get(&argp, "outputformat", 0));
        return 1;
    }

    nsessions = mvx_argp_get_int(&argp, "nsessions", 0);

    pthread_t tid[nsessions];
    for (int i = 0; i < nsessions; ++i)
    {
        stringstream ss;
        int ret;

        ss << mvx_argp_get(&argp, "output", 0) << "." << i;
        cout << "output file name " << ss.str() << endl;
        job *j = new job(mvx_argp_get(&argp, "dev", 0),
                         string(mvx_argp_get(&argp, "input", 0)),
                         inputFormat, ss.str(), outputFormat,
                         mvx_argp_get_int(&argp, "stride", 0),
                         mvx_argp_get_int(&argp, "width", 0),
                         mvx_argp_get_int(&argp, "height", 0),
                         mvx_argp_get_int(&argp, "frames", 0));

        ret = pthread_create(&tid[i], NULL, encodeThread, j);
        if (ret != 0)
        {
            throw Exception("Failed to create input thread.");
        }
    }

    ret = 0;
    for (int i = 0; i < nsessions; ++i)
    {
        job *j;

        pthread_join(tid[i], reinterpret_cast<void **>(&j));
        ret += j->ret;
        delete(j);
    }

    return ret;
}

