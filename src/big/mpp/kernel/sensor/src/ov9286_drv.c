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

// #include "gpio.h"
#include "drv_gpio.h"

#include "k_board_config_comm.h"

#include "k_vicap_comm.h"

#define pr_info(...) //rt_kprintf(__VA_ARGS__)
#define pr_debug(...) //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)    rt_kprintf(__VA_ARGS__)

#define OV9286_REG_CHIP_ID_H    0x300a
#define OV9286_REG_CHIP_ID_L    0x300b

#define OV9286_REG_LONG_EXP_TIME_H    0x3501
#define OV9286_REG_LONG_EXP_TIME_L    0x3502

#define OV9286_REG_LONG_AGAIN	0x3509
//#define OV9286_REG_LONG_AGAIN_H    0x350a
//#define OV9286_REG_LONG_AGAIN_L    0x350b

#define OV9286_MIN_GAIN_STEP    (1.0f/16.0f)

static const k_sensor_reg ov9286_mipi2lane_720p_60fps_linear[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x3006, 0x06},
    {0x300f, 0xf4},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x302c, 0x00},
    {0x302f, 0x05},
    {0x3030, 0x10},
    {0x303f, 0x03},
    {0x31ff, 0x01},
    {0x3210, 0x04},
    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03},
    {0x3508, 0x00},
    {0x3509, 0x10},	//gain
    {0x350d, 0x07},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3610, 0x80},
    {0x3611, 0xb8},
    {0x3612, 0x02},
    {0x3614, 0x80},
    {0x3620, 0x6e},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x15},
    {0x3666, 0x70},
    {0x3670, 0x68},
    {0x3673, 0xe4},
    {0x3678, 0x00},
    {0x367e, 0x90},
    {0x3680, 0x84},
    {0x3707, 0x6c},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x90},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x44},
    {0x3799, 0x20},
    {0x379b, 0x01},
    {0x379c, 0x10},
    {0x37a8, 0x42},
    {0x37aa, 0x52},
    {0x3803, 0xc8},	//y start: 200
    {0x3804, 0x05},
    {0x3805, 0x3f},	//x end: 1344 = 32 + 1280 +32
    {0x3806, 0x03},
    {0x3807, 0x9f},	//y end: 0x3a7 = 936, 0x39f = 928
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x02},
    {0x380b, 0xd0},
    {0x380c, 0x02},	//HTS = 0x2b8 = 696
    {0x380d, 0xb8},
    {0x380e, 0x07},	//VTS = 0x77b = 1915
    {0x380f, 0x7b},
    {0x3810, 0x00},
    {0x3811, 0x20},
    {0x3816, 0x00},
    {0x3817, 0x01},
    {0x3818, 0x00},
    {0x3819, 0x05},
    {0x382b, 0x3a},
    {0x382c, 0x09},
    {0x382d, 0x9a},
    {0x3880, 0x10},
    {0x3881, 0x42},
    {0x3882, 0x01},
    {0x3883, 0xcc},
    {0x3885, 0x07},
    {0x389d, 0x03},
    {0x38a6, 0x00},
    {0x38a7, 0x01},
    {0x38b1, 0x00},
    {0x38b3, 0x07},
    {0x38e5, 0x02},
    {0x38e7, 0x00},

    {0x4003, 0x10},	//black level target
    {0x4008, 0x02},
    {0x4009, 0x09},
    {0x400a, 0x01},
    {0x400b, 0x30},
    {0x400c, 0x02},
    {0x400d, 0x09},
    {0x4010, 0xf0},
    {0x4016, 0x00},
    {0x4012, 0x08},
    {0x4017, 0x10},
    {0x4042, 0xd3},
    {0x4043, 0x60},
    {0x4045, 0x20},
    {0x404b, 0x20},
    {0x4307, 0x88},
    {0x4507, 0x40},
    {0x450b, 0x80},
    {0x450f, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x01},
    {0x4603, 0x01},
    {0x4800, 0x60},
    {0x481f, 0x30},
    {0x4825, 0x35},
    {0x4837, 0x14},
    {0x4f00, 0x00},
    {0x4f07, 0x00},
    {0x4f08, 0x03},
    {0x4f09, 0x08},
    {0x4f0c, 0x04},
    {0x4f0d, 0xe4},
    {0x4f10, 0x00},
    {0x4f11, 0x00},
    {0x4f12, 0x0f},
    {0x4f13, 0xc4},
    {0x4f07, 0x00},
    {0x3880, 0x00},
    {0x4837, 0x1b},
    {0x3006, 0xea},
    {0x3210, 0x10},
    {0x3007, 0x02},
    {0x301c, 0x22},
    {0x3020, 0x20},
    {0x3025, 0x02},
    {0x382c, 0x07},
    {0x382d, 0xc0},
    {0x3920, 0xff},
    {0x3923, 0x00},
    {0x3924, 0x00},
    {0x3925, 0x00},
    {0x3928, 0x80},
    {0x392b, 0x00},
    {0x392c, 0x00},
    {0x392d, 0x03},
    {0x392e, 0xe0},
    {0x392f, 0xcb},
    {0x38b3, 0x07},
    {0x3885, 0x07},
    {0x382b, 0x5a},
    {0x3670, 0x68},
    {0x3006, 0xea},
    {0x3501, 0x11},	//ET = 287, 2.5ms
    {0x3502, 0xf0},	//
    {0x3927, 0x00},	//strobe width
    {0x3928, 0x73},
    {0x3929, 0x07},	//strobe start point
    {0x392a, 0x08},	// -7 : 01
    {0x3208, 0x10},
    {0x3208, 0xa0},
    {REG_NULL, 0x00},
};

static const k_sensor_reg ov9286_mipi2lane_720p_30fps_linear[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x0300, 0x04},	//MIPI clock: 500Mbps, PLL1: 50MHz
    {0x0302, 0x7d},
    {0x030a, 0x01},
    {0x030b, 0x02},	//PLL2(PCLK): 50MHz
    {0x030d, 0x32},
    {0x030e, 0x06},
    {0x030f, 0x02},
    {0x0312, 0x0b},
    {0x0313, 0x02},
    {0x3006, 0x06},
    {0x300f, 0xf4},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x302c, 0x00},
    {0x302d, 0x0d},
    {0x302e, 0x6b},
    {0x302f, 0x2d},
    {0x3030, 0x10},
    {0x303f, 0x03},
    {0x31ff, 0x01},
    {0x3210, 0x04},
    {0x3500, 0x00},
    {0x3501, 0x6f},     //ET = 0x6f0
    {0x3502, 0x00},     //
    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03},
    {0x3508, 0x00},
    {0x3509, 0x10},	//sensor analog gain, 1x
    {0x350d, 0x07},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3610, 0x80},
    {0x3611, 0xb8},
    {0x3612, 0x02},
    {0x3614, 0x80},
    {0x3620, 0x6e},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x15},
    {0x3666, 0x70},
    {0x3670, 0x68},
    {0x3673, 0xe4},
    {0x3678, 0x00},
    {0x367e, 0x90},
    {0x3680, 0x84},
    {0x3707, 0x6c},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x90},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x44},
    {0x3799, 0x20},
    {0x379b, 0x01},
    {0x379c, 0x10},
    {0x37a8, 0x42},
    {0x37aa, 0x52},
    {0x37ab, 0x3c},
    {0x3802, 0x00},
    {0x3803, 0xc8},
    {0x3806, 0x03},
    {0x3807, 0xa7},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x02},
    {0x380b, 0xd0},
    {0x380c, 0x02},	//HTS = 760
    {0x380d, 0xf8},
    {0x380e, 0x08},	//VTS = 2193
    {0x380f, 0x91},
    {0x3810, 0x00},
    {0x3811, 0x20},
    {0x3817, 0x01},
    {0x3818, 0x00},
    {0x3819, 0x05},
    {0x382b, 0x3a},
    {0x382c, 0x09},
    {0x382d, 0x9a},
    {0x3880, 0x10},
    {0x3881, 0x42},
    {0x3882, 0x01},
    {0x3883, 0xcc},
    {0x3885, 0x07},
    {0x389d, 0x03},
    {0x38a6, 0x00},
    {0x38a7, 0x01},
    {0x38b1, 0x00},
    {0x38b3, 0x07},
    {0x38e5, 0x02},
    {0x38e7, 0x00},
    {0x4008, 0x02},
    {0x4009, 0x09},
    {0x400a, 0x01},
    {0x400b, 0x30},
    {0x400c, 0x02},
    {0x400d, 0x09},
    {0x4010, 0xf0},
    {0x4016, 0x00},
    {0x4012, 0x08},
    {0x4017, 0x10},
    {0x4042, 0xd3},
    {0x4043, 0x60},
    {0x4045, 0x20},
    {0x404b, 0x20},
    {0x4307, 0x88},
    {0x4507, 0x40},
    {0x450b, 0x80},
    {0x450f, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x01},
    {0x4603, 0x01},
    {0x4800, 0x60},
    {0x481f, 0x30},
    {0x4825, 0x35},
    {0x4837, 0x35},
    {0x4f00, 0x00},
    {0x4f07, 0x00},
    {0x4f08, 0x03},
    {0x4f09, 0x08},
    {0x4f0c, 0x03},
    {0x4f0d, 0x54},
    {0x4f10, 0x00},
    {0x4f11, 0x00},
    {0x4f12, 0x0f},
    {0x4f13, 0xc4},
    {0x4f07, 0x00},
    {0x5000, 0x9f},
    {0x5e00, 0x00},
    {0x3006, 0xea},
    {0x3210, 0x10},
    {0x3007, 0x02},
    {0x301c, 0x22},
    {0x3020, 0x20},
    {0x3025, 0x02},
    {0x382c, 0x11},
    {0x382d, 0x70},
    {0x3920, 0xff},
    {0x3923, 0x00},
    {0x3924, 0x00},
    {0x3925, 0x00},
    {0x3926, 0x00},
    {0x3928, 0x80},
    {0x392b, 0x00},
    {0x392c, 0x00},
    {0x392d, 0x08},
    {0x392e, 0xb8},
    {0x392f, 0xcb},
    {0x38b3, 0x07},
    {0x3885, 0x07},
    {0x382b, 0x5a},
    {0x3670, 0x68},
    {0x3006, 0xea},
    {0x3501, 0x08},  // ET = 0x9c = 132 ET line (2ms)
    {0x3502, 0x40},
    {0x3509, 0x10},
    {0x3927, 0x00},	//strobe width, 3ms
    {0x3928, 0xc6},
    {0x3929, 0x07},	//strobe start point
    {0x392a, 0xc4},
    {0x3208, 0x10},
    {0x3208, 0xa0},
    {REG_NULL, 0x00},
};

static const k_sensor_reg ov9286_mipi2lane_720p_30fps_mclk_25m_linear[] = {
    {0x0103, 0x01},
    {0x0100, 0x00},
    // {0x0300, 0x04},	//MIPI clock: 500Mbps, PLL1: 50MHz
    // {0x0301, 0x00},
    // {0x0302, 0x7d},
    // {0x0303, 0x00},
    // {0x030a, 0x01},
    // {0x030b, 0x02},	//PLL2(PCLK): 50MHz
    // {0x030d, 0x32},
    {0x0300, 0x00},	//MIPI clock: 500Mbps, PLL1: 50MHz
    {0x0301, 0x00},
    {0x0302, 0x14},
    {0x0303, 0x00},
    {0x030a, 0x00},
    {0x030b, 0x00},	//PLL2(PCLK): 50MHz
    {0x030d, 0x18},

    {0x030e, 0x06},
    {0x030f, 0x02},
    {0x0312, 0x0b},
    {0x0313, 0x02},
    {0x3004, 0x00},
    {0x3006, 0x06},
    {0x300f, 0xf4},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x302c, 0x00},
    {0x302d, 0x0d},
    {0x302e, 0x6b},
    {0x302f, 0x2d},
    {0x3030, 0x10},
    {0x303f, 0x03},
    {0x31ff, 0x01},
    {0x3210, 0x04},
    {0x3500, 0x00},
    {0x3501, 0x6f},     //ET = 0x6f0
    {0x3502, 0x00},     //
    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03},
    {0x3508, 0x00},
    {0x3509, 0x10},	//sensor analog gain, 1x
    {0x350d, 0x07},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3610, 0x80},
    {0x3611, 0xb8},
    {0x3612, 0x02},
    {0x3614, 0x80},
    {0x3620, 0x6e},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x15},
    {0x3666, 0x70},
    {0x3670, 0x68},
    {0x3673, 0xe4},
    {0x3678, 0x00},
    {0x367e, 0x90},
    {0x3680, 0x84},
    {0x3707, 0x6c},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x90},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x44},
    {0x3799, 0x20},
    {0x379b, 0x01},
    {0x379c, 0x10},
    {0x37a8, 0x42},
    {0x37aa, 0x52},
    {0x37ab, 0x3c},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0xc8},
    {0x3804, 0x05},
    {0x3805, 0x3f},
    {0x3806, 0x03},
    {0x3807, 0xa7},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x02},
    {0x380b, 0xd0},
    {0x380c, 0x02},	//HTS = 760
    {0x380d, 0xf8},
    {0x380e, 0x08},	//VTS = 2193
    {0x380f, 0x91},
    {0x3810, 0x00},
    {0x3811, 0x20},
    {0x3812, 0x00},
    {0x3813, 0x08},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3816, 0x00},
    {0x3817, 0x01},
    {0x3818, 0x00},
    {0x3819, 0x05},
    {0x3820, 0x40},
    {0x3821, 0x00},
    {0x382b, 0x3a},
    {0x382c, 0x09},
    {0x382d, 0x9a},
    {0x3880, 0x10},
    {0x3881, 0x42},
    {0x3882, 0x01},
    {0x3883, 0xcc},
    {0x3885, 0x07},
    {0x389d, 0x03},
    {0x38a6, 0x00},
    {0x38a7, 0x01},
    {0x38a8, 0x00},
    {0x38a9, 0xf0},
    {0x38b1, 0x00},
    {0x38b3, 0x07},
    {0x38c4, 0x01},
    {0x38c5, 0x18},
    {0x38c6, 0x02},
    {0x38c7, 0xa8},
    {0x38e5, 0x02},
    {0x38e7, 0x00},
    {0x38e8, 0x00},
    {0x38ed, 0x00},
    {0x38ee, 0x00},
    {0x38ef, 0x00},
    {0x3920, 0xa5},
    {0x3921, 0x00},
    {0x3922, 0x00},
    {0x3923, 0x00},
    {0x3924, 0x05},
    {0x3925, 0x00},
    {0x3926, 0x00},
    {0x3927, 0x00},
    {0x3928, 0x1a},
    {0x3929, 0x01},
    {0x392a, 0xb4},
    {0x392b, 0x00},
    {0x392c, 0x10},
    {0x392f, 0x40},
    {0x393e, 0x00},
    {0x393f, 0x00},
    {0x4001, 0x00},
    {0x4003, 0x10},
    {0x4008, 0x02},
    {0x4009, 0x09},
    {0x400a, 0x01},
    {0x400b, 0x30},
    {0x400c, 0x02},
    {0x400d, 0x09},
    {0x4010, 0xf0},
    {0x4016, 0x00},
    {0x4012, 0x08},
    {0x4017, 0x10},
    {0x4042, 0xd3},
    {0x4043, 0x60},
    {0x4045, 0x20},
    {0x404b, 0x20},
    {0x4307, 0x88},
    {0x4507, 0x40},
    {0x450b, 0x80},
    {0x450f, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x01},
    {0x4603, 0x01},
    {0x4800, 0x60},
    {0x481f, 0x30},
    {0x4825, 0x35},
    {0x4837, 0x35},
    {0x4f00, 0x00},
    {0x4f07, 0x00},
    {0x4f08, 0x03},
    {0x4f09, 0x08},
    {0x4f0c, 0x03},
    {0x4f0d, 0x54},
    {0x4f10, 0x00},
    {0x4f11, 0x00},
    {0x4f12, 0x0f},
    {0x4f13, 0xc4},
    {0x4f07, 0x00},
    {0x5000, 0x9f},
    {0x5e00, 0x00},
    {0x3006, 0xea},
    {0x3210, 0x10},
    {0x3007, 0x02},
    {0x301c, 0x22},
    {0x3020, 0x20},
    {0x3025, 0x02},
    {0x382c, 0x11},
    {0x382d, 0x70},
    {0x3920, 0xff},
    {0x3923, 0x00},
    {0x3924, 0x00},
    {0x3925, 0x00},
    {0x3926, 0x00},
    {0x3927, 0x00},
    {0x3928, 0x80},
    {0x392b, 0x00},
    {0x392c, 0x00},
    {0x392d, 0x08},
    {0x392e, 0xb8},
    {0x392f, 0xcb},
    {0x38b3, 0x07},
    {0x3885, 0x07},
    {0x382b, 0x5a},
    {0x3670, 0x68},
    {0x3006, 0xea},
    {0x3208, 0x00},
    // {0x3501, 0x09},  // ET = 0x9c = 156 ET line (2.371ms)
    // {0x3502, 0xc0},
    {0x3501, 0x02},  // ET = 0x9c = 156 ET line (2.371ms)
    {0x3502, 0xd0},
    {0x3509, 0x10},
    // {0x3927, 0x02}, // 20ms
    // {0x3928, 0xcf}, // 20ms
    {0x3927, 0x00}, // 20ms
    {0x3928, 0x42}, // 20ms
    // {0x3929, 0x04},
    // {0x392a, 0x9c},
    {0x3929, 0x08},
    {0x392a, 0x48},
    {0x3208, 0x10},
    {0x3208, 0xa0},
    //{0x0100, 0x01},
    {REG_NULL, 0x00},
};

static k_sensor_mode ov9286_mode_info[] = {
    {
        .index = 0,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 1,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 2,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_mclk_25m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16, //32,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 3,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_mclk_25m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16, //32,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 4,
        .sensor_type = OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR,
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
        .reg_list = ov9286_mipi2lane_720p_60fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 5,
        .sensor_type = OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_SPECKLE,
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
        .reg_list = ov9286_mipi2lane_720p_60fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 6,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 7,
        .sensor_type = OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE,
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
        .reg_list = ov9286_mipi2lane_720p_60fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
    {
        .index = 8,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_mclk_25m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16, //32,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 9,
        .sensor_type = OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR_V2,
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
        .reg_list = ov9286_mipi2lane_720p_30fps_mclk_25m_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16, //32,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
};

static k_bool ov9286_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int ov9286_power_rest(k_s32 on)
{
    // #define OV9286_RST_PIN      (49)

    kd_pin_mode(VICAP_OV9286_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_OV9286_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(VICAP_OV9286_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(VICAP_OV9286_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(VICAP_OV9286_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}


static int ov9286_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}


static k_s32 ov9286_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    ov9286_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, OV9286_REG_CHIP_ID_L, &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}


static k_s32 ov9286_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        if (!ov9286_init_flag) {
            ov9286_power_rest(on);
            ov9286_i2c_init(&dev->i2c_info);
        }
        ret = ov9286_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        ov9286_init_flag = K_FALSE;
        ov9286_power_rest(on);
    }

    return ret;
}

static k_s32 ov9286_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(ov9286_mode_info) / sizeof(k_sensor_mode); i++) {
            if (ov9286_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(ov9286_mode_info[i]);
                dev->sensor_mode = &(ov9286_mode_info[i]);
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
    case 8:
    case 9:
        if (!ov9286_init_flag) {
            ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);
        }

        current_mode->ae_info.frame_length = 2193;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.0000152;//s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 15.9375;

        current_mode->ae_info.int_time_delay_frame = 0;
        current_mode->ae_info.gain_delay_frame = 0;
        //current_mode->ae_info.ae_min_interval_frame =1.0;
        current_mode->ae_info.color_type = SENSOR_MONO;	//mono sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = OV9286_MIN_GAIN_STEP;

        current_mode->ae_info.max_integraion_line = 165;    //2.5ms //197;  // 3ms
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length;
        current_mode->ae_info.min_vs_integraion_line =     current_mode->ae_info.frame_length - 1;

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


        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;

        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 15.9375;
        current_mode->ae_info.a_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 15.9375;
        current_mode->ae_info.a_vs_gain.step = (1.0f/16.0f);//

        current_mode->ae_info.d_gain.max = 1.0;
        current_mode->ae_info.d_gain.min = 1.0;
        current_mode->ae_info.d_gain.step = (1.0f/1024.0f);

        current_mode->ae_info.d_vs_gain.max = 1.0;
        current_mode->ae_info.d_vs_gain.min = 1.0;
        current_mode->ae_info.d_vs_gain.step = (1.0f/1024.0f);//
        current_mode->ae_info.cur_fps = current_mode->fps;
        break;

    case 4:
    case 5:
    case 7:
        if (!ov9286_init_flag) {
            ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);
        }

        current_mode->ae_info.frame_length = 1915;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.0000087;//s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 15.9375;

        current_mode->ae_info.int_time_delay_frame = 0;
        current_mode->ae_info.gain_delay_frame = 0;
        //current_mode->ae_info.ae_min_interval_frame =1.2;
        current_mode->ae_info.color_type = SENSOR_MONO;	//mono sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = OV9286_MIN_GAIN_STEP;

        current_mode->ae_info.max_integraion_line = 287;	// 2.5ms
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length;
        current_mode->ae_info.min_vs_integraion_line =     current_mode->ae_info.frame_length - 1;

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

        current_mode->ae_info.cur_again = 0.0;
        current_mode->ae_info.cur_dgain = 0.0;

        current_mode->ae_info.cur_vs_again = 0.0;
        current_mode->ae_info.cur_vs_dgain = 0.0;

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 15.9375;
        current_mode->ae_info.a_gain.step = (1.0f/16.0f);

        current_mode->ae_info.a_vs_gain.min = 1.0;
        current_mode->ae_info.a_vs_gain.max = 15.9375;
        current_mode->ae_info.a_vs_gain.step = (1.0f/16.0f);//

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

    k_u16 again_h, again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_LONG_AGAIN, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_LONG_AGAIN_H, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_LONG_AGAIN_L, &again_l);
    again =  (float)again_h / 16.0f;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_LONG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, OV9286_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) | ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;
    ov9286_init_flag = K_TRUE;

    return ret;
}


k_s32 ov9286_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(ov9286_mode_info) / sizeof(k_sensor_mode); i++) {
        if (ov9286_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &ov9286_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(ov9286_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

k_s32 ov9286_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

k_s32 ov9286_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

k_s32 ov9286_sensor_set_stream(void *ctx, k_s32 enable)
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

k_s32 ov9286_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

k_s32 ov9286_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16 + 0.5);
        //if(current_mode->sensor_again !=again)
        {
	        ret = sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN,(again & 0xff));
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/16.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS]* 16 + 0.5);
        ret = sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN,(again & 0xff));
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

k_s32 ov9286_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

k_s32 ov9286_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_AGAIN_L,(again & 0xff));
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

k_s32 ov9286_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

k_s32 ov9286_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u16 exp_line = 0;
    k_u16 Strobe_StartPoint = 0;
    k_u16 Strobe_Width = 0;
    float integraion_time = 0;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];

        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(1, exp_line));
        //if (current_mode->et_line != exp_line)
        {
	        ret |= sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_EXP_TIME_H, ( exp_line >> 4) & 0xff);
	        ret |= sensor_reg_write(&dev->i2c_info, OV9286_REG_LONG_EXP_TIME_L, ( exp_line << 4) & 0xf0);
	        current_mode->et_line = exp_line;
/*	        //set strobe
	        Strobe_Width = (exp_line + 52)/3;
	        Strobe_StartPoint = Sensor_VTS - Strobe_Width - 7;
	        //Strobe_StartPoint = Sensor_VTS - Strobe_Width;

	        ret |= sensor_reg_write(&dev->i2c_info, 0x3927, ( Strobe_Width >> 8) & 0xff);	//strobe width
	        ret |= sensor_reg_write(&dev->i2c_info, 0x3928,  Strobe_Width & 0xff);
	        ret |= sensor_reg_write(&dev->i2c_info, 0x3929, ( Strobe_StartPoint >> 8) & 0xff);	//strobe start point
	        ret |= sensor_reg_write(&dev->i2c_info, 0x392a,  Strobe_StartPoint & 0xff);*/
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

k_s32 ov9286_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

k_s32 ov9286_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

k_s32 ov9286_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

k_s32 ov9286_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

k_s32 ov9286_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 ov9286_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

k_s32 ov9286_sensor_get_otp_data(void *ctx, void *data)
{
    struct sensor_driver_dev *dev = ctx;
    k_s32 ret = 0;
    k_sensor_otp_date otp_date;
    k_u16 reg_data = 0;
    rt_kprintf("%s enter\n", __func__);

    //get opt data 
    memcpy(&otp_date, data, sizeof(k_sensor_otp_date));
    //clear 0x3D00~0x3D1F 0
    for(int i = 0; i < 32; i++)
    {   
        sensor_reg_write(&dev->i2c_info, 0x3d00 + i, 0x00);
    }
    sensor_reg_write(&dev->i2c_info, 0x3d81, 0x01);
    rt_thread_mdelay(15);

    ret = sensor_reg_read(&dev->i2c_info,  0x3d10, &reg_data);
    rt_kprintf("0x3d10 val is %x \n", reg_data);

    if(otp_date.otp_type == 1)
    {
        for(int i = 0; i < 17; i++)
        {
            ret = sensor_reg_read(&dev->i2c_info,  0x3d00 + i, &reg_data);
            otp_date.otp_date[i] = (k_u8)reg_data;
            // rt_kprintf("read otp tpye is %d read reg(%d) val is %x \n", otp_date.otp_type, i, reg_data);
        }
    }
    else
    {
        for(int i = 0; i < 15; i++)
        {
            ret = sensor_reg_read(&dev->i2c_info,  0x3d11 + i, &reg_data);
            otp_date.otp_date[i] = (k_u8)reg_data;
            // rt_kprintf("read otp tpye is %d read reg(%d) val is %x \n", otp_date.otp_type, i, reg_data);
        }
    }
    memcpy(data, &otp_date, sizeof(k_sensor_otp_date));

    return ret;
}

k_s32 ov9286_sensor_set_otp_data(void *ctx, void *data)
{
    struct sensor_driver_dev *dev = ctx;
    k_s32 ret = 0;
    k_sensor_otp_date otp_date;
    k_sensor_otp_date read_otp_date;
    k_u16 reg_data = 0;
    k_u16 data_len = 0;
    pr_info("%s enter\n", __func__);

    memset(&read_otp_date, 0, sizeof(k_sensor_otp_date));
    memcpy(&otp_date, data, sizeof(k_sensor_otp_date));

    // for(int i = 0; i < 15; i++)
    // {
    //     rt_kprintf("set otp tpye is %d read reg(%d) val is %x \n", otp_date.otp_type, i, otp_date.otp_date[i]);
    // }

    rt_kprintf("read otp tpye   char val is %s  \n", otp_date.otp_date);

    ret = sensor_reg_read(&dev->i2c_info,  0x3d10, &reg_data);
    if(reg_data == 0x01)
    {
        read_otp_date.otp_type = 0;
        ov9286_sensor_get_otp_data(ctx, &read_otp_date);
        // compare date 
        rt_kprintf("otp alredy write %s \n", read_otp_date.otp_date);
        return K_ERR_VICAP_OPT_ALREADY_WRITE;
    }
        
#if 1
    // clear otp blank
    for(int i = 0; i < 32; i++)
    {
        if(0x3d00 + i == 0x3d10)
            continue;
        sensor_reg_write(&dev->i2c_info, 0x3d00 + i, 0x00);
        // ret = sensor_reg_read(&dev->i2c_info,  0x3d00 + i, &reg_data);
        // // if(reg_data != 0x0)
        // //     rt_kprintf("i is %x val is %x \n", 0x3d00 + i, reg_data);
        
    }

    // write reg 
    data_len = strlen(otp_date.otp_date);
    if(data_len > 15)
        return -1;
    for(int i = 0; i < data_len; i++)
    {
        sensor_reg_write(&dev->i2c_info, 0x3d11 + i, otp_date.otp_date[i]);
    }

    sensor_reg_write(&dev->i2c_info, 0x3d10, 0x01);
    sensor_reg_write(&dev->i2c_info, 0x3d80, 0x01);
    

    rt_thread_mdelay(100);

    // read opt date 
    read_otp_date.otp_type = 0;
    ov9286_sensor_get_otp_data(ctx, &read_otp_date);
    // compare date 
    for(int i = 0; i < data_len; i++)
    {
        if(read_otp_date.otp_date[i] != otp_date.otp_date[i])
        {
            rt_kprintf("otp write failed read_otp_date is %x otp_date is %x i is %d K_ERR_VICAP_OPT_ALREADY_WRITE is %d \n", read_otp_date.otp_date[i], otp_date.otp_date[i], i, K_ERR_VICAP_OPT_ALREADY_WRITE);
            ret = K_ERR_VICAP_OPT_ALREADY_WRITE;
        }
    }
#endif

    return ret;
}


static k_s32 ov9286_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev ov9286_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c0",
        .slave_addr = 0x60,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "ov9286",
    .sensor_func = {
        .sensor_power = ov9286_sensor_power_on,
        .sensor_init = ov9286_sensor_init,
        .sensor_get_chip_id = ov9286_sensor_get_chip_id,
        .sensor_get_mode = ov9286_sensor_get_mode,
        .sensor_set_mode = ov9286_sensor_set_mode,
        .sensor_enum_mode = ov9286_sensor_enum_mode,
        .sensor_get_caps = ov9286_sensor_get_caps,
        .sensor_conn_check = ov9286_sensor_conn_check,
        .sensor_set_stream = ov9286_sensor_set_stream,
        .sensor_get_again = ov9286_sensor_get_again,
        .sensor_set_again = ov9286_sensor_set_again,
        .sensor_get_dgain = ov9286_sensor_get_dgain,
        .sensor_set_dgain = ov9286_sensor_set_dgain,
        .sensor_get_intg_time = ov9286_sensor_get_intg_time,
        .sensor_set_intg_time = ov9286_sensor_set_intg_time,
        .sensor_get_exp_parm = ov9286_sensor_get_exp_parm,
        .sensor_set_exp_parm = ov9286_sensor_set_exp_parm,
        .sensor_get_fps = ov9286_sensor_get_fps,
        .sensor_set_fps = ov9286_sensor_set_fps,
        .sensor_get_isp_status = ov9286_sensor_get_isp_status,
        .sensor_set_blc = ov9286_sensor_set_blc,
        .sensor_set_wb = ov9286_sensor_set_wb,
        .sensor_get_tpg = ov9286_sensor_get_tpg,
        .sensor_set_tpg = ov9286_sensor_set_tpg,
        .sensor_get_expand_curve = ov9286_sensor_get_expand_curve,
        .sensor_get_otp_data = ov9286_sensor_get_otp_data,
        .sensor_set_otp_data = ov9286_sensor_set_otp_data,
        .sensor_mirror_set = ov9286_sensor_mirror_set,
    },
};
