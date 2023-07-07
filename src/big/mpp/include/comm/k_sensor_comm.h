/**
 * @file k_sensor_comm.h
 * @author
 * @sxp
 * @version 1.0
 * @date 2023-03-20
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
#ifndef __K_SENSOR_COMM_H__
#define __K_SENSOR_COMM_H__

#include "k_type.h"
#include "k_errno.h"
#include "k_module.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define SENSOR_NUM_MAX 4

#define REG_NULL  0xFFFF

#ifndef MIN
#define MIN(a, b)   ( ((a)<(b)) ? (a) : (b) )
#endif /* MIN */

#ifndef MAX
#define MAX(a, b)   ( ((a)>(b)) ? (a) : (b) )
#endif /* MAX */

/**
 * @brief Defines the config type of sensor
 *
 * @note The type list is maintained by the driver developer,it corresponds to the sensor configuration.
 */
typedef enum {
    OV_OV9732_MIPI_1920X1080_30FPS_10BIT_LINEAR,
    OV_OV9732_MIPI_1920X1080_30FPS_10BIT_HDR,
    OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR,
    OV_OV9286_MIPI_1920X1080_30FPS_10BIT_LINEAR,
    OV_OV9286_MIPI_1920X1080_30FPS_10BIT_HDR,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE,
    IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR,
    IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR,
    IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR,
    IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR,
    IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,
    IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,

    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR,
    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR,
    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_30FPS_LINEAR,

    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR,
    OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR,
    SENSOR_TYPE_MAX,
} k_vicap_sensor_type;

/**
 * @brief Defines the hdr mode of sensor
 *
 */
typedef enum {
    SENSOR_MODE_LINEAR,
    SENSOR_MODE_HDR_STITCH,
    SENSOR_MODE_HDR_NATIVE,
} k_sensor_hdr_mode;

/**
 * @brief Defines the bayer pattern of sensor
 *
 */
typedef enum  {
    BAYER_RGGB = 0,
    BAYER_GRBG = 1,
    BAYER_GBRG = 2,
    BAYER_BGGR = 3,
    BAYER_BUTT
} k_sensor_bayer_pattern;

/**
 * @brief Defines the hdr stich mode of sensor
 *
 */
typedef enum  {
    SENSOR_STITCHING_DUAL_DCG           = 0,    /**< dual DCG mode 3x12-bit */
    SENSOR_STITCHING_3DOL               = 1,    /**< dol3 frame 3x12-bit */
    SENSOR_STITCHING_LINEBYLINE         = 2,    /**< 3x12-bit line by line without waiting */
    SENSOR_STITCHING_16BIT_COMPRESS     = 3,    /**< 16-bit compressed data + 12-bit RAW */
    SENSOR_STITCHING_DUAL_DCG_NOWAIT    = 4,    /**< 2x12-bit dual DCG without waiting */
    SENSOR_STITCHING_2DOL               = 5,    /**< dol2 frame or 1 CG+VS sx12-bit RAW */
    SENSOR_STITCHING_L_AND_S            = 6,    /**< L+S 2x12-bit RAW */
    SENSOR_STITCHING_4DOL               = 7,    /**< dol4 frame 3x12-bit */
    SENSOR_STITCHING_MAX
} k_sensor_stitching_mode;

/**
 * @brief Defines the lens info of sensor
 *
 */
typedef struct {
    k_u32 id;
    k_u8  name[16];
} k_sensor_lens_info;

/**
 * @brief Defines the bayer pattern of sensor
 *
 */
typedef struct {
    k_u32 min;
    k_u32 max;
} k_sensor_fps_range;

/**
 * @brief Defines the bayer pattern of sensor
 *
 */
typedef struct {
    k_u8 csi_id;
    k_u8 mipi_lanes;
    k_u8 data_type;
} k_sensor_mipi_info;

/**
 * @brief Defines the CSI NUM
 *
 */
typedef struct {
    k_sensor_fps_range afps_range;
    k_u32 max_gain;
} k_sensor_auto_fps;

typedef struct  {
    float min;
    float max;
    float step;
} k_sensor_gain_info;

typedef struct {
    float min;
    float max;
} k_sensor_range;

#define SENSOR_LINEAR_PARAS               0
#define SENSOR_DUAL_EXP_L_PARAS           0
#define SENSOR_DUAL_EXP_S_PARAS           1
#define SENSOR_TRI_EXP_L_PARAS            0
#define SENSOR_TRI_EXP_S_PARAS            1
#define SENSOR_TRI_EXP_VS_PARAS           2
#define SENSOR_QUAD_EXP_L_PARAS           0
#define SENSOR_QUAD_EXP_S_PARAS           1
#define SENSOR_QUAD_EXP_VS_PARAS          2
#define SENSOR_QUAD_EXP_VVS_PARAS         3

/**
 * @brief Defines the exposure type of sensor
 *
 */
typedef enum {
    SENSOR_EXPO_FRAME_TYPE_1FRAME  = 0,
    SENSOR_EXPO_FRAME_TYPE_2FRAMES = 1,
    SENSOR_EXPO_FRAME_TYPE_3FRAMES = 2,
    SENSOR_EXPO_FRAME_TYPE_4FRAMES = 3,
    SENSOR_EXPO_FRAME_TYPE_MAX
} k_sensor_exp_frame_type;

/**
 * @brief Defines the ae param of sensor
 *
 */
typedef struct {
    //k_sensor_exp_frame_type frame_type;
    k_u16 frame_length;
    k_u16 cur_frame_length;

    float one_line_exp_time;

    k_u32 gain_accuracy;

    float min_gain;
    float max_gain;

    float integration_time_increment;
    float gain_increment;

    k_u16 max_long_integraion_line;
    k_u16 min_long_integraion_line;

    k_u16 max_integraion_line;
    k_u16 min_integraion_line;

    k_u16 max_vs_integraion_line;
    k_u16 min_vs_integraion_line;

    //u16  max_vvs_integraion_line;
    //u16  min_vvs_integraion_line;

    float max_long_integraion_time;
    float min_long_integraion_time;

    float max_integraion_time;
    float min_integraion_time;

    float max_vs_integraion_time;
    float min_vs_integraion_time;

    //foat max_vvs_integraion_time;
    //foat min_vvs_integraion_time;

    float cur_long_integration_time;
    float cur_integration_time;
    float cur_vs_integration_time;
    //float cur_vvs_integration_time;

    float cur_long_gain;
    float cur_long_again;
    float cur_long_dgain;

    float cur_gain;
    float cur_again;
    float cur_dgain;

    float cur_vs_gain;
    float cur_vs_again;
    float cur_vs_dgain;

    //oat cur_vvs_gain;
    //oat cur_vvs_again;
    //oat cur_vvs_dgain;

    k_sensor_gain_info long_gain;
    k_sensor_gain_info gain;
    k_sensor_gain_info vs_gain;
    //sensor_gain_info vvs_gain;

    k_sensor_gain_info a_long_gain;
    k_sensor_gain_info a_gain;
    k_sensor_gain_info   a_vs_gain;
    //sensor_gain_info a_vvs_gain;

    k_sensor_gain_info d_long_gain;
    k_sensor_gain_info d_gain;
    k_sensor_gain_info d_vs_gain;
    //sensor_gain_info   d_vvs_gain;

    k_u32 max_fps;
    k_u32 min_fps;
    k_u32 cur_fps;
    k_sensor_auto_fps afps_info;
    k_u32 hdr_ratio;
} k_sensor_ae_info;

/**
 * @brief Defines the data compress of sensor
 *
 */
typedef struct {
    k_u32 enable;
    k_u32 x_bit;
    k_u32 y_bit;
} k_sensor_data_compress;

/**
 * @brief Defines the image size of sensor
 *
 */
typedef struct {
    k_u32 bounds_width;
    k_u32 bounds_height;
    k_u32 top;
    k_u32 left;
    k_u32 width;
    k_u32 height;
} k_sensor_size;


typedef enum {
    SENSOR_REG_VALUE_8BIT = 1,
    SENSOR_REG_VALUE_16BIT = 2,
} k_sensor_reg_bits;


/**
 * @brief Defines the register list of sensor
 *
 */
typedef struct {
    k_u16 reg_addr;
    k_u8  reg_val;
} k_sensor_reg_list;

/**
 * @brief Defines the register info sensor
 *
 */
typedef struct {
    k_u32 addr;
    k_u32 val;
} k_sensor_reg;

typedef struct {
    k_u32 mclk;
    k_u32 pclk;
} k_sensor_clk_info;

/**
 * @brief Defines the mode of sensor
 *
 */
typedef struct {
    k_u32 index;
    k_vicap_sensor_type sensor_type;
    k_sensor_size size;
    k_u32 fps;
    k_u32 hdr_mode;
    k_u32 stitching_mode;
    k_u32 bit_width;
    k_sensor_data_compress compress;
    k_u32 bayer_pattern;
    k_sensor_mipi_info mipi_info;
    k_sensor_ae_info ae_info;
    // k_sensor_reg_list *reg_list;
    k_sensor_reg *reg_list;
    k_u32 max_fps;
    k_sensor_clk_info *clk_info;
} k_sensor_mode;

/**
 * @brief Defines the sensor enum mode
 *
 */
typedef struct {
    k_u32 index;
    k_sensor_mode mode;
} k_sensor_enum_mode;

/**
 * @brief Defines the sensor resolution
 *
 */
typedef struct {
    k_u16 width;
    k_u16 height;
} k_sensor_resolution;

/**
 * @brief Defines the sensor capabilities
 *
 */
typedef struct {
    k_u32 bit_width;                  /**< supported bus-width */
    k_u32 mode;                      /**< supported operating modes */
    k_u32 bayer_pattern;              /**< bayer pattern */
    k_sensor_resolution resolution;         /**< supported resolutions */
    k_u32 mipi_mode;
    k_u32 mipi_lanes;
    k_u32 vin_type;
} k_sensor_caps;

/**
 * @brief Defines the compand curve of sensor
 *
 */
typedef struct {
    k_u32 x_bit;
    k_u32 y_bit;
    k_u8  compand_px[64];
    k_u32 compand_x_data[65];
    k_u32 compand_y_data[65];
} k_sensor_compand_curve;

/**
 * @brief Defines the test pattern of sensor
 *
 */
typedef struct {
    k_u32 enable;
    k_u32 pattern;
} k_sensor_test_pattern;

/**
 * @brief Defines the blc info of sensor
 *
 */
typedef struct {
    k_u32 red;
    k_u32 gr;
    k_u32 gb;
    k_u32 blue;
} k_sensor_blc;

/**
 * @brief Defines the white balance info of sensor
 *
 */
typedef struct {
    k_u32 r_gain;
    k_u32 gr_gain;
    k_u32 gb_gain;
    k_u32 b_gain;
} k_sensor_white_balance;

/**
 * @brief Defines the isp status of sensor
 *
 */
typedef struct {
    k_bool use_sensor_ae;
    k_bool use_sensor_blc;
    k_bool use_sensor_awb;
} k_sensor_isp_status;

/**
 * @brief Defines the sensor gain
 *
 */
typedef struct {
    k_u8    exp_frame_type;
    float   gain[SENSOR_EXPO_FRAME_TYPE_MAX];
} k_sensor_gain;

/**
 * @brief Defines the integration time of sensor
 *
 */
typedef struct {
    k_u8    exp_frame_type;
    float   intg_time[SENSOR_EXPO_FRAME_TYPE_MAX];
} k_sensor_intg_time;

/**
 * @brief Defines the exposure param of sensor
 *
 */
typedef struct {
    k_u8    exp_frame_type;
    float   gain[SENSOR_EXPO_FRAME_TYPE_MAX];
    float   exp_time[SENSOR_EXPO_FRAME_TYPE_MAX];
} k_sensor_exposure_param;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

