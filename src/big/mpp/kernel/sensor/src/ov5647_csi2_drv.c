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

#include "sensor_dev.h"
#include "k_board_config_comm.h"

//#include <riscv_io.h>
#include "io.h"
#include "drv_gpio.h"

#define pr_info(...)    //rt_kprintf(__VA_ARGS__)
#define pr_debug(...)   //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)     rt_kprintf(__VA_ARGS__)

#define DELAY_MS_SENSOR_DEFAULT                             100

#define OV5647_REG_CHIP_ID_H                                0x300a
#define OV5647_REG_CHIP_ID_L                                0x300b
#define OV5647_REG_MIPI_CTRL00                              0x4800
#define OV5647_REG_FRAME_OFF_NUMBER                         0x4202
#define OV5647_REG_PAD_OUT                                  0x300d

#define OV5647_REG_MIPI_CTRL14                              0x4814

#define OV5647_SW_STANDBY                                   0x0100


#define OV5647_REG_LONG_AGAIN_H                             0x350a
#define OV5647_REG_LONG_AGAIN_L                             0x350b

#define OV5647_REG_LONG_EXP_TIME_H                          0x3501
#define OV5647_REG_LONG_EXP_TIME_L                          0x3502

#define OV5647_MIN_GAIN_STEP                                (1.0f/16.0f)
#define OV5647_SW_RESET                                           0x0103
#define MIPI_CTRL00_CLOCK_LANE_GATE                         (1 << 5)
#define MIPI_CTRL00_LINE_SYNC_ENABLE                        (1 << 4)
#define MIPI_CTRL00_BUS_IDLE                                (1 << 1)
#define MIPI_CTRL00_CLOCK_LANE_DISABLE                      (1 << 0)

static k_bool mirror_flag = 0;

static k_sensor_reg sensor_mirror_reg[] = {
    {0x3820, 0x00},
    {0x3821, 0x00},
    {REG_NULL, 0x00},
};

static const k_sensor_reg sensor_oe_disable_regs[] = {
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {REG_NULL, 0x00},
};

static const k_sensor_reg sensor_oe_enable_regs[] = {
    {0x3000, 0x0f},
    {0x3001, 0xff},
    {0x3002, 0xe4},
    {REG_NULL, 0x00},
};


static const k_sensor_reg ov5647_mipi2lane_1080p_30fps_linear[] = {
    //pixel_rate = 81666700
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3034, 0x1a},
    {0x3035, 0x21},
    {0x3036, 0x62},
    {0x303c, 0x11},
    {0x3106, 0xf5},
    {0x3821, 0x02},
    {0x3820, 0x00},
    {0x3827, 0xec},
    {0x370c, 0x03},
    {0x3612, 0x5b},
    {0x3618, 0x04},
    {0x5000, 0x06},
    {0x5001, 0x00},         // set awb disble
    {0x5002, 0x00},//41
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3016, 0x08},
    {0x3017, 0xe0},
    {0x3018, 0x44},
    {0x301c, 0xf8},
    {0x301d, 0xf0},
    {0x3a18, 0x03},//00
    {0x3a19, 0xff},//f8
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x09},         // h totle  high
    {0x380d, 0x70},         // h totle low
    {0x380e, 0x04},         // v totle high
    {0x380f, 0x66},         // v totle lo 50   0x66
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3708, 0x64},
    {0x3709, 0x12},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x3800, 0x01},
    {0x3801, 0x5c},
    {0x3802, 0x01},
    {0x3803, 0xb2},
    {0x3804, 0x08},
    {0x3805, 0xe3},
    {0x3806, 0x05},
    {0x3807, 0xf1},
    {0x3811, 0x04},
    {0x3813, 0x02},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3636, 0x06},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a00, 0x00},
    {0x3a08, 0x01},
    {0x3a09, 0x4b},
    {0x3a0a, 0x01},
    {0x3a0b, 0x13},
    {0x3a0d, 0x04},
    {0x3a0e, 0x03},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x04},
    {0x4000, 0x09},
    {0x4837, 0x19},
    {0x4800, 0x34},
    {0x3503, 0x07},         //0x0f
    {0x350b, 0x10},         // gain
    // {0x0100, 0x01},
    {REG_NULL, 0x00},
};


static const k_sensor_reg  ov5647_2592x1944_10bpp[] = {
    //pixel_rate = 87500000
    {0x0100, 0x00},
    {0x0103, 0x01},
    {0x3034, 0x1a},
    {0x3035, 0x21},
    {0x3036, 0x69},
    {0x303c, 0x11},
    {0x3106, 0xf5},
    {0x3821, 0x02},  //0x00
    {0x3820, 0x00},
    {0x3827, 0xec},
    {0x370c, 0x03},
    {0x3612, 0x5b},
    {0x3618, 0x04},
    {0x5000, 0x06},
    {0x5001, 0x00},         // set awb disble
    {0x5002, 0x00},
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3016, 0x08},
    {0x3017, 0xe0},
    {0x3018, 0x44},
    {0x301c, 0xf8},
    {0x301d, 0xf0},
    {0x3a18, 0x03},
    {0x3a19, 0xff},
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x0b},
    {0x380d, 0x1c},
    {0x380e, 0x08},
    {0x380f, 0x03},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3708, 0x64},
    {0x3709, 0x12},
    {0x3808, 0x0a},
    {0x3809, 0x20},
    {0x380a, 0x07},
    {0x380b, 0x98},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x3f},
    {0x3806, 0x07},
    {0x3807, 0xa3},
    {0x3811, 0x10},
    {0x3813, 0x06},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3636, 0x06},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a00, 0x00},
    {0x3a08, 0x01},
    {0x3a09, 0x28},
    {0x3a0a, 0x00},
    {0x3a0b, 0xf6},
    {0x3a0d, 0x08},
    {0x3a0e, 0x06},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x04},
    {0x4000, 0x09},
    {0x4837, 0x19},
    {0x4800, 0x34}, //0x24},
    {0x3503, 0x07},         //0x0f
    {0x350b, 0x10},         // gain
    {0x0100, 0x01},
    {REG_NULL, 0x00},
};

static const k_sensor_reg ov5647_640x480_10bpp[] = {
    //pixel_rate = 55000000
    {0x0100, 0x00},
    {0x0103, 0x01},
    {0x3035, 0x11},
    {0x3036, 0x46},
    {0x303c, 0x11},
    {0x3821, 0x01},
    {0x3820, 0x41},
    {0x370c, 0x03},
    {0x3612, 0x59},
    {0x3618, 0x00},
    {0x5000, 0x06},
    {0x5001, 0x00},         // set awb disble
    {0x5002, 0x00},
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0xff},
    {0x3001, 0xff},
    {0x3002, 0xff},
    {0x301d, 0xf0},
    {0x3a18, 0x03},
    {0x3a19, 0xff},
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x07},         // 73c = 1852
    {0x380d, 0x3c},

    {0x380e, 0x01},         // v totle high
     {0x380f, 0xf8},         // v totle lo  504

    {0x3814, 0x35},
    {0x3815, 0x35},
    {0x3708, 0x64},
    {0x3709, 0x52},
    {0x3808, 0x02},
    {0x3809, 0x80},
    {0x380a, 0x01},
    {0x380b, 0xe0},
    {0x3800, 0x00},
    {0x3801, 0x10},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x2f},
    {0x3806, 0x07},
    {0x3807, 0x9f},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a00, 0x00},
    {0x3a08, 0x01},
    {0x3a09, 0x2e},
    {0x3a0a, 0x00},
    {0x3a0b, 0xfb},
    {0x3a0d, 0x02},
    {0x3a0e, 0x01},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x02},
    {0x4000, 0x09},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3017, 0xe0},
    {0x301c, 0xfc},
    {0x3636, 0x06},
    {0x3016, 0x08},
    {0x3827, 0xec},
    {0x3018, 0x44},
    {0x3035, 0x21},
    {0x3106, 0xf5},
    {0x3034, 0x1a},
    {0x301c, 0xf8},
    {0x4800, 0x34},
    {0x3503, 0x07},         //0x0f
    {0x350b, 0x10},         // gain
    {0x0100, 0x01},
    {REG_NULL, 0x00},
};


static const k_sensor_reg ov5647_mipi2lane_1080p_30fps_flip_linear[] = {
    //pixel_rate = 81666700
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3034, 0x1a},
    {0x3035, 0x21},
    {0x3036, 0x62},
    {0x303c, 0x11},
    {0x3106, 0xf5},
    {0x3821, 0x00},
    {0x3820, 0x02},  //0x00
    {0x3827, 0xec},
    {0x370c, 0x03},
    {0x3612, 0x5b},
    {0x3618, 0x04},
    {0x5000, 0x06},
    {0x5001, 0x00},         // set awb disble
    {0x5002, 0x00},//41
    {0x5003, 0x08},
    {0x5a00, 0x08},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3016, 0x08},
    {0x3017, 0xe0},
    {0x3018, 0x44},
    {0x301c, 0xf8},
    {0x301d, 0xf0},
    {0x3a18, 0x03},//00
    {0x3a19, 0xff},//f8
    {0x3c01, 0x80},
    {0x3b07, 0x0c},
    {0x380c, 0x08},         // h totle  high
    {0x380d, 0xdf},         // h totle low
    {0x380e, 0x04},         // v totle high
    {0x380f, 0xaf},         // v totle low
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3708, 0x64},
    {0x3709, 0x12},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x3800, 0x01},
    {0x3801, 0x5c},
    {0x3802, 0x01},
    {0x3803, 0xb2},
    {0x3804, 0x08},
    {0x3805, 0xe3},
    {0x3806, 0x05},
    {0x3807, 0xf1},
    {0x3811, 0x04},
    {0x3813, 0x02},
    {0x3630, 0x2e},
    {0x3632, 0xe2},
    {0x3633, 0x23},
    {0x3634, 0x44},
    {0x3636, 0x06},
    {0x3620, 0x64},
    {0x3621, 0xe0},
    {0x3600, 0x37},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x3731, 0x02},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3f05, 0x02},
    {0x3f06, 0x10},
    {0x3f01, 0x0a},
    {0x3a00, 0x00},
    {0x3a08, 0x01},
    {0x3a09, 0x4b},
    {0x3a0a, 0x01},
    {0x3a0b, 0x13},
    {0x3a0d, 0x04},
    {0x3a0e, 0x03},
    {0x3a0f, 0x58},
    {0x3a10, 0x50},
    {0x3a1b, 0x58},
    {0x3a1e, 0x50},
    {0x3a11, 0x60},
    {0x3a1f, 0x28},
    {0x4001, 0x02},
    {0x4004, 0x04},
    {0x4000, 0x09},
    {0x4837, 0x19},
    {0x4800, 0x34},
    {0x3501, 0x02},	//ET = 42 lines
    {0x3502, 0xa0},
    {0x3503, 0x07},         //0x0f
    {0x350b, 0x10},         // gain
    // {0x0100, 0x01},
    {REG_NULL, 0x00},
};


static k_sensor_mode ov5647_mode_info[] = {
    {
        .index = 0,
        .sensor_type = OV_OV5647_MIPI_1920X1080_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1920,
            .bounds_height = 1080,
            .top = 0,
            .left = 0,
            .width = 1920,
            .height = 1080,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_GBRG,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov5647_mipi2lane_1080p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 1,
        .sensor_type = OV_OV5647_MIPI_2592x1944_10FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 10000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_GBRG,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov5647_2592x1944_10bpp,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 2,
        .sensor_type = OV_OV5647_MIPI_640x480_60FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 640,
            .bounds_height = 480,
            .top = 0,
            .left = 0,
            .width = 640,
            .height = 480,
        },
        .fps = 60000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov5647_640x480_10bpp,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 3,
        .sensor_type = OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1920,
            .bounds_height = 1080,
            .top = 0,
            .left = 0,
            .width = 1920,
            .height = 1080,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_GBRG,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov5647_mipi2lane_1080p_30fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },
            {K_FALSE},
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },
        },
    },
    {
        .index = 4,
        .sensor_type = OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2,
        .size = {
            .bounds_width = 1920,
            .bounds_height = 1080,
            .top = 0,
            .left = 0,
            .width = 1920,
            .height = 1080,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_GRBG,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov5647_mipi2lane_1080p_30fps_flip_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },

            {K_FALSE},
        },
    },

};

static k_sensor_mode *current_mode = NULL;

static int ov5647_power_rest(k_s32 on)
{
    // #define OV5647_CAM_PIN_CSI2      (24)

    rt_kprintf("ov5647_power_rest OV5647_CAM_PIN_CSI2 is %d \n", OV5647_CAM_PIN_CSI2);
    // rst
    kd_pin_mode(OV5647_CAM_PIN_CSI2, GPIO_DM_OUTPUT);

    kd_pin_write(OV5647_CAM_PIN_CSI2, GPIO_PV_HIGH);

    if (on)
    {
        rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);
        kd_pin_write(OV5647_CAM_PIN_CSI2, GPIO_PV_LOW);  //GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);
        kd_pin_write(OV5647_CAM_PIN_CSI2, GPIO_PV_HIGH);
    }
    else
    {
        rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);
        kd_pin_write(OV5647_CAM_PIN_CSI2, GPIO_PV_LOW);  //GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);

    return 0;
}

static k_s32 ov5647_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    ret = sensor_reg_read(&dev->i2c_info, OV5647_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, OV5647_REG_CHIP_ID_L, &id_low);
    if (ret) {
        rt_kprintf("%s error\n", __func__);;
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);

    return ret;
}


static int ov5647_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 ov5647_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);

    
    if (on) {
        ov5647_power_rest(on);
        ov5647_i2c_init(&dev->i2c_info);
        ov5647_sensor_get_chip_id(ctx, &chip_id);
        // write power on
        // ret = sensor_reg_list_write(&dev->i2c_info, sensor_oe_enable_regs);
    } else {
        ov5647_power_rest(on);
        // ret = sensor_reg_list_write(&dev->i2c_info, sensor_oe_disable_regs);
    }

    return ret;
}

static k_s32 ov5647_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    k_u16 rdval = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(ov5647_mode_info) / sizeof(k_sensor_mode); i++) {
            if (ov5647_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(ov5647_mode_info[i]);
                dev->sensor_mode = &(ov5647_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    // sensor_reg_write(&dev->i2c_info, OV5647_SW_STANDBY, 0x00);

    // k_u8 val = MIPI_CTRL00_CLOCK_LANE_GATE | MIPI_CTRL00_BUS_IDLE | MIPI_CTRL00_CLOCK_LANE_DISABLE;
    // sensor_reg_write(&dev->i2c_info, OV5647_REG_MIPI_CTRL00, val);

    // sensor_reg_write(&dev->i2c_info, OV5647_REG_FRAME_OFF_NUMBER, 0x0f);
    // sensor_reg_write(&dev->i2c_info, OV5647_REG_PAD_OUT, 0x01);

    // sensor_reg_list_write(&dev->i2c_info, sensor_oe_disable_regs);

    // rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);

    // // check standby is opne
    // sensor_reg_read(&dev->i2c_info, OV5647_SW_STANDBY, &rdval);
    // pr_info("ov5647_sensor_init OV5647_SW_STANDBY is %d \n", rdval);
    // if (!(rdval & 0x01)) {
    //     // pr_err("%s, Device was in SW standby \n",  __func__);
    //     ret = sensor_reg_write(&dev->i2c_info, OV5647_SW_STANDBY, 0x01);
    // }

    k_u16 channel_id;
    sensor_reg_read(&dev->i2c_info, OV5647_REG_MIPI_CTRL14, &channel_id);
    pr_info("ov5647_sensor_init OV5647_REG_MIPI_CTRL14 is %d \n", channel_id);

    channel_id &= ~(3 << 6);
    ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_MIPI_CTRL14, channel_id | (0 << 6));

    //printf("index = %d\n", current_mode->index);
    k_u16 ae_addr, ae_val;
    switch (current_mode->index) {
    case 0:
    case 3:
    case 4:

        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        if(mirror_flag == 1)
        {
            ret = sensor_reg_list_write(&dev->i2c_info, sensor_mirror_reg);
        }

    // default:
    //     ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        current_mode->ae_info.frame_length = 1199;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000027808;//0.00003025
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 8.0;//63.9375;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        current_mode->ae_info.color_type = SENSOR_COLOR;    //color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = (1.0f/16.0f);

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_long_integraion_line = 2;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_integraion_line = 2;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_vs_integraion_line = 2;

        current_mode->ae_info.max_long_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_long_integraion_line;

        current_mode->ae_info.min_long_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_integraion_line;

        current_mode->ae_info.min_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.max_vs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_vs_integraion_line;

        current_mode->ae_info.min_vs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_vs_integraion_line;

        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;

        current_mode->ae_info.cur_long_again = 0.0;
        current_mode->ae_info.cur_long_dgain = 0.0;

        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;

        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        current_mode->ae_info.a_long_gain.min = 1.0;
        current_mode->ae_info.a_long_gain.max = 8.0;
        current_mode->ae_info.a_long_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 8.0;
        current_mode->ae_info.a_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 8.0;
        current_mode->ae_info.a_vs_gain.step = (1.0f/16.0f);

        current_mode->ae_info.d_long_gain.max = 1.0;
        current_mode->ae_info.d_long_gain.min = 1.0;
        current_mode->ae_info.d_long_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.cur_fps = current_mode->fps;
        current_mode->sensor_again = 0;
        current_mode->et_line = 0;

        break;
    case 1:

        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        if(mirror_flag == 1)
        {
            ret = sensor_reg_list_write(&dev->i2c_info, sensor_mirror_reg);
        }

         current_mode->ae_info.frame_length = 2051;
         current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
         current_mode->ae_info.one_line_exp_time = 0.00003250;
         current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 8.0;//63.9375;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        current_mode->ae_info.color_type = SENSOR_COLOR;    //color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = (1.0f/16.0f);

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_long_integraion_line = 2;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_integraion_line = 2;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length - 12;
        current_mode->ae_info.min_vs_integraion_line = 2;

        current_mode->ae_info.max_long_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_long_integraion_line;

        current_mode->ae_info.min_long_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_integraion_line;

        current_mode->ae_info.min_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.max_vs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_vs_integraion_line;

        current_mode->ae_info.min_vs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_vs_integraion_line;

        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;

        current_mode->ae_info.cur_long_again = 0.0;
        current_mode->ae_info.cur_long_dgain = 0.0;

        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;

        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        current_mode->ae_info.a_long_gain.min = 1.0;
        current_mode->ae_info.a_long_gain.max = 8.0;
        current_mode->ae_info.a_long_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 8.0;
        current_mode->ae_info.a_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 8.0;
        current_mode->ae_info.a_vs_gain.step = (1.0f/16.0f);

        current_mode->ae_info.d_long_gain.max = 1.0;
        current_mode->ae_info.d_long_gain.min = 1.0;
        current_mode->ae_info.d_long_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.cur_fps = current_mode->fps;
        current_mode->sensor_again = 0;
        current_mode->et_line = 0;
        break;
    }

    k_u16 again_h, again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    ret = sensor_reg_read(&dev->i2c_info, OV5647_REG_LONG_AGAIN_H, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, OV5647_REG_LONG_AGAIN_L, &again_l);
    again = (float)(((again_h & 0x03) << 8) + again_l) / 16.0f;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, OV5647_REG_LONG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, OV5647_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) + ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;

    return ret;
}

static k_s32 ov5647_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(ov5647_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov5647_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &ov5647_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(ov5647_mode_info[i]);
            return 0;
        }
    }
    pr_err("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    if (enum_mode->index >= (sizeof(ov5647_mode_info) / sizeof(ov5647_mode_info[0]))) {
        pr_err("%s, invalid mode index.\n", __func__);
        return -1;
    }

    for (k_s32 i = 0; i < sizeof(ov5647_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov5647_mode_info[i].index == enum_mode->index) {
            memcpy(&enum_mode->mode, &ov5647_mode_info[i], sizeof(k_sensor_mode));
            return 0;
        }
    }
    return ret;
}

static k_s32 ov5647_sensor_get_caps(void *ctx, k_sensor_caps *caps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(caps, 0, sizeof(k_sensor_caps));
    caps->bit_width = current_mode->bit_width;
    caps->bayer_pattern = current_mode->bayer_pattern;
    caps->resolution.width = current_mode->size.width;
    caps->resolution.height = current_mode->size.height;

    return ret;
}

static k_s32 ov5647_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 ov5647_sensor_set_stream(void *ctx, k_s32 enable)
{
    k_s32 ret = 0;
    k_u8 val;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {

        // val = MIPI_CTRL00_BUS_IDLE | MIPI_CTRL00_CLOCK_LANE_GATE |MIPI_CTRL00_LINE_SYNC_ENABLE;
        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_MIPI_CTRL00, val);

        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_FRAME_OFF_NUMBER, 0x00);
        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_PAD_OUT, 0x00);
        // sensor_reg_list_write(&dev->i2c_info, sensor_oe_enable_regs);

        ret = sensor_reg_write(&dev->i2c_info, OV5647_SW_STANDBY, 0x01);


    } else {
        // val = MIPI_CTRL00_CLOCK_LANE_GATE | MIPI_CTRL00_BUS_IDLE | MIPI_CTRL00_CLOCK_LANE_DISABLE;
        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_MIPI_CTRL00, val);

        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_FRAME_OFF_NUMBER, 0x0f);
        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_PAD_OUT, 0x01);
        // // sensor_reg_list_write(&dev->i2c_info, sensor_oe_disable_regs);
        ret = sensor_reg_write(&dev->i2c_info, 0x3018, 0xff);
        ret = sensor_reg_write(&dev->i2c_info, OV5647_SW_STANDBY, 0x00);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 ov5647_sensor_get_again(void *ctx, k_sensor_gain *gain)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        gain->gain[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_again;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        gain->gain[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_again;
        gain->gain[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_vs_again;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }

    return ret;
}

static k_s32 ov5647_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16 + 0.5);
        if(current_mode->sensor_again !=again)
        {
            ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
            ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_L,(again & 0xff));
            current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/16.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS]  * 16 + 0.5);
         ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
         ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_again = (float)again/16.0f;

        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 16 + 0.5);
        //TODO
        current_mode->ae_info.cur_vs_again = (float)again/16.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, exp_frame_type(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again*1000) );

    return ret;
}

static k_s32 ov5647_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        gain->gain[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_dgain;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        gain->gain[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_dgain;
        gain->gain[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_vs_dgain;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }

    return ret;
}

static k_s32 ov5647_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter exp_frame_type(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_H,(dgain & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_L,(dgain & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        // ret = sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        // ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 1024);
        //TODO wirte vs gain register
        current_mode->ae_info.cur_vs_dgain = (float)dgain/1024.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    current_mode->ae_info.cur_gain = current_mode->ae_info.cur_again * current_mode->ae_info.cur_dgain;
    pr_debug("%s,cur_gain(%d)\n", __func__, (k_u32)(current_mode->ae_info.cur_gain*10000));

    return ret;
}


static k_s32 ov5647_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        time->intg_time[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_integration_time;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        time->intg_time[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_integration_time;
        time->intg_time[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_vs_integration_time;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }

    return ret;
}

static k_s32 ov5647_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];
        //printf("int_time = %f, one_line_time = %f \n", integraion_time, current_mode->ae_info.one_line_exp_time);

        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));
        if (current_mode->et_line != exp_line)
        {
            ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_EXP_TIME_H, ( exp_line >> 4) & 0xff);
            ret |= sensor_reg_write(&dev->i2c_info, OV5647_REG_LONG_EXP_TIME_L, ( exp_line << 4) & 0xff);
            current_mode->et_line = exp_line;
        }
        current_mode->ae_info.cur_integration_time = (float)current_mode->et_line * current_mode->ae_info.one_line_exp_time;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        integraion_time = time.intg_time[SENSOR_DUAL_EXP_L_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));

        current_mode->ae_info.cur_integration_time = (float)exp_line * current_mode->ae_info.one_line_exp_time;

        integraion_time = time.intg_time[SENSOR_DUAL_EXP_S_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));

        current_mode->ae_info.cur_vs_integration_time = (float)exp_line * current_mode->ae_info.one_line_exp_time;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s exp_frame_type(%d), exp_line(%d), integraion_time(%u)\n",\
        __func__, current_mode->hdr_mode, exp_line, (k_u32)(integraion_time * 1000000000));

    return ret;
}

static k_s32 ov5647_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 ov5647_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 ov5647_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 ov5647_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 ov5647_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 ov5647_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 ov5647_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}


static k_s32 ov5647_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    rt_kprintf("mirror mirror is %d , sensor tpye is %d \n", mirror.mirror, mirror.sensor_type);

    // get current sensor type
    for (k_s32 i = 0; i < sizeof(ov5647_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov5647_mode_info[i].sensor_type == mirror.sensor_type) {
            if(mirror.sensor_type == OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2)
            {
                // default flip (3820) = 0x2
                switch(mirror.mirror)
                {
                    case VICAP_MIRROR_NONE :
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_GRBG;
                        mirror_flag = 0;
                        return 0;
                    case VICAP_MIRROR_HOR :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x2;
                        sensor_mirror_reg[1].val = 0x2;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_RGGB;
                        break;
                    case VICAP_MIRROR_VER :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x0;
                        sensor_mirror_reg[1].val = 0x0;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_BGGR;
                        break;
                    case VICAP_MIRROR_BOTH :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x0;
                        sensor_mirror_reg[1].val = 0x2;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_GBRG;
                        break;
                }
            }
            else
            {
                // default mirror(3821) = 0x2
                switch(mirror.mirror)
                {
                    case VICAP_MIRROR_NONE :
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_GBRG;
                        mirror_flag = 0;
                        return 0;
                    case VICAP_MIRROR_HOR :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x0;
                        sensor_mirror_reg[1].val = 0x0;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_BGGR;
                        break;
                    case VICAP_MIRROR_VER :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x2;
                        sensor_mirror_reg[1].val = 0x2;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_RGGB;
                        break;
                    case VICAP_MIRROR_BOTH :
                        // set mirror
                        sensor_mirror_reg[0].val = 0x2;
                        sensor_mirror_reg[1].val = 0x0;
                        // set sensor info bayer pattern 
                        ov5647_mode_info[i].bayer_pattern = BAYER_PAT_GRBG;
                        break;
                }
            }
            
            // set mirror flag
            mirror_flag = 1;
            return 0;
        }
    }
    rt_kprintf("ov5647 sensor mirror set failed , sensor type is err \n");
    return -1;
}

struct sensor_driver_dev ov5647_sensor_csi2_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = OV5647_CSI2_IIC ,//"i2c1", // "i2c3",
        .slave_addr = 0x36,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "ov5647_csi2",
    .sensor_func = {
        .sensor_power = ov5647_sensor_power_on,
        .sensor_init = ov5647_sensor_init,
        .sensor_get_chip_id = ov5647_sensor_get_chip_id,
        .sensor_get_mode = ov5647_sensor_get_mode,
        .sensor_set_mode = ov5647_sensor_set_mode,
        .sensor_enum_mode = ov5647_sensor_enum_mode,
        .sensor_get_caps = ov5647_sensor_get_caps,
        .sensor_conn_check = ov5647_sensor_conn_check,
        .sensor_set_stream = ov5647_sensor_set_stream,
        .sensor_get_again = ov5647_sensor_get_again,
        .sensor_set_again = ov5647_sensor_set_again,
        .sensor_get_dgain = ov5647_sensor_get_dgain,
        .sensor_set_dgain = ov5647_sensor_set_dgain,
        .sensor_get_intg_time = ov5647_sensor_get_intg_time,
        .sensor_set_intg_time = ov5647_sensor_set_intg_time,
        .sensor_get_exp_parm = ov5647_sensor_get_exp_parm,
        .sensor_set_exp_parm = ov5647_sensor_set_exp_parm,
        .sensor_get_fps = ov5647_sensor_get_fps,
        .sensor_set_fps = ov5647_sensor_set_fps,
        .sensor_get_isp_status = ov5647_sensor_get_isp_status,
        .sensor_set_blc = ov5647_sensor_set_blc,
        .sensor_set_wb = ov5647_sensor_set_wb,
        .sensor_get_tpg = ov5647_sensor_get_tpg,
        .sensor_set_tpg = ov5647_sensor_set_tpg,
        .sensor_get_expand_curve = ov5647_sensor_get_expand_curve,
        .sensor_get_otp_data = ov5647_sensor_get_otp_data,
        .sensor_mirror_set = ov5647_sensor_mirror_set,
    },
};
