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
#include <math.h>
#include "drv_gpio.h"

#include "k_board_config_comm.h"

#define pr_info(...) /* rt_kprintf(__VA_ARGS__) */
#define pr_debug(...) /*rt_kprintf(__VA_ARGS__)*/
#define pr_warn(...) /*rt_kprintf(__VA_ARGS__)*/
#define pr_err(...) rt_kprintf(__VA_ARGS__)

/* Streaming Mode */
#define SC201CS_REG_MODE_SELECT 0x3000
#define SC201CS_MODE_STANDBY 0x01
#define SC201CS_MODE_STREAMING 0x00

/* Lines per frame */
#define SC201CS_REG_LPFR 0x3030

/* Chip ID */
#define SC201CS_REG_ID                     0x3107

/* Exposure control */
#define SC201CS_REG_EXP_TIME_H    0x3e01
#define SC201CS_REG_EXP_TIME_L    0x3e02
#define SC201CS_VMAX 1250//4500

/* Analog gain control */
#define SC201CS_REG_AGAIN			0x3e09
#define SC201CS_REG_DGAIN			0x3e07
#define SC201CS_AGAIN_STEP (1.0f/64.0f)

/* Group hold register */
#define SC201CS_REG_HOLD 0x3001

/* Input clock rate */
#define SC201CS_INCLK_RATE 24000000

/* CSI2 HW configuration */
#define SC201CS_LINK_FREQ 594000000
#define SC201CS_NUM_DATA_LANES 4

#define SC201CS_REG_MIN 0x00
#define SC201CS_REG_MAX 0xfffff



static const k_sensor_reg sc201cs_mipi_1lane_raw10_1600x1200_30fps_mclk_27m_regs[] = {
	//MIPI clock 720Mbps, MCLK 72MHz
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36ea,0xc8},
    {0x36eb,0x25},
    {0x36ec,0x00},
    {0x36ed,0x04},
    {0x36e9,0x41},
    {0x300b,0x44},
    {0x301f,0x31},
    //{0x320e,0x05},	//HTS:1920, VTS: 1500
    //{0x320f,0xdc},	//VTS default setting is 1250
    {0x3248,0x02},
    {0x3253,0x0a},
    {0x3301,0xff},
    {0x3302,0xff},
    {0x3303,0x10},
    {0x3306,0x28},
    {0x3307,0x02},
    {0x330a,0x00},
    {0x330b,0xb0},
    {0x3318,0x02},
    {0x3320,0x06},
    {0x3321,0x02},
    {0x3326,0x12},
    {0x3327,0x0e},
    {0x3328,0x03},
    {0x3329,0x0f},
    {0x3364,0x4f},
    {0x33b3,0x40},
    {0x33f9,0x2c},
    {0x33fb,0x38},
    {0x33fc,0x0f},
    {0x33fd,0x1f},
    {0x349f,0x03},
    {0x34a6,0x01},
    {0x34a7,0x1f},
    {0x34a8,0x40},
    {0x34a9,0x30},
    {0x34ab,0xa6},
    {0x34ad,0xa6},
    {0x3622,0x60},
    {0x3623,0x40},
    {0x3624,0x61},
    {0x3625,0x08},
    {0x3626,0x03},
    {0x3630,0xa8},
    {0x3631,0x84},
    {0x3632,0x90},
    {0x3633,0x43},
    {0x3634,0x09},
    {0x3635,0x82},
    {0x3636,0x48},
    {0x3637,0xe4},
    {0x3641,0x22},
    {0x3670,0x0f},
    {0x3674,0xc0},
    {0x3675,0xc0},
    {0x3676,0xc0},
    {0x3677,0x86},
    {0x3678,0x88},
    {0x3679,0x8c},
    {0x367c,0x01},
    {0x367d,0x0f},
    {0x367e,0x01},
    {0x367f,0x0f},
    {0x3690,0x63},
    {0x3691,0x63},
    {0x3692,0x73},
    {0x369c,0x01},
    {0x369d,0x1f},
    {0x369e,0x8a},
    {0x369f,0x9e},
    {0x36a0,0xda},
    {0x36a1,0x01},
    {0x36a2,0x03},
    {0x3900,0x0d},
    {0x3904,0x06},
    {0x3905,0x98},
    {0x3908,0x10},	//black level target
    {0x391b,0x81},
    {0x391c,0x10},
    {0x391d,0x19},
    {0x3933,0x01},
    {0x3934,0x82},
    {0x3940,0x5d},
    {0x3942,0x01},
    {0x3943,0x82},
    {0x3949,0xc8},
    {0x394b,0x64},
    {0x3952,0x02},
    {0x3e00,0x00},
    {0x3e01,0x4d},
    {0x3e02,0xe0},
    {0x4502,0x34},
    {0x4509,0x30},
    {0x450a,0x71},
    // {0x0100,0x01}
    { REG_NULL, 0x00 }
};

static const k_sensor_reg sc201cs_slave_mode_mipi_1lane_raw10_1600x1200_30fps_mclk_27m_regs[] = {
	//MIPI clock 720Mbps, MCLK 72MHz
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36ea,0xc8},
    {0x36eb,0x25},
    {0x36ec,0x00},
    {0x36ed,0x04},
    {0x36e9,0x41},
    {0x300b,0x40},
    {0x301f,0x32},
    //{0x320e,0x05},	//HTS:1920, VTS: 1500
    //{0x320f,0xdc},
    {0x3222,0x02},
    {0x3224,0x82},
    {0x322e,0x04},//0x04de = VTS - 4 = 1250 - 4 ?
    {0x322f,0xde},
    {0x3248,0x02},
    {0x3253,0x0e},
    {0x3301,0xff},
    {0x3302,0xff},
    {0x3303,0x10},
    {0x3306,0x28},
    {0x3307,0x02},
    {0x330a,0x00},
    {0x330b,0xb0},
    {0x3318,0x02},
    {0x3320,0x06},
    {0x3321,0x02},
    {0x3326,0x12},
    {0x3327,0x0e},
    {0x3328,0x03},
    {0x3329,0x0f},
    {0x3364,0x4f},
    {0x33b3,0x40},
    {0x33f9,0x2c},
    {0x33fb,0x38},
    {0x33fc,0x0f},
    {0x33fd,0x1f},
    {0x349f,0x03},
    {0x34a6,0x01},
    {0x34a7,0x1f},
    {0x34a8,0x40},
    {0x34a9,0x30},
    {0x34ab,0xa6},
    {0x34ad,0xa6},
    {0x3622,0x60},
    {0x3623,0x40},
    {0x3624,0x61},
    {0x3625,0x08},
    {0x3626,0x03},
    {0x3630,0xa8},
    {0x3631,0x84},
    {0x3632,0x90},
    {0x3633,0x43},
    {0x3634,0x09},
    {0x3635,0x82},
    {0x3636,0x48},
    {0x3637,0xe4},
    {0x3641,0x22},
    {0x3670,0x0f},
    {0x3674,0xc0},
    {0x3675,0xc0},
    {0x3676,0xc0},
    {0x3677,0x86},
    {0x3678,0x88},
    {0x3679,0x8c},
    {0x367c,0x01},
    {0x367d,0x0f},
    {0x367e,0x01},
    {0x367f,0x0f},
    {0x3690,0x63},
    {0x3691,0x63},
    {0x3692,0x73},
    {0x369c,0x01},
    {0x369d,0x1f},
    {0x369e,0x8a},
    {0x369f,0x9e},
    {0x36a0,0xda},
    {0x36a1,0x01},
    {0x36a2,0x03},
    {0x3900,0x0d},
    {0x3904,0x06},
    {0x3905,0x98},
    {0x3908,0x10},	//black level target
    {0x391b,0x81},
    {0x391c,0x10},
    {0x391d,0x19},
    {0x3933,0x01},
    {0x3934,0x82},
    {0x3940,0x5d},
    {0x3942,0x01},
    {0x3943,0x82},
    {0x3949,0xc8},
    {0x394b,0x64},
    {0x3952,0x02},
    {0x3e00,0x00},
    {0x3e01,0x4d},
    {0x3e02,0xe0},
    {0x4502,0x34},
    {0x4509,0x30},
    {0x450a,0x71},
    // {0x0100,0x01},
    { REG_NULL, 0x00 }
};

static k_sensor_mode sc201cs_mode_info[] = {
    {
        .index = 0,
        .sensor_type = SC_SC201CS_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
        .size = {
            .bounds_width = 1600,
            .bounds_height = 1200,
            .top = 0,
            .left = 0,
            .width = 1600,
            .height = 1200,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,//BAYER_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 1,
            .data_type = 0x2B,
        },
        .reg_list = sc201cs_mipi_1lane_raw10_1600x1200_30fps_mclk_27m_regs,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 22,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 1,
        .sensor_type = SC_SC201CS_SLAVE_MODE_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
        .size = {
            .bounds_width = 1600,
            .bounds_height = 1200,
            .top = 0,
            .left = 0,
            .width = 1600,
            .height = 1200,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,//BAYER_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 1,
            .data_type = 0x2B,
        },
        .reg_list = sc201cs_slave_mode_mipi_1lane_raw10_1600x1200_30fps_mclk_27m_regs,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 22,
            },
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK1,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 22,
            },
            {K_FALSE},
        },
    },
};

static k_sensor_mode* current_mode = NULL;

static int sc201cs_power_reset(k_s32 on)
{
    // #define SC201CS_RST_PIN                  46
    // #define SC201CS_MASTER_PIN               28

    k_u8 shutdown_gpio;

    shutdown_gpio = 49;

    kd_pin_mode(shutdown_gpio, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(shutdown_gpio, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(shutdown_gpio, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(shutdown_gpio, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(shutdown_gpio, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int sc201cs_i2c_init(k_sensor_i2c_info* i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL) {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 sc201cs_sensor_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev* dev = ctx;
    pr_info("%s enter\n", __func__);

    sc201cs_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, SC201CS_REG_ID, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, SC201CS_REG_ID + 1, &id_low);
    if (ret) {
        // rt_kprintf("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}

static k_s32 sc201cs_sensor_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter, %d\n", __func__, on);
    if (on) {
        sc201cs_power_reset(1);
        sc201cs_i2c_init(&dev->i2c_info);
        ret = sc201cs_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        sc201cs_power_reset(0);
    }

    return ret;
}

static k_s32 sc201cs_sensor_init(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(sc201cs_mode_info) / sizeof(k_sensor_mode); i++) {
            if (sc201cs_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(sc201cs_mode_info[i]);
                dev->sensor_mode = &(sc201cs_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    switch (current_mode->index) {
    default:
        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        current_mode->ae_info.frame_length = SC201CS_VMAX;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000026667; // s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 31.75;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        //current_mode->ae_info.ae_min_interval_frame = 2.5;
        current_mode->ae_info.color_type = SENSOR_COLOR;	//color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = SC201CS_AGAIN_STEP;

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length - 6;
        current_mode->ae_info.min_long_integraion_line = 1;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 6;
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length - 6;
        current_mode->ae_info.min_vs_integraion_line = 1;

        current_mode->ae_info.max_long_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_long_integraion_line;

        current_mode->ae_info.min_long_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_integraion_line;

        current_mode->ae_info.min_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.max_vs_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_vs_integraion_line;

        current_mode->ae_info.min_vs_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_vs_integraion_line;

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
        current_mode->ae_info.a_long_gain.max = 100.0;
        current_mode->ae_info.a_long_gain.step = (1.0f / 256.0f);

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 100.0;
        current_mode->ae_info.a_gain.step = (1.0f / 256.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 100.0;
        current_mode->ae_info.a_vs_gain.step = (1.0f / 256.0f);

        current_mode->ae_info.d_long_gain.max = 1.0;
        current_mode->ae_info.d_long_gain.min = 1.0;
        current_mode->ae_info.d_long_gain.step = (1.0f / 1024.0f);

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f / 1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f / 1024.0f);

        current_mode->ae_info.cur_fps = current_mode->fps;
        current_mode->sensor_again = 0;
        current_mode->et_line = 0;

        break;
    }

	k_u16 dgain, again;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;


    ret = sensor_reg_read(&dev->i2c_info, SC201CS_REG_DGAIN, &dgain);
    ret = sensor_reg_read(&dev->i2c_info, SC201CS_REG_AGAIN, &again);
    current_mode->ae_info.cur_gain = (float)((again+1) * dgain) / 128.0f;

    ret = sensor_reg_read(&dev->i2c_info, SC201CS_REG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, SC201CS_REG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) | ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;

    //pr_info("%s exit, sensor_type:%d\n", __func__, mode.sensor_type);
    return ret;
}

static k_s32 sc201cs_sensor_get_mode(void* ctx, k_sensor_mode* mode)
{
    k_s32 ret = -1;

    for (k_s32 i = 0; i < sizeof(sc201cs_mode_info) / sizeof(k_sensor_mode); i++) {
        if (sc201cs_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &sc201cs_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(sc201cs_mode_info[i]);
            return 0;
        }
    }
    pr_debug("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_set_mode(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_enum_mode(void* ctx, k_sensor_enum_mode* enum_mode)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    if (enum_mode->index >= (sizeof(sc201cs_mode_info) / sizeof(k_sensor_mode))) {
        pr_err("%s, invalid mode index.\n", __func__);
        return -1;
    }

    for (k_s32 i = 0; i < sizeof(sc201cs_mode_info) / sizeof(k_sensor_mode); i++) {
        if (sc201cs_mode_info[i].index == enum_mode->index) {
            memcpy(&enum_mode->mode, &sc201cs_mode_info[i], sizeof(k_sensor_mode));
            return 0;
        }
    }
    return ret;
}

static k_s32 sc201cs_sensor_get_caps(void* ctx, k_sensor_caps* caps)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(caps, 0, sizeof(k_sensor_caps));
    caps->bit_width = current_mode->bit_width;
    caps->bayer_pattern = current_mode->bayer_pattern;
    caps->resolution.width = current_mode->size.width;
    caps->resolution.height = current_mode->size.height;

    return ret;
}

static k_s32 sc201cs_sensor_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 sc201cs_sensor_set_stream(void* ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x01);
    } else {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x00);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 sc201cs_sensor_get_again(void* ctx, k_sensor_gain* gain)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

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

static k_s32 sc201cs_sensor_set_again(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 SensorGain, again, dgain;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR)
    {
        SensorGain = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5)<<1;
        if(current_mode->sensor_again !=SensorGain)
        {
        	if(SensorGain>=2048)
        	{
         		again = 15;	//16x
         		dgain = SensorGain>>4;
         	}
			else if(SensorGain>=1024)
			{
				again = 7;	//8x
				dgain = SensorGain>>3;
			}
			else if(SensorGain>=512)
			{
				again = 3;	//4x
				dgain = SensorGain>>2;
			}
			else if(SensorGain>=256)
			{
				again = 1;	//2x
				dgain = SensorGain>>1;
			}
			else// if(SensorGain>=128)
			{
				again = 0;	//1x
				dgain = SensorGain;
			}

			ret = sensor_reg_write(&dev->i2c_info, SC201CS_REG_AGAIN,again);
			ret |= sensor_reg_write(&dev->i2c_info, SC201CS_REG_DGAIN,dgain);
			current_mode->sensor_again = SensorGain;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/128.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        SensorGain = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5)<<1;
        if(current_mode->sensor_again !=SensorGain)
        {
        	if(SensorGain>=2048)
        	{
         		again = 15;	//16x
         		dgain = SensorGain>>4;
         	}
			else if(SensorGain>=1024)
			{
				again = 7;	//8x
				dgain = SensorGain>>3;
			}
			else if(SensorGain>=512)
			{
				again = 3;	//4x
				dgain = SensorGain>>2;
			}
			else if(SensorGain>=256)
			{
				again = 1;	//2x
				dgain = SensorGain>>1;
			}
			else// if(SensorGain>=128)
			{
				again = 0;	//1x
				dgain = SensorGain;
			}

			ret = sensor_reg_write(&dev->i2c_info, SC201CS_REG_AGAIN,again);
			ret |= sensor_reg_write(&dev->i2c_info, SC201CS_REG_DGAIN,dgain);
			current_mode->sensor_again = SensorGain;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/128.0f;
        //TODO
        current_mode->ae_info.cur_vs_again = (float)current_mode->sensor_again/128.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, exp_frame_type(%d), cur_again(%u)\n", __func__, gain.exp_frame_type, (k_u32)(current_mode->ae_info.cur_again * 1000));

    return ret;
}

static k_s32 sc201cs_sensor_get_dgain(void* ctx, k_sensor_gain* gain)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

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

static k_s32 sc201cs_sensor_set_dgain(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev* dev = ctx;

    pr_debug("%s enter exp_frame_type(%d)\n", __func__, gain.exp_frame_type);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        current_mode->ae_info.cur_dgain = dgain / 1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        current_mode->ae_info.cur_dgain = dgain / 1024.0f;

        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 1024);
        // TODO wirte vs gain register
        current_mode->ae_info.cur_vs_dgain = dgain / 1024.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    current_mode->ae_info.cur_gain = current_mode->ae_info.cur_again * current_mode->ae_info.cur_dgain;
    pr_debug("%s,cur_gain(%d)\n", __func__, (k_u32)(current_mode->ae_info.cur_gain * 10000));

    return ret;
}

static k_s32 sc201cs_sensor_get_intg_time(void* ctx, k_sensor_intg_time* time)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

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

static k_s32 sc201cs_sensor_set_intg_time(void* ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR)
    {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));
        if (current_mode->et_line != exp_line)
        {
	        ret |= sensor_reg_write(&dev->i2c_info, SC201CS_REG_EXP_TIME_H, ( exp_line >> 4) & 0xff);
	        ret |= sensor_reg_write(&dev->i2c_info, SC201CS_REG_EXP_TIME_L, ( exp_line << 4) & 0xff);
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
    pr_debug("%s exp_frame_type(%d), exp_line(%d), integraion_time(%u)\n",
        __func__, time.exp_frame_type, exp_line, (k_u32)(integraion_time * 1000000000));

    return ret;
}

static k_s32 sc201cs_sensor_get_exp_parm(void* ctx, k_sensor_exposure_param* exp_parm)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 sc201cs_sensor_set_exp_parm(void* ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_get_fps(void* ctx, k_u32* fps)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    *fps = current_mode->fps;

    return ret;
}

static k_s32 sc201cs_sensor_set_fps(void* ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_get_isp_status(void* ctx, k_sensor_isp_status* staus)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 sc201cs_sensor_set_blc(void* ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_set_wb(void* ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_get_tpg(void* ctx, k_sensor_test_pattern* tpg)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 sc201cs_sensor_set_tpg(void* ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 sc201cs_sensor_get_expand_curve(void* ctx, k_sensor_compand_curve* curve)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 sc201cs_sensor_get_otp_data(void* ctx, void* data)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(data, 0, sizeof(void*));

    return ret;
}

static k_s32 sc201cs_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev sc201cs_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c1",
        .slave_addr = 0x30,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "sc201cs",
    .sensor_func = {
        .sensor_power = sc201cs_sensor_power_on,
        .sensor_init = sc201cs_sensor_init,
        .sensor_get_chip_id = sc201cs_sensor_get_chip_id,
        .sensor_get_mode = sc201cs_sensor_get_mode,
        .sensor_set_mode = sc201cs_sensor_set_mode,
        .sensor_enum_mode = sc201cs_sensor_enum_mode,
        .sensor_get_caps = sc201cs_sensor_get_caps,
        .sensor_conn_check = sc201cs_sensor_conn_check,
        .sensor_set_stream = sc201cs_sensor_set_stream,
        .sensor_get_again = sc201cs_sensor_get_again,
        .sensor_set_again = sc201cs_sensor_set_again,
        .sensor_get_dgain = sc201cs_sensor_get_dgain,
        .sensor_set_dgain = sc201cs_sensor_set_dgain,
        .sensor_get_intg_time = sc201cs_sensor_get_intg_time,
        .sensor_set_intg_time = sc201cs_sensor_set_intg_time,
        .sensor_get_exp_parm = sc201cs_sensor_get_exp_parm,
        .sensor_set_exp_parm = sc201cs_sensor_set_exp_parm,
        .sensor_get_fps = sc201cs_sensor_get_fps,
        .sensor_set_fps = sc201cs_sensor_set_fps,
        .sensor_get_isp_status = sc201cs_sensor_get_isp_status,
        .sensor_set_blc = sc201cs_sensor_set_blc,
        .sensor_set_wb = sc201cs_sensor_set_wb,
        .sensor_get_tpg = sc201cs_sensor_get_tpg,
        .sensor_set_tpg = sc201cs_sensor_set_tpg,
        .sensor_get_expand_curve = sc201cs_sensor_get_expand_curve,
        .sensor_get_otp_data = sc201cs_sensor_get_otp_data,
        .sensor_mirror_set = sc201cs_sensor_mirror_set,
    },
};
