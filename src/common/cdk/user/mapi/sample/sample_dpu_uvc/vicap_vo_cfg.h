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

#ifndef __VICAP_VO_CFG__
#define __VICAP_VO_CFG__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "k_type.h"
#include "mapi_vo_api.h"
#include "mapi_sys_api.h"
#include "k_video_comm.h"

#include "k_vo_comm.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#define VICAP_ALIGN_2_BYTE 0x10
#define DISPLAY_WITDH  1088
#define DISPLAY_HEIGHT 1920
#define MAX_VO_LAYER_NUM 2

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
    //only layer0、layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;
} layer_info;

// typedef struct
// {
//     k_u16 width[MAX_VO_LAYER_NUM];
//     k_u16 height[MAX_VO_LAYER_NUM];
//     k_u16 rotation[MAX_VO_LAYER_NUM];
//     k_vo_layer layer[MAX_VO_LAYER_NUM];
//     k_bool enable[MAX_VO_LAYER_NUM];
// } k_vicap_vo_layer_conf;

// typedef struct
// {
//     k_u32 buffsize_with_align;
//     k_u32 buffsize;
//     k_char *suffix;
// } buf_size_calc;

typedef struct
{
    k_u32 page_size;
    k_u64 page_mask;
} kd_ts_sys_paga_vi_cap;

k_s32 sample_vo_creat_layer(k_vo_layer chn_id, layer_info *info);
k_s32 sample_connector_init(void);
k_s32 sample_vicap_vo_init(void);
void sample_vicap_disable_vo_layer(k_vo_layer layer);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
