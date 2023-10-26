/**
 * @file k_vo_comm.h
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
#ifndef __K_VO_COMM_H__
#define __K_VO_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_type.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define K_VO_MAX_DEV_NUMS                            (1)
#define K_VO_MAX_CHN_NUMS                            (7)

#define K_VO_DISPLAY_DEV_ID                          0


#define K_VO_DISPLAY_CHN_ID0                         0
#define K_VO_DISPLAY_CHN_ID1                         1
#define K_VO_DISPLAY_CHN_ID2                         2
#define K_VO_DISPLAY_CHN_ID3                         3
#define K_VO_DISPLAY_CHN_ID4                         4
#define K_VO_DISPLAY_CHN_ID5                         5
#define K_VO_DISPLAY_CHN_ID6                         6

#define K_VO_RSV_SYNC_DEPTH                          60

#define K_ERR_VO_INVALID_DEVID                       K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_VO_INVALID_CHNID                       K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_VO_ILLEGAL_PARAM                       K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VO_EXIST                               K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_VO_UNEXIST                             K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VO_NULL_PTR                            K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VO_NOT_CONFIG                          K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_VO_NOT_SUPPORT                         K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_VO_NOT_PERM                            K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VO_NOMEM                               K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VO_NOBUF                               K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VO_BUF_EMPTY                           K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_VO_BUF_FULL                            K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_VO_NOTREADY                            K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VO_BADADDR                             K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_VO_BUSY                                K_DEF_ERR(K_ID_VO, K_ERR_LEVEL_ERROR, K_ERR_BUSY)


typedef enum
{
    K_VO_LAYER0 = 0,
    K_VO_LAYER1 = 1,
    K_VO_LAYER2 = 2,
    K_MAX_VO_LAYER_NUM,
} k_vo_layer;

typedef enum
{
    K_VO_OSD0 = 0,
    K_VO_OSD1 = 1,
    K_VO_OSD2 = 2,
    K_VO_OSD3 = 3,
    K_MAX_VO_OSD_NUM,
} k_vo_osd;

typedef enum
{
    K_VO_BIND_CHANGE_POSTION = 1,
    K_VO_ONLY_CHANGE_PHYADDR = 2,
    K_VO_CHANGE_POSTION = 3,
} k_vo_priv_work_type;

typedef enum
{
    K_ADDR_MODE_ONLY_PING = 0,
    K_ADDR_MODE_ONLY_PANG,
    K_ADDR_MODE_PANG_PANG
} k_addr_select_mode;

typedef enum
{
    K_VO_LAYER_Y_ENDIAN_DODE0 = 0,              // 45670123
    K_VO_LAYER_Y_ENDIAN_DODE1,                  // 76543210
    K_VO_LAYER_Y_ENDIAN_DODE2,                  // 01234567
    K_VO_LAYER_Y_ENDIAN_DODE3,                  // 32107654

} k_vo_layer_y_endian_mode;

typedef enum
{
    K_VO_LAYER_UV_ENDIAN_DODE0 = 0,             //U2 V2 U3 V3 U0 V0 U1 V1
    K_VO_LAYER_UV_ENDIAN_DODE1,
    K_VO_LAYER_UV_ENDIAN_DODE2,
    K_VO_LAYER_UV_ENDIAN_DODE3,
    K_VO_LAYER_UV_ENDIAN_DODE4,
    K_VO_LAYER_UV_ENDIAN_DODE5,
    K_VO_LAYER_UV_ENDIAN_DODE6,
    K_VO_LAYER_UV_ENDIAN_DODE7,

} k_vo_layer_uv_endian_mode;

typedef enum
{
    K_VO_INTF_MIPI = 0,
} k_vo_intf_type;

typedef enum
{
    K_VO_OUT_1080P30,
    K_VO_OUT_1080P60,
} k_vo_intf_sync;


typedef enum
{
    K_VO_LP_MODE,
    K_VO_HS_MODE,
} k_vo_dsi_cmd_mode;

typedef enum
{
    K_VO_OSD_MAP_ORDER = 0,
    K_VO_OSD_MAP_1234_TO_2143 = 2,
    K_VO_OSD_MAP_1234_TO_4321 = 3,

} k_vo_osd_dma_map;

typedef enum
{
    K_DSI_1LAN = 0,
    K_DSI_2LAN = 1,
    K_DSI_4LAN = 3,
} k_dsi_lan_num;


typedef enum
{
    K_BURST_MODE = 0,
    K_NON_BURST_MODE_WITH_SYNC_EVENT = 1,
    K_NON_BURST_MODE_WITH_PULSES = 2,
} k_dsi_work_mode;


typedef enum
{
    K_ROTATION_0 = (0x01L << 0),
    K_ROTATION_90 = (0x01L << 1),
    K_ROTATION_180 = (0x01L << 2),
    K_ROTATION_270 = (0x01L << 3),
} k_vo_rotation;

typedef enum
{
    K_VO_MIRROR_NONE = (0x01L << 4),
    K_VO_MIRROR_HOR = (0x01L << 5),
    K_VO_MIRROR_VER = (0x01L << 6),
    K_VO_MIRROR_BOTH = (0x01L << 7),
} k_vo_mirror_mode;

typedef enum
{
    K_VO_GRAY_ENABLE = (0x01L << 8),
    K_VO_GRAY_DISABLE = (0x01L << 9),
} k_vo_gray_mode;

typedef enum
{
    K_VO_SCALER_ENABLE = (0x01L << 10),
    K_VO_SCALER_DISABLE = (0x01L << 11),

} k_vo_scaler_mode;

typedef struct
{
    k_u32 x;
    k_u32 y;
} k_vo_point;


typedef struct
{
    k_u32 width;
    k_u32 height;
} k_vo_size;


typedef struct
{
    k_vo_size out_size;
    k_u32 stride;
} k_vo_scaler_attr;


typedef struct
{
    k_u32 pclk;
    k_u32 phyclk;
    k_u32 htotal;
    k_u32 hdisplay;
    k_u32 hsync_len;
    k_u32 hback_porch;
    k_u32 hfront_porch;
    k_u32 vtotal;
    k_u32 vdisplay;
    k_u32 vsync_len;
    k_u32 vback_porch;
    k_u32 vfront_porch;
} k_vo_display_resolution;


typedef struct
{
    k_u32 hsync_start;
    k_u32 hsync_stop;
    k_u32 hsync1_start;
    k_u32 hsync1_stop;
    k_u32 hsync2_start;
    k_u32 hsync2_stop;
    k_u32 vsync1_start;
    k_u32 vsync1_stop;
    k_u32 vsync2_start;
    k_u32 vsync2_stop;
} k_vo_sync_attr;


typedef struct
{
    k_u32 n;
    k_u32 m;
    k_u32 voc;
    k_u32 phy_lan_num;
    k_u32 hs_freq;
} k_vo_mipi_phy_attr;


typedef struct
{

    k_u32  bg_color;        //yuv
    k_vo_intf_type intf_type;
    k_vo_intf_sync intf_sync;
    k_vo_display_resolution *sync_info;          //

} k_vo_pub_attr;


typedef struct
{
    k_u32 pre_div;
    k_u32 clk_en;

} k_vo_user_sync_info;

typedef struct
{

    k_vo_size target_size;
    k_pixel_format pixel_format;
    k_u32 stride;
    k_u32 y_phy_addr;

} k_vo_wbc_attr;


typedef struct
{
    k_vo_display_resolution resolution;
    k_dsi_lan_num lan_num;
    k_vo_dsi_cmd_mode cmd_mode;
    k_dsi_work_mode work_mode;
    k_u32 lp_div;
} k_vo_dsi_attr;

typedef struct
{

    k_vo_size target_size;
    k_pixel_format pixel_format;
    k_u32 stride;
    k_u64 y_phy_addr;

} k_vo_wbc_frame_attr;


typedef struct
{
    k_vo_point display_rect;
    k_vo_size img_size;
    k_pixel_format pixel_format;
    k_u32 stride;
    k_u32 func;
    k_vo_scaler_attr scaler_attr;

} k_vo_video_layer_attr;


typedef struct
{
    k_vo_point display_rect;
    k_vo_size img_size;
    k_pixel_format pixel_format;
    k_u32 stride;
    k_u8 global_alptha;
} k_vo_video_osd_attr;


typedef struct
{
    k_u32 draw_en;
    k_u32 line_x_start;
    k_u32 line_y_start;
    k_u32 line_x_end;
    k_u32 line_y_end;
    k_u32 frame_num;
} k_vo_draw_frame;


typedef struct
{
    k_vo_layer layer;
    k_vo_video_layer_attr *attr;

} k_vo_video_layer_attr_p;

typedef struct
{
    k_vo_layer layer;
    k_vo_mirror_mode mode;
} k_vo_mirror_mode_p;

typedef struct
{
    k_vo_layer layer;
    k_vo_rotation mode;
} k_vo_rotation_p;

typedef struct
{
    k_vo_layer layer;
    k_vo_gray_mode gray;
} k_vo_gray_p;

typedef struct
{
    k_vo_layer layer;
    k_vo_scaler_attr *attr;
} k_vo_scaler_attr_p;

typedef struct
{
    k_vo_layer layer;
    k_s32 priority;
} k_vo_priority_p;

typedef struct
{
    k_vo_layer layer;
    k_vo_point *display_pos;
} k_vo_layer_pos_p;

typedef struct
{
    k_vo_osd layer;
    k_u32 grade;
} k_vo_osd_alpha_grade_p;

typedef struct
{
    k_vo_osd layer;
    k_pixel_format format;
} k_vo_osd_pix_format_p;

typedef struct
{
    k_vo_osd layer;
    k_u32 stride;
} k_vo_osd_stride_p;

typedef struct
{
    k_vo_osd layer;
    k_vo_video_osd_attr *attr;
} k_vo_osd_attr_p;

typedef struct
{
    k_vo_osd layer;
    k_u32 rb_swap;
} k_vo_osd_rb_swap_p;

typedef struct
{
    k_vo_osd layer;
    k_vo_osd_dma_map map;
} k_vo_osd_dma_map_p;

typedef struct
{
    k_u8 data[100];
    k_s32 cmd_len;
} k_vo_dsi_send_cmd_p;


typedef struct
{
    k_u8 send_data[10];
    k_u32 rv_data[10];
    k_s32 cmd_len;
} k_vo_dsi_read_cmd_p;

typedef struct
{
    k_vo_osd layer;
    k_u64 phy_addr;
} k_vo_phyaddr;

typedef struct
{
    k_u32 chn_num;
    k_u32 timeout_ms;
    k_video_frame_info info;
} k_vo_chn_vf_info;


/** @} */ /** <!-- ==== VO End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
