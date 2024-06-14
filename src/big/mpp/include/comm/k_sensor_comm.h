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

#define SENSOR_NUM_MAX   13

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
    OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR = 0,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR = 1,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE = 2,

    OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR = 3,
    OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_SPECKLE = 4,

    OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE = 5,
    OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE  = 6,

    IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR = 7,
    IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR = 8,
    IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR = 9,
    IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR = 10,
    IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR = 11,
    IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR = 12,

    IMX335_MIPI_4LANE_RAW10_2XDOL = 13,
    IMX335_MIPI_4LANE_RAW10_3XDOL = 14,

    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR = 15,
    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR = 16,
    SC_SC035HGS_MIPI_1LANE_RAW10_640X480_30FPS_LINEAR = 17,

    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE = 18,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR = 19,
    OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR = 20,

    OV_OV5647_MIPI_1920X1080_30FPS_10BIT_LINEAR = 21,
    OV_OV5647_MIPI_2592x1944_10FPS_10BIT_LINEAR = 22,
    OV_OV5647_MIPI_640x480_60FPS_10BIT_LINEAR = 23,
    OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR = 24,

    SC_SC201CS_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR = 25,
    SC_SC201CS_SLAVE_MODE_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR = 26,

    OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR = 27,
    OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR = 28,

    XS9922B_MIPI_CSI0_1280X720_30FPS_YUV422_DOL3 = 29,

    XS9950_MIPI_CSI0_1280X720_30FPS_YUV422 = 30,
    XS9950_MIPI_CSI1_1280X720_30FPS_YUV422 = 31,
    XS9950_MIPI_CSI2_1280X720_30FPS_YUV422 = 32,
    XS9950_MIPI_CSI0_1920X1080_30FPS_YUV422 = 33,

    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2 = 34,
    OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR_V2 = 35,
    OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR_V2 = 36,

    OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR_V2 = 37,
    OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR_V2 = 38,
    OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2 = 39,

    GC2053_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR = 40,

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

typedef enum {
    SENSOR_AF_MODE_NOTSUPP,
    SENSOR_AF_MODE_CDAF,
    SENSOR_AF_MODE_PDAF,
} k_sensor_af_mode;

typedef enum {
	SENSOR_COLOR = 0,	//color sensor and with IR filter, output color image
	SENSOR_MONO = 1,	//mono sensor
	SENSOR_COLOR_GRAY = 2,	//color sensor, output gray image, don't need WB&color correction
	SENSOR_COLOR_IR = 3,	//color sensor and without IR filter, output color image
} k_sensor_color_type;

/**
 * @brief Defines the bayer pattern of sensor
 *
 */
typedef enum  {
    BAYER_PAT_RGGB      = 0x00,
    BAYER_PAT_GRBG      = 0x01,
    BAYER_PAT_GBRG      = 0x02,
    BAYER_PAT_BGGR      = 0x03,
    BAYER_PAT_BGGIR     = 0x10,
    BAYER_PAT_GRIRG     = 0x11,
    BAYER_PAT_RGGIR     = 0x12,
    BAYER_PAT_GBIRG     = 0x13,
    BAYER_PAT_GIRRG     = 0x14,
    BAYER_PAT_IRGGB     = 0x15,
    BAYER_PAT_GIRBG     = 0x16,
    BAYER_PAT_IRGGR     = 0x17,
    BAYER_PAT_RGIRB     = 0x18,
    BAYER_PAT_GRBIR     = 0x19,
    BAYER_PAT_IRBRG     = 0x20,
    BAYER_PAT_BIRGR     = 0x21,
    BAYER_PAT_BGIRR     = 0x22,
    BAYER_PAT_GBRIR     = 0x23,
    BAYER_PAT_IRRBG     = 0x24,
    BAYER_PAT_RIRGB     = 0x25,
    BAYER_PAT_RCCC      = 0x30,
    BAYER_PAT_RCCB      = 0x40,
    BAYER_PAT_RYYCY     = 0x50,
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
 * @brief Defines the hdr native mode of sensor
 *
 */
typedef enum {
    SENSOR_NATIVE_DCG               = 0,    /**< hcg and lcg combine in sensor*/
    SENSOR_NATIVE_L_AND_S           = 1,    /**< L+S combine in sensor*/
    SENSOR_NATIVE_3DOL              = 2,    /**< 3dol combine in sensor*/
    SENSOR_NATIVE_4DOL              = 3,    /**< 4dol combine in sensor*/
    SENSOR_NATIVE_DCG_SPD_VS        = 4,    /**< 4dol combine in sensor*/
    SENSOR_NATIVE_MAX
} k_sensor_native_mode;

/**
 * @brief Defines the pll info of sensor
 *
 */
typedef enum {
    SENSOR_PLL0_CLK_DIV4 = 5,
    SENSOR_PLL1_CLK_DIV3 = 8,
    SENSOR_PLL1_CLK_DIV4 = 9,
} k_sensor_mclk_sel;

/**
 * @brief Defines the mclk id of sensor
 *
 */
typedef enum {
    SENSOR_MCLK0 = 1,
    SENSOR_MCLK1 = 2,
    SENSOR_MCLK2 = 3,
    SENSOR_MCLK_MAX,
} k_sensor_mclk_id;

/**
 * @brief Defines the mclk info of sensor
 *
 */
typedef struct {
    k_sensor_mclk_id id;
    k_sensor_mclk_sel mclk_sel;
    k_u8 mclk_div;
} k_sensor_mclk;

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
    //SENSOR_EXPO_FRAME_TYPE_1FRAME  = 0,
    //SENSOR_EXPO_FRAME_TYPE_2FRAMES = 1,
    //SENSOR_EXPO_FRAME_TYPE_3FRAMES = 2,
    //SENSOR_EXPO_FRAME_TYPE_4FRAMES = 3,
    SENSOR_EXPO_FRAME_TYPE_MAX = 4
} k_sensor_exp_frame_type;

/**
 * @brief Defines the ae param of sensor
 *
 */
typedef struct {
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

    float max_long_integraion_time;
    float min_long_integraion_time;

    float max_integraion_time;
    float min_integraion_time;

    float max_vs_integraion_time;
    float min_vs_integraion_time;


    float cur_long_integration_time;
    float cur_integration_time;
    float cur_vs_integration_time;

    float cur_long_gain;
    float cur_long_again;
    float cur_long_dgain;

    float cur_gain;
    float cur_again;
    float cur_dgain;

    float cur_vs_gain;
    float cur_vs_again;
    float cur_vs_dgain;


    k_sensor_gain_info long_gain;
    k_sensor_gain_info gain;
    k_sensor_gain_info vs_gain;

    k_sensor_gain_info a_long_gain;
    k_sensor_gain_info a_gain;
    k_sensor_gain_info   a_vs_gain;

    k_sensor_gain_info d_long_gain;
    k_sensor_gain_info d_gain;
    k_sensor_gain_info d_vs_gain;

    k_u32 max_fps;
    k_u32 min_fps;
    k_u32 cur_fps;
    k_sensor_auto_fps afps_info;
    k_u32 hdr_ratio;

    k_u32 int_time_delay_frame;
    k_u32 gain_delay_frame;

    //float ae_min_interval_frame;
    k_u8 color_type;		//0, color image; 1, mono sensor image; 2, color sensor gray image.
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

typedef struct {
    k_bool mclk_setting_en;
    k_sensor_mclk setting;
} k_sensor_mclk_setting;

/**
 * @brief Defines the mode of sensor
 *
 */
typedef struct {
    k_u32 index;
    k_vicap_sensor_type sensor_type;
    k_sensor_size size;
    k_sensor_hdr_mode hdr_mode;
    k_sensor_stitching_mode stitching_mode;
    k_sensor_native_mode native_mode;
    k_sensor_data_compress compress;
    k_sensor_bayer_pattern bayer_pattern;
    k_sensor_mipi_info mipi_info;
    k_sensor_ae_info ae_info;
    k_u32 bit_width;
    const k_sensor_reg *reg_list;
    k_sensor_clk_info *clk_info;
    k_sensor_af_mode af_mode;
    k_u32 sensor_again;
    k_u32 et_line;
    k_u32 fps;
    k_sensor_mclk_setting mclk_setting[SENSOR_MCLK_MAX - 1];
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
    float r_gain;
    float gr_gain;
    float gb_gain;
    float b_gain;
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
    //k_u8    exp_frame_type;
    float   gain[4];
} k_sensor_gain;

/**
 * @brief Defines the integration time of sensor
 *
 */
typedef struct {
    //k_u8    exp_frame_type;
    float   intg_time[4];
} k_sensor_intg_time;

/**
 * @brief Defines the exposure param of sensor
 *
 */
typedef struct {
    //k_u8    exp_frame_type;
    float   gain[4];
    float   exp_time[4];
} k_sensor_exposure_param;

typedef struct {
    k_u8   otp_type;
    k_u8   otp_date[20];
} k_sensor_otp_date;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
