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
#define GC2093_REG_ID 0x03f0

/* Exposure control */
#define GC2093_REG_EXP_SHORT_TIME_H    0x0001
#define GC2093_REG_EXP_SHORT_TIME_L    0x0002
#define GC2093_REG_EXP_TIME_H    0x0003
#define GC2093_REG_EXP_TIME_L    0x0004

/* Analog gain control */
#define GC2093_REG_DGAIN_H	0x00b1	//0x00b8
#define GC2093_REG_DGAIN_L	0x00b2	//0x00b9
#define GC2093_MIN_GAIN_STEP    (1.0f/64.0f)

static k_u8 mirror_flag = 0; 


static k_sensor_reg gc2093_mirror[] = {
    {0x0017, 0x03}, 
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_1080p_15fps_hdr[] = {
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x30},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},
    {0x0002, 0x02},
    {0x0003, 0x04},
    {0x0004, 0x02},
    {0x0005, 0x02},
    {0x0006, 0x94},
    {0x0007, 0x00},
    {0x0008, 0x11},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},
    {0x000e, 0x40},
    {0x000f, 0x07},
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},
    {0x0042, 0xe2},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    {0x00b6, 0xc0},
    {0x00b0, 0x68},
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x010e, 0x01},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    {0x0192, 0x02},
    {0x0194, 0x03},
    {0x0195, 0x04},
    {0x0196, 0x38},
    {0x0197, 0x07},
    {0x0198, 0x80},
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x5f},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x0027, 0x71},
    {0x0215, 0x92},
    {0x024d, 0x01},
    {0x001a, 0x9c},
    {0x005a, 0x00},
    {0x005b, 0x49},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_1080p_30fps_hdr[] = {
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x63},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},
    {0x0002, 0x02},
    {0x0003, 0x04},
    {0x0004, 0x02},
    {0x0005, 0x02},
    {0x0006, 0x94},
    {0x0007, 0x00},
    {0x0008, 0x11},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},
    {0x000e, 0x40},
    {0x000f, 0x07},
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},
    {0x0042, 0xe2},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    {0x00b6, 0xc0},
    {0x00b0, 0x68},
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x010e, 0x01},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    {0x0192, 0x02},
    {0x0194, 0x03},
    {0x0195, 0x04},
    {0x0196, 0x38},
    {0x0197, 0x07},
    {0x0198, 0x80},
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x5f},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x0027, 0x71},
    {0x0215, 0x92},
    {0x024d, 0x01},
    {0x001a, 0x9c},
    {0x005a, 0x00},
    {0x005b, 0x49},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};


static const k_sensor_reg gc2093_mipi2lane_1080p_30fps_linear[] = {
	//MCLK = 23.76MHz, PCLK = 47.52MHz = 2628 x 1206 x 29.98705/2
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0x00},
    {0x03f2,0x00},
    {0x03f3,0x00},
    {0x03f4,0x36},
    {0x03f5,0xc0},
    {0x03f6,0x0B},
    {0x03f7,0x11},
    {0x03f8,0x30},
    {0x03f9,0x42},
    {0x03fc,0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x05},	//line width = 0x522 = 1314 x 2 = 2628
    {0x0006, 0x22},
    {0x0007, 0x00},	//Vblank = 17
    {0x0008, 0x62},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},	//win_height = 1088
    {0x000e, 0x40},
    {0x000f, 0x07},	//win_width = 1932
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x04b6 = 1206
    {0x0042, 0xb6},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x40},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x30},
    {0x004a, 0x01},
    {0x004b, 0x28},
    {0x0055, 0x30},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0x65},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2 
    {0x0194, 0x03},	//out_win_x_off = 3 
    {0x0195, 0x04},	//out_win_height = 1080
    {0x0196, 0x38}, 
    {0x0197, 0x07},	//out_win_width = 1920
    {0x0198, 0x80}, 
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    { REG_NULL, 0x00 }
};

static const k_sensor_reg gc2093_mipi2lane_1080p_60fps_linear[] = {
    //MCLK = 23.76MHz, PCLK = 96.03MHz = 2628 x 1218 x 60/2
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x61},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x6e},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},	//win_height = 1088
    {0x000e, 0x40},
    {0x000f, 0x07},	//win_width = 1932
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x04c2 = 1218
    {0x0042, 0xc2},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x04},	//out_win_height = 1080
    {0x0196, 0x38},
    {0x0197, 0x07},	//out_win_width = 1920
    {0x0198, 0x80},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_960p_90fps_linear[] = {
    //PCLK = 126.72MHz = 2628 x 1072 x 89.96/2
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x7c},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x58},
    {0x0009, 0x00},	//y start = 0x3e = 62
    {0x000a, 0x3e},
    {0x000b, 0x02},	//x start = 0x144 = 324
    {0x000c, 0x88},
    {0x000d, 0x03},	//win_height = 964
    {0x000e, 0xc4},
    {0x000f, 0x05},	//win_width = 1288
    {0x0010, 0x08},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x0430= 1072
    {0x0042, 0x30},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x03},	//out_win_height = 960
    {0x0196, 0xc0},
    {0x0197, 0x05},	//out_win_width = 1280
    {0x0198, 0x00},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_720p_90fps_linear[] = {
    //MCLK = 23.76MHz, PCLK = 99MHz = 2628 x 837 x 90.015/2
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x64},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x5d},
    {0x0009, 0x00},	//y start = 0xb6 = 182
    {0x000a, 0xb6},
    {0x000b, 0x02},	//x start = 0x144 = 324
    {0x000c, 0x88},
    {0x000d, 0x02},	//win_height = 724
    {0x000e, 0xd4},
    {0x000f, 0x05},	//win_width = 1288
    {0x0010, 0x08},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x03},	// frame length = 0x0345 = 837
    {0x0042, 0x45},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x02},	//out_win_height = 720
    {0x0196, 0xd0},
    {0x0197, 0x05},	//out_win_width = 1280
    {0x0198, 0x00},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};


static const k_sensor_reg gc2093_mipi2lane_1080p_30fps_mclk_24m_linear[] = {
	//MCLK = 24MHz, PCLK = 48MHz = 2628 x 1218 x 29.99153/2
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0xf0},
    {0x03fe,0x00},
    {0x03f2,0x00},
    {0x03f3,0x00},
    {0x03f4,0x36},
    {0x03f5,0xc0},
    {0x03f6,0x0B},
    {0x03f7,0x11},
    {0x03f8,0x30},
    {0x03f9,0x42},
    {0x03fc,0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x05},	//line width = 0x522 = 1314 x 2 = 2628
    {0x0006, 0x22},
    {0x0007, 0x00},	//Vblank = 17
    {0x0008, 0x6e},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},	//win_height = 1088
    {0x000e, 0x40},
    {0x000f, 0x07},	//win_width = 1932
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x04c2 = 1218
    {0x0042, 0xc2},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x40},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x30},
    {0x004a, 0x01},
    {0x004b, 0x28},
    {0x0055, 0x30},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0x65},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2 
    {0x0194, 0x03},	//out_win_x_off = 3 
    {0x0195, 0x04},	//out_win_height = 1080
    {0x0196, 0x38}, 
    {0x0197, 0x07},	//out_win_width = 1920
    {0x0198, 0x80}, 
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    { REG_NULL, 0x00 }
};

static const k_sensor_reg gc2093_mipi2lane_1080p_60fps_mclk_24m_linear[] = {
    //MCLK = 24MHz, PCLK = 96MHz = 2628 x 1218 x 59.983/2
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x60},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x6e},
    {0x0009, 0x00},
    {0x000a, 0x02},
    {0x000b, 0x00},
    {0x000c, 0x04},
    {0x000d, 0x04},	//win_height = 1088
    {0x000e, 0x40},
    {0x000f, 0x07},	//win_width = 1932
    {0x0010, 0x8c},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x04c2 = 1218
    {0x0042, 0xc2},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x04},	//out_win_height = 1080
    {0x0196, 0x38},
    {0x0197, 0x07},	//out_win_width = 1920
    {0x0198, 0x80},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_960p_90fps_mclk_24m_linear[] = {
    //MCLK = 24MHz, PCLK = 120MHz = 2628 x 1074 x 85.03/2
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0d},	//refmp_div = 5
    {0x03f7, 0x01},
    {0x03f8, 0xc4},	//0xc9= 196(200)
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x5a},
    {0x0009, 0x00},	//y start = 0x3e = 62
    {0x000a, 0x3e},
    {0x000b, 0x02},	//x start = 0x144 = 324
    {0x000c, 0x88},
    {0x000d, 0x03},	//win_height = 964
    {0x000e, 0xc4},
    {0x000f, 0x05},	//win_width = 1288
    {0x0010, 0x08},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x04},	// frame length = 0x042b = 1074
    {0x0042, 0x32},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x03},	//out_win_height = 960
    {0x0196, 0xc0},
    {0x0197, 0x05},	//out_win_width = 1280
    {0x0198, 0x00},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

static const k_sensor_reg gc2093_mipi2lane_720p_90fps_mclk_24m_linear[] = {
    //MCLK = 24MHz, PCLK = 99MHz = 2628 x 837 x 90.015/2, MIPI clk = 792Mbps
    /****system****/
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03f2, 0x00},
    {0x03f3, 0x00},
    {0x03f4, 0x36},
    {0x03f5, 0xc0},
    {0x03f6, 0x0B},
    {0x03f7, 0x01},
    {0x03f8, 0x63},
    {0x03f9, 0x40},
    {0x03fc, 0x8e},
    /****CISCTL & ANALOG****/
    {0x0087, 0x18},
    {0x00ee, 0x30},
    {0x00d0, 0xbf},
    {0x01a0, 0x00},
    {0x01a4, 0x40},
    {0x01a5, 0x40},
    {0x01a6, 0x40},
    {0x01af, 0x09},
    {0x0001, 0x00},	//short frame ET
    {0x0002, 0x02},
    {0x0003, 0x00},	//ET
    {0x0004, 0x64},
    {0x0005, 0x02},	//line length = 0x291 = 657 x 4 = 2628
    {0x0006, 0x91},
    {0x0007, 0x00},	//VBlank = 17
    {0x0008, 0x5d},
    {0x0009, 0x00},	//y start = 0xb6 = 182
    {0x000a, 0xb6},
    {0x000b, 0x02},	//x start = 0x144 = 324
    {0x000c, 0x88},
    {0x000d, 0x02},	//win_height = 724
    {0x000e, 0xd4},
    {0x000f, 0x05},	//win_width = 1288
    {0x0010, 0x08},
    {0x0013, 0x15},
    {0x0019, 0x0c},
    {0x0041, 0x03},	// frame length = 0x0345 = 837
    {0x0042, 0x45},
    {0x0053, 0x60},
    {0x008d, 0x92},
    {0x0090, 0x00},
    {0x00c7, 0xe1},
    {0x001b, 0x73},
    {0x0028, 0x0d},
    {0x0029, 0x24},
    {0x002b, 0x04},
    {0x002e, 0x23},
    {0x0037, 0x03},
    {0x0043, 0x04},
    {0x0044, 0x28},
    {0x004a, 0x01},
    {0x004b, 0x20},
    {0x0055, 0x28},
    {0x0066, 0x3f},
    {0x0068, 0x3f},
    {0x006b, 0x44},
    {0x0077, 0x00},
    {0x0078, 0x20},
    {0x007c, 0xa1},
    {0x00ce, 0x7c},
    {0x00d3, 0xd4},
    {0x00e6, 0x50},
    /*gain*/
    {0x00b6, 0xc0},
    {0x00b0, 0x68},//0x60
    {0x00b3, 0x00},
    {0x00b8, 0x01},
    {0x00b9, 0x00},
    {0x00b1, 0x01},
    {0x00b2, 0x00},
    /*isp*/
    {0x0101, 0x0c},
    {0x0102, 0x89},
    {0x0104, 0x01},
    {0x0107, 0xa6},
    {0x0108, 0xa9},
    {0x0109, 0xa8},
    {0x010a, 0xa7},
    {0x010b, 0xff},
    {0x010c, 0xff},
    {0x010f, 0x00},
    {0x0158, 0x00},
    {0x0428, 0x86},
    {0x0429, 0x86},
    {0x042a, 0x86},
    {0x042b, 0x68},
    {0x042c, 0x68},
    {0x042d, 0x68},
    {0x042e, 0x68},
    {0x042f, 0x68},
    {0x0430, 0x4f},
    {0x0431, 0x68},
    {0x0432, 0x67},
    {0x0433, 0x66},
    {0x0434, 0x66},
    {0x0435, 0x66},
    {0x0436, 0x66},
    {0x0437, 0x66},
    {0x0438, 0x62},
    {0x0439, 0x62},
    {0x043a, 0x62},
    {0x043b, 0x62},
    {0x043c, 0x62},
    {0x043d, 0x62},
    {0x043e, 0x62},
    {0x043f, 0x62},
    /*dark sun*/
    {0x0123, 0x08},
    {0x0123, 0x00},
    {0x0120, 0x01},
    {0x0121, 0x04},
    {0x0122, 0xd8},
    {0x0124, 0x03},
    {0x0125, 0xff},
    {0x001a, 0x8c},
    {0x00c6, 0xe0},
    /*blk*/
    {0x0026, 0x30},
    {0x0142, 0x00},
    {0x0149, 0x1e},
    {0x014a, 0x0f},
    {0x014b, 0x00},
    {0x0155, 0x07},
    {0x0160, 0x10},	//WB_offset(dark offset)
    {0x0414, 0x78},
    {0x0415, 0x78},
    {0x0416, 0x78},
    {0x0417, 0x78},
    {0x0454, 0x78},
    {0x0455, 0x78},
    {0x0456, 0x78},
    {0x0457, 0x78},
    {0x04e0, 0x18},
    /*window*/
    {0x0192, 0x02},	//out_win_y_off = 2
    {0x0194, 0x03},	//out_win_x_off = 3
    {0x0195, 0x02},	//out_win_height = 720
    {0x0196, 0xd0},
    {0x0197, 0x05},	//out_win_width = 1280
    {0x0198, 0x00},
    /****DVP & MIPI****/
    {0x0199, 0x00},	//out window offset
    {0x019a, 0x06},
    {0x007b, 0x2a},
    {0x0023, 0x2d},
    {0x0201, 0x27},
    {0x0202, 0x56},
    {0x0203, 0xb6},
    {0x0212, 0x80},
    {0x0213, 0x07},
    {0x0215, 0x10},
    {0x003e, 0x91},
    {REG_NULL, 0x00},
};

// sensor ae info
static k_sensor_ae_info sensor_ae_info[] = {
     // list  for external  clk 23.76M
    // 1080P30 
    {
        .frame_length = 1206,
        .cur_frame_length = 1206,
        .one_line_exp_time = 0.000027652,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000027652,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1206 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000027652 * (1206 - 1),
        .min_integraion_time = 0.000027652 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 30,
    },
    // 1080P60 
    {
        .frame_length = 1218,
        .cur_frame_length = 1218,
        .one_line_exp_time = 0.000013683,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000013683,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1218 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000013683 * (1218 - 1),
        .min_integraion_time = 0.000013683 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 60,
    },
     // 960P 90fps
    {
        .frame_length = 1072,
        .cur_frame_length = 1072,
        .one_line_exp_time =0.000010369,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000010369,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1072 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000010369 * (1072 - 1),
        .min_integraion_time = 0.000010369 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 90,
    },
    //720P 90fps  
    {
        .frame_length = 837,
        .cur_frame_length = 837,
        .one_line_exp_time =0.000013273,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000013273,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 837 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000013273 * (837 - 1),
        .min_integraion_time = 0.000013273 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 90,
    },
    // list  for external  clk 24M
    {
        .frame_length = 1218,
        .cur_frame_length = 1218,
        .one_line_exp_time = 0.000027375,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000027375,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1218 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000027375 * (1218 - 1),
        .min_integraion_time = 0.000027375 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 30,
    },
    // 1080P60 
    {
        .frame_length = 1218,
        .cur_frame_length = 1218,
        .one_line_exp_time = 0.000013688,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000013688,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1218 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000013688 * (1218 - 1),
        .min_integraion_time = 0.000013688 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 60,
    },
     // 960P 90fps
    {
        .frame_length = 1074,
        .cur_frame_length = 1074,
        .one_line_exp_time =0.00001095,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.00001095,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 1074 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.00001095 * (1074 - 1),
        .min_integraion_time = 0.00001095 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
        .cur_fps = 90,
    },
    //720P 90fps  
    {
        .frame_length = 837,
        .cur_frame_length = 837,
        .one_line_exp_time =0.000013273,
        .gain_accuracy = 1024,
        .min_gain = 1,
        .max_gain = 18,
        .int_time_delay_frame = 2,
        .gain_delay_frame = 2,
        .color_type = SENSOR_COLOR,
        .integration_time_increment = 0.000013273,
        .gain_increment = GC2093_MIN_GAIN_STEP,
        .max_integraion_line = 837 - 1,
        .min_integraion_line = 1,
        .max_integraion_time = 0.000013273 * (837 - 1),
        .min_integraion_time = 0.000013273 * 1,
        .cur_integration_time = 0.0,
        .cur_again = 1.0,
        .cur_dgain = 1.0,
        .a_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/64.0f),
        },
        .d_gain = {
            .min = 1.0,
            .max = 63.984375,
            .step = (1.0f/1024.0f),
        },
    },
};

static k_sensor_mode gc2093_mode_info[] = {
    {
        .index = 0,
        .sensor_type = GC2093_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
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
        .bayer_pattern = BAYER_PAT_RGGB, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B,
        },
#if (BOARD_CSI2_CONN)
        .reg_list = gc2093_mipi2lane_1080p_30fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
#if defined(CONFIG_BOARD_K230_CANMV_RTT_EVB) 
                .setting.id = SENSOR_MCLK0,
#else
                .setting.id = SENSOR_MCLK2,
#endif
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[0],    
#else
        .reg_list = gc2093_mipi2lane_1080p_60fps_mclk_24m_linear,
        .mclk_setting = {
            {K_FALSE},
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[4],
#endif
    },
    {
        .index = 1,
        .sensor_type = GC2093_MIPI_CSI2_1920X1080_60FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1920,
            .bounds_height = 1080,
            .top = 0,
            .left = 0,
            .width = 1920,
            .height = 1080,
        },
        .fps = 60000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_RGGB, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B,
        },
#if (BOARD_CSI2_CONN)
        .reg_list = gc2093_mipi2lane_1080p_60fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[5],
#else
        .reg_list = gc2093_mipi2lane_1080p_60fps_mclk_24m_linear,
        .mclk_setting = {
            {K_FALSE},
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[5],
#endif
    },
    {
        .index = 2,
        .sensor_type = GC2093_MIPI_CSI2_1280X960_90FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 960,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 960,
        },
        .fps = 90000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_RGGB, //BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B,
        },
#if (BOARD_CSI2_CONN)
        .reg_list = gc2093_mipi2lane_960p_90fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
#if defined(CONFIG_BOARD_K230_CANMV_RTT_EVB) 
                .setting.id = SENSOR_MCLK0,
#else
                .setting.id = SENSOR_MCLK2,
#endif
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[2],
#else
        .reg_list = gc2093_mipi2lane_960p_90fps_mclk_24m_linear,
        .mclk_setting = {
            {K_FALSE},
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[6],
#endif
    },
    {
        .index = 3,
        .sensor_type = GC2093_MIPI_CSI2_1280X720_90FPS_10BIT_LINEAR,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 720,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 720,
        },
        .fps = 90000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 10,
        .bayer_pattern =BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2B,
        },
#if (BOARD_CSI2_CONN)
        .reg_list = gc2093_mipi2lane_720p_90fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK2,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 25,
            },
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[3],
#else
        .reg_list = gc2093_mipi2lane_720p_90fps_mclk_24m_linear,
        .mclk_setting = {
            {K_FALSE},
            {K_FALSE},
            {K_FALSE},
        },
        .sensor_ae_info = &sensor_ae_info[7],
#endif
    },
};

static k_bool gc2093_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int gc2093_power_rest(k_s32 on)
{
    // #define GC2093_CSI2_RST_GPIO     (0)  //24// 

    kd_pin_mode(GC2093_CSI2_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(GC2093_CSI2_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(GC2093_CSI2_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(GC2093_CSI2_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(GC2093_CSI2_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}

static int gc2093_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 gc2093_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    kd_pin_mode(GC2093_CSI2_RST_GPIO, GPIO_DM_OUTPUT);
    kd_pin_write(GC2093_CSI2_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH

    gc2093_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, GC2093_REG_ID, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, GC2093_REG_ID + 1, &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    // rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    if(*chip_id != 0x2093)
        ret = -1;

    return ret;
}


static k_s32 gc2093_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!gc2093_init_flag) {
            gc2093_power_rest(on);
            gc2093_i2c_init(&dev->i2c_info);
        // }
        ret = gc2093_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        gc2093_init_flag = K_FALSE;

        gc2093_i2c_init(&dev->i2c_info);
        sensor_reg_write(&dev->i2c_info, 0x03fe, 0xf0);

        gc2093_power_rest(on);
        
    }

    return ret;
}


static k_s32 gc2093_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(gc2093_mode_info) / sizeof(k_sensor_mode); i++) {
            if (gc2093_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(gc2093_mode_info[i]);
                dev->sensor_mode = &(gc2093_mode_info[i]);
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
        sensor_reg_list_write(&dev->i2c_info, gc2093_mirror);
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

    ret = sensor_reg_read(&dev->i2c_info, GC2093_REG_DGAIN_H, &again_h);
    ret = sensor_reg_read(&dev->i2c_info, GC2093_REG_DGAIN_L, &again_l);
    again = (float)(again_l)/64.0f + again_h;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    ret = sensor_reg_read(&dev->i2c_info, GC2093_REG_EXP_TIME_H, &exp_time_h);
    ret = sensor_reg_read(&dev->i2c_info, GC2093_REG_EXP_TIME_L, &exp_time_l);
    exp_time = ((exp_time_h & 0x3f) << 8) + exp_time_l;

    current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time *  exp_time;

    gc2093_init_flag = K_TRUE;
    return ret;
}


static k_s32 gc2093_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(gc2093_mode_info) / sizeof(k_sensor_mode); i++) {
        if (gc2093_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &gc2093_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(gc2093_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 gc2093_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 gc2093_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 gc2093_sensor_set_stream(void *ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        // ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x01);
    } else {
        // ret = sensor_reg_write(&dev->i2c_info, 0x0100, 0x00);
        sensor_reg_write(&dev->i2c_info, 0x03fe, 0xf0);
        sensor_reg_write(&dev->i2c_info, 0x03fe, 0xf0);
        sensor_reg_write(&dev->i2c_info, 0x03fe, 0xf0);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 gc2093_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

static k_u16 regValTable[25][7] = {
//   0xb3 0xb8 0xb9 0x155 0xc2 0xcf 0xd9
    {0x00,0x01,0x00,0x08,0x10,0x08,0x0a},
    {0x10,0x01,0x0c,0x08,0x10,0x08,0x0a},
    {0x20,0x01,0x1b,0x08,0x10,0x08,0x0a},
    {0x30,0x01,0x2c,0x08,0x11,0x08,0x0c},
    {0x40,0x01,0x3f,0x08,0x12,0x08,0x0e},
    {0x50,0x02,0x16,0x08,0x14,0x08,0x12},
    {0x60,0x02,0x35,0x08,0x15,0x08,0x14},
    {0x70,0x03,0x16,0x08,0x17,0x08,0x18},
    {0x80,0x04,0x02,0x08,0x18,0x08,0x1a},
    {0x90,0x04,0x31,0x08,0x19,0x08,0x1c},
    {0xa0,0x05,0x32,0x08,0x1b,0x08,0x20},
    {0xb0,0x06,0x35,0x08,0x1c,0x08,0x22},
    {0xc0,0x08,0x04,0x08,0x1e,0x08,0x26},
    {0x5a,0x09,0x19,0x08,0x1c,0x08,0x26},
    {0x83,0x0b,0x0f,0x08,0x1c,0x08,0x26},
    {0x93,0x0d,0x12,0x08,0x1f,0x08,0x28},
    {0x84,0x10,0x00,0x0b,0x20,0x08,0x2a},
    {0x94,0x12,0x3a,0x0b,0x22,0x08,0x2e},
    {0x5d,0x1a,0x02,0x0b,0x27,0x08,0x38},
    {0x9b,0x1b,0x20,0x0b,0x28,0x08,0x3a},
    {0x8c,0x20,0x0f,0x0b,0x2a,0x08,0x3e},
    {0x9c,0x26,0x07,0x12,0x2d,0x08,0x44},
    {0xB6,0x36,0x21,0x12,0x2d,0x08,0x44},
    {0xad,0x37,0x3a,0x12,0x2d,0x08,0x44},
    {0xbd,0x3d,0x02,0x12,0x2d,0x08,0x44},
};

static k_u32 gainLevelTable[26] = {
	64,
	76,
	91,
	107,
	125,
	147,
	177,
	211,
	248,
	297,
	356,
	425,
	504,
	599,
	709,
	836,
	978,
	1153,
	1647,
	1651,
	1935,
	2292,
	3239,
	3959,
	4686,
	0xffffffff,
};

static k_s32 gc2093_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 again, dgain, total;
    k_u8 i;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
		again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5);
		if(current_mode->sensor_again !=again)
        {
        	total = sizeof(gainLevelTable) / sizeof(k_u32);
			for (i = 0; i < total; i++)
			{
				if ((gainLevelTable[i] <= again) && (again < gainLevelTable[i + 1]))
				break;
			}
			dgain = (again <<6) / gainLevelTable[i];
			ret = sensor_reg_write(&dev->i2c_info, 0x00b3,regValTable[i][0]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b8,regValTable[i][1]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b9,regValTable[i][2]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x0155,regValTable[i][3]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x031d,0x2d);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00c2,regValTable[i][4]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00cf,regValTable[i][5]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00d9,regValTable[i][6]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x031d,0x28);

			ret |= sensor_reg_write(&dev->i2c_info, 0x00b1,(dgain>>6));
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b2,((dgain&0x3f)<<2));
			current_mode->sensor_again = again;
		}
		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/64.0f;
		
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 64 + 0.5);
		if(current_mode->sensor_again !=again)
        {
        	total = sizeof(gainLevelTable) / sizeof(k_u32);
			for (i = 0; i < total; i++)
			{
				if ((gainLevelTable[i] <= again) && (again < gainLevelTable[i + 1]))
				break;
			}
			dgain = (again <<6) / gainLevelTable[i];
			ret = sensor_reg_write(&dev->i2c_info, 0x00b3,regValTable[i][0]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b8,regValTable[i][1]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b9,regValTable[i][2]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x0155,regValTable[i][3]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x031d,0x2d);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00c2,regValTable[i][4]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00cf,regValTable[i][5]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x00d9,regValTable[i][6]);
			ret |= sensor_reg_write(&dev->i2c_info, 0x031d,0x28);

			ret |= sensor_reg_write(&dev->i2c_info, 0x00b1,(dgain>>6));
			ret |= sensor_reg_write(&dev->i2c_info, 0x00b2,((dgain&0x3f)<<2));
			current_mode->sensor_again = again;
		}
		current_mode->ae_info.cur_again = (float)current_mode->sensor_again/64.0f;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again*1000) );

    return ret;
}

static k_s32 gc2093_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 gc2093_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, GC2093_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, GC2093_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, GC2093_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, GC2093_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 gc2093_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 gc2093_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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
            ret |= sensor_reg_write(&dev->i2c_info, GC2093_REG_EXP_TIME_H, (exp_line >> 8) & 0x3f);
            ret |= sensor_reg_write(&dev->i2c_info, GC2093_REG_EXP_TIME_L, (exp_line ) & 0xff);
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

static k_s32 gc2093_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 gc2093_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 gc2093_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 gc2093_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 gc2093_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 gc2093_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 gc2093_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 gc2093_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;

    rt_kprintf("mirror mirror is %d , sensor tpye is %d \n", mirror.mirror, mirror.sensor_type);

    // get current sensor type
    for (k_s32 i = 0; i < sizeof(gc2093_mode_info) / sizeof(k_sensor_mode); i++) {
        if (gc2093_mode_info[i].sensor_type == mirror.sensor_type) {
            switch(mirror.mirror)
            {
                case VICAP_MIRROR_NONE :
                    gc2093_mode_info[i].bayer_pattern =  BAYER_PAT_RGGB;	//BAYER_PAT_BGGR;
                    mirror_flag = 0;
                    return 0;
                case VICAP_MIRROR_HOR :
                    // set mirror
                    gc2093_mirror[0].val = 0x01; //0x02;
                    // set sensor info bayer pattern 
                    gc2093_mode_info[i].bayer_pattern = BAYER_PAT_GRBG; //BAYER_PAT_GBRG;
                    break;
                case VICAP_MIRROR_VER :
                    // set mirror
                    gc2093_mirror[0].val = 0x02; //0x01;
                    // set sensor info bayer pattern 
                    gc2093_mode_info[i].bayer_pattern = BAYER_PAT_GBRG;// BAYER_PAT_GRBG;
                    break;
                case VICAP_MIRROR_BOTH :
                    // set mirror
                    gc2093_mirror[0].val = 0x03;
                    // set sensor info bayer pattern 
                    gc2093_mode_info[i].bayer_pattern = BAYER_PAT_BGGR;
                    break;
                default: 
                    rt_kprintf("mirror type is not support \n");
                    return -1;
                    break;
            }
            rt_kprintf("mirror_flag is gc2093_mirror[0].val %d", mirror_flag);
            mirror_flag = 1;
            return 0;
        }
    }
}


struct sensor_driver_dev gc2093_csi2_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = GC2093_CSI2_IIC, //"i2c3",   //"i2c0", //"i2c3",
        .slave_addr = GC2093_CSI2_SLAVE_ADDR,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "gc2093_csi2",
    .sensor_func = {
        .sensor_power = gc2093_sensor_power_on,
        .sensor_init = gc2093_sensor_init,
        .sensor_get_chip_id = gc2093_sensor_get_chip_id,
        .sensor_get_mode = gc2093_sensor_get_mode,
        .sensor_set_mode = gc2093_sensor_set_mode,
        .sensor_enum_mode = gc2093_sensor_enum_mode,
        .sensor_get_caps = gc2093_sensor_get_caps,
        .sensor_conn_check = gc2093_sensor_conn_check,
        .sensor_set_stream = gc2093_sensor_set_stream,
        .sensor_get_again = gc2093_sensor_get_again,
        .sensor_set_again = gc2093_sensor_set_again,
        .sensor_get_dgain = gc2093_sensor_get_dgain,
        .sensor_set_dgain = gc2093_sensor_set_dgain,
        .sensor_get_intg_time = gc2093_sensor_get_intg_time,
        .sensor_set_intg_time = gc2093_sensor_set_intg_time,
        .sensor_get_exp_parm = gc2093_sensor_get_exp_parm,
        .sensor_set_exp_parm = gc2093_sensor_set_exp_parm,
        .sensor_get_fps = gc2093_sensor_get_fps,
        .sensor_set_fps = gc2093_sensor_set_fps,
        .sensor_get_isp_status = gc2093_sensor_get_isp_status,
        .sensor_set_blc = gc2093_sensor_set_blc,
        .sensor_set_wb = gc2093_sensor_set_wb,
        .sensor_get_tpg = gc2093_sensor_get_tpg,
        .sensor_set_tpg = gc2093_sensor_set_tpg,
        .sensor_get_expand_curve = gc2093_sensor_get_expand_curve,
        .sensor_get_otp_data = gc2093_sensor_get_otp_data,
        .sensor_mirror_set = gc2093_sensor_mirror_set,
    },
};