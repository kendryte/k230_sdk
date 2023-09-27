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
#ifndef _VO_CFG_H_
#define _VO_CFG_H_

#include "mapi_vo_api.h"
#include "mapi_sys_api.h"
#include "k_video_comm.h"

#include "k_vo_comm.h"
#include "k_type.h"

#define ENABLE_VO_LAYER   1

#define VO_WIDTH                                                1080
#define VO_HEIGHT                                               1920
#define VO_MAX_FRAME_COUNT                                      5

#define PRIVATE_POLL_SZE                                        (1920 * 1080 * 3 / 2)
#define PRIVATE_POLL_NUM                                        (4)

typedef struct
{
    k_u64 osd_phy_addr;
    void *osd_virt_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
} osd_info;

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
    //only layer0¡¢layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;
} layer_info;

int vo_init();
void vo_enable();

void vo_layer_init(k_u32 width, k_u32 height);
void vo_layer_deinit();

#endif //_VO_CFG_H_
