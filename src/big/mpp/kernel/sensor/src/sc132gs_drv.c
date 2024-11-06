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

#define SC132GS_MIN_GAIN_STEP    (1.0f/16.0f)

static k_u8 mirror_flag = 0; 


static k_sensor_reg sc132gs_mirror[] = {
    
    {REG_NULL, 0x00},
};


/*
 * Xclk 24Mhz
 * Pclk 72Mhz
 * linelength 1696(0x06a0)
 * framelength 2122(0x084a)
 * grabwindow_width 1080
 * grabwindow_height 1280
 * mipi 2 lane
 * 10 bit
 * max_framerate 30fps
 * mipi_datarate per lane 360Mbps
 */
static k_sensor_reg sc132gs_mipi_2lane_1080x1280_init[] =
{

   {0x0103, 0x01},
	{0x0100, 0x00},

	//PLL bypass
	{0x36e9, 0x80},
	{0x36f9, 0x80},

	{0x3018, 0x32},
	{0x3019, 0x0c},
	{0x301a, 0xb4},
	{0x3031, 0x0a},
	{0x3032, 0x60},
	{0x3038, 0x44},
	{0x3207, 0x17},
	{0x320c, 0x05},             // 0x5dc = 1500
	{0x320d, 0xdc},
	{0x320e, 0x09},             // 0x960 = 2400
	{0x320f, 0x60},
	{0x3250, 0xcc},
	{0x3251, 0x02},
	{0x3252, 0x09},
	{0x3253, 0x5b},
	{0x3254, 0x05},
	{0x3255, 0x3b},
	{0x3306, 0x78},
	{0x330a, 0x00},
	{0x330b, 0xc8},
	{0x330f, 0x24},
	{0x3314, 0x80},
	{0x3315, 0x40},
	{0x3317, 0xf0},
	{0x331f, 0x12},
	{0x3364, 0x00},
	{0x3385, 0x41},
	{0x3387, 0x41},
	{0x3389, 0x09},
	{0x33ab, 0x00},
	{0x33ac, 0x00},
	{0x33b1, 0x03},
	{0x33b2, 0x12},
	{0x33f8, 0x02},
	{0x33fa, 0x01},
	{0x3409, 0x08},
	{0x34f0, 0xc0},
	{0x34f1, 0x20},
	{0x34f2, 0x03},
	{0x3622, 0xf5},
	{0x3630, 0x5c},
	{0x3631, 0x80},
	{0x3632, 0xc8},
	{0x3633, 0x32},
	{0x3638, 0x2a},
	{0x3639, 0x07},
	{0x363b, 0x48},
	{0x363c, 0x83},
	{0x363d, 0x10},
	{0x36ea, 0x38},
	{0x36fa, 0x25},
	{0x36fb, 0x05},
	{0x36fd, 0x04},
	{0x3900, 0x11},
	{0x3901, 0x05},
	{0x3902, 0xc5},
	{0x3904, 0x04},
	{0x3908, 0x91},
	{0x391e, 0x00},
	{0x3e01, 0x11},     // 1200 x 16
	{0x3e02, 0x20},
	{0x3e09, 0x20},
	{0x3e0e, 0xd2},
	{0x3e14, 0xb0},
	{0x3e1e, 0x7c},
	{0x3e26, 0x20},
	{0x4418, 0x38},
	{0x4503, 0x10},
	{0x4837, 0x21},
	{0x5000, 0x0e},
	{0x540c, 0x51},
	{0x550f, 0x38},
	{0x5780, 0x67},
	{0x5784, 0x10},
	{0x5785, 0x06},
	{0x5787, 0x02},
	{0x5788, 0x00},
	{0x5789, 0x00},
	{0x578a, 0x02},
	{0x578b, 0x00},
	{0x578c, 0x00},
	{0x5790, 0x00},
	{0x5791, 0x00},
	{0x5792, 0x00},
	{0x5793, 0x00},
	{0x5794, 0x00},
	{0x5795, 0x00},
	{0x5799, 0x04},

	//flip
	//{0x3221, (0x3 << 5)},

	//mirror
	{0x3221, (0x3 << 1)},

	//flip & mirror
	//{0x3221, ((0x3 << 1)|(0x3 << 5))},

	//PLL set
	{0x36e9, 0x20},
	{0x36f9, 0x24},
    {REG_NULL, 0x00},

};

/*
SensorName=SC132GS_MIPI_2L_60
width=1080     //sensor width
height=1280     //sensor height
sensorFormat=0     //0:RAW10 1:YUV 2:RAW8 3:RAW12 4:RAW12
sensorPort=2     //0:DVP 1:mipi_1x 2:mipi_2x 3:mipi_3x 4:mipi_4x 5:mtk_1x,6:mtk_2x
pwdn=1     //0:low 1:high
SlaveID=0x60     //sensor i2c address
i2c_mode=1     //0:8ADDR_8REG 1:16ADDR_8REG 2:8ADDR_16REG 3:16ADDR_16REG
SensorIDAddr=0x36ff     //sensor id register address
SensorID=0x0     //sensor id register value
disply_mode=3     //0:YCbYCr/RG_GB  1:YCrYCb/GR_BG  2:CbYCrY/GB_RG 3:CrYCbY/BG_GR
AVDD=2800     //sensor analog power
DOVDD=1800     //sensor io power
DVDD=1500     //sensor core power
mclk=12     //sensor input clock 6-99 Mhz
mipiclk=0     //0:mipiclk continuous 1:mipiclk LP
*/
static k_sensor_reg sc132gs_mipi_2lane_640x480_init[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3018,0x32},
    {0x3019,0x0c},
    {0x301a,0xb4},
    {0x301f,0x3c},
    {0x3032,0x60},
    {0x3038,0x44},
    {0x3207,0x17},
    {0x320c,0x02},
    {0x320d,0xee},
    {0x3250,0xcc},
    {0x3251,0x02},
    {0x3252,0x05},
    {0x3253,0x41},
    {0x3254,0x05},
    {0x3255,0x3b},
    {0x3306,0x78},
    {0x330a,0x00},
    {0x330b,0xc8},
    {0x330f,0x24},
    {0x3314,0x80},
    {0x3315,0x40},
    {0x3317,0xf0},
    {0x331f,0x12},
    {0x3364,0x00},
    {0x3385,0x41},
    {0x3387,0x41},
    {0x3389,0x09},
    {0x33ab,0x00},
    {0x33ac,0x00},
    {0x33b1,0x03},
    {0x33b2,0x12},
    {0x33f8,0x02},
    {0x33fa,0x01},
    {0x3409,0x08},
    {0x34f0,0xc0},
    {0x34f1,0x20},
    {0x34f2,0x03},
    {0x3622,0xf5},
    {0x3630,0x5c},
    {0x3631,0x80},
    {0x3632,0xc8},
    {0x3633,0x32},
    {0x3638,0x2a},
    {0x3639,0x07},
    {0x363b,0x48},
    {0x363c,0x83},
    {0x363d,0x10},
    {0x36ea,0x36},
    {0x36eb,0x04},
    {0x36ec,0x03},
    {0x36ed,0x24},
    {0x36fa,0xe5},
    {0x36fb,0x05},
    {0x36fc,0x00},
    {0x36fd,0x04},
    {0x3900,0x11},
    {0x3901,0x05},
    {0x3902,0xc5},
    {0x3904,0x04},
    {0x3908,0x91},
    {0x391e,0x00},
    {0x3e01,0x53},
    {0x3e02,0xe0},
    {0x3e09,0x20},
    {0x3e0e,0xd2},
    {0x3e14,0xb0},
    {0x3e1e,0x7c},
    {0x3e26,0x20},
    {0x4418,0x38},
    {0x4503,0x10},
    {0x4837,0x0d},
    {0x5000,0x0e},
    {0x540c,0x51},
    {0x550f,0x38},
    {0x5780,0x67},
    {0x5784,0x10},
    {0x5785,0x06},
    {0x5787,0x02},
    {0x5788,0x00},
    {0x5789,0x00},
    {0x578a,0x02},
    {0x578b,0x00},
    {0x578c,0x00},
    {0x5790,0x00},
    {0x5791,0x00},
    {0x5792,0x00},
    {0x5793,0x00},
    {0x5794,0x00},
    {0x5795,0x00},
    {0x5799,0x04},

    //640x480}
    {0x3200,0x00},
    {0x3201,0xdc},
    {0x3202,0x01},
    {0x3203,0x94},
    {0x3204,0x03},
    {0x3205,0x7b},
    {0x3206,0x03},
    {0x3207,0x83},
    {0x3208,0x02},
    {0x3209,0x80},
    {0x320a,0x01},
    {0x320b,0xe0},
    {0x3210,0x00},
    {0x3211,0x10},
    {0x3212,0x00},
    {0x3213,0x08},

    //240fps
    {0x320e,0x02},
    {0x320f,0xa3},
    {0x3252,0x02},
    {0x3253,0x9e},
    {0x3e01,0x02},
    {0x3e02,0x00},

    {0x4837,0x1b},
    {0x36ec,0x13},

    {0x36e9,0x24},
    {0x36f9,0x51},

    {REG_NULL, 0x00},
};

static k_sensor_ae_info sensor_ae_info[] = {
    // 1080P30 
     {
        .frame_length = 0x486,
        .cur_frame_length = 0x486,
        .one_line_exp_time = 0.000014,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 28.547,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_MONO,
        .integration_time_increment = 0.000014,
        .gain_increment = SC132GS_MIN_GAIN_STEP,
        .max_integraion_line = 0x486 - 8,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000014 * (0x486 - 8),
        .min_integraion_time = 0.000014 * 1,
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
    {
        .frame_length = 0x486,
        .cur_frame_length = 0x486,
        .one_line_exp_time = 0.000014,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 28.547,
        .int_time_delay_frame = 4,
        .gain_delay_frame = 4,
        .color_type = SENSOR_MONO,
        .integration_time_increment = 0.000014,
        .gain_increment = SC132GS_MIN_GAIN_STEP,
        .max_integraion_line = 0x486 - 8,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000014 * (0x486 - 8),
        .min_integraion_time = 0.000014 * 1,
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

static k_sensor_mode sc132gs_mode_info[] = {
    {
        .index = 0,
        .sensor_type = SC132GS_MIPI_CSI2_1080X1200_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1080,
            .bounds_height = 1280,
            .top = 0,
            .left = 0,
            .width = 1080,
            .height = 1280,
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
        .reg_list = sc132gs_mipi_2lane_1080x1280_init, //sc132gs_mipi_2lane_640x480_init, // sc132gs_mipi_2lane_1080x1280_init, 
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,	// 594/25 = 23.76MHz
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[0],
    },

    {
        .index = 1,
        .sensor_type = SC132GS_MIPI_CSI2_640X480_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 640,
            .bounds_height = 480,
            .top = 0,
            .left = 0,
            .width = 640,
            .height = 480,
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
        .reg_list = sc132gs_mipi_2lane_640x480_init, 
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 24,	// 594/25 = 23.76MHz
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[1],
    },
};

static k_bool sc132gs_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

#define VICAP_SC132GS_RST_GPIO     (62)  //24// 


static int sc132gs_power_rest(k_s32 on)
{
     
    kd_pin_mode(VICAP_SC132GS_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_SC132GS_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_SC132GS_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_SC132GS_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

        rt_kprintf("VICAP_SC132GS_RST_GPIO gpio is %d xxx \n", VICAP_SC132GS_RST_GPIO);
    } else {
        kd_pin_write(VICAP_SC132GS_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int sc132gs_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 sc132gs_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    kd_pin_mode(VICAP_SC132GS_RST_GPIO, GPIO_DM_OUTPUT);
    kd_pin_write(VICAP_SC132GS_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

    sc132gs_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, 0x3107, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, 0x3108 , &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    if(*chip_id != 0x0132)
        ret = -1;

    return ret;
}


static k_s32 sc132gs_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!sc132gs_init_flag) {
            sc132gs_power_rest(on);
            sc132gs_i2c_init(&dev->i2c_info);
        // }
        ret = sc132gs_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        sc132gs_init_flag = K_FALSE;
        sc132gs_power_rest(on);
    }

    return ret;
}

static k_s32 sc132gs_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(sc132gs_mode_info) / sizeof(k_sensor_mode); i++) {
            if (sc132gs_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(sc132gs_mode_info[i]);
                dev->sensor_mode = &(sc132gs_mode_info[i]);
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
        sensor_reg_list_write(&dev->i2c_info, sc132gs_mirror);
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
    //ret = sensor_reg_read(&dev->i2c_info, 0x3509, &again_l);
    again = 1.0 ;//(float)(again_l)/64.0f + again_h;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    // ret = sensor_reg_read(&dev->i2c_info, 0x3e00, &exp_time_lh);
    // ret = sensor_reg_read(&dev->i2c_info, 0x3e01, &exp_time_h);
    // ret = sensor_reg_read(&dev->i2c_info, 0x3e02, &exp_time_l);
    exp_time = ((exp_time_h & 0xff) << 8) + exp_time_l;

    current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time *  exp_time;

    sc132gs_init_flag = K_TRUE;
    return ret;
}


static k_s32 sc132gs_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(sc132gs_mode_info) / sizeof(k_sensor_mode); i++) {
        if (sc132gs_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &sc132gs_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(sc132gs_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 sc132gs_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 sc132gs_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 sc132gs_sensor_set_stream(void *ctx, k_s32 enable)
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

static k_s32 sc132gs_sensor_get_again(void *ctx, k_sensor_gain *gain)
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


static k_s32 sc132gs_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 again, dgain, total;
    k_u8 i;
    k_u32 coarse_again, fine_again, fine_again_reg, coarse_again_reg;
    k_u32 a_gain;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {

        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 1000 + 0.5);

        if((again >= 1000) && (again < (1.813 * 1000)))
        {
            /*1x~1.813*/
            coarse_again = 0x03;
            fine_again_reg = 0x20 + (((again - 1000) / 32 ) + 0.5);
            if(fine_again_reg >= 0x39)
                fine_again_reg = 0x39;
        }
        else if((again >= (1000 * 1.813)) && (again < (3.568 * 1000))){
            /*1.813~3.568x*/
            coarse_again = 0x23;
            fine_again_reg = 0x20 + (((again - 1813) / 56 ) + 0.5);
            if (fine_again > 0x3f)
			    fine_again = 0x3f;
        }
        else if((again >= (1000 * 3.568)) && (again < (7.250 * 1000))){
            /*3.568x~7.250x*/
            coarse_again = 0x27;
            fine_again_reg = 0x20 + (((again - 3568) / 113 ) + 0.5);
            if (fine_again > 0x3f)
			    fine_again = 0x3f;
        }
        else if((again >= (1000 * 7.250)) && (again < (14.5 * 1000))){
            /*7.250x~14.5x*/
            coarse_again = 0x2f;
            fine_again_reg = 0x20 + (((again - 7250) / 226 ) + 0.5);
            if (fine_again > 0x3f)
			    fine_again = 0x3f;
        }
        else {
            /*14.5x~28.547*/
            coarse_again = 0x3f;
            fine_again_reg = 0x20 + (((again - 14500) / 453 ) + 0.5);
            if (fine_again > 0x3f)
			    fine_again = 0x3f;
        }

        // pr_err("again is %x current_mode->sensor_again is %x coarse_again is %x fine_again_reg is %x \n", again, current_mode->sensor_again, coarse_again, fine_again_reg); 

        ret =  sensor_reg_write(&dev->i2c_info,0x3e08,coarse_again);
        ret |=  sensor_reg_write(&dev->i2c_info,0x3e09,fine_again_reg);

        // ret =  sensor_reg_write(&dev->i2c_info,0x3e08,0x3f);
        // ret |=  sensor_reg_write(&dev->i2c_info,0x3e09,0x3f);

        current_mode->sensor_again = again;

		
		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/1000.0f;
		
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5);
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

static k_s32 sc132gs_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 sc132gs_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, SC132GS_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, SC132GS_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, SC132GS_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, SC132GS_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 sc132gs_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 sc132gs_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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

        
        ret = sensor_reg_read(&dev->i2c_info, 0x3e01, &exp_reg);
        ret = sensor_reg_read(&dev->i2c_info, 0x3e02, &exp_reg_l);

        // pr_err("current_mode->et_line is %d exp_line is %d exp_reg is %x exp_reg_l is %x \n", current_mode->et_line, exp_line, exp_reg, exp_reg_l);
        // if (current_mode->et_line != exp_line)
        // {
            exp_line = exp_line * 16;

             ret |= sensor_reg_write(&dev->i2c_info, 0x3e01, (exp_line >>8) & 0xff);
             ret |= sensor_reg_write(&dev->i2c_info, 0x3e02, (exp_line & 0xff));
            current_mode->et_line = exp_line / 16;
	    // }
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

static k_s32 sc132gs_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 sc132gs_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 sc132gs_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 sc132gs_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 sc132gs_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc132gs_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 sc132gs_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 sc132gs_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    return 0;
}


struct sensor_driver_dev sc132gs_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c4", //"i2c3",   //"i2c0", //"i2c3",
        .slave_addr = 0x30, //0x30,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "sc132gs",
    .sensor_func = {
        .sensor_power = sc132gs_sensor_power_on,
        .sensor_init = sc132gs_sensor_init,
        .sensor_get_chip_id = sc132gs_sensor_get_chip_id,
        .sensor_get_mode = sc132gs_sensor_get_mode,
        .sensor_set_mode = sc132gs_sensor_set_mode,
        .sensor_enum_mode = sc132gs_sensor_enum_mode,
        .sensor_get_caps = sc132gs_sensor_get_caps,
        .sensor_conn_check = sc132gs_sensor_conn_check,
        .sensor_set_stream = sc132gs_sensor_set_stream,
        .sensor_get_again = sc132gs_sensor_get_again,
        .sensor_set_again = sc132gs_sensor_set_again,
        .sensor_get_dgain = sc132gs_sensor_get_dgain,
        .sensor_set_dgain = sc132gs_sensor_set_dgain,
        .sensor_get_intg_time = sc132gs_sensor_get_intg_time,
        .sensor_set_intg_time = sc132gs_sensor_set_intg_time,
        .sensor_get_exp_parm = sc132gs_sensor_get_exp_parm,
        .sensor_set_exp_parm = sc132gs_sensor_set_exp_parm,
        .sensor_get_fps = sc132gs_sensor_get_fps,
        .sensor_set_fps = sc132gs_sensor_set_fps,
        .sensor_get_isp_status = sc132gs_sensor_get_isp_status,
        .sensor_set_blc = sc132gs_sensor_set_blc,
        .sensor_set_wb = sc132gs_sensor_set_wb,
        .sensor_get_tpg = sc132gs_sensor_get_tpg,
        .sensor_set_tpg = sc132gs_sensor_set_tpg,
        .sensor_get_expand_curve = sc132gs_sensor_get_expand_curve,
        .sensor_get_otp_data = sc132gs_sensor_get_otp_data,
        .sensor_mirror_set = sc132gs_sensor_mirror_set,
    },
};
