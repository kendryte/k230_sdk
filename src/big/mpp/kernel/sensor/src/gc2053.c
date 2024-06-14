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
#include "io.h"
#include "drv_gpio.h"

#include "k_board_config_comm.h"

#define pr_info(...) //rt_kprintf(__VA_ARGS__)
#define pr_debug(...) //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)    rt_kprintf(__VA_ARGS__)

#define GC2053_REG_CHIP_ID_H    0xf0
#define GC2053_REG_CHIP_ID_L    0xf1

#define GC2053_REG_EXP_TIME_H    0x03
#define GC2053_REG_EXP_TIME_L    0x04

#define GC2053_REG_AGAIN_H    0xb1
#define GC2053_REG_AGAIN_L    0xb2

#define GC2053_MIN_GAIN_STEP    (1.0f/64.0f)



static const k_sensor_reg gc2053_mipi4lane_1080p_30fps_linear[] = {

#if 0 
    {0xfe,0x80},
    {0xfe,0x80},
    {0xfe,0x80},	//pllmp_div = 128
    {0xfe,0x00},
    {0xf2,0x00},
    {0xf3,0x00},
    {0xf4,0x36},	//PLL_ldo_set = 3, spi_clk_div = 6
    {0xf5,0xc0},
    {0xf6,0x44},	//cp_clk_del = 0, cp_clk_div = 4
    {0xf7,0x01},
    {0xf8,0x63},	//pllmp_div = 99
    {0xf9,0x40},	//rpllckl_div = div4
    {0xfc,0x8e},
/****CISCTL & ANALOG****/
    {0xfe,0x00},
    {0x87,0x18},
    {0xee,0x30},
    {0xd0,0xb7},
    {0x03,0x04},	//ET = 1120
    {0x04,0x60},
    {0x05,0x04},	//Line length = 1100
    {0x06,0x4c},
    {0x07,0x00},	//Vblank = 17
    {0x08,0x11},
    {0x09,0x00},	//Row start = 2
    {0x0a,0x02},
    {0x0b,0x00},	//Col Start = 2
    {0x0c,0x02},
    {0x0d,0x04},	//win_height = 1088, default win_width = 1936
    {0x0e,0x40},
    {0x12,0xe2},
    {0x13,0x16},
    {0x19,0x0a},
    {0x21,0x1c},
    {0x28,0x0a},
    {0x29,0x24},
    {0x2b,0x04},
    {0x32,0xf8},
    {0x37,0x03},
    {0x39,0x15},
    {0x43,0x07},
    {0x44,0x40},
    {0x46,0x0b},
    {0x4b,0x20},
    {0x4e,0x08},
    {0x55,0x20},
    {0x66,0x05},
    {0x67,0x05},
    {0x77,0x01},
    {0x78,0x00},
    {0x7c,0x93},
    {0x8c,0x12},
    {0x8d,0x92},
    {0x90,0x01},
    {0x9d,0x10},
    {0xce,0x7c},
    {0xd2,0x41},
    {0xd3,0xdc},
    {0xe6,0x50},
/*gain*/
    {0xb6,0xc0},
    {0xb0,0x70},
    {0xb1,0x01},	//gain = 1.0x
    {0xb2,0x00},
    {0xb3,0x00},	//
    {0xb4,0x00},
    {0xb8,0x01},	//col_gain = 1.0x
    {0xb9,0x00},
/*blk*/
    {0x26,0x30},
    {0xfe,0x01},
    {0x40,0x23},	//black level & offset enable
    {0x55,0x07},
    {0x60,0x40},
    {0xfe,0x04},
    {0x14,0x78},
    {0x15,0x78},
    {0x16,0x78},
    {0x17,0x78},
/*window*/
    {0xfe,0x01},
    {0x92,0x00},
    {0x94,0x03},
    {0x95,0x04},	//out_win_height = 1080
    {0x96,0x38},
    {0x97,0x07},	//out_win_width = 1920
    {0x98,0x80},
/*ISP*/
    {0xfe,0x01},
    {0x01,0x05},
    {0x02,0x89},
    {0x04,0x01},
    {0x07,0xa6},
    {0x08,0xa9},
    {0x09,0xa8},
    {0x0a,0xa7},
    {0x0b,0xff},
    {0x0c,0xff},
    {0x0f,0x00},
    {0x50,0x1c},
    {0x89,0x03},
    {0xfe,0x04},
    {0x28,0x86},
    {0x29,0x86},
    {0x2a,0x86},
    {0x2b,0x68},
    {0x2c,0x68},
    {0x2d,0x68},
    {0x2e,0x68},
    {0x2f,0x68},
    {0x30,0x4f},
    {0x31,0x68},
    {0x32,0x67},
    {0x33,0x66},
    {0x34,0x66},
    {0x35,0x66},
    {0x36,0x66},
    {0x37,0x66},
    {0x38,0x62},
    {0x39,0x62},
    {0x3a,0x62},
    {0x3b,0x62},
    {0x3c,0x62},
    {0x3d,0x62},
    {0x3e,0x62},
    {0x3f,0x62},
/****DVP & MIPI****/
    {0xfe,0x01},
    {0x9a,0x06},
    {0xfe,0x00},
    {0x7b,0x2a},
    {0x23,0x2d},
    {0xfe,0x03},
    {0x01,0x27},
    {0x02,0x56},
    {0x03,0xb6},
    {0x12,0x80},
    {0x13,0x07},
    {0x15,0x12},
    {0xfe,0x00},
    {0x3e,0x91},

#else
/****system****/
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x80},
	{0xfe, 0x00},
	{0xf2, 0x00},
	{0xf3, 0x00},
	{0xf4, 0x36},
	{0xf5, 0xc0},
	{0xf6, 0x44},
	{0xf7, 0x01},
	{0xf8, 0x63},
	{0xf9, 0x40},
	{0xfc, 0x8e},
	/****CISCTL & ANALOG****/
	{0xfe, 0x00},	//Page 0
	{0x87, 0x18},
	{0xee, 0x30},
	{0xd0, 0xb7},
	{0x03, 0x01},	//ET = 0x160 = 352
	{0x04, 0x60},
	{0x05, 0x04},	//line length = 1100 * 2 = 2200
	{0x06, 0x4c},
	{0x07, 0x00},	//Vblank = 0x11 = 17
	{0x08, 0x11},
	{0x09, 0x00},	//raw start = 0x02
	{0x0a, 0x02},
	{0x0b, 0x00},	//col start = 0x02
	{0x0c, 0x02},
	{0x0d, 0x04},	//win_height = 1088
	{0x0e, 0x40},
	{0x12, 0xe2},
	{0x13, 0x16},
	{0x19, 0x0a},
	{0x21, 0x1c},
	{0x28, 0x0a},
	{0x29, 0x24},
	{0x2b, 0x04},
	{0x32, 0xf8},
	{0x37, 0x03},
	{0x39, 0x15},
	{0x43, 0x07},
	{0x44, 0x40},
	{0x46, 0x0b},
	{0x4b, 0x20},
	{0x4e, 0x08},
	{0x55, 0x20},
	{0x66, 0x05},
	{0x67, 0x05},
	{0x77, 0x01},
	{0x78, 0x00},
	{0x7c, 0x93},
	{0x8c, 0x12},
	{0x8d, 0x92},
	{0x90, 0x01},
	{0x9d, 0x10},
	{0xce, 0x7c},
	{0xd2, 0x41},
	{0xd3, 0xdc},
	{0xe6, 0x50},
	/*gain*/
	{0xb6, 0xc0},
	{0xb0, 0x60},
	{0xb1, 0x01},	//gain: 1x
	{0xb2, 0x00},
	{0xb3, 0x00},
	{0xb4, 0x00},
	{0xb8, 0x01},
	{0xb9, 0x00},
	/*blk*/
	{0x26, 0x30},
	{0xfe, 0x01},
	{0x40, 0x23},
	{0x55, 0x07},
	{0x60, 0x40},
	{0xfe, 0x04},
	{0x14, 0x78},
	{0x15, 0x78},
	{0x16, 0x78},
	{0x17, 0x78},
	/*window*/
	{0xfe, 0x01},
	{0x92, 0x00},
	{0x94, 0x03},
	{0x95, 0x04},//[10:0]win_out_height-1080
	{0x96, 0x38},
	{0x97, 0x07},//[11:0]win_out_width-1920
	{0x98, 0x80},
	/*ISP*/
	{0xfe, 0x01},
	{0x01, 0x05},
	{0x02, 0x89},
	{0x04, 0x01},
	{0x07, 0xa6},
	{0x08, 0xa9},
	{0x09, 0xa8},
	{0x0a, 0xa7},
	{0x0b, 0xff},
	{0x0c, 0xff},
	{0x0f, 0x00},
	{0x50, 0x1c},
	{0x89, 0x03},
	{0xfe, 0x04},
	{0x28, 0x86},
	{0x29, 0x86},
	{0x2a, 0x86},
	{0x2b, 0x68},
	{0x2c, 0x68},
	{0x2d, 0x68},
	{0x2e, 0x68},
	{0x2f, 0x68},
	{0x30, 0x4f},
	{0x31, 0x68},
	{0x32, 0x67},
	{0x33, 0x66},
	{0x34, 0x66},
	{0x35, 0x66},
	{0x36, 0x66},
	{0x37, 0x66},
	{0x38, 0x62},
	{0x39, 0x62},
	{0x3a, 0x62},
	{0x3b, 0x62},
	{0x3c, 0x62},
	{0x3d, 0x62},
	{0x3e, 0x62},
	{0x3f, 0x62},
	/****DVP & MIPI****/
	{0xfe, 0x01},
	{0x9a, 0x06},
	{0x99, 0x00},
	{0xfe, 0x00},
	{0x7b, 0x2a},
	{0x23, 0x2d},
	{0xfe, 0x03},
	{0x01, 0x27},
	{0x02, 0x56},
	{0x03, 0x8e},
	{0x12, 0x80},
	{0x13, 0x07},
	{0xfe, 0x00},
	{0x3e, 0x81},
    {0x3e, 0x91},

	{REG_NULL, 0x00},
#endif
};


static k_sensor_mode gc2053_mode_info[] = {
    {
        .index = 0,
        .sensor_type = GC2053_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
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
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B,
        },
        .reg_list = gc2053_mipi4lane_1080p_30fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
};

static k_bool gc2053_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int gc2053_power_rest(k_s32 on)
{
    #define VICAP_GC2053_RST_GPIO     (0)  //24// 

    kd_pin_mode(VICAP_GC2053_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_GC2053_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_GC2053_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_GC2053_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(VICAP_GC2053_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static k_s32 gc2053_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    ret = sensor_reg_read(&dev->i2c_info, GC2053_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, GC2053_REG_CHIP_ID_L, &id_low);
    if (ret) {
        pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}


static int gc2053_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 gc2053_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!gc2053_init_flag) {
            gc2053_power_rest(on);
            gc2053_i2c_init(&dev->i2c_info);
        // }
        gc2053_sensor_get_chip_id(ctx, &chip_id);
    } else {
        gc2053_init_flag = K_FALSE;
        gc2053_power_rest(on);
    }

    return ret;
}

static k_s32 gc2053_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(gc2053_mode_info) / sizeof(k_sensor_mode); i++) {
            if (gc2053_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(gc2053_mode_info[i]);
                dev->sensor_mode = &(gc2053_mode_info[i]);
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
    case 3:
    case 6:

        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);


        current_mode->ae_info.frame_length = 2200;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000015152;//s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 15.984375;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        current_mode->ae_info.color_type = SENSOR_COLOR ;

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = GC2053_MIN_GAIN_STEP;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.cur_frame_length - 1;    //2.5ms //197;  // 3ms
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length -1;
        current_mode->ae_info.min_vs_integraion_line =  1;

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

        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;

        current_mode->ae_info.cur_again = 1.0;
        current_mode->ae_info.cur_dgain = 1.0;

        current_mode->ae_info.cur_vs_again = 1.0;
        current_mode->ae_info.cur_vs_dgain = 1.0;

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 15.984375;
        current_mode->ae_info.a_gain.step = (1.0f/64.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 15.984375;
        current_mode->ae_info.a_vs_gain.step = (1.0f/64.0f);//

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f/1024.0f);//
        current_mode->ae_info.cur_fps = current_mode->fps;
        current_mode->sensor_again = 0;
        current_mode->et_line = 0;

        break;


    default:
        break;
    }

    k_u16 again_h;
    k_u16 again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    ret = sensor_reg_read(&dev->i2c_info, GC2053_REG_AGAIN_H, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, GC2053_REG_AGAIN_L, &again_l);
    again = (float)(again_l>>2)/64.0f + again_h;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, GC2053_REG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, GC2053_REG_EXP_TIME_L, &exp_time_l);
    exp_time = ((exp_time_h & 0x3f) << 8) + exp_time_l;

    current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time *  exp_time;
    gc2053_init_flag = K_TRUE;

    return ret;
}


static k_s32 gc2053_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(gc2053_mode_info) / sizeof(k_sensor_mode); i++) {
        if (gc2053_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &gc2053_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(gc2053_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 gc2053_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2053_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 gc2053_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 gc2053_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 gc2053_sensor_set_stream(void *ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        // ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x01);
    } else {
        // ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x00);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 gc2053_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

static k_s32 gc2053_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5)<<2;
        if(current_mode->sensor_again !=again)
        {
	        ret = sensor_reg_write(&dev->i2c_info, GC2053_REG_AGAIN_H,(again>>8) & 0x0f);
	        ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_AGAIN_L, again & 0xfc);
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/256.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5)<<2;
        if(current_mode->sensor_again !=again)
        {
	        ret = sensor_reg_write(&dev->i2c_info, GC2053_REG_AGAIN_H,(again>>8) & 0x0f);
	        ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_AGAIN_L, again & 0xfc);
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/256.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again*1000) );

    return ret;
}

static k_s32 gc2053_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 gc2053_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, GC2053_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, GC2053_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 gc2053_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 gc2053_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev *dev = ctx;

    k_u16 exp_reg = 0;
    k_u16 exp_reg_l = 0;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];

        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));
        if (current_mode->et_line != exp_line)
        {
            ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_EXP_TIME_H, (exp_line >> 8) & 0x3f);
            ret |= sensor_reg_write(&dev->i2c_info, GC2053_REG_EXP_TIME_L, (exp_line ) & 0xff);
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

k_s32 gc2053_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

k_s32 gc2053_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 gc2053_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

k_s32 gc2053_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 gc2053_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

k_s32 gc2053_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 gc2053_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 gc2053_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

k_s32 gc2053_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 gc2053_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

k_s32 gc2053_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 gc2053_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}


struct sensor_driver_dev gc2053_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c3",   //"i2c0", //"i2c3",
        .slave_addr = 0x37,
        .reg_addr_size = SENSOR_REG_VALUE_8BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "gc2053",
    .sensor_func = {
        .sensor_power = gc2053_sensor_power_on,
        .sensor_init = gc2053_sensor_init,
        .sensor_get_chip_id = gc2053_sensor_get_chip_id,
        .sensor_get_mode = gc2053_sensor_get_mode,
        .sensor_set_mode = gc2053_sensor_set_mode,
        .sensor_enum_mode = gc2053_sensor_enum_mode,
        .sensor_get_caps = gc2053_sensor_get_caps,
        .sensor_conn_check = gc2053_sensor_conn_check,
        .sensor_set_stream = gc2053_sensor_set_stream,
        .sensor_get_again = gc2053_sensor_get_again,
        .sensor_set_again = gc2053_sensor_set_again,
        .sensor_get_dgain = gc2053_sensor_get_dgain,
        .sensor_set_dgain = gc2053_sensor_set_dgain,
        .sensor_get_intg_time = gc2053_sensor_get_intg_time,
        .sensor_set_intg_time = gc2053_sensor_set_intg_time,
        .sensor_get_exp_parm = gc2053_sensor_get_exp_parm,
        .sensor_set_exp_parm = gc2053_sensor_set_exp_parm,
        .sensor_get_fps = gc2053_sensor_get_fps,
        .sensor_set_fps = gc2053_sensor_set_fps,
        .sensor_get_isp_status = gc2053_sensor_get_isp_status,
        .sensor_set_blc = gc2053_sensor_set_blc,
        .sensor_set_wb = gc2053_sensor_set_wb,
        .sensor_get_tpg = gc2053_sensor_get_tpg,
        .sensor_set_tpg = gc2053_sensor_set_tpg,
        .sensor_get_expand_curve = gc2053_sensor_get_expand_curve,
        .sensor_get_otp_data = gc2053_sensor_get_otp_data,
        .sensor_mirror_set = gc2053_sensor_mirror_set,
    },
};
