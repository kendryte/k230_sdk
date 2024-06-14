/**
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

#ifndef __K_VIDEO_COMM_H__
#define __K_VIDEO_COMM_H__

#include "k_type.h"
#include "k_video_comm.h"
#include "k_module.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /**<  __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

#define DCF_DRSCRIPTION_LENGTH  32
#define DCF_CAPTURE_TIME_LENGTH 20

typedef enum
{
    PIXEL_FORMAT_RGB_444 = 0,
    PIXEL_FORMAT_RGB_555,
    PIXEL_FORMAT_RGB_565,
    PIXEL_FORMAT_RGB_888,

    PIXEL_FORMAT_BGR_444,
    PIXEL_FORMAT_BGR_555,
    PIXEL_FORMAT_BGR_565,
    PIXEL_FORMAT_BGR_888,

    PIXEL_FORMAT_ARGB_1555,
    PIXEL_FORMAT_ARGB_4444,
    PIXEL_FORMAT_ARGB_8565,
    PIXEL_FORMAT_ARGB_8888,
    PIXEL_FORMAT_ARGB_2BPP,

    PIXEL_FORMAT_ABGR_1555,
    PIXEL_FORMAT_ABGR_4444,
    PIXEL_FORMAT_ABGR_8565,
    PIXEL_FORMAT_ABGR_8888,

    PIXEL_FORMAT_BGRA_8888,

    PIXEL_FORMAT_RGB_MONOCHROME_8BPP,

    PIXEL_FORMAT_RGB_BAYER_8BPP,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    PIXEL_FORMAT_RGB_BAYER_14BPP,
    PIXEL_FORMAT_RGB_BAYER_16BPP,


    PIXEL_FORMAT_YVU_PLANAR_422,
    PIXEL_FORMAT_YVU_PLANAR_420,
    PIXEL_FORMAT_YVU_PLANAR_444,

    PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    PIXEL_FORMAT_YVU_SEMIPLANAR_444,

    PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    PIXEL_FORMAT_YUYV_PACKAGE_422,
    PIXEL_FORMAT_YVYU_PACKAGE_422,
    PIXEL_FORMAT_UYVY_PACKAGE_422,
    PIXEL_FORMAT_VYUY_PACKAGE_422,
    PIXEL_FORMAT_YYUV_PACKAGE_422,
    PIXEL_FORMAT_YYVU_PACKAGE_422,
    PIXEL_FORMAT_UVYY_PACKAGE_422,
    PIXEL_FORMAT_VUYY_PACKAGE_422,
    PIXEL_FORMAT_VY1UY0_PACKAGE_422,
    PIXEL_FORMAT_YUV_PACKAGE_444,

    PIXEL_FORMAT_YUV_400,
    PIXEL_FORMAT_UV_420,


    /* SVP data format */
    PIXEL_FORMAT_BGR_888_PLANAR,
    PIXEL_FORMAT_RGB_888_PLANAR,
    PIXEL_FORMAT_HSV_888_PACKAGE,
    PIXEL_FORMAT_HSV_888_PLANAR,
    PIXEL_FORMAT_LAB_888_PACKAGE,
    PIXEL_FORMAT_LAB_888_PLANAR,
    PIXEL_FORMAT_S8C1,
    PIXEL_FORMAT_S8C2_PACKAGE,
    PIXEL_FORMAT_S8C2_PLANAR,
    PIXEL_FORMAT_S8C3_PLANAR,
    PIXEL_FORMAT_S16C1,
    PIXEL_FORMAT_U8C1,
    PIXEL_FORMAT_U16C1,
    PIXEL_FORMAT_S32C1,
    PIXEL_FORMAT_U32C1,
    PIXEL_FORMAT_U64C1,
    PIXEL_FORMAT_S64C1,

    PIXEL_FORMAT_RGB_565_LE = 300,
    PIXEL_FORMAT_BGR_565_LE,

    PIXEL_FORMAT_BUTT
} k_pixel_format;

typedef enum
{
    VIDEO_FIELD_TOP         = 0x1,    /* even field */
    VIDEO_FIELD_BOTTOM      = 0x2,    /* odd field */
    VIDEO_FIELD_INTERLACED  = 0x3,    /* two interlaced fields */
    VIDEO_FIELD_FRAME       = 0x4,    /* frame */

    VIDEO_FIELD_BUTT
} k_video_field;

typedef enum
{
    VIDEO_FORMAT_LINEAR = 0,       /* nature video line */
    VIDEO_FORMAT_TILE_64x16,       /* tile cell: 64pixel x 16line */
    VIDEO_FORMAT_TILE_16x8,        /* tile cell: 16pixel x 8line */
    VIDEO_FORMAT_LINEAR_DISCRETE,  /* The data bits are aligned in bytes */
    VIDEO_FORMAT_BUTT
} k_video_format;

typedef enum
{
    DYNAMIC_RANGE_SDR8 = 0,
    DYNAMIC_RANGE_SDR10,
    DYNAMIC_RANGE_HDR10,
    DYNAMIC_RANGE_HLG,
    DYNAMIC_RANGE_SLF,
    DYNAMIC_RANGE_XDR,
    DYNAMIC_RANGE_BUTT
} k_dynamic_range;

typedef enum
{
    COLOR_GAMUT_BT601 = 0,
    COLOR_GAMUT_BT709,
    COLOR_GAMUT_BT2020,
    COLOR_GAMUT_USER,
    COLOR_GAMUT_BUTT
} k_color_gamut;

typedef enum
{
    COMPRESS_MODE_NONE = 0,   /* no compress */
    COMPRESS_MODE_SEG,        /* compress unit is 256x1 bytes as a segment.*/
    COMPRESS_MODE_TILE,       /* compress unit is a tile.*/
    COMPRESS_MODE_LINE,       /* compress unit is the whole line.  raw for VI  */
    COMPRESS_MODE_FRAME,      /* compress unit is the whole frame. YUV for VI(3DNR), RGB for TDE(write)/VO(read) */

    COMPRESS_MODE_BUTT
} k_compress_mode;


typedef struct
{
    k_u8       image_description[DCF_DRSCRIPTION_LENGTH];        /**< describes image*/
    k_u8       make[DCF_DRSCRIPTION_LENGTH];                    /**< shows manufacturer of digital cameras*/
    k_u8       model[DCF_DRSCRIPTION_LENGTH];                   /**< shows model number of digital cameras*/
    k_u8       software[DCF_DRSCRIPTION_LENGTH];                /**< shows firmware (internal software of digital cameras) version number*/

    k_u8       light_source;                                      /**< light source, actually this means white balance setting. '0' means unknown, '1' daylight, '2'
                                                                               fluorescent, '3' tungsten, '10' flash, '17' standard light A, '18' standard light B, '19' standard light
                                                                               C, '20' D55, '21' D65, '22' D75, '255' other*/
    k_u32      focal_length;                                     /**< focal length of lens used to take image. unit is millimeter*/
    k_u8       scene_type;                                        /**< indicates the type of scene. value '0x01' means that the image was directly photographed.*/
    k_u8       custom_rendered;                                   /**< indicates the use of special processing on image data, such as rendering geared to output.
                                                                               0 = normal process  1 = custom process   */
    k_u8       focal_length_in35mm_film;                            /**< indicates the equivalent focal length assuming a 35mm film camera, in mm*/
    k_u8       scene_capture_type;                                 /**< indicates the type of scene that was shot. 0 = standard,1 = landscape,2 = portrait,3 = night scene. */
    k_u8       gain_control;                                      /**< indicates the degree of overall image gain adjustment. 0 = none,1 = low gain up,2 = high gain up,3 = low gain down,4 = high gain down. */
    k_u8       contrast;                                         /**< indicates the direction of contrast processing applied by the camera when the image was shot.
                                                                               0 = normal,1 = soft,2 = hard */
    k_u8       saturation;                                       /**< indicates the direction of saturation processing applied by the camera when the image was shot.
                                                                              0 = normal,1 = low saturation,2 = high saturation*/
    k_u8       sharpness;                                        /**< indicates the direction of sharpness processing applied by the camera when the image was shot.
                                                                              0 = normal,1 = soft,2 = hard .*/
    k_u8       metering_mode;                                     /**< exposure metering method. '0' means unknown, '1' average, '2' center weighted average, '3'
                                                                              spot, '4' multi-spot, '5' multi-segment, '6' partial, '255' other*/
} k_isp_dcf_const_info;

typedef struct
{
    k_u32      iso_speed_ratings;                                 /**< CCD sensitivity equivalent to ag-hr film speedrate*/
    k_u32      exposure_time;                                    /**< exposure time (reciprocal of shutter speed).*/
    k_u32      exposure_bias_value;                               /**< exposure bias (compensation) value of taking picture*/
    k_u8       exposure_program;                                  /**< exposure program that the camera used when image was taken. '1' means manual control, '2'
                                                                              program normal, '3' aperture priority, '4' shutter priority, '5' program creative (slow program),
                                                                              '6' program action(high-speed program), '7' portrait mode, '8' landscape mode*/
    k_u32      f_number;                                         /**< the actual F-number (F-stop) of lens when the image was taken*/
    k_u32      max_aperture_value;                                /**< maximum aperture value of lens.*/
    k_u8       exposure_mode;                                     /**< indicates the exposure mode set when the image was shot.
                                                                              0 = auto exposure,1 = manual exposure, 2 = auto bracket*/
    k_u8       white_balance;                                     /**<  indicates the white balance mode set when the image was shot.
                                                                                0 = auto white balance ,1 = manual white balance */
} k_isp_dcf_update_info;

typedef struct
{
    k_isp_dcf_const_info  isp_dcf_const_info;
    k_isp_dcf_update_info isp_dcf_update_info;
} k_isp_dcf_info;

typedef struct
{
    k_u8           capture_time[DCF_CAPTURE_TIME_LENGTH];            /**< the date and time when the picture data was generated*/
    k_u32         flash;                                             /**< whether the picture is captured when a flash lamp is on*/
    k_u32          digital_zoom_ratio;                                /**< indicates the digital zoom ratio when the image was shot.
                                                                                   if the numerator of the recorded value is 0, this indicates that digital zoom was not used.*/
    k_isp_dcf_info isp_dcf_info;
} k_jpeg_dcf;

typedef struct
{
    k_u32      iso;                    /**<  ISP internal ISO : again*dgain*is_pgain */
    k_u32      exposure_time;           /**<  exposure time (reciprocal of shutter speed),unit is us */
    k_u32      isp_dgain;
    k_u32      again;
    k_u32      dgain;
    k_u32      ratio[3];
    k_u32      isp_nr_strength;
    k_u32      f_number;                /**<  the actual F-number (F-stop) of lens when the image was taken */
    k_u32      sensor_id;               /**<  which sensor is used */
    k_u32      sensor_mode;
    k_u32      hmax_times;              /**<  sensor hmax_times,unit is ns */
    k_u32      vmax;                   /**<  sensor vmax,unit is line */
    k_u32      vc_num;                  /**<  when dump wdr frame, which is long or short  exposure frame. */
} k_isp_frame_info;

typedef struct
{
    k_u64   jpeg_dcf_phy_addr;
    k_u64   isp_info_phy_addr;

    void *jpeg_dcf_kvirt_addr;        /**<  jpeg_dcf, used in JPEG DCF */
    void *isp_info_kvirt_addr;        /**<  isp_frame_info, used in ISP debug, when get raw and send raw */
} k_video_supplement;

/**
 * @brief Describe the data structure of video
 *
 */
typedef struct
{
    k_u32               width;   /**< Picture width  */
    k_u32               height;  /**< Picture height */
    k_video_field       field;   /**< video frame filed*/
    k_pixel_format      pixel_format; /**< Pixel format of a  picture */
    k_video_format      video_format;
    k_dynamic_range     dynamic_range;
    k_compress_mode     compress_mode;
    k_color_gamut       color_gamut;

    k_u32               header_stride[3];
    k_u32               stride[3];

    k_u64               header_phys_addr[3];
    k_u64               header_virt_addr[3];

    k_u64               phys_addr[3];
    k_u64               virt_addr[3];

    k_s16               offset_top;        /* top offset of show area */
    k_s16               offset_bottom;    /* bottom offset of show area */
    k_s16               offset_left;        /* left offset of show area */
    k_s16               offset_right;        /* right offset of show area */

    k_u32               time_ref;
    k_u64               pts;

    k_u64               priv_data;     /* bit 0:7 to fill light ctrl state */
    k_video_supplement  supplement; /**< Supplementary information about images */
} k_video_frame;

/**
 * @brief Defines the video picture information
 *
 */
typedef struct
{
    k_video_frame v_frame;  /**< Video picture frame */
    k_u32        pool_id;   /**< VB pool ID */
    k_mod_id      mod_id;   /**< Logical unit for generating video frames */
} k_video_frame_info;

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus */

#endif /*  __K_VIDEO_COMM_H__ */
