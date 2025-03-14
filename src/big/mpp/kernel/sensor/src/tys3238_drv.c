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

/* Chip ID */


/* Exposure control */

/* Analog gain control */

#define TTY3238_MIN_GAIN_STEP    (1.0f/15.0f)

static k_u8 mirror_flag = 0; 


static k_sensor_reg tys3238_mirror[] = {
    
    {REG_NULL, 0x00},
};


/***********************************************************************
//BF3238_MIPI_Raw10_XCLK24M_PCLK74.25M_V1_20210323
//XCLK:24M;  MIPICLK:742.5M  PCLK(RAW10):74.25M
//行长：2200  帧长：1125
//Max fps:30fps
//1928*1088
************************************************************************/
static k_sensor_reg tys3238_mipi_2lane_1920x1080_init[] =
{
#if 1
    {0xf2, 0x01},
    {0xf2, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0x00, 0x41},
    {0x03, 0x20},//10bit

    {0x06, 0x50},//     add vblank(default 0x13 = 19 ) -> 80 = 0x50

    {0x0b, 0x4c},
    {0x0c, 0x04},
    {0x0F, 0x48},
    {0x15, 0x4b},
    {0x16, 0x63},
    {0x19, 0x4e},
    {0x1b, 0x7a},
    {0x1d, 0x7a},
    {0x25, 0x88},
    {0x26, 0x48},
    {0x27, 0x86},
    {0x28, 0x44},
    {0x2a, 0x7c},
    {0x2B, 0x7a},
    {0x2E, 0x06},
    {0x2F, 0x53},
    {0xe0, 0x00},
    {0xe1, 0xef},
    {0xe2, 0x47},
    {0xe3, 0x43},
    {0xe7, 0x2B},
    {0xe8, 0x69},
    {0xe9, 0x8b},//0x0b},
    {0xea, 0xb7},
    {0xeb, 0x04},
    {0xe4, 0x7a},
    {0x7d, 0x0e},  //MIPI
    {0xc9, 0x80}, //1928*1088
    // {0xcd, 0x88},
    // {0xcf, 0x40},

    {0xca ,0x70},       // hwin
    {0xcb ,0x40},       // v win 

    {0xcc ,0x00},       // h start = 0x8c = 12
    {0xce ,0x00},       // v start 

    {0xcd ,0x80},       //  hstop  1920*1080
    {0xcf ,0x38},       // v stop 

    {0x30, 0x01},
    {0x4d, 0x00},

    {0x59, 0x10},
    {0x5a, 0x10},
    {0x5b, 0x10},
    {0x5c, 0x10},
    {0x5e, 0x22},
    {0x6a, 0x1f},
    {0x6b, 0x04},
    {0x6c, 0x20},
    {0x6f, 0x10},
    {REG_NULL, 0x00},

#else
    {0xf2 ,0x01},
    {0xf2 ,0x00},
    {0xf3 ,0x00},
    {0xf3 ,0x00},
    {0xf3 ,0x00},
    {0x00 ,0x41},
    {0x03 ,0x20},//10bit
    {0x0b ,0x28},
    {0x0c ,0x05},
    {0x0F ,0x48},
    {0x15 ,0x4b},
    {0x16 ,0x63},
    {0x19 ,0x4e},
    {0x1b ,0x7a},
    {0x1d ,0x7a},
    {0x25 ,0x88},
    {0x26 ,0x48},
    {0x27 ,0x86},
    {0x28 ,0x44},
    {0x2a ,0x7c},
    {0x2B ,0x7a},
    {0x2E ,0x06},
    {0x2F ,0x53},
    {0xe0 ,0x00},
    {0xe1 ,0xef},
    {0xe2 ,0x47},
    {0xe3 ,0x43},
    {0xe7 ,0x2B},
    {0xe8 ,0x69},
    {0xe9 ,0x0b},
    {0xea ,0xb7},
    {0xeb ,0x04},
    {0xe4 ,0x7a},
    {0x7d ,0x0e},//MIPI
    {0xc9 ,0x80},
    // {0xcd ,0x88},  //1928*1088
    // {0xcf ,0x40},

    {0xcd ,0x80},  //1920*1080
    {0xcf ,0x38},

    {0x59 ,0x10},
    {0x5a ,0x10},
    {0x5b ,0x10},
    {0x5c ,0x10},
    {0x5e ,0x22},
    {0x6a ,0x1f},
    {0x6b ,0x08},
    {0x6c ,0x20},
    {0x6f ,0x10},
    {REG_NULL, 0x00},
#endif
};


/***********************************************************************
//BF3238_MIPI_Raw10_XCLK24M_PCLK74.25M_V1_20210323
//XCLK:24M;  MIPICLK:742.5M  PCLK(RAW10):74.25M
//行长：2200  帧长：1125
//Max fps:30fps
//1928*1088
************************************************************************/
static k_sensor_reg tys3238_mipi_1lane_1280x960_init[] =
{

    {0xf2, 0x01},
    {0xf2, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0xf3, 0x00},
    {0x00, 0x41},
    {0x03, 0x20},//10bit
    {0x0b, 0x4c},
    {0x0c, 0x04},
    {0x0F, 0x48},
    {0x15, 0x4b},
    {0x16, 0x63},
    {0x19, 0x4e},
    {0x1b, 0x7a},
    {0x1d, 0x7a},
    {0x25, 0x88},
    {0x26, 0x48},
    {0x27, 0x86},
    {0x28, 0x44},
    {0x2a, 0x7c},
    {0x2B, 0x7a},
    {0x2E, 0x06},
    {0x2F, 0x53},
    {0xe0, 0x00},
    {0xe1, 0xef},
    {0xe2, 0x47},
    {0xe3, 0x43},
    {0xe7, 0x2B},
    {0xe8, 0x69},
    {0xe9, 0x0b},
    {0xea, 0xb7},
    {0xeb, 0x04},
    {0xe4, 0x7a},
    {0x7d, 0x0e},  //MIPI
    {0xc9, 0x80}, //1928*1088
    // {0xcd, 0x88},
    // {0xcf, 0x40},
    {0xca, 0x50},
    {0xcb, 0x30},
    {0xcc, 0x00},    
    {0xcd ,0x00},           //act = 1280 x 960
    {0xce ,0x00}, 
    {0xcf ,0xc0},

    {0x30, 0x01},
    {0x4d, 0x00},

    {0x59, 0x10},
    {0x5a, 0x10},
    {0x5b, 0x10},
    {0x5c, 0x10},
    {0x5e, 0x22},
    {0x6a, 0x1f},
    {0x6b, 0x04},
    {0x6c, 0x20},
    {0x6f, 0x10},
    {REG_NULL, 0x00},
};

static k_sensor_ae_info sensor_ae_info[] = {
    // 1080P30 
     {
        .frame_length = 1125,
        .cur_frame_length = 1125,
        .one_line_exp_time = 0.000028,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 16,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000028,
        .gain_increment = TTY3238_MIN_GAIN_STEP,
        .max_integraion_line = 1125 - 8,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000028 * (1125 - 8),
        .min_integraion_time = 0.000028 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 15.5,
            .step = (1.0f/128.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 15.5,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 30,
    },

};

static k_sensor_mode tys3238_mode_info[] = {
    {
        .index = 0,
        .sensor_type = BY3238_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
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
        .bayer_pattern = BAYER_PAT_BGGR,//BAYER_PAT_GRBG, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2B,
        },
        .reg_list = tys3238_mipi_2lane_1920x1080_init, //tys3238_mipi_2lane_640x480_init, // tys3238_mipi_2lane_1080x1280_init, 
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 24, //8,	// 74.25M
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[0],
    },

    {
        .index = 1,
        .sensor_type = BY3238_MIPI_CSI2_1280X960_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 960,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 960,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_BGGR,//BAYER_PAT_GRBG, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2B,
        },
        .reg_list = tys3238_mipi_1lane_1280x960_init, //tys3238_mipi_2lane_640x480_init, // tys3238_mipi_2lane_1080x1280_init, 
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 24, //8,	// 74.25M
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[0],
    },

};

static k_bool tys3238_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

#define VICAP_TTY3238_RST_GPIO     (62)  //24// 


static int tys3238_power_rest(k_s32 on)
{
     
    kd_pin_mode(VICAP_TTY3238_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_TTY3238_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_TTY3238_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_TTY3238_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

        rt_kprintf("VICAP_TTY3238_RST_GPIO gpio is %d xxx \n", VICAP_TTY3238_RST_GPIO);
    } else {
        kd_pin_write(VICAP_TTY3238_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int tys3238_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 tys3238_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    kd_pin_mode(VICAP_TTY3238_RST_GPIO, GPIO_DM_OUTPUT);
    kd_pin_write(VICAP_TTY3238_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

    tys3238_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, 0xfc, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, 0xfd , &id_low);
    if (ret) {
        pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    if(*chip_id != 0x03238)
        ret = -1;

    return ret;
}


static k_s32 tys3238_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!tys3238_init_flag) {
            tys3238_power_rest(on);
            tys3238_i2c_init(&dev->i2c_info);
        // }
        ret = tys3238_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        tys3238_init_flag = K_FALSE;
        tys3238_power_rest(on);
    }

    return ret;
}

static k_s32 tys3238_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(tys3238_mode_info) / sizeof(k_sensor_mode); i++) {
            if (tys3238_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(tys3238_mode_info[i]);
                dev->sensor_mode = &(tys3238_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    // write sensor reg 
    ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

    // set mirror
    if(mirror_flag == 1)
    {
        sensor_reg_list_write(&dev->i2c_info, tys3238_mirror);
    }

    // config ae info
    memcpy(&current_mode->ae_info, current_mode->sensor_ae_info, sizeof(k_sensor_ae_info));
    current_mode->sensor_again = 0;
    current_mode->et_line = 0;

    k_u16 again_h;
    k_u16 again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    //ret = sensor_reg_read(&dev->i2c_info, 0x3508, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, 0x6a, &again_l);
    if(again_l <= 0xf)
        again = 1.0;
    else
        again = (float)again_l / 15 ;  // 0xf = 1gain  0x32 - 0xf * /15

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    // ret = sensor_reg_read(&dev->i2c_info, 0x3e00, &exp_time_lh);
    ret = sensor_reg_read(&dev->i2c_info, 0x6b, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, 0x6c, &exp_time_l);
    exp_time = ((exp_time_h & 0xff) << 8) + exp_time_l;

    current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time *  exp_time;

    tys3238_init_flag = K_TRUE;
    return ret;
}


static k_s32 tys3238_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(tys3238_mode_info) / sizeof(k_sensor_mode); i++) {
        if (tys3238_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &tys3238_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(tys3238_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 tys3238_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 tys3238_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 tys3238_sensor_set_stream(void *ctx, k_s32 enable)
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

static k_s32 tys3238_sensor_get_again(void *ctx, k_sensor_gain *gain)
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


static k_s32 tys3238_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 again, dgain, total;
    k_u8 i;
    k_u32 coarse_again, fine_again, fine_again_reg, coarse_again_reg;
    k_u32 a_gain;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 15 + 0.5);
		if(current_mode->sensor_again !=again)
        {
            ret = sensor_reg_write(&dev->i2c_info, 0x6a, again);

            current_mode->sensor_again = again;
		}

		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/15;
		
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 15 + 0.5);
		if(current_mode->sensor_again !=again)
        {  
		}

		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/64.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again*1000) );

    return ret;
}

static k_s32 tys3238_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 tys3238_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, TTY3238_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, TTY3238_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, TTY3238_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, TTY3238_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 tys3238_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 tys3238_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));

        // pr_err("current_mode->et_line is %d exp_line is %d exp_reg is %x exp_reg_l is %x \n", current_mode->et_line, exp_line, exp_reg, exp_reg_l);
        if (current_mode->et_line != exp_line)
        {
            exp_line = exp_line;

             ret |= sensor_reg_write(&dev->i2c_info, 0x6b, (exp_line >>8) & 0xff);
             ret |= sensor_reg_write(&dev->i2c_info, 0x6c, (exp_line & 0xff));
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

static k_s32 tys3238_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 tys3238_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 tys3238_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 tys3238_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 tys3238_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 tys3238_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 tys3238_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 tys3238_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    return 0;
}


struct sensor_driver_dev tys3238_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c4", 
        .slave_addr = 0x6e, //0x30,
        .reg_addr_size = SENSOR_REG_VALUE_8BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "tys3238",
    .sensor_func = {
        .sensor_power = tys3238_sensor_power_on,
        .sensor_init = tys3238_sensor_init,
        .sensor_get_chip_id = tys3238_sensor_get_chip_id,
        .sensor_get_mode = tys3238_sensor_get_mode,
        .sensor_set_mode = tys3238_sensor_set_mode,
        .sensor_enum_mode = tys3238_sensor_enum_mode,
        .sensor_get_caps = tys3238_sensor_get_caps,
        .sensor_conn_check = tys3238_sensor_conn_check,
        .sensor_set_stream = tys3238_sensor_set_stream,
        .sensor_get_again = tys3238_sensor_get_again,
        .sensor_set_again = tys3238_sensor_set_again,
        .sensor_get_dgain = tys3238_sensor_get_dgain,
        .sensor_set_dgain = tys3238_sensor_set_dgain,
        .sensor_get_intg_time = tys3238_sensor_get_intg_time,
        .sensor_set_intg_time = tys3238_sensor_set_intg_time,
        .sensor_get_exp_parm = tys3238_sensor_get_exp_parm,
        .sensor_set_exp_parm = tys3238_sensor_set_exp_parm,
        .sensor_get_fps = tys3238_sensor_get_fps,
        .sensor_set_fps = tys3238_sensor_set_fps,
        .sensor_get_isp_status = tys3238_sensor_get_isp_status,
        .sensor_set_blc = tys3238_sensor_set_blc,
        .sensor_set_wb = tys3238_sensor_set_wb,
        .sensor_get_tpg = tys3238_sensor_get_tpg,
        .sensor_set_tpg = tys3238_sensor_set_tpg,
        .sensor_get_expand_curve = tys3238_sensor_get_expand_curve,
        .sensor_get_otp_data = tys3238_sensor_get_otp_data,
        .sensor_mirror_set = tys3238_sensor_mirror_set,
    },
};
