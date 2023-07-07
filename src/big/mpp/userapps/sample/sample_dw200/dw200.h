/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DW200_H__
#define __DW200_H__

#include "k_dewarp_comm.h"
#include <stdint.h>

#define BLOCK_SIZE 16
#define VS_PI   3.1415926535897932384626433832795
#define VS_2PI 6.283185307179586476925286766559

#define DEWARP_BUFFER_ALIGNMENT 16
#define PHYSICAL_ADDR_SHIFT  4  // for 34bit address
#define MAX_MAP_SIZE 0xF0000

typedef uint32_t u32;

typedef enum {
    FALSE = 0,
    TRUE = 1
} bool;

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

struct dewarp_single_pixel {
    unsigned char Y, U, V;
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

struct dw200_parameters {
    struct dw200_resolution input_res[2];
    struct dw200_resolution output_res[4];
    struct dewarp_buffer_size roi_start;
    struct dewarp_single_pixel boundary_pixel;
    uint32_t scale_factor;  // dwe scale [1.0, 4.0] << 8
    uint32_t split_horizon_line;
    uint32_t split_vertical_line_up;
    uint32_t split_vertical_line_down;
    uint32_t dewarp_type;
    bool rotation;
    bool hflip;
    bool vflip;
    bool bypass;
    struct fov_parameter fov;
        
    uint32_t vse_inputSelect;
    struct k_vse_crop_size vse_cropSize[3];
    struct k_vse_format_conv_settings vse_format_conv[3];
    bool vse_enableResizeOutput[3];
    struct k_vse_mi_settings mi_settings[3];
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

enum format_t {
    MEDIA_PIX_FMT_YUV422SP = 0,
    MEDIA_PIX_FMT_YUV422I,
    MEDIA_PIX_FMT_YUV420SP,
    MEDIA_PIX_FMT_YUV444,
    MEDIA_PIX_FMT_RGB888,
    MEDIA_PIX_FMT_RGB888P,
    MEDIA_PIX_FMT_RAW8,
    MEDIA_PIX_FMT_RAW12,
};

#define INT_FRAME_DONE      (1 << 0)
#define INT_ERR_STATUS_MASK  0x000000FE
#define INT_ERR_STATUS_SHIFT 1
#define INT_MSK_STATUS_MASK  0x0000FF00
#define INT_MSK_STATUS_SHIFT 8
#define INT_FRAME_BUSY       0x00010000

#endif