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

#ifndef _MVX_V4L2_CONTROLS_H_
#define _MVX_V4L2_CONTROLS_H_

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/version.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <drm/drm_fourcc.h>

/****************************************************************************
 * Pixel formats
 ****************************************************************************/

#define V4L2_PIX_FMT_YUV420_AFBC_8   v4l2_fourcc('Y', '0', 'A', '8')
#define V4L2_PIX_FMT_YUV420_AFBC_10  v4l2_fourcc('Y', '0', 'A', 'A')
#define V4L2_PIX_FMT_YUV422_AFBC_8   v4l2_fourcc('Y', '2', 'A', '8')
#define V4L2_PIX_FMT_YUV422_AFBC_10  v4l2_fourcc('Y', '2', 'A', 'A')
#define V4L2_PIX_FMT_AQB1            v4l2_fourcc('Y', '0', 'A', 'B')
#define V4L2_PIX_FMT_Y210            v4l2_fourcc('Y', '2', '1', '0')
#define V4L2_PIX_FMT_P010            v4l2_fourcc('Y', '0', 'P', '1')
#define V4L2_PIX_FMT_Y0L2            v4l2_fourcc('Y', '0', 'Y', 'L')
#define V4L2_PIX_FMT_RGB_3P          v4l2_fourcc('R', 'G', 'B', 'M')


#define V4L2_PIX_FMT_Y10_LE            v4l2_fourcc('Y', '1', '0', 'L')
#define V4L2_PIX_FMT_YUV444_10         v4l2_fourcc('Y', '4', 'P', '3')
#define V4L2_PIX_FMT_YUV422_1P_10      v4l2_fourcc('Y', '2', 'P', '1')
#define V4L2_PIX_FMT_YUV420_2P_10      v4l2_fourcc('Y', '0', 'P', '2')
#define V4L2_PIX_FMT_YUV420_I420_10    v4l2_fourcc('Y', '0', 'P', '3')


#define V4L2_PIX_FMT_RV              v4l2_fourcc('R', 'V', '0', '0')

#ifndef V4L2_PIX_FMT_HEVC
#define V4L2_PIX_FMT_HEVC            v4l2_fourcc('H', 'E', 'V', 'C')
#endif

#ifndef V4L2_PIX_FMT_VP9
#define V4L2_PIX_FMT_VP9             v4l2_fourcc('V', 'P', '9', '0')
#endif

#ifndef V4L2_PIX_FMT_AV1
#define V4L2_PIX_FMT_AV1             v4l2_fourcc('A', 'V', '1', '0')
#endif

#define V4L2_PIX_FMT_AVS              v4l2_fourcc('A', 'V', 'S', '1')
#define V4L2_PIX_FMT_AVS2             v4l2_fourcc('A', 'V', 'S', '2')

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP        (V4L2_CID_MPEG_BASE + 600)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP        (V4L2_CID_MPEG_BASE + 601)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP    (V4L2_CID_MPEG_BASE + 602)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP    (V4L2_CID_MPEG_BASE + 603)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP    (V4L2_CID_MPEG_BASE + 604)
#endif
/****************************************************************************
 * Buffers
 * @see v4l2_buffer
 ****************************************************************************/

/*
 * Extended buffer flags.
 */
/*
#define V4L2_BUF_FLAG_MVX_DECODE_ONLY           0x01000000
#define V4L2_BUF_FLAG_MVX_CODEC_CONFIG          0x02000000
#define V4L2_BUF_FLAG_MVX_AFBC_TILED_HEADERS    0x10000000
#define V4L2_BUF_FLAG_MVX_AFBC_TILED_BODY       0x20000000
#define V4L2_BUF_FLAG_MVX_AFBC_32X8_SUPERBLOCK  0x40000000
#define V4L2_BUF_FLAG_MVX_MASK                  0xff000000
#define V4L2_BUF_FLAG_END_OF_SUB_FRAME          0x04000000
#define V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT  0x08000000
#define V4L2_BUF_FLAG_MVX_BUFFER_NEED_REALLOC   0x07000000


#define V4L2_BUF_FRAME_FLAG_ROTATION_90   0x81000000
#define V4L2_BUF_FRAME_FLAG_ROTATION_180  0x82000000
#define V4L2_BUF_FRAME_FLAG_ROTATION_270  0x83000000
#define V4L2_BUF_FRAME_FLAG_ROTATION_MASK 0x83000000
#define V4L2_BUF_FRAME_FLAG_MIRROR_HORI   0x90000000
#define V4L2_BUF_FRAME_FLAG_MIRROR_VERT   0xA0000000
#define V4L2_BUF_FRAME_FLAG_MIRROR_MASK   0xB0000000
#define V4L2_BUF_FRAME_FLAG_SCALING_2     0x84000000
#define V4L2_BUF_FRAME_FLAG_SCALING_4     0x88000000
#define V4L2_BUF_FRAME_FLAG_SCALING_MASK  0x8C000000

#define V4L2_BUF_FLAG_MVX_BUFFER_EPR      0xC0000000
#define V4L2_BUF_FLAG_MVX_BUFFER_ROI      0x70000000
*/
//redefine these flags
/*use encode/decode frame/bitstream to update these flags*/

#define V4L2_BUF_FLAG_MVX_MASK                  0xfff00000

//for decode frame flag
#define V4L2_BUF_FRAME_FLAG_ROTATION_90         0x01000000  /* Frame is rotated 90 degrees */
#define V4L2_BUF_FRAME_FLAG_ROTATION_180        0x02000000  /* Frame is rotated 180 degrees */
#define V4L2_BUF_FRAME_FLAG_ROTATION_270        0x03000000  /* Frame is rotated 270 degrees */
#define V4L2_BUF_FRAME_FLAG_ROTATION_MASK       0x03000000
#define V4L2_BUF_FRAME_FLAG_SCALING_2           0x04000000  /* Frame is scaled by half */
#define V4L2_BUF_FRAME_FLAG_SCALING_4           0x08000000  /* Frame is scaled by quarter */
#define V4L2_BUF_FRAME_FLAG_SCALING_MASK        0x0C000000
#define V4L2_BUF_FLAG_MVX_BUFFER_FRAME_PRESENT  0x10000000
#define V4L2_BUF_FLAG_MVX_BUFFER_NEED_REALLOC   0x20000000

//for decode bitstream flag
#define V4L2_BUF_FLAG_MVX_CODEC_CONFIG          0xC1000000
#define V4L2_BUF_FLAG_END_OF_SUB_FRAME          0xC2000000
#define V4L2_BUF_FLAG_MVX_DECODE_ONLY           0xC4000000

//for encode frame flag
#define V4L2_BUF_FRAME_FLAG_MIRROR_HORI         0x01000000
#define V4L2_BUF_FRAME_FLAG_MIRROR_VERT         0x02000000
#define V4L2_BUF_FRAME_FLAG_MIRROR_MASK         0x03000000
#define V4L2_BUF_ENCODE_FLAG_ROTATION_90        0x10000000  /* Frame is rotated 90 degrees */
#define V4L2_BUF_ENCODE_FLAG_ROTATION_180       0x20000000  /* Frame is rotated 180 degrees */
#define V4L2_BUF_ENCODE_FLAG_ROTATION_270       0x30000000  /* Frame is rotated 270 degrees */
#define V4L2_BUF_ENCODE_FLAG_ROTATION_MASK      0x30000000

#define V4L2_BUF_FLAG_MVX_BUFFER_ROI            0x04000000  /* this buffer has a roi region */
#define V4L2_BUF_FLAG_MVX_BUFFER_EPR            0x08000000  /* EPR buffer flag */
#define V4L2_BUF_FLAG_MVX_BUFFER_GENERAL        0x08000000
#define V4L2_BUF_FLAG_MVX_BUFFER_CHR            0x40000000
#define V4L2_BUF_FLAG_MVX_BUFFER_GOP_RESET      0x80000000  /* reset GOP */
#define V4L2_BUF_FLAG_MVX_BUFFER_LTR_RESET      0x00200000  /* reset GOP */
#define V4L2_BUF_FLAG_MVX_BUFFER_ENC_STATS      0x00400000  /* reset LTR */

//afbc flag
#define V4L2_BUF_FLAG_MVX_AFBC_TILED_HEADERS    0x01000000
#define V4L2_BUF_FLAG_MVX_AFBC_TILED_BODY       0x02000000
#define V4L2_BUF_FLAG_MVX_AFBC_32X8_SUPERBLOCK  0x04000000

//for customeized flag, set to v4l2_buffer.reserved2
#define V4L2_BUF_FLAG_MVX_MINIFRAME             0x00000001
#define V4L2_BUF_FLAG_MVX_OSD_1                 0x00000002
#define V4L2_BUF_FLAG_MVX_OSD_2                 0x00000004
#define V4L2_BUF_FLAG_MVX_OSD_MASK              0x00000006
#define V4L2_BUF_FLAG_MVX_AD_STATS              0x00000008

/****************************************************************************
 * HDR color description.
 ****************************************************************************/

#define V4L2_EVENT_MVX_COLOR_DESC       V4L2_EVENT_PRIVATE_START
#define V4L2_MVX_MAX_FRAME_REGIONS 16
#define V4L2_MAX_FRAME_OSD_REGION 2
enum v4l2_mvx_range {
    V4L2_MVX_RANGE_UNSPECIFIED,
    V4L2_MVX_RANGE_FULL,
    V4L2_MVX_RANGE_LIMITED
};

enum v4l2_mvx_primaries {
    V4L2_MVX_PRIMARIES_UNSPECIFIED,
    V4L2_MVX_PRIMARIES_BT709,         /* Rec.ITU-R BT.709 */
    V4L2_MVX_PRIMARIES_BT470M,        /* Rec.ITU-R BT.470 System M */
    V4L2_MVX_PRIMARIES_BT601_625,     /* Rec.ITU-R BT.601 625 */
    V4L2_MVX_PRIMARIES_BT601_525,     /* Rec.ITU-R BT.601 525 */
    V4L2_MVX_PRIMARIES_GENERIC_FILM,  /* Generic Film */
    V4L2_MVX_PRIMARIES_BT2020         /* Rec.ITU-R BT.2020 */
};

enum v4l2_mvx_transfer {
    V4L2_MVX_TRANSFER_UNSPECIFIED,
    V4L2_MVX_TRANSFER_LINEAR,         /* Linear transfer characteristics */
    V4L2_MVX_TRANSFER_SRGB,           /* sRGB */
    V4L2_MVX_TRANSFER_SMPTE170M,      /* SMPTE 170M */
    V4L2_MVX_TRANSFER_GAMMA22,        /* Assumed display gamma 2.2 */
    V4L2_MVX_TRANSFER_GAMMA28,        /* Assumed display gamma 2.8 */
    V4L2_MVX_TRANSFER_ST2084,         /* SMPTE ST 2084 */
    V4L2_MVX_TRANSFER_HLG,            /* ARIB STD-B67 hybrid-log-gamma */
    V4L2_MVX_TRANSFER_SMPTE240M,      /* SMPTE 240M */
    V4L2_MVX_TRANSFER_XVYCC,          /* IEC 61966-2-4 */
    V4L2_MVX_TRANSFER_BT1361,         /* Rec.ITU-R BT.1361 extended gamut */
    V4L2_MVX_TRANSFER_ST428           /* SMPTE ST 428-1 */
};

enum v4l2_mvx_matrix {
    V4L2_MVX_MATRIX_UNSPECIFIED,
    V4L2_MVX_MATRIX_BT709,            /* Rec.ITU-R BT.709 */
    V4L2_MVX_MATRIX_BT470M,           /* KR=0.30, KB=0.11 */
    V4L2_MVX_MATRIX_BT601,            /* Rec.ITU-R BT.601 625 */
    V4L2_MVX_MATRIX_SMPTE240M,        /* SMPTE 240M or equivalent */
    V4L2_MVX_MATRIX_BT2020,           /* Rec.ITU-R BT.2020 non-const lum */
    V4L2_MVX_MATRIX_BT2020Constant    /* Rec.ITU-R BT.2020 constant lum */
};

enum v4l2_nalu_format {
    V4L2_OPT_NALU_FORMAT_START_CODES,
    V4L2_OPT_NALU_FORMAT_ONE_NALU_PER_BUFFER,
    V4L2_OPT_NALU_FORMAT_ONE_BYTE_LENGTH_FIELD,
    V4L2_OPT_NALU_FORMAT_TWO_BYTE_LENGTH_FIELD,
    V4L2_OPT_NALU_FORMAT_FOUR_BYTE_LENGTH_FIELD,
    V4L2_OPT_NALU_FORMAT_ONE_FRAME_PER_BUFFER
};

struct v4l2_mvx_primary {
    unsigned short x;
    unsigned short y;
};

/**
 * struct v4l2_mvx_color_desc - HDR color description.
 * @flags:            Flags which fields that are valid.
 * @range:            enum v4l2_mvx_range.
 * @primaries:            enum v4l2_mvx_primaries.
 * @transfer:            enum v4l2_mvx_transfer.
 * @matrix:            enum v4l2_mvx_matrix.
 * @display.r:            Red point.
 * @display.g:            Green point.
 * @display.b:            Blue point.
 * @display.w:            White point.
 * @display.luminance_min:    Minimum display luminance.
 * @display.luminance_max:    Maximum display luminance.
 * @content.luminance_max:    Maximum content luminance.
 * @content.luminance_average:    Average content luminance.
 *
 * Color- and white point primaries are given in increments of 0.00002
 * and in the range of 0 to 50'000.
 *
 * Luminance is given in increments of 0.0001 candelas per m3.
 */
struct v4l2_mvx_color_desc {
    unsigned int flags;
        #define V4L2_BUFFER_PARAM_COLOUR_FLAG_MASTERING_DISPLAY_DATA_VALID  (1)
        #define V4L2_BUFFER_PARAM_COLOUR_FLAG_CONTENT_LIGHT_DATA_VALID      (2)
    unsigned char range;
    unsigned char primaries;
    unsigned char transfer;
    unsigned char matrix;
    struct {
        struct v4l2_mvx_primary r;
        struct v4l2_mvx_primary g;
        struct v4l2_mvx_primary b;
        struct v4l2_mvx_primary w;
        unsigned short luminance_min;
        unsigned short luminance_max;
    } display;
    struct {
        unsigned short luminance_max;
        unsigned short luminance_average;
    } content;

    unsigned char video_format;
    unsigned char aspect_ratio_idc;
    unsigned short sar_width;
    unsigned short sar_height;
    unsigned int num_units_in_tick;
    unsigned int time_scale;
} __attribute__ ((packed));

struct v4l2_buffer_param_region
{
    unsigned short mbx_left;   /**< X coordinate of the left most macroblock */
    unsigned short mbx_right;  /**< X coordinate of the right most macroblock */
    unsigned short mby_top;    /**< Y coordinate of the top most macroblock */
    unsigned short mby_bottom; /**< Y coordinate of the bottom most macroblock */
    short qp_delta;   /**< QP delta value. This region will be encoded
                         *   with qp = qp_default + qp_delta. */
    unsigned short prio;
    unsigned short force_intra;
};

struct v4l2_mvx_roi_regions
{
    unsigned int pic_index;
    unsigned char qp_present;
    unsigned char qp;
    unsigned char roi_present;
    unsigned char num_roi;
    struct v4l2_buffer_param_region roi[V4L2_MVX_MAX_FRAME_REGIONS];
};

struct v4l2_sei_user_data
{
    unsigned char flags;
        #define V4L2_BUFFER_PARAM_USER_DATA_UNREGISTERED_VALID  (1)
    unsigned char uuid[16];
    char user_data[256 - 35];
    unsigned char user_data_len;
};

struct v4l2_rate_control
{
    unsigned int rc_type;
        #define V4L2_OPT_RATE_CONTROL_MODE_OFF          (0)
        #define V4L2_OPT_RATE_CONTROL_MODE_STANDARD     (1)
        #define V4L2_OPT_RATE_CONTROL_MODE_VARIABLE     (2)
        #define V4L2_OPT_RATE_CONTROL_MODE_CONSTANT     (3)
        #define V4L2_OPT_RATE_CONTROL_MODE_C_VARIABLE   (4)
    unsigned int target_bitrate;
    unsigned int maximum_bitrate;
};

struct v4l2_mvx_dsl_frame
{
    unsigned int width;
    unsigned int height;
};

struct v4l2_mvx_dsl_ratio
{
    unsigned int hor;
    unsigned int ver;
};

struct v4l2_mvx_long_term_ref
{
    unsigned int mode;
    unsigned int period;
};

struct v4l2_buffer_param_rectangle
{
    unsigned short x_left;   /* pixel x left edge   (inclusive) */
    unsigned short x_right;  /* pixel x right edge  (exclusive) */
    unsigned short y_top;    /* pixel y top edge    (inclusive) */
    unsigned short y_bottom; /* pixel y bottom edge (exclusive) */
};

/* input for encoder,
 * indicate which parts of the source picture has changed.
 * The encoder can (optionally) use this information to
 * reduce memory bandwidth.
 *
 * n_rectangles=0 indicates the source picture is unchanged.
 *
 * This parameter only applies to the picture that immediately
 * follows (and not to subsequent ones).
 */
struct v4l2_mvx_chr_config
{
    unsigned int pic_index;
    unsigned int num_chr;
    #define V4L2_MAX_FRAME_CHANGE_RECTANGLES 2
    struct v4l2_buffer_param_rectangle rectangle[V4L2_MAX_FRAME_CHANGE_RECTANGLES];
};

struct v4l2_mvx_osd_cfg
{
    uint8_t osd_inside_enable;
    uint8_t osd_inside_alpha_enable;
    uint8_t osd_inside_convert_color_enable;
    uint8_t osd_inside_alpha_value; /* as alpha range [0~16], use u8 */
    uint8_t osd_inside_convert_color_threshold;/* threshold range [0~255], if input is 10bit, th * 4 */
    uint8_t osd_inside_rgb2yuv_mode;/* 0-601L, 1-601F, 2-709_L, 3-709_F  */
    uint16_t osd_inside_start_x;   /* pixel x left edge   (inclusive) */
    uint16_t osd_inside_start_y;    /* pixel y top edge    (inclusive) */
    uint16_t reserved[3];
};

struct v4l2_osd_config
{
    unsigned int pic_index;
    unsigned int num_osd;
    struct v4l2_mvx_osd_cfg osd_single_cfg[V4L2_MAX_FRAME_OSD_REGION];/* include single osd region config and index */
};

struct v4l2_osd_info
{
    uint32_t width_osd[V4L2_MAX_FRAME_OSD_REGION];
    uint32_t height_osd[V4L2_MAX_FRAME_OSD_REGION];
    uint32_t inputFormat_osd[V4L2_MAX_FRAME_OSD_REGION];
};

#define V4L2_MVX_COLOR_DESC_DISPLAY_VALID       0x1
#define V4L2_MVX_COLOR_DESC_CONTENT_VALID       0x2

/****************************************************************************
 * Custom IOCTL
 ****************************************************************************/

#define VIDIOC_G_MVX_COLORDESC  _IOWR('V', BASE_VIDIOC_PRIVATE,    \
                      struct v4l2_mvx_color_desc)
#define VIDIOC_S_MVX_ROI_REGIONS _IOWR('V', BASE_VIDIOC_PRIVATE + 1,    \
                      struct v4l2_mvx_roi_regions)
#define VIDIOC_S_MVX_QP_EPR _IOWR('V', BASE_VIDIOC_PRIVATE + 2,    \
                      struct v4l2_buffer_param_qp)
#define VIDIOC_S_MVX_COLORDESC  _IOWR('V', BASE_VIDIOC_PRIVATE + 3,    \
                      struct v4l2_mvx_color_desc)
#define VIDIOC_S_MVX_SEI_USERDATA _IOWR('V', BASE_VIDIOC_PRIVATE + 4,    \
                      struct v4l2_sei_user_data)
#define VIDIOC_S_MVX_RATE_CONTROL _IOWR('V', BASE_VIDIOC_PRIVATE + 5,    \
                      struct v4l2_rate_control)
#define VIDIOC_S_MVX_DSL_FRAME _IOWR('V', BASE_VIDIOC_PRIVATE + 6,    \
                      struct v4l2_mvx_dsl_frame)
#define VIDIOC_S_MVX_DSL_RATIO _IOWR('V', BASE_VIDIOC_PRIVATE + 7,    \
                      struct v4l2_mvx_dsl_ratio)
#define VIDIOC_S_MVX_LONG_TERM_REF _IOWR('V', BASE_VIDIOC_PRIVATE + 8,    \
                      struct v4l2_mvx_long_term_ref)
#define VIDIOC_S_MVX_DSL_MODE _IOWR('V', BASE_VIDIOC_PRIVATE + 9,    \
                      int)
#define VIDIOC_S_MVX_MINI_FRAME_HEIGHT _IOWR('V', BASE_VIDIOC_PRIVATE + 10,    \
                      int)
#define VIDIOC_S_MVX_STATS_MODE _IOWR('V', BASE_VIDIOC_PRIVATE + 11,    \
                      struct v4l2_buffer_param_enc_stats)
#define VIDIOC_S_MVX_CHR_CFG _IOWR('V', BASE_VIDIOC_PRIVATE + 12,    \
                      struct v4l2_mvx_chr_config)
#define VIDIOC_S_MVX_HUFF_TABLE _IOWR('V', BASE_VIDIOC_PRIVATE + 13,    \
                      struct v4l2_mvx_huff_table)
#define VIDIOC_S_MVX_SEAMLESS_TARGET _IOWR('V', BASE_VIDIOC_PRIVATE + 14,    \
                      struct v4l2_mvx_seamless_target)
#define VIDIOC_S_MVX_COLOR_CONV_COEF _IOWR('V', BASE_VIDIOC_PRIVATE + 15,    \
                      struct v4l2_mvx_color_conv_coef)
#define VIDIOC_S_MVX_ENC_SRC_CROP _IOWR('V', BASE_VIDIOC_PRIVATE + 16,    \
                      struct v4l2_mvx_crop_cfg)
#define VIDIOC_S_MVX_DEC_DST_CROP _IOWR('V', BASE_VIDIOC_PRIVATE + 17,    \
                      struct v4l2_mvx_crop_cfg)
#define VIDIOC_S_MVX_RGB2YUV_COLOR_CONV_COEF _IOWR('V', BASE_VIDIOC_PRIVATE + 18,    \
                      struct v4l2_mvx_rgb2yuv_color_conv_coef)
#define VIDIOC_S_MVX_OSD_CONFIG _IOWR('V', BASE_VIDIOC_PRIVATE + 19,    \
                      struct v4l2_osd_config)
#define VIDIOC_S_MVX_OSD_INFO _IOWR('V', BASE_VIDIOC_PRIVATE + 20,    \
                      struct v4l2_osd_info)
/****************************************************************************
 * Custom controls
 ****************************************************************************/

/*
 * Video for Linux 2 custom controls.
 */
enum v4l2_cid_mve_video {
#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 0, 0)
    V4L2_CID_MVE_VIDEO_FRAME_RATE = V4L2_CTRL_CLASS_CODEC + 0x2000,
#else
    V4L2_CID_MVE_VIDEO_FRAME_RATE = V4L2_CTRL_CLASS_MPEG + 0x2000,
#endif
    V4L2_CID_MVE_VIDEO_NALU_FORMAT,
    V4L2_CID_MVE_VIDEO_STREAM_ESCAPING,
    V4L2_CID_MVE_VIDEO_H265_PROFILE,
    V4L2_CID_MVE_VIDEO_VC1_PROFILE,
    V4L2_CID_MVE_VIDEO_H265_LEVEL,
    V4L2_CID_MVE_VIDEO_IGNORE_STREAM_HEADERS,
    V4L2_CID_MVE_VIDEO_FRAME_REORDERING,
    V4L2_CID_MVE_VIDEO_INTBUF_SIZE,
    V4L2_CID_MVE_VIDEO_P_FRAMES,
    V4L2_CID_MVE_VIDEO_GOP_TYPE,
    V4L2_CID_MVE_VIDEO_CONSTR_IPRED,
    V4L2_CID_MVE_VIDEO_ENTROPY_SYNC,
    V4L2_CID_MVE_VIDEO_TEMPORAL_MVP,
    V4L2_CID_MVE_VIDEO_TILE_ROWS,
    V4L2_CID_MVE_VIDEO_TILE_COLS,
    V4L2_CID_MVE_VIDEO_MIN_LUMA_CB_SIZE,
    V4L2_CID_MVE_VIDEO_MB_MASK,
    V4L2_CID_MVE_VIDEO_VP9_PROB_UPDATE,
    V4L2_CID_MVE_VIDEO_BITDEPTH_CHROMA,
    V4L2_CID_MVE_VIDEO_BITDEPTH_LUMA,
    V4L2_CID_MVE_VIDEO_FORCE_CHROMA_FORMAT,
    V4L2_CID_MVE_VIDEO_RGB_TO_YUV_MODE,
    V4L2_CID_MVE_VIDEO_BANDWIDTH_LIMIT,
    V4L2_CID_MVE_VIDEO_CABAC_INIT_IDC,
    V4L2_CID_MVE_VIDEO_VPX_B_FRAME_QP,
    V4L2_CID_MVE_VIDEO_SECURE_VIDEO,
    V4L2_CID_MVE_VIDEO_CROP_LEFT,
    V4L2_CID_MVE_VIDEO_CROP_RIGHT,
    V4L2_CID_MVE_VIDEO_CROP_TOP,
    V4L2_CID_MVE_VIDEO_CROP_BOTTOM,
    V4L2_CID_MVE_VIDEO_HRD_BUFFER_SIZE,
    V4L2_CID_MVE_VIDEO_INIT_QP_I,
    V4L2_CID_MVE_VIDEO_INIT_QP_P,
    V4L2_CID_MVE_VIDEO_SAO_LUMA,
    V4L2_CID_MVE_VIDEO_SAO_CHROMA,
    V4L2_CID_MVE_VIDEO_QP_DELTA_I_P,
    V4L2_CID_MVE_VIDEO_QP_REF_RB_EN,
    V4L2_CID_MVE_VIDEO_RC_CLIP_TOP,
    V4L2_CID_MVE_VIDEO_RC_CLIP_BOT,
    V4L2_CID_MVE_VIDEO_QP_MAP_CLIP_TOP,
    V4L2_CID_MVE_VIDEO_QP_MAP_CLIP_BOT,
    V4L2_CID_MVE_VIDEO_MAX_QP_I,
    V4L2_CID_MVE_VIDEO_MIN_QP_I,
    V4L2_CID_MVE_VIDEO_FW_PROFILING,
    V4L2_CID_MVE_VIDEO_VISIBLE_WIDTH,
    V4L2_CID_MVE_VIDEO_VISIBLE_HEIGHT,
    V4L2_CID_MVE_VIDEO_JPEG_QUALITY_LUMA,
    V4L2_CID_MVE_VIDEO_JPEG_QUALITY_CHROMA,
    V4L2_CID_MVE_VIDEO_RC_I_MODE,
    V4L2_CID_MVE_VIDEO_RC_I_RATIO,
    V4L2_CID_MVE_VIDEO_INTER_MED_BUF_SIZE,
    V4L2_CID_MVE_VIDEO_SVCT3_LEVEL1_PERIOD,
    V4L2_CID_MVE_VIDEO_GOP_RESET_PFRAMES,
    V4L2_CID_MVE_VIDEO_LTR_RESET_PERIOD,
    V4L2_CID_MVE_VIDEO_QP_FIXED,
    V4L2_CID_MVE_VIDEO_GDR_NUMBER,
    V4L2_CID_MVE_VIDEO_GDR_PERIOD,
    V4L2_CID_MVE_VIDEO_MULTI_SPS_PPS,
    V4L2_CID_MVE_VIDEO_ENABLE_VISUAL,
    V4L2_CID_MVE_VIDEO_SCD_ENABLE,
    V4L2_CID_MVE_VIDEO_SCD_PERCENT,
    V4L2_CID_MVE_VIDEO_SCD_THRESHOLD,
    V4L2_CID_MVE_VIDEO_AQ_SSIM_EN,
    V4L2_CID_MVE_VIDEO_AQ_NEG_RATIO,
    V4L2_CID_MVE_VIDEO_AQ_POS_RATIO,
    V4L2_CID_MVE_VIDEO_AQ_QPDELTA_LMT,
    V4L2_CID_MVE_VIDEO_AQ_INIT_FRM_AVG_SVAR,
    V4L2_CID_MVE_VIDEO_COLOR_CONVERSION,
    V4L2_CID_MVE_VIDEO_RGB2YUV_COLOR_CONV_COEF,
    V4L2_CID_MVE_VIDEO_FORCED_UV_VALUE,
    V4L2_CID_MVE_VIDEO_DSL_INTERP_MODE,
    V4L2_CID_MVE_VIDEO_DISABLED_FEATURES,
    V4L2_CID_MVE_VIDEO_ENABLE_ADAPTIVE_INTRA_BLOCK,
    V4L2_CID_MVE_VIDEO_CHANGE_POS,
};

struct v4l2_buffer_param_enc_stats
{
    unsigned int mms_buffer_size;
    unsigned int bitcost_buffer_size;
    unsigned int qp_buffer_size;
    unsigned int flags;
    //ENC_STATS_FLAGS
        #define V4L2_BUFFER_ENC_STATS_FLAG_MMS        (1<<0)
        #define V4L2_BUFFER_ENC_STATS_FLAG_BITCOST    (1<<1)
        #define V4L2_BUFFER_ENC_STATS_FLAG_QP         (1<<2)
        #define V4L2_BUFFER_ENC_STATS_FLAG_DROP       (1<<3)
    unsigned int pic_index_or_mb_size;
};


/* block configuration uncompressed rows header. this configures the size of the
 * uncompressed body. */
struct v4l2_buffer_general_rows_uncomp_hdr
{
    unsigned char n_cols_minus1; /* number of quad cols in picture minus 1 */
    unsigned char n_rows_minus1; /* number of quad rows in picture minus 1 */
    unsigned char reserved[2];
};

struct v4l2_buffer_general_block_configs
{
    unsigned char blk_cfg_type;
        #define V4L2_BLOCK_CONFIGS_TYPE_NONE       (0x00)
        #define V4L2_BLOCK_CONFIGS_TYPE_ROW_UNCOMP (0xff)
    unsigned char reserved[3];
    union
    {
        struct v4l2_buffer_general_rows_uncomp_hdr rows_uncomp;
    } blk_cfgs;
};

struct v4l2_buffer_general_ad_stats
{
    unsigned int frame_averages;
        // bitfields
        #define v4l2_AD_STATS_PIC_AVGS_Y      (0)
        #define v4l2_AD_STATS_PIC_AVGS_Y_SZ  (12)
        #define v4l2_AD_STATS_PIC_AVGS_CB    (12)
        #define v4l2_AD_STATS_PIC_AVGS_CB_SZ (10)
        #define v4l2_AD_STATS_PIC_AVGS_CR    (22)
        #define v4l2_AD_STATS_PIC_AVGS_CR_SZ (10)
    unsigned short thumbnail_width;
    unsigned short thumbnail_height;
    unsigned char ad_stats_flags;
        #define v4l2_AD_STATS_PIC_FMT_PROGRESSIVE (0)
        #define v4l2_AD_STATS_PIC_FMT_INTERLACED  (1)
    unsigned char reserved[3];
};

/* input for encoder */
struct v4l2_buffer_param_qp
{
    /* QP (quantization parameter) for encode.
     *
     * When used to set fixed QP for encode, with rate control
     * disabled, then the valid ranges are:
     *   H264: 0-51
     *   HEVC: 0-51
     *   VP8:  0-63
     *   VP9:  0-63
     * Note: The QP must be set separately for I, P and B frames.
     *
     * But when this message is used with the regions-feature,
     * then the valid ranges are the internal bitstream ranges:
     *   H264: 0-51
     *   HEVC: 0-51
     *   VP8:  0-127
     *   VP9:  0-255
     */
    int qp;
    int epr_iframe_enable;
};

/* the block parameter record specifies the various properties of a quad */
struct v4l2_block_param_record
{
    unsigned int qp_delta;
        /* Bitset of four 4-bit QP delta values for a quad.
         * For H.264 and HEVC these are qp delta values in the range -8 to +7.
         * For Vp9 these are segment map values in the range 0 to 7.
         */
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16     (0)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_LEFT_16X16_SZ  (6)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16    (6)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_TOP_RIGHT_16X16_SZ (6)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16     (12)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_LEFT_16X16_SZ  (6)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16    (18)
        #define V4L2_BLOCK_PARAM_RECORD_QP_DELTA_BOT_RIGHT_16X16_SZ (6)

        #define V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD              (24)
        #define V4L2_BLOCK_PARAM_RECORD_QP_FORCE_FIELD_SZ           (5)
        #define V4L2_BLOCK_PARAM_RECORD_QUAD_FORCE_INTRA            (29)
        #define V4L2_BLOCK_PARAM_RECORD_QUAD_FORCE_INTRA_SZ         (1)
        #define V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE                 (30)
        #define V4L2_BLOCK_PARAM_RECORD_QP_ABSOLUTE_SZ              (1)
        #define V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP                (31)
        #define V4L2_BLOCK_PARAM_RECORD_QP_QUAD_SKIP_SZ             (1)

        #define V4L2_BLOCK_PARAM_RECORD_FORCE_NONE  (0x00)
        #define V4L2_BLOCK_PARAM_RECORD_FORCE_QP    (0x01)
    unsigned int min_qp;
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_LEFT_16X16     (0)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_LEFT_16X16_SZ  (6)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_RIGHT_16X16    (6)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_TOP_RIGHT_16X16_SZ (6)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_LEFT_16X16     (12)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_LEFT_16X16_SZ  (6)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_RIGHT_16X16    (18)
        #define V4L2_BLOCK_PARAM_RECORD_MIN_QP_BOT_RIGHT_16X16_SZ (6)
        #define V4L2_BLOCK_PARAM_RECORD_QUAD_SELECT_QP_DELTA      (24)
        #define V4L2_BLOCK_PARAM_RECORD_QUAD_SELECT_QP_DELTA_SZ   (1)

};

struct v4l2_buffer_general_rows_uncomp_body
{
    /* the size of this array is variable and not necessarily equal to 1.
     * therefore the sizeof operator should not be used
     */
    struct v4l2_block_param_record bpr[1];
};

struct v4l2_core_buffer_header_general
{
    //uint64_t user_data_tag;   // User supplied tracking identifier
    //uint64_t app_handle;    // Host buffer handle number
    unsigned short type;  // type of config, value is one of V4L2_BUFFER_GENERAL_TYPE_X

        #define V4L2_BUFFER_GENERAL_TYPE_INVALID       (0) /* invalid */
        #define V4L2_BUFFER_GENERAL_TYPE_BLOCK_CONFIGS (1) /* block_configs */
        #define V4L2_BUFFER_GENERAL_TYPE_AD_STATS      (2) /* assertive display statistics */
    unsigned short config_size;  // size of the configuration
    unsigned int buffer_size;
    union {
        struct v4l2_buffer_general_block_configs config;
        struct v4l2_buffer_general_ad_stats      ad_stats;
    } config;
};

struct v4l2_mvx_huff_table
{
    unsigned int type;
        #define V4L2_OPT_HUFFMAN_TABLE_DC_LUMA               (1)
        #define V4L2_OPT_HUFFMAN_TABLE_AC_LUMA               (2)
        #define V4L2_OPT_HUFFMAN_TABLE_DC_CHROMA             (4)
        #define V4L2_OPT_HUFFMAN_TABLE_AC_CHROMA             (8)
    unsigned char dc_luma_code_lenght[16];
    unsigned char ac_luma_code_lenght[16];
    unsigned char dc_chroma_code_lenght[16];
    unsigned char ac_chroma_code_lenght[16];
    unsigned char dc_luma_table[162];
    unsigned char ac_luma_table[162];
    unsigned char dc_chroma_table[162];
    unsigned char ac_chroma_table[162];
};
struct v4l2_mvx_seamless_target
{
    unsigned int seamless_mode;
    unsigned int target_width;
    unsigned int target_height;
    unsigned int target_stride[3];
    unsigned int target_size[3];
};
struct v4l2_mvx_color_conv_coef
{
    short  coef[3][3];
    unsigned short offset[3];
};

struct v4l2_mvx_rgb2yuv_color_conv_coef
{
    short  coef[3 * 3]; //coef[Y|U|V][R|G|B]
    unsigned char luma_range[2];
    unsigned char chroma_range[2];
    unsigned char rgb_range[2];
};

struct v4l2_mvx_crop_cfg
{
    unsigned char crop_en;
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
};
#endif /* _MVX_V4L2_CONTROLS_H_ */
