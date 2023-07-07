/**
 * @file k_venc_comm.h
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
#ifndef __K_VENC_COMM_H__
#define __K_VENC_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_payload_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VENC */
/** @{ */ /** <!-- [VENC] */

#define VENC_MAX_DEV_NUMS        (1)
#define VENC_MAX_CHN_NUMS        (4)

/**
 * @brief Defines the attributes of a VENC channel
 *
 */

typedef enum
{
    VENC_PROFILE_NONE,            /*no profile */
    VENC_PROFILE_H264_BASELINE,   /*h264 baseline profile */
    VENC_PROFILE_H264_MAIN,       /*h264 main profile */
    VENC_PROFILE_H264_HIGH,       /*h264 high profile */
    VENC_PROFILE_H264_HIGH_10,    /*h264 high 10 profile */
    VENC_PROFILE_H265_MAIN,       /*h265 main profile */
    VENC_PROFILE_H265_MAIN_10,    /*h265 main 10 profile */
    VENC_PROFILE_BUTT
} k_venc_profile;

typedef struct
{
    k_payload_type type;          /*stream payload type */
    k_u32 stream_buf_size;        /*single output vb size */
    k_u32 stream_buf_cnt;         /*output vb count */
    k_u32 pic_width;              /*channel picture width */
    k_u32 pic_height;             /*channel picture height */
    k_venc_profile profile;       /*channel profile */
} k_venc_attr;

typedef enum
{
    K_VENC_RC_MODE_CBR = 1,       /*constant bitrate mode */
    K_VENC_RC_MODE_VBR,           /*variable bitrate mode */
    K_VENC_RC_MODE_FIXQP,         /*fixed QP mode */
    K_VENC_RC_MODE_MJPEG_FIXQP,   /*mjpeg fixed QP mode */
    K_VENC_RC_MODE_BUTT
} k_venc_rc_mode;

typedef enum
{
    K_VENC_MIRROR_HORI = 1,
    K_VENC_MIRROR_VERT = 2,
    K_VENC_MIRROR_BUTT
} k_venc_mirror;

typedef struct
{
    k_u32 gop;                      /* the interval of ISLICE. */
    k_u32 stats_time;               /* the rate statistic time, the unit is senconds(s) */
    k_u32 src_frame_rate;           /* the input frame rate of the venc channel */
    k_u32 dst_frame_rate ;          /* the target frame rate of the venc channel */
    k_u32 bit_rate;                 /* the target bitrate */
} k_venc_cbr;

typedef struct
{
    k_u32 gop;                      /* the interval of ISLICE. */
    k_u32 stats_time;               /* the rate statistic time, the unit is senconds(s) */
    k_u32 src_frame_rate;           /* the input frame rate of the venc channel */
    k_u32 dst_frame_rate ;          /* the target frame rate of the venc channel */
    k_u32 max_bit_rate;             /* the max bitrate */
    k_u32 bit_rate;                 /* the target bitrate */
} k_venc_vbr;

typedef struct
{
    k_u32 gop;                      /* the interval of I SLICE. */
    k_u32 src_frame_rate;           /* the input frame rate of the venc channel */
    k_u32 dst_frame_rate;           /* the target frame rate of the venc channel */
    k_u32 i_qp;                     /* qp for I SLICE */
    k_u32 p_qp;                     /* qp for P SLICE */
} k_venc_fixqp;

typedef struct
{
    k_u32 src_frame_rate;           /* the input frame rate of the venc channel */
    k_u32 dst_frame_rate;           /* the target frame rate of the venc channel */
    k_u32 q_factor;                 /* image quality :[1,99] */
} k_venc_mjpeg_fixqp;

typedef struct
{
    k_venc_rc_mode rc_mode;                 /* the type of rc */
    union
    {
        k_venc_cbr cbr;                     /* h264/h265 cbr paramters */
        k_venc_vbr vbr;                     /* h264/h265 vbr paramters */
        k_venc_fixqp fixqp;                 /* h264/h265 fixqp paramters */
        k_venc_mjpeg_fixqp mjpeg_fixqp;     /* mjpeg fixqp paramters */
    };
} k_venc_rc_attr;

typedef struct
{
    k_venc_attr venc_attr;          /* the attribute of video encoder */
    k_venc_rc_attr rc_attr;         /* the attribute of rate  ctrl */
} k_venc_chn_attr;

typedef enum
{
    K_VENC_P_FRAME = 1,
    K_VENC_I_FRAME = 2,
    K_VENC_HEADER = 3,
    K_VENC_BUTT
} k_venc_pack_type;

typedef enum
{
    ENTROPY_MODE_CAVLC = 0,
    ENTROPY_MODE_CABAC,
    ENTROPY_MODE_BUTT
} k_venc_entropy_mode;

typedef struct
{
    k_u64  phys_addr;
    k_u32  len;
    k_u64  pts;
    k_venc_pack_type type;
} k_venc_pack;

typedef struct
{
    k_venc_pack  *pack;
    k_u32  pack_cnt;
} k_venc_stream;

typedef struct
{
    k_u32 u32PicBytesNum;
    k_u32 u32PicCnt;
    k_u32 u32StartQp;
    k_u32 u32MeanQp;
} k_venc_stream_info;


typedef struct
{
    k_u32 cur_packs;
    k_u64 release_pic_pts;
    k_bool end_of_stream;
    k_venc_stream_info stream_info;
} k_venc_chn_status;

typedef struct
{
    k_u32 slice_sao_luma_flag;
    k_u32 slice_sao_chroma_flag;
} k_venc_h265_sao;

typedef struct
{
    k_s32 left;
    k_s32 right;
    k_u32 top;
    k_u32 bottom;
} k_venc_rect;

typedef struct
{
    k_u32 idx;
    k_bool enable;
    k_bool is_abs_qp;
    k_s32 qp;
    k_venc_rect rect;
} k_venc_roi_attr;

typedef struct
{
    k_venc_entropy_mode entropy_coding_mode;
    k_u8 cabac_init_idc;
} k_venc_h264_entropy;

typedef struct
{
    k_u8 cabac_init_flag;
} k_venc_h265_entropy;

#define K_VENC_MAX_2D_OSD_REGION_NUM    (8)     /* Max region number of osd */
#define K_VENC_MAX_2D_BORDER_NUM        (32)    /* Max number of border */
#define K_VENC_2D_MAX_CHN_NUM          (3)     /* Max number of 2d channel */
#define K_VENC_2D_COEF_NUM             (12)    /* 2d coefficient */

typedef enum
{
    K_VENC_2D_CALC_MODE_CSC = 0,       /* Color space conversion */
    K_VENC_2D_CALC_MODE_OSD,           /* On Screen Display */
    K_VENC_2D_CALC_MODE_BORDER,        /* Draw border */
    K_VENC_2D_CALC_MODE_OSD_BORDER,    /* OSD first, then draw border */
    K_VENC_2D_CALC_MODE_BUTT
} k_venc_2d_calc_mode;

typedef enum
{
    K_VENC_2D_SRC_DST_FMT_YUV420_NV12 = 0, /* Source/destination format of 2d: NV12 */
    K_VENC_2D_SRC_DST_FMT_YUV420_NV21,     /* Source/destination format of 2d: NV21 */
    K_VENC_2D_SRC_DST_FMT_YUV420_I420,     /* Source/destination format of 2d: I420/420p */
    K_VENC_2D_SRC_DST_FMT_ARGB8888 = 4,    /* Source/destination format of 2d: ARGB8888 */
    K_VENC_2D_SRC_DST_FMT_ARGB4444,        /* Source/destination format of 2d: ARGB4444 */
    K_VENC_2D_SRC_DST_FMT_ARGB1555,        /* Source/destination format of 2d: ARGB1555 */
    K_VENC_2D_SRC_DST_FMT_XRGB8888,        /* Source/destination format of 2d: XRGB8888 */
    K_VENC_2D_SRC_DST_FMT_XRGB4444,        /* Source/destination format of 2d: XRGB4444 */
    K_VENC_2D_SRC_DST_FMT_XRGB1555,        /* Source/destination format of 2d: XRGB1555 */
    K_VENC_2D_SRC_DST_FMT_BGRA8888,        /* Source/destination format of 2d: BGRA8888 */
    K_VENC_2D_SRC_DST_FMT_BGRA4444,        /* Source/destination format of 2d: BGRA4444 */
    K_VENC_2D_SRC_DST_FMT_BGRA5551,        /* Source/destination format of 2d: BGRA5551 */
    K_VENC_2D_SRC_DST_FMT_BGRX8888,        /* Source/destination format of 2d: BGRX8888 */
    K_VENC_2D_SRC_DST_FMT_BGRX4444,        /* Source/destination format of 2d: BGRX4444 */
    K_VENC_2D_SRC_DST_FMT_BGRX5551,        /* Source/destination format of 2d: BGRX5551 */
    K_VENC_2D_SRC_DST_FMT_RGB888,          /* Source/destination format of 2d: RGB888 */
    K_VENC_2D_SRC_DST_FMT_BGR888,          /* Source/destination format of 2d: BGR888 */
    K_VENC_2D_SRC_DST_FMT_RGB565,          /* Source/destination format of 2d: RGB565 */
    K_VENC_2D_SRC_DST_FMT_BGR565,          /* Source/destination format of 2d: BGR565 */
    K_VENC_2D_SRC_DST_FMT_SEPERATE_RGB,    /* Source/destination format of 2d: SEPERATE_RGB */
    K_VENC_2D_SRC_DST_FMT_BUTT
} k_venc_2d_src_dst_fmt;

typedef enum
{
    K_VENC_2D_OSD_FMT_ARGB8888 = 0,        /* OSD format: ARGB8888 */
    K_VENC_2D_OSD_FMT_ARGB4444,            /* OSD format: ARGB4444 */
    K_VENC_2D_OSD_FMT_ARGB1555,            /* OSD format: ARGB1555 */
    K_VENC_2D_OSD_FMT_XRGB8888,            /* OSD format: XRGB8888 */
    K_VENC_2D_OSD_FMT_XRGB4444,            /* OSD format: XRGB4444 */
    K_VENC_2D_OSD_FMT_XRGB1555,            /* OSD format: XRGB1555 */
    K_VENC_2D_OSD_FMT_BGRA8888,            /* OSD format: BGRA8888 */
    K_VENC_2D_OSD_FMT_BGRA4444,            /* OSD format: BGRA4444 */
    K_VENC_2D_OSD_FMT_BGRA5551,            /* OSD format: BGRA5551 */
    K_VENC_2D_OSD_FMT_BGRX8888,            /* OSD format: BGRX8888 */
    K_VENC_2D_OSD_FMT_BGRX4444,            /* OSD format: BGRX4444 */
    K_VENC_2D_OSD_FMT_BGRX5551,            /* OSD format: BGRX5551 */
    K_VENC_2D_OSD_FMT_RGB888,              /* OSD format: RGB888 */
    K_VENC_2D_OSD_FMT_BGR888,              /* OSD format: BGR888 */
    K_VENC_2D_OSD_FMT_RGB565,              /* OSD format: RGB565 */
    K_VENC_2D_OSD_FMT_BGR565,              /* OSD format: BGR565 */
    K_VENC_2D_OSD_FMT_SEPERATE_RGB,        /* OSD format: SEPERATE_RGB */
    K_VENC_2D_OSD_FMT_BUTT
} k_venc_2d_osd_fmt;

typedef enum
{
    /* bottom ------> top */
    K_VENC_2D_ADD_ORDER_VIDEO_OSD = 0,     /* Add order of OSD, from bottom to top: VIDEO_OSD */
    K_VENC_2D_ADD_ORDER_OSD_VIDEO,         /* Add order of OSD, from bottom to top: OSD_VIDEO */
    K_VENC_2D_ADD_ORDER_VIDEO_BG,          /* Add order of OSD, from bottom to top: VIDEO_BG */
    K_VENC_2D_ADD_ORDER_BG_VIDEO,          /* Add order of OSD, from bottom to top: BG_VIDEO */
    K_VENC_2D_ADD_ORDER_VIDEO_BG_OSD,      /* Add order of OSD, from bottom to top: VIDEO_BG_OSD */
    K_VENC_2D_ADD_ORDER_VIDEO_OSD_BG,      /* Add order of OSD, from bottom to top: VIDEO_OSD_BG */
    K_VENC_2D_ADD_ORDER_BG_VIDEO_OSD,      /* Add order of OSD, from bottom to top: BG_VIDEO_OSD */
    K_VENC_2D_ADD_ORDER_BG_OSD_VIDEO,      /* Add order of OSD, from bottom to top: BG_OSD_VIDEO */
    K_VENC_2D_ADD_ORDER_OSD_VIDEO_BG,      /* Add order of OSD, from bottom to top: OSD_VIDEO_BG */
    K_VENC_2D_ADD_ORDER_OSD_BG_VIDEO,      /* Add order of OSD, from bottom to top: OSD_BG_VIDEO */
    K_VENC_2D_ADD_ORDER_BUTT
} k_venc_2d_add_order;

typedef enum
{
    K_VENC_2D_ATTR_TYPE_BASE = 0,           /* Basic attribution of 2D */
    K_VENC_2D_ATTR_TYPE_OSD,                /* OSD attribution of 2D */
    K_VENC_2D_ATTR_TYPE_BORDER,             /* Border attribution of 2D */
    K_VENC_2D_ATTR_TYPE_CSC_COEF,           /* Custom CSC coefficient*/
    K_VENC_2D_ATTR_TYPE_OSD_COEF,           /* Custom OSD coefficient*/
    K_VENC_2D_ATTR_TYPE_BUTT
} k_venc_2d_attr_type;

typedef enum
{
    VENC_2D_COLOR_GAMUT_BT601 = 0,
    VENC_2D_COLOR_GAMUT_BT709,
    VENC_2D_COLOR_GAMUT_BT2020,
    VENC_2D_COLOR_GAMUT_BUTT
} k_venc_2d_color_gamut;

typedef struct
{
    k_venc_2d_src_dst_fmt dst_fmt;                                 /* Format of output image */
} k_venc_2d_csc_attr;

typedef struct
{
    k_u16 width;                      /* Width of OSD image */
    k_u16 height;                     /* Height of OSD image */
    k_u16 startx;                     /* Start coordinate in horizontal of OSD image */
    k_u16 starty;                      /* Start coordinate in vertical of OSD image */
    k_u32 phys_addr[3];               /* Physical address of OSD image */
    k_u8 bg_alpha;                    /* Alpha of background in OSD region */
    k_u8 osd_alpha;                   /* Alpha of OSD in OSD region */
    k_u8 video_alpha;                 /* Alpha of input image in OSD region */
    k_venc_2d_add_order add_order;    /* Add order of OSD region */
    k_u32 bg_color;                   /* Background color in OSD region */
    k_venc_2d_osd_fmt fmt;            /* Format of OSD image */
} k_venc_2d_osd_attr;

typedef struct
{
    k_u16 width;           /* Width of border */
    k_u16 height;          /* Height of border */
    k_u16 line_width;      /* Wide of border line */
    k_u32 color;           /* Color of border */
    k_u16 startx;          /* Start coordinate in horizontal of border */
    k_u16 starty;          /* Start coordinate in vertical of border */
} k_venc_2d_border_attr;

typedef struct
{
    k_s16 coef[12];                /* Pointer of coefficent */
} k_venc_2d_coef_attr;


#define K_ERR_VENC_INVALID_DEVID     K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_VENC_INVALID_CHNID     K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_VENC_ILLEGAL_PARAM     K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VENC_EXIST             K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_VENC_UNEXIST           K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VENC_NULL_PTR          K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VENC_NOT_CONFIG        K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_VENC_NOT_SUPPORT       K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_VENC_NOT_PERM          K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VENC_NOMEM             K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VENC_NOBUF             K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VENC_BUF_EMPTY         K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_VENC_BUF_FULL          K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_VENC_NOTREADY          K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VENC_BADADDR           K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_VENC_BUSY              K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_ERR_VENC_NOT_PERM          K_DEF_ERR(K_ID_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)


/** @} */ /** <!-- ==== VENC End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
