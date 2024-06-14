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

//#include <riscv_io.h>
#include "io.h"
#include "drv_gpio.h"

#include "k_board_config_comm.h"

#define pr_info(...) //rt_kprintf(__VA_ARGS__)
#define pr_debug(...) //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)    rt_kprintf(__VA_ARGS__)

#define OV9732_REG_CHIP_ID_H    0x300a
#define OV9732_REG_CHIP_ID_L    0x300b

#define OV9732_REG_LONG_EXP_TIME_H    0x3501
#define OV9732_REG_LONG_EXP_TIME_L    0x3502

#define OV9732_REG_LONG_AGAIN_H    0x350a
#define OV9732_REG_LONG_AGAIN_L    0x350b

#define OV9732_MIN_GAIN_STEP    (1.0f/16.0f)

#define DELAY_MS_SENSOR_DEFAULT     1

static const k_sensor_reg ov9732_mipi2lane_1080p_30fps_linear[] = {
    {REG_NULL, 0x00},
};

static const k_sensor_reg ov9732_mipi2lane_1080p_30fps_hdr[] = {
    {REG_NULL, 0x00},
};


static const k_sensor_reg ov9732_mipi2lane_720p_30fps_linear[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3007, 0x1f},
    {0x3008, 0xff},
    {0x3009, 0x02},
    {0x3010, 0x00},
    {0x3011, 0x08},
    {0x3014, 0x22},
    {0x301e, 0x15},
    {0x3030, 0x19},
    {0x3080, 0x02},
    {0x3081, 0x3c},
    {0x3082, 0x04},
    {0x3083, 0x00},
    {0x3084, 0x02},
    {0x3085, 0x01},
    {0x3086, 0x01},
    {0x3089, 0x01},
    {0x308a, 0x00},
    {0x3103, 0x01},
    {0x3600, 0xf6},
    {0x3601, 0x72},
    {0x3605, 0x66},
    {0x3610, 0x0c},
    {0x3611, 0x60},
    {0x3612, 0x35},
    {0x3654, 0x10},
    {0x3655, 0x77},
    {0x3656, 0x77},
    {0x3657, 0x07},
    {0x3658, 0x22},
    {0x3659, 0x22},
    {0x365a, 0x02},
    {0x3700, 0x1f},
    {0x3701, 0x10},
    {0x3702, 0x0c},
    {0x3703, 0x0b},
    {0x3704, 0x3c},
    {0x3705, 0x51},
    {0x370d, 0x20},
    {0x3710, 0x0d},
    {0x3782, 0x58},
    {0x3783, 0x60},
    {0x3784, 0x05},
    {0x3785, 0x55},
    {0x37c0, 0x07},
    {0x3800, 0x00},
    {0x3801, 0x04},
    {0x3802, 0x00},
    {0x3803, 0x04},
    {0x3804, 0x05},
    {0x3805, 0x0b},
    {0x3806, 0x02},
    {0x3807, 0xdb},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x02},
    {0x380b, 0xd0},
    {0x380c, 0x05},	//HTS = 1467
    {0x380d, 0xbb},
    {0x380e, 0x03},	//VTS = 818
    {0x380f, 0x32},
    {0x3810, 0x00},
    {0x3811, 0x04},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3816, 0x00},
    {0x3817, 0x00},
    {0x3818, 0x00},
    {0x3819, 0x04},
    {0x3820, 0x10},
    {0x3821, 0x00},
    {0x382c, 0x06},
    {0x3500, 0x00},
    {0x3501, 0x0c},	//ET = 204 ET line
    {0x3502, 0xc0},
    {0x3503, 0x03},	//sensor gain delay 1 frame, 0x23/0x27
    {0x3504, 0x00},	//sensor SNR gain, 1x
    {0x3505, 0x00},
    {0x3509, 0x10},
    {0x350a, 0x00},
    {0x350b, 0x10},	//sensor analog gain, 1x
    {0x3d00, 0x00},
    {0x3d01, 0x00},
    {0x3d02, 0x00},
    {0x3d03, 0x00},
    {0x3d04, 0x00},
    {0x3d05, 0x00},
    {0x3d06, 0x00},
    {0x3d07, 0x00},
    {0x3d08, 0x00},
    {0x3d09, 0x00},
    {0x3d0a, 0x00},
    {0x3d0b, 0x00},
    {0x3d0c, 0x00},
    {0x3d0d, 0x00},
    {0x3d0e, 0x00},
    {0x3d0f, 0x00},
    {0x3d80, 0x00},
    {0x3d81, 0x00},
    {0x3d82, 0x38},
    {0x3d83, 0xa4},
    {0x3d84, 0x00},
    {0x3d85, 0x00},
    {0x3d86, 0x1f},
    {0x3d87, 0x03},
    {0x3d8b, 0x00},
    {0x3d8f, 0x00},
    {0x4001, 0xe0},	//black level auto mode
    {0x4004, 0x00},
    {0x4005, 0x02},
    {0x4006, 0x01},
    {0x4007, 0x40},
    {0x4009, 0x0b},
    {0x4300, 0x03},
    {0x4301, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4309, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x04},
    {0x4800, 0x00},
    {0x4805, 0x00},
    {0x4821, 0x50},
    {0x4823, 0x50},
    {0x4837, 0x2d},
    {0x4a00, 0x00},
    {0x4f00, 0x80},
    {0x4f01, 0x10},
    {0x4f02, 0x00},
    {0x4f03, 0x00},
    {0x4f04, 0x00},
    {0x4f05, 0x00},
    {0x4f06, 0x00},
    {0x4f07, 0x00},
    {0x4f08, 0x00},
    {0x4f09, 0x00},
    {0x5000, 0x07},	//black level enable
    {0x500c, 0x00},
    {0x500d, 0x00},
    {0x500e, 0x00},
    {0x500f, 0x00},
    {0x5010, 0x00},
    {0x5011, 0x00},
    {0x5012, 0x00},
    {0x5013, 0x00},
    {0x5014, 0x00},
    {0x5015, 0x00},
    {0x5016, 0x00},
    {0x5017, 0x00},
    {0x5080, 0x00},
    {0x5708, 0x06},
    {0x5781, 0x0e},	//dpc enable
    {0x5783, 0x0f},
    {0x3603, 0x70},
    {0x3620, 0x1e},
    {0x400a, 0x01},
    {0x400b, 0xc0},
    //{0x0100, 0x01},
    {REG_NULL, 0x00},
};


static const k_sensor_reg ov9732_mipi2lane_720p_30fps_mclk_16m_linear[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3001, 0x00},
    {0x3002, 0x00},
    {0x3007, 0x1f},
    {0x3008, 0xff},
    {0x3009, 0x02},
    {0x3010, 0x00},
    {0x3011, 0x08},
    {0x3014, 0x22},
    {0x301e, 0x15},
    {0x3030, 0x19},
    {0x3080, 0x02},
    // {0x3081, 0x3c},
    {0x3081, 0x5A},
    {0x3082, 0x04},
    {0x3083, 0x00},
    {0x3084, 0x02},
    {0x3085, 0x01},
    {0x3086, 0x01},
    {0x3089, 0x01},
    {0x308a, 0x00},
    {0x3103, 0x01},
    {0x3600, 0xf6},
    {0x3601, 0x72},
    {0x3605, 0x66},
    {0x3610, 0x0c},
    {0x3611, 0x60},
    {0x3612, 0x35},
    {0x3654, 0x10},
    {0x3655, 0x77},
    {0x3656, 0x77},
    {0x3657, 0x07},
    {0x3658, 0x22},
    {0x3659, 0x22},
    {0x365a, 0x02},
    {0x3700, 0x1f},
    {0x3701, 0x10},
    {0x3702, 0x0c},
    {0x3703, 0x0b},
    {0x3704, 0x3c},
    {0x3705, 0x51},
    {0x370d, 0x20},
    {0x3710, 0x0d},
    {0x3782, 0x58},
    {0x3783, 0x60},
    {0x3784, 0x05},
    {0x3785, 0x55},
    {0x37c0, 0x07},
    {0x3800, 0x00},
    {0x3801, 0x04},
    {0x3802, 0x00},
    {0x3803, 0x04},
    {0x3804, 0x05},
    {0x3805, 0x0b},
    {0x3806, 0x02},
    {0x3807, 0xdb},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x02},
    {0x380b, 0xd0},
    {0x380c, 0x05},	//HTS = 1467
    {0x380d, 0xbb},
    {0x380e, 0x03},	//VTS = 818
    {0x380f, 0x32},
    {0x3810, 0x00},
    {0x3811, 0x04},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3816, 0x00},
    {0x3817, 0x00},
    {0x3818, 0x00},
    {0x3819, 0x04},
    {0x3820, 0x10},
    {0x3821, 0x00},
    {0x382c, 0x06},
    {0x3500, 0x00},
    {0x3501, 0x0c},	//ET = 204 ET line
    {0x3502, 0xc0},
    {0x3503, 0x03},	//sensor gain delay 1 frame, 0x23/0x27
    {0x3504, 0x00},	//sensor SNR gain, 1x
    {0x3505, 0x00},
    {0x3509, 0x10},
    {0x350a, 0x00},
    {0x350b, 0x10},	//sensor analog gain, 1x
    {0x3d00, 0x00},
    {0x3d01, 0x00},
    {0x3d02, 0x00},
    {0x3d03, 0x00},
    {0x3d04, 0x00},
    {0x3d05, 0x00},
    {0x3d06, 0x00},
    {0x3d07, 0x00},
    {0x3d08, 0x00},
    {0x3d09, 0x00},
    {0x3d0a, 0x00},
    {0x3d0b, 0x00},
    {0x3d0c, 0x00},
    {0x3d0d, 0x00},
    {0x3d0e, 0x00},
    {0x3d0f, 0x00},
    {0x3d80, 0x00},
    {0x3d81, 0x00},
    {0x3d82, 0x38},
    {0x3d83, 0xa4},
    {0x3d84, 0x00},
    {0x3d85, 0x00},
    {0x3d86, 0x1f},
    {0x3d87, 0x03},
    {0x3d8b, 0x00},
    {0x3d8f, 0x00},
    {0x4001, 0xe0},	//black level auto mode
    {0x4004, 0x00},
    {0x4005, 0x02},
    {0x4006, 0x01},
    {0x4007, 0x40},
    {0x4009, 0x0b},
    {0x4300, 0x03},
    {0x4301, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4309, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x04},
    {0x4800, 0x00},
    {0x4805, 0x00},
    {0x4821, 0x50},
    {0x4823, 0x50},
    {0x4837, 0x2d},
    {0x4a00, 0x00},
    {0x4f00, 0x80},
    {0x4f01, 0x10},
    {0x4f02, 0x00},
    {0x4f03, 0x00},
    {0x4f04, 0x00},
    {0x4f05, 0x00},
    {0x4f06, 0x00},
    {0x4f07, 0x00},
    {0x4f08, 0x00},
    {0x4f09, 0x00},
    {0x5000, 0x07},	//black level enable
    {0x500c, 0x00},
    {0x500d, 0x00},
    {0x500e, 0x00},
    {0x500f, 0x00},
    {0x5010, 0x00},
    {0x5011, 0x00},
    {0x5012, 0x00},
    {0x5013, 0x00},
    {0x5014, 0x00},
    {0x5015, 0x00},
    {0x5016, 0x00},
    {0x5017, 0x00},
    {0x5080, 0x00},
    {0x5708, 0x06},
    {0x5781, 0x0e},	//dpc enable
    {0x5783, 0x0f},
    {0x3603, 0x70},
    {0x3620, 0x1e},
    {0x400a, 0x01},
    {0x400b, 0xc0},
    //{0x0100, 0x01},
    {REG_NULL, 0x00},
};

static k_sensor_mode ov9732_mode_info[] = {
    {
        .index = 0,
        .sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 720,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 720,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov9732_mipi2lane_720p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 1,
        .sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 720,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 720,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov9732_mipi2lane_720p_30fps_mclk_16m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK1,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 2,
        .sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR_V2,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 720,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 720,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = ov9732_mipi2lane_720p_30fps_mclk_16m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK1,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
};

static k_sensor_mode *current_mode = NULL;

static int ov9732_power_rest(k_s32 on)
{
    // #define OV9732_RST_PIN    (24)//  (28)

    // rst
    kd_pin_mode(OV9732_RST_PIN, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(OV9732_RST_PIN, GPIO_PV_HIGH);  //GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);
        kd_pin_write(OV9732_RST_PIN, GPIO_PV_LOW);  //GPIO_PV_LOW  GPIO_PV_LOW
        rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);
        kd_pin_write(OV9732_RST_PIN, GPIO_PV_HIGH);  //GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(OV9732_RST_PIN, GPIO_PV_LOW);  //GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(DELAY_MS_SENSOR_DEFAULT);

    return 0;
}

static k_s32 ov9732_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, OV9732_REG_CHIP_ID_L, &id_low);
    if (ret) {
        rt_kprintf("%s error\n", __func__);;
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}


static int ov9732_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 ov9732_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        ov9732_power_rest(on);
        ov9732_i2c_init(&dev->i2c_info);
        ov9732_sensor_get_chip_id(ctx, &chip_id);
    } else {
        ov9732_power_rest(on);
    }

    return ret;
}

static k_s32 ov9732_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(ov9732_mode_info) / sizeof(k_sensor_mode); i++) {
            if (ov9732_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(ov9732_mode_info[i]);
                dev->sensor_mode = &(ov9732_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    switch (current_mode->index) {
    case 0:
    case 1:
    case 2:
        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

///////////////////
        current_mode->ae_info.frame_length = 818;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.00004075;//s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 63.9375;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        //current_mode->ae_info.ae_min_interval_frame = 2.5;
        current_mode->ae_info.color_type = SENSOR_COLOR_IR;	//color sensor without IR filter

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = OV9732_MIN_GAIN_STEP;

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length;
        current_mode->ae_info.min_long_integraion_line =     current_mode->ae_info.frame_length - 1;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 1;
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length;
        current_mode->ae_info.min_vs_integraion_line =     current_mode->ae_info.frame_length - 1;

        //current_mode->ae_info.max_vvs_integraion_line = current_mode->ae_info.frame_length;
        //current_mode->ae_info.min_vvs_integraion_line =     current_mode->ae_info.frame_length - 1;

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

        //current_mode->ae_info.max_vvs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.max_vvs_integraion_line;

        //current_mode->ae_info.min_vvs_integraion_time = \
            current_mode->ae_info.integration_time_increment * \
            current_mode->ae_info.min_vvs_integraion_line;


/////
        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;
        //current_mode->ae_info.cur_vvs_integration_time = 0.0;

/////
        current_mode->ae_info.cur_long_again = 0.0;
        current_mode->ae_info.cur_long_dgain = 0.0;

        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;

        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        //current_mode->ae_info.cur_vvs_again = 0.0;
        //current_mode->ae_info.cur_vvs_dgain = 0.0;
/////
        current_mode->ae_info.a_long_gain.min = 1.0;
        current_mode->ae_info.a_long_gain.max = 63.9375;
        current_mode->ae_info.a_long_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 63.9375;
        current_mode->ae_info.a_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 63.9375;
        current_mode->ae_info.a_vs_gain.step = (1.0f/16.0f);

        //current_mode->ae_info.a_vvs_gain.max = 1.0;
        //current_mode->ae_info.a_vvs_gain.min = 63.9375;
        //current_mode->ae_info.a_vvs_gain.step = (1.0f/16.0f);//
/////
        current_mode->ae_info.d_long_gain.max = 1.0;
        current_mode->ae_info.d_long_gain.min = 1.0;
        current_mode->ae_info.d_long_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f/1024.0f);

        //current_mode->ae_info.d_vvs_gain.max = 1.0;//
        //current_mode->ae_info.d_vvs_gain.min = 1.0;//
        //current_mode->ae_info.d_vvs_gain.step = (1.0f/1024.0f);//

        current_mode->ae_info.cur_fps = current_mode->fps;
        current_mode->sensor_again = 0;
        current_mode->et_line = 0;

        break;

    default:
        break;
    }

    k_u16 again_h, again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L, &again_l);
    again = (float)(((again_h & 0x03) << 8) | again_l) / 16.0f;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_LONG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) | ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;

    return ret;
}

k_s32 ov9732_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(ov9732_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov9732_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &ov9732_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(ov9732_mode_info[i]);
            return 0;
        }
    }
    pr_err("%s, the mode not exit.\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    if (enum_mode->index >= (sizeof(ov9732_mode_info) / sizeof(ov9732_mode_info[0]))) {
        pr_err("%s, invalid mode index.\n", __func__);
        return -1;
    }

    for (k_s32 i = 0; i < sizeof(ov9732_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov9732_mode_info[i].index == enum_mode->index) {
            memcpy(&enum_mode->mode, &ov9732_mode_info[i], sizeof(k_sensor_mode));
            return 0;
        }
    }
    return ret;
}

k_s32 ov9732_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

k_s32 ov9732_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

k_s32 ov9732_sensor_set_stream(void *ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x01);
    } else {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x00);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

k_s32 ov9732_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

k_s32 ov9732_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16 + 0.5);
        if(current_mode->sensor_again !=again)
        {
	        ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
	        ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L,(again & 0xff));
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/16.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS]  * 16 + 0.5);
        ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_again = (float)again/16.0f;

        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 16 + 0.5);
        //TODO
        current_mode->ae_info.cur_vs_again = (float)again/16.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again*1000) );

    return ret;
}

k_s32 ov9732_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

k_s32 ov9732_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L,(again & 0xff));
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


k_s32 ov9732_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

k_s32 ov9732_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];

        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));
        if (current_mode->et_line != exp_line)
        {
	        ret |= sensor_reg_write(&dev->i2c_info, 0x3501, ( exp_line >> 4) & 0xff);
	        ret |= sensor_reg_write(&dev->i2c_info, 0x3502, ( exp_line << 4) & 0xff);
	        current_mode->et_line = exp_line;
        }
        current_mode->ae_info.cur_integration_time = (float)current_mode->et_line * current_mode->ae_info.one_line_exp_time;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        integraion_time = time.intg_time[SENSOR_DUAL_EXP_L_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));

        current_mode->ae_info.cur_integration_time = (float)exp_line * current_mode->ae_info.one_line_exp_time;

        integraion_time = time.intg_time[SENSOR_DUAL_EXP_S_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));

        current_mode->ae_info.cur_vs_integration_time = (float)exp_line * current_mode->ae_info.one_line_exp_time;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s hdr_mode(%d), exp_line(%d), integraion_time(%u)\n",\
        __func__, current_mode->hdr_mode, exp_line, (k_u32)(integraion_time * 1000000000));

    return ret;
}

k_s32 ov9732_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

k_s32 ov9732_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

k_s32 ov9732_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

k_s32 ov9732_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

k_s32 ov9732_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9732_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

k_s32 ov9732_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 ov9732_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev ov9732_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c1",
        .slave_addr = 0x36,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "ov9732",
    .sensor_func = {
        .sensor_power = ov9732_sensor_power_on,
        .sensor_init = ov9732_sensor_init,
        .sensor_get_chip_id = ov9732_sensor_get_chip_id,
        .sensor_get_mode = ov9732_sensor_get_mode,
        .sensor_set_mode = ov9732_sensor_set_mode,
        .sensor_enum_mode = ov9732_sensor_enum_mode,
        .sensor_get_caps = ov9732_sensor_get_caps,
        .sensor_conn_check = ov9732_sensor_conn_check,
        .sensor_set_stream = ov9732_sensor_set_stream,
        .sensor_get_again = ov9732_sensor_get_again,
        .sensor_set_again = ov9732_sensor_set_again,
        .sensor_get_dgain = ov9732_sensor_get_dgain,
        .sensor_set_dgain = ov9732_sensor_set_dgain,
        .sensor_get_intg_time = ov9732_sensor_get_intg_time,
        .sensor_set_intg_time = ov9732_sensor_set_intg_time,
        .sensor_get_exp_parm = ov9732_sensor_get_exp_parm,
        .sensor_set_exp_parm = ov9732_sensor_set_exp_parm,
        .sensor_get_fps = ov9732_sensor_get_fps,
        .sensor_set_fps = ov9732_sensor_set_fps,
        .sensor_get_isp_status = ov9732_sensor_get_isp_status,
        .sensor_set_blc = ov9732_sensor_set_blc,
        .sensor_set_wb = ov9732_sensor_set_wb,
        .sensor_get_tpg = ov9732_sensor_get_tpg,
        .sensor_set_tpg = ov9732_sensor_set_tpg,
        .sensor_get_expand_curve = ov9732_sensor_get_expand_curve,
        .sensor_get_otp_data = ov9732_sensor_get_otp_data,
        .sensor_mirror_set = ov9732_sensor_mirror_set,
    },
};
