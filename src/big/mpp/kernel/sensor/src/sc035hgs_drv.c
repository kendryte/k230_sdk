
#include "sensor_dev.h"
#include "io.h"
#include <ioremap.h>
#include "drv_gpio.h"

#define pr_info(...) /*rt_kprintf(__VA_ARGS__)*/
#define pr_debug(...) /*rt_kprintf(__VA_ARGS__)*/
#define pr_warn(...) /*rt_kprintf(__VA_ARGS__)*/
#define pr_err(...) rt_kprintf(__VA_ARGS__)

#define CHIP_ID 0x310b
#define SC035HGS_REG_CHIP_ID_H 0x3107
#define SC035HGS_REG_CHIP_ID_L 0x3108

#define SC035HGS_LANES 1
#define SC035HGS_BITS_PER_SAMPLE 10
#define SC035HGS_LINK_FREQ_270MHZ 270000000
#define SC035HGS_PIXEL_RATE \
    (SC035HGS_LINK_FREQ_270MHZ * 2 * SC035HGS_LANES / SC035HGS_BITS_PER_SAMPLE)
#define SC035HGS_XVCLK_FREQ 24000000

#define SC035HGS_REG_CTRL_MODE 0x0100
#define SC035HGS_MODE_SW_STANDBY 0x0
#define SC035HGS_MODE_STREAMING BIT(0)

#define SC035HGS_REG_EXPOSURE 0x3e01
#define SC035HGS_EXPOSURE_MIN 6
#define SC035HGS_EXPOSURE_STEP 1
#define SC035HGS_REG_VTS 0x320e
#define SC035HGS_REG_HTS 0x320f
#define SC035HGS_VTS_MAX 0xffff

#define SC035HGS_REG_COARSE_DGAIN 0x3e06
#define SC035HGS_REG_FINE_DGAIN 0x3e07
#define SC035HGS_REG_COARSE_AGAIN 0x3e08
#define SC035HGS_REG_FINE_AGAIN 0x3e09
#define ANALOG_GAIN_MIN 0x10
#define ANALOG_GAIN_MAX 0x7c0
#define ANALOG_GAIN_STEP 1
#define ANALOG_GAIN_DEFAULT 0x10

#define SC035HGS_REG_TEST_PATTERN 0x4501
#define SC035HGS_TEST_PATTERN_ENABLE 0xcc
#define SC035HGS_TEST_PATTERN_DISABLE 0xc4

#define SC035HGS_REG_FLIP_MIRROR 0x3221
#define SC035HGS_MIRROR_MASK 0x06
#define SC035HGS_FLIP_MASK 0x60

#define SC035HGS_GROUP_HOLD 0x3812
#define SC035HGS_GROUP_HOLD_START 0X00
#define SC035HGS_GROUP_HOLD_LUNCH 0x30

#define SC035HGS_REG_VALUE_08BIT 1
#define SC035HGS_REG_VALUE_16BIT 2
#define SC035HGS_REG_VALUE_24BIT 3

#define SC035HGS_NAME "sc035hgs"
#if 0
static k_sensor_reg sc035hgs_mipi_1lane_raw10_640x480_120fps_regs[] = {
    { 0x0103, 0x01 },
    { 0x0100, 0x00 },
    { 0x36e9, 0x80 },
    { 0x36f9, 0x80 },
    { 0x3000, 0x00 },
    { 0x3001, 0x00 },
    { 0x300f, 0x0f },
    { 0x3018, 0x13 },
    { 0x3019, 0xfc },
    { 0x301c, 0x78 },
    { 0x301f, 0x87 },
    { 0x3031, 0x0a },
    { 0x3037, 0x20 },
    { 0x303f, 0x01 },
    { 0x320c, 0x03 }, // Line Length
    { 0x320d, 0x54 },
    { 0x320e, 0x04 }, // Frame Length
    { 0x320f, 0x20 },
    { 0x3217, 0x00 },
    { 0x3218, 0x00 },
    { 0x3220, 0x10 },
    { 0x3223, 0x48 },
    { 0x3226, 0x74 },
    { 0x3227, 0x07 },
    { 0x323b, 0x00 },
    { 0x3250, 0xf0 },
    { 0x3251, 0x02 },
    { 0x3252, 0x02 },
    { 0x3253, 0x08 },
    { 0x3254, 0x02 },
    { 0x3255, 0x07 },
    { 0x3304, 0x48 },
    { 0x3305, 0x00 },
    { 0x3306, 0x60 },
    { 0x3309, 0x50 },
    { 0x330a, 0x00 },
    { 0x330b, 0xc0 },
    { 0x330c, 0x18 },
    { 0x330f, 0x40 },
    { 0x3310, 0x10 },
    { 0x3314, 0x1e },
    { 0x3315, 0x30 },
    { 0x3316, 0x68 },
    { 0x3317, 0x1b },
    { 0x3329, 0x5c },
    { 0x332d, 0x5c },
    { 0x332f, 0x60 },
    { 0x3335, 0x64 },
    { 0x3344, 0x64 },
    { 0x335b, 0x80 },
    { 0x335f, 0x80 },
    { 0x3366, 0x06 },
    { 0x3385, 0x31 },
    { 0x3387, 0x39 },
    { 0x3389, 0x01 },
    { 0x33b1, 0x03 },
    { 0x33b2, 0x06 },
    { 0x33bd, 0xe0 },
    { 0x33bf, 0x10 },
    { 0x3621, 0xa4 },
    { 0x3622, 0x05 },
    { 0x3624, 0x47 },
    { 0x3630, 0x4a },
    { 0x3631, 0x58 },
    { 0x3633, 0x52 },
    { 0x3635, 0x03 },
    { 0x3636, 0x25 },
    { 0x3637, 0x8a },
    { 0x3638, 0x0f },
    { 0x3639, 0x08 },
    { 0x363a, 0x00 },
    { 0x363b, 0x48 },
    { 0x363c, 0x86 },
    { 0x363d, 0x10 },
    { 0x363e, 0xf8 },
    { 0x3640, 0x00 },
    { 0x3641, 0x01 },
    { 0x36ea, 0x37 },
    { 0x36eb, 0x0e },
    { 0x36ec, 0x0e },
    { 0x36ed, 0x23 },
    { 0x36fa, 0x37 },
    { 0x36fb, 0x00 },
    { 0x36fc, 0x02 },
    { 0x36fd, 0x03 },
    { 0x3908, 0x91 },
    { 0x391b, 0x81 },
    { 0x3d08, 0x01 },
    { 0x3e01, 0x14 }, // ET[7:0]
    { 0x3e02, 0x80 }, // ET[7:4]
    { 0x3e03, 0x2b },
    { 0x3e06, 0x0c },
    { 0x3e08, 0x00 }, // Coarse gain[4:2]
    { 0x3e09, 0x10 }, // Fine gain [7:0]
    { 0x3f04, 0x03 },
    { 0x3f05, 0x34 },
    { 0x4500, 0x59 },
    { 0x4501, 0xc4 },
    { 0x4603, 0x00 },
    { 0x4800, 0x64 },
    { 0x4809, 0x01 },
    { 0x4810, 0x00 },
    { 0x4811, 0x01 },
    { 0x4837, 0x13 },
    { 0x5011, 0x00 },
    { 0x5988, 0x02 },
    { 0x598e, 0x03 },
    { 0x598f, 0x10 },
    { 0x36e9, 0x24 },
    { 0x36f9, 0x20 },
    { REG_NULL, 0x00 }
};
#else
static k_sensor_reg sc035hgs_mipi_1lane_raw10_640x480_120fps_regs[] =
{
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    {0x3000, 0x00},
    {0x3001, 0x00},
    {0x300f, 0x0f},
    {0x3018, 0x13},
    {0x3019, 0xfc},
    {0x301c, 0x78},
    {0x301f, 0x87},
    {0x3031, 0x0a},
    {0x3037, 0x20},
    {0x303f, 0x01},
    {0x320c, 0x03}, //Line Length
    {0x320d, 0x54},
    {0x320e, 0x04}, //Frame Length
    {0x320f, 0x20},
    {0x3217, 0x00},
    {0x3218, 0x00},
    {0x3220, 0x10},
    {0x3223, 0x48},
    {0x3226, 0x74},
    {0x3227, 0x07},
    {0x323b, 0x00},
    {0x3250, 0xf0},
    {0x3251, 0x02},
    {0x3252, 0x02},
    {0x3253, 0x08},
    {0x3254, 0x02},
    {0x3255, 0x07},
    {0x3304, 0x48},
    {0x3305, 0x00},
    {0x3306, 0x60},
    {0x3309, 0x50},
    {0x330a, 0x00},
    {0x330b, 0xc0},
    {0x330c, 0x18},
    {0x330f, 0x40},
    {0x3310, 0x10},
    {0x3314, 0x1e},
    {0x3315, 0x30},
    {0x3316, 0x68},
    {0x3317, 0x1b},
    {0x3329, 0x5c},
    {0x332d, 0x5c},
    {0x332f, 0x60},
    {0x3335, 0x64},
    {0x3344, 0x64},
    {0x335b, 0x80},
    {0x335f, 0x80},
    {0x3366, 0x06},
    {0x3385, 0x31},
    {0x3387, 0x39},
    {0x3389, 0x01},
    {0x33b1, 0x03},
    {0x33b2, 0x06},
    {0x33bd, 0xe0},
    {0x33bf, 0x10},
    {0x3621, 0xa4},
    {0x3622, 0x05},
    {0x3624, 0x47},
    {0x3630, 0x4a},
    {0x3631, 0x58},
    {0x3633, 0x52},
    {0x3635, 0x03},
    {0x3636, 0x25},
    {0x3637, 0x8a},
    {0x3638, 0x0f},
    {0x3639, 0x08},
    {0x363a, 0x00},
    {0x363b, 0x48},
    {0x363c, 0x86},
    {0x363d, 0x10},
    {0x363e, 0xf8},
    {0x3640, 0x00},
    {0x3641, 0x01},
    {0x36ea, 0x37},
    {0x36eb, 0x0e},
    {0x36ec, 0x0e},
    {0x36ed, 0x23},
    {0x36fa, 0x37},
    {0x36fb, 0x00},
    {0x36fc, 0x02},
    {0x36fd, 0x03},
    {0x3908, 0x91},
    {0x391b, 0x81},
    {0x3d08, 0x01},
    {0x3e01, 0x14},//ET[7:0]
    {0x3e02, 0x80},//ET[7:4]
    {0x3e03, 0x2b},
    {0x3e06, 0x0c},
    {0x3e08, 0x00},//Coarse gain[4:2]
    {0x3e09, 0x10},//Fine gain [7:0]
    {0x3f04, 0x03},
    {0x3f05, 0x34},
    {0x4500, 0x59},
    {0x4501, 0xc4},
    {0x4603, 0x00},
    {0x4800, 0x64},
    {0x4809, 0x01},
    {0x4810, 0x00},
    {0x4811, 0x01},
    {0x4837, 0x13},
    {0x5011, 0x00},
    {0x5988, 0x02},
    {0x598e, 0x03},
    {0x598f, 0x10},
    {0x36e9, 0x24},
    {0x36f9, 0x20},
    {0x320e, 0x04},
    {0x320f, 0x20},
    // {0x0100, 0x01},
    {0x4418, 0x0a},
    {0x4419, 0x80},
};
#endif
static const k_sensor_reg sc035hgs_mipi_1lane_raw10_640x480_60fps_regs[] = {
    { 0x320c, 0x03 }, // Line Length
    { 0x320d, 0x54 },
    { 0x320e, 0x08 }, // Frame Length
    { 0x320f, 0x40 },
    { REG_NULL, 0x00 }
};

static const k_sensor_reg sc035hgs_mipi_1lane_raw10_640x480_30fps_regs[] = {
    { 0x320c, 0x03 }, // Line Length
    { 0x320d, 0x54 },
    { 0x320e, 0x10 }, // Frame Length
    { 0x320f, 0x80 },
    { REG_NULL, 0x00 }
};

static k_sensor_clk_info sc035hgs_clk_info[] = {
    {
        .mclk = 24000000,
        .pclk = 108000000,
    },
};

// static k_sensor_mipi_info sc035hgs_mipi_info[] = {
//     {
//         .csi_id = 2,
//         .mipi_lanes = 1,
//         .data_type = 0x2B, //RAW10
//     },
// };

static k_sensor_mode sc035hgs_mode_info[] = {
    {
        .index = 0,
        .sensor_type = SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR,
        .size = {
            .bounds_width = 640,
            .bounds_height = 480,
            .top = 0,
            .left = 0,
            .width = 640,
            .height = 480,
        },
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bayer_pattern = BAYER_PAT_BGGR,
        .bit_width = 10,
        .fps = 120000,
        .clk_info = sc035hgs_clk_info,
        .mipi_info = {
            .csi_id = 2,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = sc035hgs_mipi_1lane_raw10_640x480_120fps_regs,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 1,
        .sensor_type = SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR,
        .size = {
            .bounds_width = 640,
            .bounds_height = 480,
            .top = 0,
            .left = 0,
            .width = 640,
            .height = 480,
        },
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bayer_pattern = BAYER_PAT_BGGR,
        .bit_width = 10,
        .fps = 60000,
        .clk_info = sc035hgs_clk_info,
        .mipi_info = {
            .csi_id = 2,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = sc035hgs_mipi_1lane_raw10_640x480_60fps_regs,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 2,
        .sensor_type = SC_SC035HGS_MIPI_1LANE_RAW10_640X480_30FPS_LINEAR,
        .size = {
            .bounds_width = 640,
            .bounds_height = 480,
            .top = 0,
            .left = 0,
            .width = 640,
            .height = 480,
        },
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bayer_pattern = BAYER_PAT_BGGR,
        .bit_width = 10,
        .fps = 30000,
        .clk_info = sc035hgs_clk_info,
        .mipi_info = {
            .csi_id = 2,
            .mipi_lanes = 1,
            .data_type = 0x2B, //RAW10
        },
        .reg_list = sc035hgs_mipi_1lane_raw10_640x480_30fps_regs,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
};

static k_sensor_mode* current_mode = NULL;

static int sc035hgs_power_reset(k_s32 on)
{
    #define SC035HGS_RST_PIN (43)

    kd_pin_mode(SC035HGS_RST_PIN, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(SC035HGS_RST_PIN, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(SC035HGS_RST_PIN, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(SC035HGS_RST_PIN, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(SC035HGS_RST_PIN, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int sc035hgs_i2c_init(k_sensor_i2c_info* i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL) {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }
    return 0;
}

static k_s32 sc035hgs_sensor_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev* dev = ctx;
    pr_info("%s enter\n", __func__);

    sc035hgs_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, SC035HGS_REG_CHIP_ID_L, &id_low);
    if (ret) {
        // rt_kprintf("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}

static k_s32 sc035hgs_sensor_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        sc035hgs_power_reset(on);
        sc035hgs_i2c_init(&dev->i2c_info);
        ret = sc035hgs_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        sc035hgs_power_reset(on);
    }

    return ret;
}

static k_s32 sc035hgs_sensor_init(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    k_u16 reg_msb = 0;
    k_u16 reg_lsb = 0;
    struct sensor_driver_dev* dev = ctx;
    void *vi_addr = rt_ioremap((void *)0x90009000, 0x10000);

    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(sc035hgs_mode_info) / sizeof(k_sensor_mode); i++) {
            if (sc035hgs_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(sc035hgs_mode_info[i]);
                dev->sensor_mode = &(sc035hgs_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    rt_kprintf("%s enter, sensor mode %d -- max fps :%d\n", __func__, mode.index, mode.fps);
    ret = sensor_reg_list_write(&dev->i2c_info, sc035hgs_mipi_1lane_raw10_640x480_120fps_regs);

#define VI_CSI2_DIS_FRAME_M                     (0x8C)
#define VI_CSI2_DIS_FRAME_N                     (0x90)
#define VI_DIS_FRAME_EN                         (0x94)

    switch (current_mode->index) {
    case 0:
        /* todo fix, sc035hgs 120fps to 30fps */
        writel(1, vi_addr + VI_CSI2_DIS_FRAME_M);
        writel(3, vi_addr + VI_CSI2_DIS_FRAME_N);
        writel(4, vi_addr + VI_DIS_FRAME_EN);
        break;

    case 1:
        ret = sensor_reg_list_write(&dev->i2c_info, sc035hgs_mipi_1lane_raw10_640x480_60fps_regs);
        /* todo fix, sc035hgs 60fps to 60fps */
        writel(1, vi_addr + VI_CSI2_DIS_FRAME_M);
        writel(1, vi_addr + VI_CSI2_DIS_FRAME_N);
        writel(4, vi_addr + VI_DIS_FRAME_EN);

        break;

    case 2:
        ret = sensor_reg_list_write(&dev->i2c_info, sc035hgs_mipi_1lane_raw10_640x480_30fps_regs);
        break;

    default:
        return -1;
    }


    switch (current_mode->index) {
    case 0:

    case 1:

    case 2:

        ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_VTS, &reg_msb);
        ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_VTS, &reg_lsb);
        current_mode->ae_info.frame_length = reg_msb << 8 | reg_lsb;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;

        //exp
        ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_HTS, &reg_msb);
        ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_HTS, &reg_lsb);
        current_mode->ae_info.one_line_exp_time = (reg_msb << 8 | reg_lsb) * 1000.0f / current_mode->clk_info->pclk  ; // ms
        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length - 6;
        current_mode->ae_info.min_long_integraion_line = 2;
        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 6;
        current_mode->ae_info.min_integraion_line = 2;
        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length -6;
        current_mode->ae_info.min_vs_integraion_line = 2;

        current_mode->ae_info.max_long_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_long_integraion_line;
        current_mode->ae_info.min_long_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_long_integraion_line;
        current_mode->ae_info.max_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_integraion_line;
        current_mode->ae_info.min_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_integraion_line;
        current_mode->ae_info.max_vs_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.max_vs_integraion_line;
        current_mode->ae_info.min_vs_integraion_time = current_mode->ae_info.integration_time_increment * current_mode->ae_info.min_vs_integraion_line;

        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;

        //gain
        current_mode->ae_info.gain_accuracy = 1024;
        current_mode->ae_info.gain_increment = (1.0f / 16.0f);
        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 120.125;

        current_mode->ae_info.int_time_delay_frame = 2;
        current_mode->ae_info.gain_delay_frame = 2;
        //current_mode->ae_info.ae_min_interval_frame = 0.8;
        current_mode->ae_info.color_type = SENSOR_MONO;	//mono sensor

        current_mode->ae_info.cur_long_again = 0.0;
        current_mode->ae_info.cur_long_dgain = 0.0;
        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;
        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        current_mode->ae_info.a_long_gain.max = 1.0;
        current_mode->ae_info.a_long_gain.min = 63.9375;
        current_mode->ae_info.a_long_gain.step = (1.0f / 16.0f);
        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 63.9375;
        current_mode->ae_info.a_gain.step = (1.0f / 16.0f);
        current_mode->ae_info.a_vs_gain.max = 1.0;
        current_mode->ae_info.a_vs_gain.min = 63.9375;
        current_mode->ae_info.a_vs_gain.step = (1.0f / 16.0f);

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


    default:
        break;
    }

    k_u16 exp_time;
    float again = 0, dgain = 0;
    again = 1.0;
    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    // ret = sensor_reg_read(&dev->i2c_info, OV9732_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = 1;

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;

    return ret;
}

static k_s32 sc035hgs_sensor_get_mode(void* ctx, k_sensor_mode* mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(sc035hgs_mode_info) / sizeof(k_sensor_mode); i++) {
        if (sc035hgs_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &sc035hgs_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(sc035hgs_mode_info[i]);
            return 0;
        }
    }
    pr_err("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_set_mode(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_enum_mode(void* ctx, k_sensor_enum_mode* enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    if (enum_mode->index >= (sizeof(sc035hgs_mode_info) / sizeof(k_sensor_mode))) {
        pr_err("%s, invalid mode index.\n", __func__);
        return -1;
    }

    for (k_s32 i = 0; i < sizeof(sc035hgs_mode_info) / sizeof(k_sensor_mode); i++) {
        if (sc035hgs_mode_info[i].index == enum_mode->index) {
            memcpy(&enum_mode->mode, &sc035hgs_mode_info[i], sizeof(k_sensor_mode));
            return 0;
        }
    }
    return ret;
}

static k_s32 sc035hgs_sensor_get_caps(void* ctx, k_sensor_caps* caps)
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

static k_s32 sc035hgs_sensor_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 sc035hgs_sensor_set_stream(void* ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;

    rt_kprintf("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x01);
    } else {
        ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x00);
    }
    rt_kprintf("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 sc035hgs_sensor_get_again(void* ctx, k_sensor_gain* gain)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    // if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
    //     gain->gain[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_again;
    // } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    //     gain->gain[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_again;
    //     gain->gain[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_vs_again;
    // } else {
    //     pr_err("%s, unsupport exposure frame.\n", __func__);
    //     return -1;
    // }

    return ret;
}

static k_s32 sc035hgs_sensor_set_again(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    // k_u16 again;
    // k_u16 coarse;
    // k_u16 fine;
    // struct sensor_driver_dev* dev = ctx;

    // if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
    //     again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16);
    //     ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_COARSE_AGAIN, &coarse);
    //     coarse &= ~(0x07 << 2); // clear [4:2] bits for coarse gain
	// 	if(again > 15.5 * 16) {
    //         again = 15.5 * 16;
	// 		coarse |= 0x07 << 2;
	// 		fine = 0x1f;
	// 	} else if (again >= 8 << 4) {
	// 		coarse |= 0x07 << 2;
	// 		fine = again >> 3;
	// 	} else if (again >= 4 << 4) {
	// 		coarse |= 0x03 << 2;
	// 		fine = again >> 2;
	// 	} else if (again >= 2 << 4) {
	// 		coarse |= 0x01 << 2;
	// 		fine = again >> 1;
	// 	} else if (again >= 1 << 4) {
	// 		coarse |= 0x00 << 2;
	// 		fine = again;
	// 	} else {
    //         again = 1.0 * 16;
	// 		coarse |= 0x00 << 2;
	// 		fine = 0x10;
    //     }
    //     ret = sensor_reg_write(&dev->i2c_info, SC035HGS_REG_COARSE_AGAIN, coarse);
    //     ret |= sensor_reg_write(&dev->i2c_info, SC035HGS_REG_FINE_AGAIN, fine);
    //     current_mode->ae_info.cur_again = (float)again / 16.0f;
    // } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    //     again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 16);
    //     // ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H, (again & 0x0300) >> 8);
    //     // ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L, (again & 0xff));
    //     current_mode->ae_info.cur_again = (float)again / 16.0f;

    //     again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 16);
    //     // TODO
    //     current_mode->ae_info.cur_vs_again = (float)again / 16.0f;
    // } else {
    //     pr_err("%s, unsupport exposure frame.\n", __func__);
    //     return -1;
    // }
    // pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again * 1000));

    return ret;
}

static k_s32 sc035hgs_sensor_get_dgain(void* ctx, k_sensor_gain* gain)
{
    k_s32 ret = 0;

    // pr_info("%s enter\n", __func__);

    // if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
    //     gain->gain[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_dgain;
    // } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    //     gain->gain[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_dgain;
    //     gain->gain[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_vs_dgain;
    // } else {
    //     pr_err("%s, unsupport exposure frame.\n", __func__);
    //     return -1;
    // }

    return ret;
}

static k_s32 sc035hgs_sensor_set_dgain(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    // k_u32 dgain;
    // k_u16 coarse;
    // k_u16 fine;
    // struct sensor_driver_dev* dev = ctx;

    // pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    // if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
    //     dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 128);
    //     ret = sensor_reg_read(&dev->i2c_info, SC035HGS_REG_COARSE_DGAIN, &coarse);
    //     coarse &= ~0x03; // clear [1:0] bits for coarse gain
	// 	if(dgain > 7.96875 * 128) {
    //         dgain = 7.96875 * 128;
	// 		coarse |= 0x03;
	// 		fine = 0xff;
	// 	} else if (dgain >= 4 * 128) {
	// 		coarse |= 0x03;
	// 		fine = dgain >> 2;
	// 	} else if (dgain >= 2 * 128) {
	// 		coarse |= 0x01;
	// 		fine = dgain >> 1;
	// 	} else if (dgain >= 1 * 128) {
	// 		coarse |= 0x00;
	// 		fine = dgain;
	// 	} else {
    //         dgain = 128;
	// 		coarse |= 0x00;
	// 		fine = 0x80;
    //     }
    //     ret = sensor_reg_write(&dev->i2c_info, SC035HGS_REG_COARSE_DGAIN, coarse);
    //     ret |= sensor_reg_write(&dev->i2c_info, SC035HGS_REG_FINE_DGAIN, fine);
    //     current_mode->ae_info.cur_dgain = (float)dgain / 128.0f;

    // } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    //     dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
    //     // ret = sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
    //     // ret |= sensor_reg_write(&dev->i2c_info, OV9732_REG_LONG_AGAIN_L,(again & 0xff));
    //     current_mode->ae_info.cur_dgain = (float)dgain / 1024.0f;

    //     dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 1024);
    //     // TODO wirte vs gain register
    //     current_mode->ae_info.cur_vs_dgain = (float)dgain / 1024.0f;
    // } else {
    //     pr_err("%s, unsupport exposure frame.\n", __func__);
    //     return -1;
    // }
    // current_mode->ae_info.cur_gain = current_mode->ae_info.cur_again * current_mode->ae_info.cur_dgain;
    // pr_debug("%s,cur_gain(%d)\n", __func__, (k_u32)(current_mode->ae_info.cur_gain * 10000));

    return ret;
}

static k_s32 sc035hgs_sensor_get_intg_time(void* ctx, k_sensor_intg_time* time)
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

static k_s32 sc035hgs_sensor_set_intg_time(void* ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];

        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));

        // ret |= sensor_reg_write(&dev->i2c_info, 0x3501, (exp_line >> 4) & 0xff);
        // ret |= sensor_reg_write(&dev->i2c_info, 0x3502, (exp_line << 4) & 0xff);

        current_mode->ae_info.cur_integration_time = (float)exp_line * current_mode->ae_info.one_line_exp_time;
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
    pr_debug("%s hdr_mode(%d), exp_line(%d), integraion_time(%u)\n",
        __func__, current_mode->hdr_mode, exp_line, (k_u32)(integraion_time * 1000000000));

    return ret;
}

static k_s32 sc035hgs_sensor_get_exp_parm(void* ctx, k_sensor_exposure_param* exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 sc035hgs_sensor_set_exp_parm(void* ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_get_fps(void* ctx, k_u32* fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = current_mode->fps;

    return ret;
}

static k_s32 sc035hgs_sensor_set_fps(void* ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_get_isp_status(void* ctx, k_sensor_isp_status* staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 sc035hgs_sensor_set_blc(void* ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_set_wb(void* ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_get_tpg(void* ctx, k_sensor_test_pattern* tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 sc035hgs_sensor_set_tpg(void* ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 sc035hgs_sensor_get_expand_curve(void* ctx, k_sensor_compand_curve* curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 sc035hgs_sensor_get_otp_data(void* ctx, void* data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void*));

    return ret;
}


static k_s32 sc035hgs_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev sc035hgs_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c3",
        .slave_addr = 0x30,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = SC035HGS_NAME,
    .sensor_func = {
        .sensor_power = sc035hgs_sensor_power_on,
        .sensor_init = sc035hgs_sensor_init,
        .sensor_get_chip_id = sc035hgs_sensor_get_chip_id,
        .sensor_get_mode = sc035hgs_sensor_get_mode,
        .sensor_set_mode = sc035hgs_sensor_set_mode,
        .sensor_enum_mode = sc035hgs_sensor_enum_mode,
        .sensor_get_caps = sc035hgs_sensor_get_caps,
        .sensor_conn_check = sc035hgs_sensor_conn_check,
        .sensor_set_stream = sc035hgs_sensor_set_stream,
        .sensor_get_again = sc035hgs_sensor_get_again,
        .sensor_set_again = sc035hgs_sensor_set_again,
        .sensor_get_dgain = sc035hgs_sensor_get_dgain,
        .sensor_set_dgain = sc035hgs_sensor_set_dgain,
        .sensor_get_intg_time = sc035hgs_sensor_get_intg_time,
        .sensor_set_intg_time = sc035hgs_sensor_set_intg_time,
        .sensor_get_exp_parm = sc035hgs_sensor_get_exp_parm,
        .sensor_set_exp_parm = sc035hgs_sensor_set_exp_parm,
        .sensor_get_fps = sc035hgs_sensor_get_fps,
        .sensor_set_fps = sc035hgs_sensor_set_fps,
        .sensor_get_isp_status = sc035hgs_sensor_get_isp_status,
        .sensor_set_blc = sc035hgs_sensor_set_blc,
        .sensor_set_wb = sc035hgs_sensor_set_wb,
        .sensor_get_tpg = sc035hgs_sensor_get_tpg,
        .sensor_set_tpg = sc035hgs_sensor_set_tpg,
        .sensor_get_expand_curve = sc035hgs_sensor_get_expand_curve,
        .sensor_get_otp_data = sc035hgs_sensor_get_otp_data,
        .sensor_mirror_set = sc035hgs_sensor_mirror_set,
    },
};
