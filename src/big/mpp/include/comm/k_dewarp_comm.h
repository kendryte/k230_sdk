/**
 * @file k_dewarp_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-01
 *
 * @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#ifndef __K_DW200_COMM_H__
#define __K_DW200_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include <k_type.h>
#include <k_vicap_comm.h>

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define DW200_MAX_DEV_NUMS        (2)
#define DW200_MAX_CHN_NUMS        (2)

#define DW200_DISPLAY_DEV_ID      1
#define DW200_DISPLAY_CHN_ID      1

typedef k_u32 u32;
typedef k_s32 s32;
typedef k_u8 u8;

struct k_vse_crop_size {
    u32 left;
    u32 right;
    u32 top;
    u32 bottom;
};

struct k_vse_size {
    u32 width;
    u32 height;
};

struct k_vse_format_conv_settings {
    u32 in_format;
    u32 out_format;
};

struct k_vse_mi_settings {
    k_bool enable;
    u32 out_format;
    u32 width;
    u32 height;
    u32 yuvbit;
};

struct k_vse_params {
    u32 src_w;
    u32 src_h;
    u32 in_format;
    u32 in_yuvbit;
    u32 input_select;
    struct k_vse_crop_size crop_size[3];
    struct k_vse_size out_size[3];
    struct k_vse_format_conv_settings format_conv[3];
    k_bool resize_enable[3];
    struct k_vse_mi_settings mi_settings[3];
};

struct k_dwe_hw_info {
    u32 split_line;
    u32 scale_factor;
    u32 in_format;
    u32 out_format;
    u32 in_yuvbit;
    u32 out_yuvbit;
    u32 hand_shake;
    u32 roi_x, roi_y;
    u32 boundary_y, boundary_u, boundary_v;
    u32 map_w, map_h;
    u32 src_auto_shadow, dst_auto_shadow;
    u32 src_w, src_stride, src_h;
    u32 dst_w, dst_stride, dst_h, dst_size_uv;
    u32 split_h, split_v1, split_v2;
};

enum k_dw_pix_format {
    K_DW_PIX_YUV422SP,
    K_DW_PIX_YUV422I,
    K_DW_PIX_YUV420SP,
    K_DW_PIX_YUV444,
    K_DW_PIX_RGB888,
    K_DW_PIX_RGB888P
};

struct k_dw_frame_info {
    u32 width;
    u32 height;
    enum k_dw_pix_format format;
    k_bool bit10;
    u8 alignment;
};

struct k_dw_settings {
    struct k_dw_frame_info input;
    k_bool split_enable;
    u32 split_horizon_line;
    u32 split_vertical_line_up;
    u32 split_vertical_line_down;
    struct k_dw_frame_info output[VICAP_CHN_ID_MAX];
    struct k_vse_crop_size crop[VICAP_CHN_ID_MAX];
    u8 output_enable_mask;
    u32 lut_phy_addr;
    void* lut_user_virt_addr;
    u32 lut_size;
    u32 lut_width;
    u32 lut_height;
    // for statistics purpose
    u8 vdev_id;
};

struct k_dw_load_request {
    k_vicap_vb_info vb_info;
    k_vicap_vb_info output_vb_info[VICAP_CHN_ID_MAX];
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
