/**
 * @file msg_vvi.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2022-10-14
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
 *
 */

#ifndef __MSG_VDSS_H__
#define __MSG_VDSS_H__

#include "k_vo_comm.h"
#include "mpi_vo_api.h"
#include "mpi_sys_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef enum {
    MSG_CMD_MEDIA_SET_BACKLIGHT,
    MSG_CMD_MEDIA_VO_RST,
    MSG_CMD_MEDIA_DSI_TEST_PATTERN,
    MSG_CMD_MEDIA_DSI_SET_ATTR,
    MSG_CMD_MEDIA_DSI_ENABLE,
    MSG_CMD_MEDIA_DSI_SEND_CMD,
    MSG_CMD_MEDIA_PHY_SET_ATTR,
    MSG_CMD_MEDIA_DSI_READ_PKG,
    MSG_CMD_MEDIA_VO_ENABLE,
    MSG_CMD_MEDIA_VO_DISABLE,
    MSG_CMD_MEDIA_VO_INIT,
    MSG_CMD_MEDIA_VO_SET_DEV_ATTR,
    MSG_CMD_MEDIA_VO_USER_SYNC_INFO,
    MSG_CMD_MEDIA_VO_ENABLE_VIDEO_LAYER,
    MSG_CMD_MEDIA_VO_DISABLE_VIDEO_LAYER,
    MSG_CMD_MEDIA_VO_SET_LAYER_ATTR,
    MSG_CMD_MEDIA_VO_SET_LAYER_PRIORITY,
    MSG_CMD_MEDIA_VO_SET_OSD_ATTR,
    MSG_CMD_MEDIA_VO_SET_OSD_ENABLE,
    MSG_CMD_MEDIA_VO_SET_OSD_DISABLE,
    MSG_CMD_MEDIA_VO_DRAW_FRAME,
    MSG_CMD_MEDIA_VO_ENABLE_WBC,
    MSG_CMD_MEDIA_VO_DISABLE_WBC,
    MSG_CMD_MEDIA_VO_SET_WBC_ATTR,
    MSG_CMD_MEDIA_VO_INSTALL_FRAME,
    MSG_CMD_MEDIA_VO_DUMP_FRAME,
    MSG_CMD_MEDIA_VO_RELEASE_FRAME,
} msg_media_vo_cmd_t;


typedef struct
{
    k_u8 data[100];
    k_s32 cmd_len;
}msg_dsi_cmd_t;


typedef struct
{
    k_u8 addr;
    k_u16 cmd_len;
    k_u32 rv_data[20];
}msg_dsi_read_cmd_t;


typedef struct
{
    k_u32 pre_div;
    k_u32 clk_en;
}msg_sync_info_t;

typedef struct
{
    k_vo_layer layer;
    k_vo_video_layer_attr attr;
}msg_video_layer_attr_t;

typedef struct
{
    k_vo_layer layer;
    k_s32 priority;
}msg_layer_priority_t;

typedef struct
{
    k_vo_layer layer;
    k_vo_video_osd_attr attr;
}msg_video_osd_attr_t;

typedef struct
{
    k_vo_layer chn_num;
    k_video_frame_info vf_info;
}msg_insert_frame_t;


typedef struct
{
    k_u32 chn_num;
    k_s32 timeout_ms;
    k_video_frame_info vf_info;
} msg_vo_frame_t;

typedef struct
{
    k_u32  bg_color;        //yuv
    k_vo_intf_type intf_type;
    k_vo_intf_sync intf_sync;
    k_vo_display_resolution sync_info;          //
} msg_vo_pub_attr;



/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VVI_H__ */