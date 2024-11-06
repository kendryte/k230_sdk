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

#define OS08A20_MIN_GAIN_STEP    (1.0f/128.0f)

static k_u8 mirror_flag = 0; 


static k_sensor_reg os08a20_mirror[] = {
    {REG_NULL, 0x00},
};


static k_sensor_reg OS08a20_mipi4lane_1080p_init[] =
{
#if 1
    {0x0100, 0x00},
	{0x0103, 0x01},

	{0x0303, 0x01},
	{0x0305, 0x2d}, /* PLL_CTRL_05 (default 0x3c) */
	{0x0306, 0x00},
	{0x0308, 0x03},
	{0x0309, 0x04},
    
	{0x0325, 0x47}, /* PLL_CTRL_25 (default 0x3c) */
	{0x032a, 0x00}, /* PLL_CTRL_2a (default 0x00) */
	{0x300f, 0x11},
	{0x3010, 0x01},
	{0x3011, 0x04},
	{0x3012, 0x41},
	{0x3016, 0xf0},
	{0x301e, 0x98},
	{0x3031, 0xa9},
	{0x3103, 0x92},
	{0x3104, 0x01},
	{0x3106, 0x10},
	{0x3400, 0x04}, /* PSV CTRL (default 0x00) bit[2]=r_psv_mode_en */
	{0x3025, 0x03}, /* PSV MODE OPT (default 0x02) not used */
	{0x3425, 0x01}, /* R ASP PD SEL bit[1:0]=stream blanking */
	{0x3428, 0x01}, /* R ASP PD SEL bit[1:0]=bpg1 N-pump1 bypass to AGND */
	{0x3406, 0x08}, /* R STREAM ST OFFS (default 0x08) can remove */
	{0x3408, 0x03}, /* CTRL08 (default 0x01) bit[3:0]=r_clk_winp_off */
	{0x340c, 0xff},
	{0x340d, 0xff},
	{0x031e, 0x0a},
	{0x3501, 0x08}, /* Long exposure */
	{0x3502, 0xe5}, /* Long exposure */
	{0x3505, 0x83},
	{0x3508, 0x00}, /* Long gain */
	{0x3509, 0x80}, /* Long gain */
	{0x350a, 0x04},
	{0x350b, 0x00},
	{0x350c, 0x00},
	{0x350d, 0x80},
	{0x350e, 0x04},
	{0x350f, 0x00},
	{0x3600, 0x00}, /* CORE0 bit[0]=stg_hdr_align_en, bit[3]=new_stgr_hdr_en */
	{0x3603, 0x2c},
	{0x3605, 0x50},
	{0x3609, 0xdb},
	{0x3610, 0x39},
	{0x360c, 0x01},
	{0x3628, 0xa4},
	{0x362d, 0x10},
	{0x3660, 0xd3}, /* CORE0 bit[0]=rip_sof_vifo_en, bit[1]=stg_hdr_long_en debug mode */
	{0x3661, 0x06}, /* CORE1 (default 0x06) can remove */
	{0x3662, 0x00},
	{0x3663, 0x28},
	{0x3664, 0x0d},
	{0x366a, 0x38},
	{0x366b, 0xa0},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x3680, 0x00},
	{0x36c0, 0x00},
	{0x3701, 0x02}, /* Sensor timing control registers 0x3700-0x37ff */
	{0x373b, 0x02},
	{0x373c, 0x02},
	{0x3736, 0x02},
	{0x3737, 0x02},
	{0x3705, 0x00},
	{0x3706, 0x72},
	{0x370a, 0x01},
	{0x370b, 0x30},
	{0x3709, 0x48},
	{0x3714, 0x21}, /* Sensor timing control registers 0x3700-0x37ff */
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x3740, 0x1b},
	{0x3741, 0x04},
	{0x375e, 0x0b},
	{0x3760, 0x10},
	{0x3776, 0x10},
	{0x3781, 0x02},
	{0x3782, 0x04},
	{0x3783, 0x02},
	{0x3784, 0x08},
	{0x3785, 0x08},
	{0x3788, 0x01},
	{0x3789, 0x01},
	{0x3797, 0x04},
	{0x3762, 0x11},  /* Sensor timing control registers 0x3700-0x37ff */
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x0c},
	{0x3804, 0x0e},
	{0x3805, 0xff},
	{0x3806, 0x08},
	{0x3807, 0x6f},

	{0x3808, 0x07}, /* X output size (default 0x07) */
	{0x3809, 0x80}, /* X output size (default 0x80) */
	{0x380a, 0x04}, /* Y output size (default 0x04) */
	{0x380b, 0x38}, /* Y output size (default 0x38) */

	{0x380c, 0x08}, /* HTS[15:8], total horizontal timing size */
	{0x380d, 0x98}, /* HTS[7:0],  total horizontal timing size */

	{0x380e, 0x04}, /* VTS[15:8], total vertical timing (default 0x04) */
	{0x380f, 0xa4}, /* VTS[7:0],  total vertical timing (default 0xA0) */


	{0x3813, 0x10}, /* ISP_Y_WIN ISP vertical windowing offset */
	{0x3814, 0x01}, /* X INC ODD (default 0x01) */
	{0x3815, 0x01}, /* X INC EVEN (default 0x01) */
	{0x3816, 0x01}, /* Y INC ODD (default 0x01) */
	{0x3817, 0x01}, /* Y INC EVEN (default 0x01) */
	{0x381c, 0x00}, /* BLC_NUM_OPTION (default 0x0e) */

	{0x3820, 0x00}, /* FORMAT1 (default 0x80) bit[0]=vertical bining */
	{0x3821, 0x04}, /* FORMAT2 bit[2]=mirror, bit[0]=horizontal bining */


	{0x3823, 0x08},
	{0x3826, 0x00},
	{0x3827, 0x08},
	{0x382d, 0x08},
	{0x3832, 0x02},
	{0x3833, 0x00}, /* REG33 (bit[0]=r_stg_hdr_grp_wr_opt, bit[2]=r_stg_grphold_nomask) */
	{0x383c, 0x48},
	{0x383d, 0xff},
	{0x3d85, 0x0b},
	{0x3d84, 0x40},
	{0x3d8c, 0x63},
	{0x3d8d, 0xd7},
	{0x4000, 0xf8},
	{0x4001, 0x2b},
	{0x4004, 0x00},
	{0x4005, 0x40},
	{0x400a, 0x01},
	{0x400f, 0xa0},
	{0x4010, 0x12},
	{0x4018, 0x00},
	{0x4008, 0x02},
	{0x4009, 0x0d}, /* BLC CTRL09 (default 0x0f) bl_end */
	{0x401a, 0x58},
	{0x4050, 0x00},
	{0x4051, 0x01},
	{0x4028, 0x2f},
	{0x4052, 0x00},
	{0x4053, 0x80},
	{0x4054, 0x00},
	{0x4055, 0x80},
	{0x4056, 0x00},
	{0x4057, 0x80},
	{0x4058, 0x00},
	{0x4059, 0x80},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4501, 0x18}, /* R1 (default 0x18) bit[4:2]=not used */
	{0x4502, 0x00},
	{0x4600, 0x00},
	{0x4601, 0x20},
	{0x4603, 0x01},
	{0x4643, 0x00},
	{0x4640, 0x01},
	{0x4641, 0x04},
	{0x4800, 0x64},
	{0x4809, 0x2b},
	{0x4813, 0x90}, /* MIPI CTRL 13 (bit[5:4]=VC1=1, bit[7:6]=VC2=2) */
	{0x4817, 0x04},
	{0x4833, 0x18},
	{0x4837, 0x10}, /* PCLK PERIOD (default 0x08) */
	{0x483b, 0x00},
	{0x484b, 0x03},
	{0x4850, 0x7c},
	{0x4852, 0x06},
	{0x4856, 0x58},
	{0x4857, 0xaa},
	{0x4862, 0x0a},
	{0x4869, 0x18},
	{0x486a, 0xaa},
	{0x486e, 0x03}, /* MIPI CTRL 6E (default 0x03) */
	{0x486f, 0x55},
	{0x4875, 0xf0},
	{0x5000, 0x89},
	{0x5001, 0x42},
	{0x5004, 0x40},
	{0x5005, 0x00},
	{0x5180, 0x00},
	{0x5181, 0x10},
	{0x580b, 0x03},
	{0x4d00, 0x03},
	{0x4d01, 0xc9},
	{0x4d02, 0xbc},
	{0x4d03, 0xc6},
	{0x4d04, 0x4a},
	{0x4d05, 0x25},
	{0x4700, 0x2b},
	{0x4e00, 0x2b},
	{0x3501, 0x09}, /* Long exposure */
	{0x3502, 0x01}, /* Long exposure */
	{0x4028, 0x4f},
	{0x4029, 0x1f},
	{0x402a, 0x7f},
	{0x402b, 0x01},
	{0x0100, 0x01},
    {REG_NULL, 0x00},
#else
    {0x0100, 0x00},
    {0x0103, 0x01},
    {0x0303, 0x01},
    {0x0305, 0x32},
    {0x0306, 0x00},
    {0x0308, 0x03},
    {0x0309, 0x04},
    {0x032a, 0x00},
    {0x300f, 0x11},
    {0x3010, 0x01},
    {0x3011, 0x04},
    {0x3012, 0x41},
    {0x3016, 0xf0},
    {0x301e, 0x98},
    {0x3031, 0xa9},
    {0x3103, 0x92},
    {0x3104, 0x01},
    {0x3106, 0x10},
    {0x3400, 0x04},
    {0x3025, 0x03},
    {0x3425, 0x01},
    {0x3428, 0x01},
    {0x3406, 0x08},
    {0x3408, 0x03},
    {0x340c, 0xff},
    {0x340d, 0xff},
    {0x031e, 0x09},
    {0x3501, 0x03},
    {0x3502, 0x62},
    {0x3505, 0x83},
    {0x3508, 0x00},
    {0x3509, 0x80},
    {0x350a, 0x04},
    {0x350b, 0x00},
    {0x350c, 0x00},
    {0x350d, 0x80},
    {0x350e, 0x04},
    {0x350f, 0x00},
    {0x3600, 0x09},
    {0x3603, 0x2c},
    {0x3605, 0x50},
    {0x3609, 0xb5},
    {0x3610, 0x39},
    {0x360c, 0x01},
    {0x3628, 0xa4},
    {0x362d, 0x10},
    {0x3660, 0x43},
    {0x3661, 0x06},
    {0x3662, 0x00},
    {0x3663, 0x28},
    {0x3664, 0x0d},
    {0x366a, 0x38},
    {0x366b, 0xa0},
    {0x366d, 0x00},
    {0x366e, 0x00},
    {0x3680, 0x00},
    {0x36c0, 0x00},
    {0x3701, 0x02},
    {0x373b, 0x02},
    {0x373c, 0x02},
    {0x3736, 0x02},
    {0x3737, 0x02},
    {0x3705, 0x00},
    {0x3706, 0x39},
    {0x370a, 0x00},
    {0x370b, 0x98},
    {0x3709, 0x49},
    {0x3714, 0x22},
    {0x371c, 0x00},
    {0x371d, 0x08},
    {0x3740, 0x1b},
    {0x3741, 0x04},
    {0x375e, 0x0b},
    {0x3760, 0x10},
    {0x3776, 0x10},
    {0x3781, 0x02},
    {0x3782, 0x04},
    {0x3783, 0x02},
    {0x3784, 0x08},
    {0x3785, 0x08},
    {0x3788, 0x01},
    {0x3789, 0x01},
    {0x3797, 0x04},
    {0x3762, 0x11},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x0c},
    {0x3804, 0x0e},
    {0x3805, 0xff},
    {0x3806, 0x08},
    {0x3807, 0x6f},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x380c, 0x04},
    {0x380d, 0x0c},
    {0x380e, 0x04},
    {0x380f, 0x86},
    {0x3813, 0x08},
    {0x3814, 0x03},
    {0x3815, 0x01},
    {0x3816, 0x03},
    {0x3817, 0x01},
    {0x381c, 0x00},
    {0x3820, 0x01},
    {0x3821, 0x05},
    {0x3823, 0x08},
    {0x3826, 0x00},
    {0x3827, 0x08},
    {0x382d, 0x08},
    {0x3832, 0x02},
    {0x3833, 0x00},
    {0x383c, 0x48},
    {0x383d, 0xff},
    {0x3d85, 0x0b},
    {0x3d84, 0x40},
    {0x3d8c, 0x63},
    {0x3d8d, 0xd7},
    {0x4000, 0xf8},
    {0x4001, 0x2b},
    {0x4004, 0x00},
    {0x4005, 0x40},
    {0x400a, 0x01},
    {0x400f, 0xa0},
    {0x4010, 0x12},
    {0x4018, 0x00},
    {0x4008, 0x02},
    {0x4009, 0x05},
    {0x401a, 0x58},
    {0x4050, 0x00},
    {0x4051, 0x01},
    {0x4028, 0x2f},
    {0x4052, 0x00},
    {0x4053, 0x80},
    {0x4054, 0x00},
    {0x4055, 0x80},
    {0x4056, 0x00},
    {0x4057, 0x80},
    {0x4058, 0x00},
    {0x4059, 0x80},
    {0x430b, 0xff},
    {0x430c, 0xff},
    {0x430d, 0x00},
    {0x430e, 0x00},
    {0x4501, 0x98},
    {0x4502, 0x00},
    {0x4643, 0x00},
    {0x4640, 0x01},
    {0x4641, 0x04},
    {0x4800, 0x64},
    {0x4809, 0x2b},
    {0x4813, 0x90},
    {0x4817, 0x04},
    {0x4833, 0x18},
    {0x4837, 0x14},
    {0x483b, 0x00},
    {0x484b, 0x03},
    {0x4850, 0x7c},
    {0x4852, 0x06},
    {0x4856, 0x58},
    {0x4857, 0xaa},
    {0x4862, 0x0a},
    {0x4869, 0x18},
    {0x486a, 0xaa},
    {0x486e, 0x03},
    {0x486f, 0x55},
    {0x4875, 0xf0},
    {0x5000, 0x89},
    {0x5001, 0x42},
    {0x5004, 0x40},
    {0x5005, 0x00},
    {0x5180, 0x00},
    {0x5181, 0x10},
    {0x580b, 0x03},
    {0x4d00, 0x03},
    {0x4d01, 0xc9},
    {0x4d02, 0xbc},
    {0x4d03, 0xc6},
    {0x4d04, 0x4a},
    {0x4d05, 0x25},
    {0x4700, 0x2b},
    {0x4e00, 0x2b},
    {0x4028, 0x4f},
    {0x4029, 0x1f},
    {0x402a, 0x7f},
    {0x402b, 0x01},
    {0x3822, 0x54},
#endif
};


static k_sensor_reg OS08a20_mipi4lane_4k_init[] =
{

    {0x0100, 0x00},
	{0x0103, 0x01},
	{0x0303, 0x01},
	{0x0305, 0x3c}, /* PLL_CTRL_05 (default 0x3c) */
	{0x0306, 0x00},
	{0x0308, 0x03},
	{0x0309, 0x04},
	{0x0325, 0x47}, /* PLL_CTRL_25 (default 0x3c) */
	{0x032a, 0x00}, /* PLL_CTRL_2a (default 0x00) */
	{0x300f, 0x11},
	{0x3010, 0x01},
	{0x3011, 0x04},
	{0x3012, 0x41},
	{0x3016, 0xf0},
	{0x301e, 0x98},
	{0x3031, 0xa9},
	{0x3103, 0x92},
	{0x3104, 0x01},
	{0x3106, 0x10},
	{0x3400, 0x04}, /* PSV CTRL (default 0x00) bit[2]=r_psv_mode_en */
	{0x3025, 0x03}, /* PSV MODE OPT (default 0x02) not used */
	{0x3425, 0x01}, /* R ASP PD SEL bit[1:0]=stream blanking */
	{0x3428, 0x01}, /* R ASP PD SEL bit[1:0]=bpg1 N-pump1 bypass to AGND */
	{0x3406, 0x08}, /* R STREAM ST OFFS (default 0x08) can remove */
	{0x3408, 0x03}, /* CTRL08 (default 0x01) bit[3:0]=r_clk_winp_off */
	{0x340c, 0xff},
	{0x340d, 0xff},
	{0x031e, 0x0a},
	{0x3501, 0x08}, /* Long exposure */
	{0x3502, 0xe5}, /* Long exposure */
	{0x3505, 0x83},
	{0x3508, 0x00}, /* Long gain */
	{0x3509, 0x80}, /* Long gain */
	{0x350a, 0x04},
	{0x350b, 0x00},
	{0x350c, 0x00},
	{0x350d, 0x80},
	{0x350e, 0x04},
	{0x350f, 0x00},
	{0x3600, 0x00}, /* CORE0 bit[0]=stg_hdr_align_en, bit[3]=new_stgr_hdr_en */
	{0x3603, 0x2c},
	{0x3605, 0x50},
	{0x3609, 0xdb},
	{0x3610, 0x39},
	{0x360c, 0x01},
	{0x3628, 0xa4},
	{0x362d, 0x10},
	{0x3660, 0xd3}, /* CORE0 bit[0]=rip_sof_vifo_en, bit[1]=stg_hdr_long_en debug mode */
	{0x3661, 0x06}, /* CORE1 (default 0x06) can remove */
	{0x3662, 0x00},
	{0x3663, 0x28},
	{0x3664, 0x0d},
	{0x366a, 0x38},
	{0x366b, 0xa0},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x3680, 0x00},
	{0x36c0, 0x00},
	{0x3701, 0x02}, /* Sensor timing control registers 0x3700-0x37ff */
	{0x373b, 0x02},
	{0x373c, 0x02},
	{0x3736, 0x02},
	{0x3737, 0x02},
	{0x3705, 0x00},
	{0x3706, 0x72},
	{0x370a, 0x01},
	{0x370b, 0x30},
	{0x3709, 0x48},
	{0x3714, 0x21}, /* Sensor timing control registers 0x3700-0x37ff */
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x3740, 0x1b},
	{0x3741, 0x04},
	{0x375e, 0x0b},
	{0x3760, 0x10},
	{0x3776, 0x10},
	{0x3781, 0x02},
	{0x3782, 0x04},
	{0x3783, 0x02},
	{0x3784, 0x08},
	{0x3785, 0x08},
	{0x3788, 0x01},
	{0x3789, 0x01},
	{0x3797, 0x04},
	{0x3762, 0x11},  /* Sensor timing control registers 0x3700-0x37ff */
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x0c},
	{0x3804, 0x0e},
	{0x3805, 0xff},
	{0x3806, 0x08},
	{0x3807, 0x6f},
	{0x3808, 0x0f}, /* X output size (default 0x07) */
	{0x3809, 0x00}, /* X output size (default 0x80) */
	{0x380a, 0x08}, /* Y output size (default 0x04) */
	{0x380b, 0x70}, /* Y output size (default 0x38) */
	{0x380c, 0x08}, /* HTS[15:8], total horizontal timing size */
	{0x380d, 0x14}, /* HTS[7:0],  total horizontal timing size */
	{0x380e, 0x08}, /* VTS[15:8], total vertical timing (default 0x04) */
	{0x380f, 0xf0}, /* VTS[7:0],  total vertical timing (default 0xA0) */
	{0x3813, 0x10}, /* ISP_Y_WIN ISP vertical windowing offset */
	{0x3814, 0x01}, /* X INC ODD (default 0x01) */
	{0x3815, 0x01}, /* X INC EVEN (default 0x01) */
	{0x3816, 0x01}, /* Y INC ODD (default 0x01) */
	{0x3817, 0x01}, /* Y INC EVEN (default 0x01) */
	{0x381c, 0x00}, /* BLC_NUM_OPTION (default 0x0e) */
	{0x3820, 0x00}, /* FORMAT1 (default 0x80) bit[0]=vertical bining */
	{0x3821, 0x04}, /* FORMAT2 bit[2]=mirror, bit[0]=horizontal bining */
	{0x3823, 0x08},
	{0x3826, 0x00},
	{0x3827, 0x08},
	{0x382d, 0x08},
	{0x3832, 0x02},
	{0x3833, 0x00}, /* REG33 (bit[0]=r_stg_hdr_grp_wr_opt, bit[2]=r_stg_grphold_nomask) */
	{0x383c, 0x48},
	{0x383d, 0xff},
	{0x3d85, 0x0b},
	{0x3d84, 0x40},
	{0x3d8c, 0x63},
	{0x3d8d, 0xd7},
	{0x4000, 0xf8},
	{0x4001, 0x2b},
	{0x4004, 0x00},
	{0x4005, 0x40},
	{0x400a, 0x01},
	{0x400f, 0xa0},
	{0x4010, 0x12},
	{0x4018, 0x00},
	{0x4008, 0x02},
	{0x4009, 0x0d}, /* BLC CTRL09 (default 0x0f) bl_end */
	{0x401a, 0x58},
	{0x4050, 0x00},
	{0x4051, 0x01},
	{0x4028, 0x2f},
	{0x4052, 0x00},
	{0x4053, 0x80},
	{0x4054, 0x00},
	{0x4055, 0x80},
	{0x4056, 0x00},
	{0x4057, 0x80},
	{0x4058, 0x00},
	{0x4059, 0x80},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4501, 0x18}, /* R1 (default 0x18) bit[4:2]=not used */
	{0x4502, 0x00},
	{0x4600, 0x00},
	{0x4601, 0x20},
	{0x4603, 0x01},
	{0x4643, 0x00},
	{0x4640, 0x01},
	{0x4641, 0x04},
	{0x4800, 0x64},
	{0x4809, 0x2b},
	{0x4813, 0x90}, /* MIPI CTRL 13 (bit[5:4]=VC1=1, bit[7:6]=VC2=2) */
	{0x4817, 0x04},
	{0x4833, 0x18},
	{0x4837, 0x10}, /* PCLK PERIOD (default 0x08) */
	{0x483b, 0x00},
	{0x484b, 0x03},
	{0x4850, 0x7c},
	{0x4852, 0x06},
	{0x4856, 0x58},
	{0x4857, 0xaa},
	{0x4862, 0x0a},
	{0x4869, 0x18},
	{0x486a, 0xaa},
	{0x486e, 0x03}, /* MIPI CTRL 6E (default 0x03) */
	{0x486f, 0x55},
	{0x4875, 0xf0},
	{0x5000, 0x89},
	{0x5001, 0x42},
	{0x5004, 0x40},
	{0x5005, 0x00},
	{0x5180, 0x00},
	{0x5181, 0x10},
	{0x580b, 0x03},
	{0x4d00, 0x03},
	{0x4d01, 0xc9},
	{0x4d02, 0xbc},
	{0x4d03, 0xc6},
	{0x4d04, 0x4a},
	{0x4d05, 0x25},
	{0x4700, 0x2b},
	{0x4e00, 0x2b},
	{0x3501, 0x09}, /* Long exposure */
	{0x3502, 0x01}, /* Long exposure */
	{0x4028, 0x4f},
	{0x4029, 0x1f},
	{0x402a, 0x7f},
	{0x402b, 0x01},
	{0x0100, 0x01},
    {REG_NULL, 0x00},

};

static k_sensor_ae_info sensor_ae_info[] = {
    // 1080P30 
    {
        .frame_length = 0x486,
        .cur_frame_length = 0x486,
        .one_line_exp_time = 0.000036,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000036,
        .gain_increment = OS08A20_MIN_GAIN_STEP,
        .max_integraion_line = 0x486 - 8,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000036 * (0x486 - 8),
        .min_integraion_time = 0.000036 * 1,
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

static k_sensor_mode os08a20_mode_info[] = {
    {
        .index = 0,
        .sensor_type = OS08A20_MIPI_CSI0_3840X2160_30FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 3840,
            .bounds_height = 2160,
            .top = 0,
            .left = 0,
            .width = 3840,
            .height = 2160,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_BGGR,//BAYER_PAT_GRBG, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2B,
        },
        .reg_list = OS08a20_mipi4lane_4k_init,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,	// 594/25 = 23.76MHz
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[0],
    },

    // {
    //     .index = 1,
    //     .sensor_type = OS08A20_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    //     .size = {
    //         .bounds_width = 1920,
    //         .bounds_height = 1080,
    //         .top = 0,
    //         .left = 0,
    //         .width = 1920,
    //         .height = 1080,
    //     },
    //     .fps = 30000,
    //     .hdr_mode = SENSOR_MODE_LINEAR,
    //     .bit_width = 12,
    //     .bayer_pattern = BAYER_PAT_BGGR,//BAYER_PAT_GRBG, //BAYER_PAT_RGGB,
    //     .mipi_info = {
    //         .csi_id = 0,
    //         .mipi_lanes = 4,
    //         .data_type = 0x2B,
    //     },
    //     .reg_list = OS08a20_mipi4lane_1080p_init, //OS08a20_mipi4lane_4k_init, //OS08a20_mipi4lane_1080p_init,
    //     .mclk_setting = {
    //         {
    //             .mclk_setting_en = K_TRUE,
    //             .setting.id = SENSOR_MCLK0,
    //             .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
    //             .setting.mclk_div = 25,	// 594/25 = 23.76MHz
    //         },
    //         {K_FALSE},
    //         {K_FALSE},
    //     },
    //     .sensor_ae_info = &sensor_ae_info[0],
    // },
};

static k_bool os08a20_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int os08a20_power_rest(k_s32 on)
{
     #define VICAP_OS08A20_RST_GPIO     (63)  //24// 

    kd_pin_mode(VICAP_OS08A20_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_OS08A20_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_OS08A20_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_OS08A20_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(VICAP_OS08A20_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int os08a20_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 os08a20_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    kd_pin_mode(VICAP_OS08A20_RST_GPIO, GPIO_DM_OUTPUT);
    kd_pin_write(VICAP_OS08A20_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

    os08a20_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, 0x300b, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, 0x300c , &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    if(*chip_id != 0x0841)
        ret = -1;

    return ret;
}


static k_s32 os08a20_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!os08a20_init_flag) {
            os08a20_power_rest(on);
            os08a20_i2c_init(&dev->i2c_info);
        // }
        ret = os08a20_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        os08a20_init_flag = K_FALSE;
        os08a20_power_rest(on);
    }

    return ret;
}

static k_s32 os08a20_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(os08a20_mode_info) / sizeof(k_sensor_mode); i++) {
            if (os08a20_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(os08a20_mode_info[i]);
                dev->sensor_mode = &(os08a20_mode_info[i]);
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
        sensor_reg_list_write(&dev->i2c_info, os08a20_mirror);
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

    ret = sensor_reg_read(&dev->i2c_info, 0x3508, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, 0x3509, &again_l);
    again = (float)(again_l)/64.0f + again_h;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, 0x3501, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, 0x3502, &exp_time_l);
    exp_time = ((exp_time_h & 0x3f) << 8) + exp_time_l;

    current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time *  exp_time;

    os08a20_init_flag = K_TRUE;
    return ret;
}


static k_s32 os08a20_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(os08a20_mode_info) / sizeof(k_sensor_mode); i++) {
        if (os08a20_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &os08a20_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(os08a20_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 os08a20_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 os08a20_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 os08a20_sensor_set_stream(void *ctx, k_s32 enable)
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

static k_s32 os08a20_sensor_get_again(void *ctx, k_sensor_gain *gain)
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


static k_s32 os08a20_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 again, dgain, total;
    k_u8 i;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {

        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 128 + 0.5);
        if(current_mode->sensor_again !=again)
        {
            ret =  sensor_reg_write(&dev->i2c_info,0x3508,(again & 0x3f00)>>8);
            ret |=  sensor_reg_write(&dev->i2c_info,0x3509,(again & 0xff));
            current_mode->sensor_again = again;
        }
		
		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/128.0f;
		
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

static k_s32 os08a20_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 os08a20_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OS08A20_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OS08A20_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, OS08A20_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, OS08A20_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 os08a20_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 os08a20_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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
            ret |= sensor_reg_write(&dev->i2c_info, 0x3501, (exp_line >>8) & 0xff);
            ret |= sensor_reg_write(&dev->i2c_info, 0x3502, (exp_line & 0xff));
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

static k_s32 os08a20_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 os08a20_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 os08a20_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 os08a20_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 os08a20_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 os08a20_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 os08a20_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 os08a20_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    return 0;
}


struct sensor_driver_dev os08a20_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c1", //"i2c3",   //"i2c0", //"i2c3",
        .slave_addr = 0x36,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "os08a20",
    .sensor_func = {
        .sensor_power = os08a20_sensor_power_on,
        .sensor_init = os08a20_sensor_init,
        .sensor_get_chip_id = os08a20_sensor_get_chip_id,
        .sensor_get_mode = os08a20_sensor_get_mode,
        .sensor_set_mode = os08a20_sensor_set_mode,
        .sensor_enum_mode = os08a20_sensor_enum_mode,
        .sensor_get_caps = os08a20_sensor_get_caps,
        .sensor_conn_check = os08a20_sensor_conn_check,
        .sensor_set_stream = os08a20_sensor_set_stream,
        .sensor_get_again = os08a20_sensor_get_again,
        .sensor_set_again = os08a20_sensor_set_again,
        .sensor_get_dgain = os08a20_sensor_get_dgain,
        .sensor_set_dgain = os08a20_sensor_set_dgain,
        .sensor_get_intg_time = os08a20_sensor_get_intg_time,
        .sensor_set_intg_time = os08a20_sensor_set_intg_time,
        .sensor_get_exp_parm = os08a20_sensor_get_exp_parm,
        .sensor_set_exp_parm = os08a20_sensor_set_exp_parm,
        .sensor_get_fps = os08a20_sensor_get_fps,
        .sensor_set_fps = os08a20_sensor_set_fps,
        .sensor_get_isp_status = os08a20_sensor_get_isp_status,
        .sensor_set_blc = os08a20_sensor_set_blc,
        .sensor_set_wb = os08a20_sensor_set_wb,
        .sensor_get_tpg = os08a20_sensor_get_tpg,
        .sensor_set_tpg = os08a20_sensor_set_tpg,
        .sensor_get_expand_curve = os08a20_sensor_get_expand_curve,
        .sensor_get_otp_data = os08a20_sensor_get_otp_data,
        .sensor_mirror_set = os08a20_sensor_mirror_set,
    },
};
