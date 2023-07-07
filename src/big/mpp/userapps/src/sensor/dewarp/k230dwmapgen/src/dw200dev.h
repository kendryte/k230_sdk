/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
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
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#ifndef _DW200_HAL_DW200DEV_H_
#define _DW200_HAL_DW200DEV_H_
#include <stdint.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#endif

#undef ALIGN_UP
#undef ALIGN_DOWN
#undef BLOCK_SIZE
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#define ALIGN_DOWN(x, align) ((x) & ~((align)-1))

#define BLOCK_SIZE 16
#define VS_PI   3.1415926535897932384626433832795
#define VS_2PI 6.283185307179586476925286766559

#define DEWARP_BUFFER_ALIGNMENT 16
#define PHYSICAL_ADDR_SHIFT  4  // for 34bit address
#define MAX_MAP_SIZE 0xF0000

#define VSE_OUTPUT_CHANNEL_NUMBER 3
#define VSE_INPUT_CHANNEL_NUMBER 6

// correction types, also used as capabilities
enum {
    DEWARP_MODEL_LENS_DISTORTION_CORRECTION = 1 << 0,
    DEWARP_MODEL_FISHEYE_EXPAND             = 1 << 1,
    DEWARP_MODEL_SPLIT_SCREEN               = 1 << 2,
    DEWARP_MODEL_FISHEYE_DEWARP             = 1 << 3,
    DEWARP_MODEL_PERSPECTIVE                = 1 << 4,
};

enum {
    DEWARP_BUFFER_TYPE_SRC = 0,
    DEWARP_BUFFER_TYPE_DST,
};

struct dewarp_buffer_size {
    uint32_t width;
    uint32_t height;
};

struct dw200_resolution {
    uint32_t yuvbit;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t enable;
};
struct fov_parameter {
    double offAngleUL;
    double offAngleUR;
    double offAngleDL;
    double offAngleDR;
    double fovUL;
    double fovUR;
    double fovDL;
    double fovDR;
    int panoAtWin;
    double centerOffsetRatioUL;
    double centerOffsetRatioUR;
    double centerOffsetRatioDL;
    double centerOffsetRatioDR;
    double circleOffsetRatioUL;
    double circleOffsetRatioUR;
    double circleOffsetRatioDL;
    double circleOffsetRatioDR;

};
struct dewarp_single_pixel {
    unsigned char Y, U, V;
};

struct dewarp_distortion_map {
    uint32_t index;  // two map, select as 0 or 1.
    uint32_t userMapSize;   //  0: set camera matrix,  calculate map at driver
                            // !0: set distortion map,  calculated by user.
    double camera_matrix[9];
    double perspective_matrix[9];
    double distortion_coeff[8];
    uint32_t* pUserMap;
};


#endif  // _DW200_HAL_DW200DEV_H_
