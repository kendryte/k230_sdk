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

#define pr_info(...) //rt_kprintf(__VA_ARGS__)
#define pr_debug(...) //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)    rt_kprintf(__VA_ARGS__)

#define XS9950_REG_CHIP_ID_H    0x40f0
#define XS9950_REG_CHIP_ID_L    0x40f1

#define XS9950_REG_LONG_EXP_TIME_H    0x3501
#define XS9950_REG_LONG_EXP_TIME_L    0x3502

#define XS9950_REG_LONG_AGAIN	0x3509
//#define XS9950_REG_LONG_AGAIN_H    0x350a
//#define XS9950_REG_LONG_AGAIN_L    0x350b

#define XS9950_MIN_GAIN_STEP    (1.0f/16.0f)

#define MIPI_LAN_RATA_800
// #define MIPI_LAN_RATA_1000
// #define MIPI_LAN_RATA_1500

/*
mipi clk = 71.53 x 2  , 没有数据

*/

static const k_sensor_reg xs9950_mipi4lane_720p_25fps_linear[] = {
    //xs9950_mipi_init
    {0x4300, 0x05},
    {0x4300, 0x15},
    {0x4080, 0x07},
    {0x4119, 0x01},
    {0x0803, 0x00},
    {0x4020, 0x00},
    {0x080e, 0x00},
    {0x080e, 0x20},
    {0x080e, 0x28},
    {0x4020, 0x03},
    {0x0803, 0x1f},         //   zs default 0x0f
    {0x0100, 0x35},
    {0x0104, 0x48},
    {0x0300, 0x3f},
    {0x0105, 0xe1},
    {0x0101, 0x42},
    {0x0102, 0x40},
    {0x0116, 0x3c},
    {0x0117, 0x23},
    {0x0333, 0x23},         //0x23   zs default  0x09
    {0x0336, 0x9e}, 
    {0x0337, 0xd9},
    {0x0338, 0x0a},
    {0x01bf, 0x4e},
    {0x010e, 0x78},
    {0x010f, 0x92},
    {0x0110, 0x70},
    {0x0111, 0x40},      
    // {0x01e1, 0xff},         // zs  default  0xff
    {0x0314, 0x66},
    {0x0130, 0x10},
    {0x0315, 0x23},
    {0x0b64, 0x02},
    {0x01e2, 0x03},
    {0x0b55, 0x80},
    {0x0b56, 0x00},
    {0x0b59, 0x04},
    {0x0b5a, 0x01},
    {0x0b5c, 0x07},
    {0x0b5e, 0x05},
    {0x0b4b, 0x10},
    {0x0b4e, 0x05},
    {0x0b51, 0x21},
    {0x0b30, 0xbc},
    {0x0b31, 0x19},
    {0x0b15, 0x03},
    {0x0b16, 0x03},
    {0x0b17, 0x03},
    {0x0b07, 0x03},
    {0x0b08, 0x05},
    {0x0b1a, 0x10},
    {0x0158, 0x03},
    {0x0a88, 0x20},
    {0x0a61, 0x09},
    {0x0a62, 0x00},
    {0x0a63, 0x0e},
    {0x0a64, 0x00},
    {0x0a65, 0xfc},
    {0x0a67, 0xe5},
    {0x0a69, 0xef},
    {0x0a6b, 0x1b},
    {0x0a6d, 0x2f},
    {0x0a6f, 0x00},
    {0x0a71, 0xc2},
    {0x0a72, 0xff},
    {0x0a73, 0xd0},
    {0x0a74, 0xff},
    {0x0a75, 0x29},
    {0x0a77, 0x57},
    {0x0a78, 0x00},
    {0x0a79, 0x10},
    {0x0a7a, 0x00},
    {0x0a7b, 0xaa},
    {0x0a7d, 0xb2},
    {0x0a7f, 0x24},
    {0x0a80, 0x00},
    {0x0a81, 0x69},
    {0x0a82, 0x00},
    {0x0802, 0x02},
    {0x0501, 0x81},
    {0x0502, 0x0c},         //zs 0x0502 default 0x00
    {0x0b74, 0xfc},
    {0x01dc, 0x01},
    {0x0804, 0x04},
    {0x4018, 0x01},
    {0x0b56, 0x01},
    {0x0b73, 0x02},
    {0x4210, 0x0c},
    {0x420b, 0x2f},     // 上边是一样的

    {0x0504, 0x89},  // bit[4:7] free_run颜色, 取值范围:[0, 8]
    {0x0507, 0x13},     // color bar  bit[0] 彩条滚动
    {0x0503, 0x00},
    {0x0502, 0x0c},        //这几个是读出来的然后在做的配置  zs

    {0x015a, 0x00},         // test pattern config  
    {0x015b, 0x24},
    {0x015c, 0x80},
    {0x015d, 0x16},
    {0x015e, 0xd0},
    {0x015f, 0x02},
    {0x0160, 0xee},
    {0x0161, 0x02},
    {0x0165, 0x00},
    {0x0166, 0x0f},

    {0x4030, 0x15},
    {0x4134, 0x0a},             // zs  default  0x06
    {0x0803, 0x0f},
    {0x4412, 0x00},
    {0x0803, 0x1f},
    {0x10e3, 0x01},             // mipi output en
    {0x10eb, 0x04},             // clk datat lan en 

    {0x50fc, 0x00},             
    {0x50fd, 0x00},
    {0x50fe, 0x0d},
    {0x50ff, 0x59},

    {0x50e4, 0x00},             // lan num 0 :1lna   1 ；2lan   ????
    {0x50e5, 0x00},
    {0x50e6, 0x00},
    {0x50e7, 0x01},

    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x32},

    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x0f},

    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},

    {0x5048, 0x00},
    {0x5049, 0x00},
    {0x504a, 0x03},
    {0x504b, 0xff},

    {0x5058, 0x00},
    {0x5059, 0x00},
    {0x505a, 0x00},
    {0x505b, 0x07},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x00},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x04},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x07},
#ifdef MIPI_LAN_RATA_1500
    {0x1003, 0x01},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0x6f},         // mipi pll clk 
#endif

#ifdef MIPI_LAN_RATA_800
    {0x1003, 0x04},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0xed},         // mipi pll clk 
#endif

#ifdef MIPI_LAN_RATA_1000
    {0x1003, 0x01},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0x4a},         // mipi pll clk 
#endif
    {0x1001, 0xe4},
    {0x1000, 0x4d},
    {0x1001, 0xe0},
    {0x1020, 0x1e},
    {0x1020, 0x1f},
#ifdef MIPI_LAN_RATA_1500
    {0x1045, 0xcd},
    {0x1046, 0x0c},
    {0x1047, 0x36},
    {0x1048, 0x0f},
    {0x1049, 0x59},
    {0x104a, 0x07},
    {0x1050, 0xc4},
    {0x1065, 0xcd},
    {0x1066, 0x0c},
    {0x1067, 0x0e},
    {0x1068, 0x0f},
    {0x1069, 0x59},
    {0x106a, 0x07},
    {0x1070, 0xc4},
    {0x1085, 0xcd},
    {0x1086, 0x0c},
    {0x1087, 0x0e},
    {0x1088, 0x0f},
    {0x1089, 0x59},
    {0x108a, 0x07},
    {0x1090, 0xc4},
#endif

#ifdef MIPI_LAN_RATA_1000
    {0x1045, 0xc5},
    {0x1046, 0x40},
    {0x1047, 0x20},
    {0x1048, 0x30},
    {0x1049, 0x4f},
    {0x104a, 0x0a},
    {0x1050, 0x04},
    {0x1065, 0xc5},
    {0x1066, 0x40},
    {0x1067, 0x09},
    {0x1068, 0x30},
    {0x1069, 0x4f},
    {0x106a, 0x0a},
    {0x1070, 0x04},
    {0x1085, 0xc5},
    {0x1086, 0x40},
    {0x1087, 0x09},
    {0x1088, 0x30},
    {0x1089, 0x4f},
    {0x108a, 0x0a},
    {0x1090, 0x04},

#endif

#ifdef MIPI_LAN_RATA_800
    // mipi 800M
    {0x1045, 0xc5},
    {0x1046, 0x70},
    {0x1047, 0x1f},
    {0x1048, 0x18},
    {0x1049, 0x4c},
    {0x104a, 0x0a},
    {0x1050, 0x04},
    {0x1065, 0xc5},
    {0x1066, 0x70},
    {0x1067, 0x09},
    {0x1068, 0x18},
    {0x1069, 0x4c},
    {0x106a, 0x0a},
    {0x1070, 0x04},
    {0x1085, 0xc5},
    {0x1086, 0x70},
    {0x1087, 0x09},
    {0x1088, 0x18},
    {0x1089, 0x4c},
    {0x108a, 0x0a},
    {0x1090, 0x04},
#endif
    {0x1040, 0x0a},  // lane clk pn swap
    {0x1060, 0x0a},  // lane 0 pn swap
    {0x1080, 0x0a},  // lane 1 pn swap

    {0x5004, 0x00},     
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},

    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},

    // {0x5004, 0x00},      // zs  default  0x06
    // {0x5005, 0x00},

    {0x0800, 0x03},         // zs  default  0x05
    {0x0805, 0x07},         // zs  default  0x05

    {0x4200, 0x02},  // 0x4200=2：选择VINA，0x4200=0：选择VINB，0x4200=4：选择VINC，0x4200=6：选择VIND
    // {0x0111, 0x68},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4202, 0x0e},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4203, 0x04},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x0100, 0x38},  // 
    // {0x01ce, 0x00},  // 调整自动曝光 

#if 1
    // xs9950_AHD_720P_25f  
    {0x060b, 0x00},
    {0x0627, 0x14},
    {0x010c, 0x00},
    {0x0800, 0x05},
    {0x0805, 0x05},
    {0x0b50, 0x08},
    {0x0e08, 0x00},
    {0x010d, 0x40},
    {0x010c, 0x01},
    {0x0121, 0x7a},
    {0x0122, 0x6b},
    {0x0130, 0x10},
    {0x01a9, 0x00},
    {0x01aa, 0x04},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0105, 0xc1},  // AHD制式如发现颜色饱和度异常，可选择开关0x105的bit5
    {0x0101, 0x42},
    {0x0102, 0x40},
    {0x0116, 0x3c},
    {0x0117, 0x23},  // AHD制式如发现颜色饱和度异常，可选择打开0x105的bit5，并微调0x117
    {0x01e2, 0x03},
    {0x420b, 0x21},
    {0x0106, 0x80},
    {0x0107, 0x00},
    {0x0108, 0x80},
    {0x0109, 0x00},
    {0x010a, 0x20},
    {0x010b, 0x00},
    {0x011d, 0x17},
    {0x0e08, 0x01},
    {0x0a60, 0x04},
    {0x0a5c, 0xf6},
    {0x0a5d, 0xc0},
    {0x0a5e, 0x2d},
    {0x0a5f, 0x1b},
    {0x0156, 0x50},
    {0x0157, 0x07},
    {0x011d, 0x17},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0503, 0x00},
    {0x015a, 0x8b},
    {0x015b, 0x0e},
    {0x015c, 0x80},
    {0x015d, 0x16},
    {0x015e, 0xd0},
    {0x015f, 0x02},
    {0x0160, 0xee},
    {0x0161, 0x02},
    {0x0165, 0x40},
    {0x0166, 0x0f},
    {0x0A00, 0xFD},
    {0x0A01, 0xFF},
    {0x0A02, 0x00},
    {0x0A03, 0x00},
    {0x0A04, 0x04},
    {0x0A05, 0x00},
    {0x0A06, 0x01},
    {0x0A07, 0x00},
    {0x0A08, 0xFB},
    {0x0A09, 0xFF},
    {0x0A0A, 0xFE},
    {0x0A0B, 0xFF},
    {0x0A0C, 0x07},
    {0x0A0D, 0x00},
    {0x0A0E, 0x03},
    {0x0A0F, 0x00},
    {0x0A10, 0xF7},
    {0x0A11, 0xFF},
    {0x0A12, 0xFA},
    {0x0A13, 0xFF},
    {0x0A14, 0x0B},
    {0x0A15, 0x00},
    {0x0A16, 0x0A},
    {0x0A17, 0x00},
    {0x0A18, 0xF3},
    {0x0A19, 0xFF},
    {0x0A1A, 0xF1},
    {0x0A1B, 0xFF},
    {0x0A1C, 0x0F},
    {0x0A1D, 0x00},
    {0x0A1E, 0x18},
    {0x0A1F, 0x00},
    {0x0A20, 0xEE},
    {0x0A21, 0xFF},
    {0x0A22, 0xDC},
    {0x0A23, 0xFF},
    {0x0A24, 0x14},
    {0x0A25, 0x00},
    {0x0A26, 0x39},
    {0x0A27, 0x00},
    {0x0A28, 0xEB},
    {0x0A29, 0xFF},
    {0x0A2A, 0x98},
    {0x0A2B, 0xFF},
    {0x0A2C, 0x16},
    {0x0A2D, 0x00},
    {0x0A2E, 0x45},
    {0x0A2F, 0x01},
    {0x0A30, 0xEA},
    {0x0A31, 0x01},
    {0x0A60, 0x01},   
    /*针对某些非标AHD72如下配置*/
    {0x0AA7, 0x7A}, 
    {0x0AA8, 0x18}, 	
    {0x0AA9, 0xC0}, 
    {0x0AAA, 0x01}, 
    {0x0AAB, 0xC2}, 
    {0x0AAC, 0x01}, 	
    {0x0AAD, 0x80}, 
    {0x0AAE, 0x43}, 
    {0x0AAF, 0x00}, 	
    {0x0AB0, 0x70}, 
    {0x0AB1, 0x00}, 
    {0x0AB2, 0x1B},	
    {0x0A88, 0x30},	

    //xs9950_mipi_reset_new
    {0x0e08, 0x01},
    // {0x1e08, 0x01},
    // {0x2e08, 0x01},
    // {0x3e08, 0x01},
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x00},

    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},
#endif
    {REG_NULL, 0x0},
};

static const k_sensor_reg xs9950_mipi4lane_1080p_25fps_linear[] = {
    //xs9950_mipi_init
    {0x4300, 0x05},
    {0x4300, 0x15},
    {0x4080, 0x07},
    {0x4119, 0x01},
    {0x0803, 0x00},
    {0x4020, 0x00},
    {0x080e, 0x00},
    {0x080e, 0x20},
    {0x080e, 0x28},
    {0x4020, 0x03},
    {0x0803, 0x1f},         //   zs default 0x0f
    {0x0100, 0x35},
    {0x0104, 0x48},
    {0x0300, 0x3f},
    {0x0105, 0xe1},
    {0x0101, 0x42},
    {0x0102, 0x40},
    {0x0116, 0x3c},
    {0x0117, 0x23},
    {0x0333, 0x23},         //0x23   zs default  0x09
    {0x0336, 0x9e}, 
    {0x0337, 0xd9},
    {0x0338, 0x0a},
    {0x01bf, 0x4e},
    {0x010e, 0x78},
    {0x010f, 0x92},
    {0x0110, 0x70},
    {0x0111, 0x40},      
    // {0x01e1, 0xff},         // zs  default  0xff
    {0x0314, 0x66},
    {0x0130, 0x10},
    {0x0315, 0x23},
    {0x0b64, 0x02},
    {0x01e2, 0x03},
    {0x0b55, 0x80},
    {0x0b56, 0x00},
    {0x0b59, 0x04},
    {0x0b5a, 0x01},
    {0x0b5c, 0x07},
    {0x0b5e, 0x05},
    {0x0b4b, 0x10},
    {0x0b4e, 0x05},
    {0x0b51, 0x21},
    {0x0b30, 0xbc},
    {0x0b31, 0x19},
    {0x0b15, 0x03},
    {0x0b16, 0x03},
    {0x0b17, 0x03},
    {0x0b07, 0x03},
    {0x0b08, 0x05},
    {0x0b1a, 0x10},
    {0x0158, 0x03},
    {0x0a88, 0x20},
    {0x0a61, 0x09},
    {0x0a62, 0x00},
    {0x0a63, 0x0e},
    {0x0a64, 0x00},
    {0x0a65, 0xfc},
    {0x0a67, 0xe5},
    {0x0a69, 0xef},
    {0x0a6b, 0x1b},
    {0x0a6d, 0x2f},
    {0x0a6f, 0x00},
    {0x0a71, 0xc2},
    {0x0a72, 0xff},
    {0x0a73, 0xd0},
    {0x0a74, 0xff},
    {0x0a75, 0x29},
    {0x0a77, 0x57},
    {0x0a78, 0x00},
    {0x0a79, 0x10},
    {0x0a7a, 0x00},
    {0x0a7b, 0xaa},
    {0x0a7d, 0xb2},
    {0x0a7f, 0x24},
    {0x0a80, 0x00},
    {0x0a81, 0x69},
    {0x0a82, 0x00},
    {0x0802, 0x02},
    {0x0501, 0x81},
    {0x0502, 0x0c},         //zs 0x0502 default 0x00
    {0x0b74, 0xfc},
    {0x01dc, 0x01},
    {0x0804, 0x04},
    {0x4018, 0x01},
    {0x0b56, 0x01},
    {0x0b73, 0x02},
    {0x4210, 0x0c},
    {0x420b, 0x2f},     // 上边是一样的

    {0x0504, 0x89},  // bit[4:7] free_run颜色, 取值范围:[0, 8]
    {0x0507, 0x13},     // color bar  bit[0] 彩条滚动
    {0x0503, 0x00},
    {0x0502, 0x0c},        //这几个是读出来的然后在做的配置  zs

    {0x015a, 0x00},         // test pattern config  
    {0x015b, 0x24},
    {0x015c, 0x80},
    {0x015d, 0x16},
    {0x015e, 0xd0},
    {0x015f, 0x02},
    {0x0160, 0xee},
    {0x0161, 0x02},
    {0x0165, 0x00},
    {0x0166, 0x0f},

    {0x4030, 0x15},
    {0x4134, 0x0a},             // zs  default  0x06
    {0x0803, 0x0f},
    {0x4412, 0x00},
    {0x0803, 0x1f},
    {0x10e3, 0x01},             // mipi output en
    {0x10eb, 0x04},             // clk datat lan en 

    {0x50fc, 0x00},             
    {0x50fd, 0x00},
    {0x50fe, 0x0d},
    {0x50ff, 0x59},

    {0x50e4, 0x00},             // lan num 0 :1lna   1 ；2lan   ????
    {0x50e5, 0x00},
    {0x50e6, 0x00},
    {0x50e7, 0x01},

    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x32},

    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x0f},

    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},

    {0x5048, 0x00},
    {0x5049, 0x00},
    {0x504a, 0x03},
    {0x504b, 0xff},

    {0x5058, 0x00},
    {0x5059, 0x00},
    {0x505a, 0x00},
    {0x505b, 0x07},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x00},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x04},

    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x07},
#ifdef MIPI_LAN_RATA_1500
    {0x1003, 0x01},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0x6f},         // mipi pll clk 
#endif

#ifdef MIPI_LAN_RATA_800
    {0x1003, 0x04},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0xed},         // mipi pll clk 
#endif

#ifdef MIPI_LAN_RATA_1000
    {0x1003, 0x01},         // mipi pll clk     016f :1.5g      800: 4ed      
    {0x1004, 0x4a},         // mipi pll clk 
#endif
    {0x1001, 0xe4},
    {0x1000, 0x4d},
    {0x1001, 0xe0},
    {0x1020, 0x1e},
    {0x1020, 0x1f},
#ifdef MIPI_LAN_RATA_1500
    {0x1045, 0xcd},
    {0x1046, 0x0c},
    {0x1047, 0x36},
    {0x1048, 0x0f},
    {0x1049, 0x59},
    {0x104a, 0x07},
    {0x1050, 0xc4},
    {0x1065, 0xcd},
    {0x1066, 0x0c},
    {0x1067, 0x0e},
    {0x1068, 0x0f},
    {0x1069, 0x59},
    {0x106a, 0x07},
    {0x1070, 0xc4},
    {0x1085, 0xcd},
    {0x1086, 0x0c},
    {0x1087, 0x0e},
    {0x1088, 0x0f},
    {0x1089, 0x59},
    {0x108a, 0x07},
    {0x1090, 0xc4},
#endif

#ifdef MIPI_LAN_RATA_1000
    {0x1045, 0xc5},
    {0x1046, 0x40},
    {0x1047, 0x20},
    {0x1048, 0x30},
    {0x1049, 0x4f},
    {0x104a, 0x0a},
    {0x1050, 0x04},
    {0x1065, 0xc5},
    {0x1066, 0x40},
    {0x1067, 0x09},
    {0x1068, 0x30},
    {0x1069, 0x4f},
    {0x106a, 0x0a},
    {0x1070, 0x04},
    {0x1085, 0xc5},
    {0x1086, 0x40},
    {0x1087, 0x09},
    {0x1088, 0x30},
    {0x1089, 0x4f},
    {0x108a, 0x0a},
    {0x1090, 0x04},

#endif

#ifdef MIPI_LAN_RATA_800
    // mipi 800M
    {0x1045, 0xc5},
    {0x1046, 0x70},
    {0x1047, 0x1f},
    {0x1048, 0x18},
    {0x1049, 0x4c},
    {0x104a, 0x0a},
    {0x1050, 0x04},
    {0x1065, 0xc5},
    {0x1066, 0x70},
    {0x1067, 0x09},
    {0x1068, 0x18},
    {0x1069, 0x4c},
    {0x106a, 0x0a},
    {0x1070, 0x04},
    {0x1085, 0xc5},
    {0x1086, 0x70},
    {0x1087, 0x09},
    {0x1088, 0x18},
    {0x1089, 0x4c},
    {0x108a, 0x0a},
    {0x1090, 0x04},
#endif
    {0x1040, 0x0a},  // lane clk pn swap
    {0x1060, 0x0a},  // lane 0 pn swap
    {0x1080, 0x0a},  // lane 1 pn swap

    {0x5004, 0x00},     
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},

    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},

    // {0x5004, 0x00},      // zs  default  0x06
    // {0x5005, 0x00},

    {0x0800, 0x03},         // zs  default  0x05
    {0x0805, 0x07},         // zs  default  0x05

    {0x4200, 0x02},  // 0x4200=2：选择VINA，0x4200=0：选择VINB，0x4200=4：选择VINC，0x4200=6：选择VIND
    // {0x0111, 0x68},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4202, 0x0e},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4203, 0x04},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x0100, 0x38},  // 
    // {0x01ce, 0x00},  // 调整自动曝光 

#if 1

#if 1
    //xs9950_AHD_1080P_25f
    {0x060b, 0x00},
    {0x0627, 0x14},
    {0x010c, 0x00},
    {0x0800, 0x05},
    {0x0805, 0x05},
    {0x0b50, 0x08},
    {0x0e08, 0x00},
    {0x010d, 0x44},
    {0x010c, 0x01},
    {0x0121, 0x7a},
    {0x0122, 0x6b},
    {0x0130, 0x10},
    {0x01a9, 0x00},
    {0x01aa, 0x04},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0105, 0xc1},
    {0x0101, 0x42},
    {0x0102, 0x40},
    {0x0116, 0x3c},
    {0x0117, 0x23},
    {0x01e2, 0x03},
    {0x420b, 0x21},
    {0x0106, 0x80},
    {0x0107, 0x00},
    {0x0108, 0x80},
    {0x0109, 0x00},
    {0x010a, 0x1b},
    {0x010b, 0x01},
    {0x011d, 0x17},
    {0x0e08, 0x01},
    {0x0a60, 0x04},
    {0x0a5c, 0x56},
    {0x0a5d, 0x00},
    {0x0a5e, 0xe7},
    {0x0a5f, 0x38},
    {0x0156, 0x50},
    {0x0157, 0x07},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0503, 0x04},
    {0x015a, 0xd1},
    {0x015b, 0x15},
    {0x015c, 0x00},
    {0x015d, 0x0f},
    {0x015e, 0x38},
    {0x015f, 0x04},
    {0x0160, 0x65},
    {0x0161, 0x04},
    {0x0165, 0x44},
    {0x0166, 0x0f},
    {0x0A00, 0x00},
    {0x0A01, 0x00},
    {0x0A02, 0xFE},
    {0x0A03, 0xFF},
    {0x0A04, 0x04},
    {0x0A05, 0x00},
    {0x0A06, 0xFD},
    {0x0A07, 0xFF},
    {0x0A08, 0x00},
    {0x0A09, 0x00},
    {0x0A0A, 0x04},
    {0x0A0B, 0x00},
    {0x0A0C, 0xF9},
    {0x0A0D, 0xFF},
    {0x0A0E, 0x06},
    {0x0A0F, 0x00},
    {0x0A10, 0x00},
    {0x0A11, 0x00},
    {0x0A12, 0xF8},
    {0x0A13, 0xFF},
    {0x0A14, 0x0E},
    {0x0A15, 0x00},
    {0x0A16, 0xF5},
    {0x0A17, 0xFF},
    {0x0A18, 0x00},
    {0x0A19, 0x00},
    {0x0A1A, 0x0F},
    {0x0A1B, 0x00},
    {0x0A1C, 0xE7},
    {0x0A1D, 0xFF},
    {0x0A1E, 0x14},
    {0x0A1F, 0x00},
    {0x0A20, 0x00},
    {0x0A21, 0x00},
    {0x0A22, 0xE3},
    {0x0A23, 0xFF},
    {0x0A24, 0x31},
    {0x0A25, 0x00},
    {0x0A26, 0xD5},
    {0x0A27, 0xFF},
    {0x0A28, 0x00},
    {0x0A29, 0x00},
    {0x0A2A, 0x4B},
    {0x0A2B, 0x00},
    {0x0A2C, 0x5F},
    {0x0A2D, 0xFF},
    {0x0A2E, 0xE6},
    {0x0A2F, 0x00},
    {0x0A30, 0x00},
    {0x0A31, 0x03},
    {0x0A60, 0x01},
#else
    //xs9950_AHD_1080P_30f
    {0x060b, 0x00},
    {0x0627, 0x14},
    {0x010c, 0x00},
    {0x0800, 0x05},
    {0x0805, 0x05},
    {0x0b50, 0x08},
    {0x0e08, 0x00},
    {0x010d, 0x45},
    {0x010c, 0x01},
    {0x0121, 0x7a},
    {0x0122, 0x6b},
    {0x0130, 0x10},
    {0x01a9, 0x00},
    {0x01aa, 0x04},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0105, 0xc1},
    {0x0101, 0x42},
    {0x0102, 0x40},
    {0x0116, 0x3c},
    {0x0117, 0x23},
    {0x01e2, 0x03},
    {0x420b, 0x21},
    {0x0106, 0x80},
    {0x0107, 0x00},
    {0x0108, 0x80},
    {0x0109, 0x00},
    {0x010a, 0x1b},
    {0x010b, 0x01},
    {0x011d, 0x17},
    {0x0e08, 0x01},
    {0x0a60, 0x04},
    {0x0a5c, 0x3c},
    {0x0a5d, 0x10},
    {0x0a5e, 0xec},
    {0x0a5f, 0x38},
    {0x0156, 0x50},
    {0x0157, 0x07},
    {0x0156, 0x00},
    {0x0157, 0x08},
    {0x0503, 0x05},
    {0x015a, 0xd1},
    {0x015b, 0x15},
    {0x015c, 0x80},
    {0x015d, 0x0c},
    {0x015e, 0x38},
    {0x015f, 0x04},
    {0x0160, 0x65},
    {0x0161, 0x04},
    {0x0165, 0x45},
    {0x0166, 0x0f},
    {0x0A00, 0x00},
    {0x0A01, 0x00},
    {0x0A02, 0xFE},
    {0x0A03, 0xFF},
    {0x0A04, 0x04},
    {0x0A05, 0x00},
    {0x0A06, 0xFD},
    {0x0A07, 0xFF},
    {0x0A08, 0x00},
    {0x0A09, 0x00},
    {0x0A0A, 0x04},
    {0x0A0B, 0x00},
    {0x0A0C, 0xF9},
    {0x0A0D, 0xFF},
    {0x0A0E, 0x06},
    {0x0A0F, 0x00},
    {0x0A10, 0x00},
    {0x0A11, 0x00},
    {0x0A12, 0xF8},
    {0x0A13, 0xFF},
    {0x0A14, 0x0E},
    {0x0A15, 0x00},
    {0x0A16, 0xF5},
    {0x0A17, 0xFF},
    {0x0A18, 0x00},
    {0x0A19, 0x00},
    {0x0A1A, 0x0F},
    {0x0A1B, 0x00},
    {0x0A1C, 0xE7},
    {0x0A1D, 0xFF},
    {0x0A1E, 0x14},
    {0x0A1F, 0x00},
    {0x0A20, 0x00},
    {0x0A21, 0x00},
    {0x0A22, 0xE3},
    {0x0A23, 0xFF},
    {0x0A24, 0x31},
    {0x0A25, 0x00},
    {0x0A26, 0xD5},
    {0x0A27, 0xFF},
    {0x0A28, 0x00},
    {0x0A29, 0x00},
    {0x0A2A, 0x4B},
    {0x0A2B, 0x00},
    {0x0A2C, 0x5F},
    {0x0A2D, 0xFF},
    {0x0A2E, 0xE6},
    {0x0A2F, 0x00},
    {0x0A30, 0x00},
    {0x0A31, 0x03},
    {0x0A60, 0x01},

#endif
    //xs9950_mipi_reset_new
    {0x0e08, 0x01},
    // {0x1e08, 0x01},
    // {0x2e08, 0x01},
    // {0x3e08, 0x01},
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x00},

    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},
#endif
    {REG_NULL, 0x0},
};


static k_sensor_mode xs9950_mode_info[] = {
    {
        .index = 0,
        .sensor_type = XS9950_MIPI_CSI0_1280X720_30FPS_YUV422,
        .size = {
            .bounds_width = 1280,
            .bounds_height = 720,
            .top = 0,
            .left = 0,
            .width = 1280,
            .height = 720,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .stitching_mode = SENSOR_STITCHING_3DOL,
        .bit_width = 8,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x1e,
        },
        .reg_list = xs9950_mipi4lane_720p_25fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_FALSE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },

    {
        .index = 1,
        .sensor_type = XS9950_MIPI_CSI0_1920X1080_30FPS_YUV422,
        .size = {
            .bounds_width = 1920,
            .bounds_height = 1080,
            .top = 0,
            .left = 0,
            .width = 1920,
            .height = 1080,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .stitching_mode = SENSOR_STITCHING_3DOL,
        .bit_width = 8,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x1e,
        },
        .reg_list = xs9950_mipi4lane_1080p_25fps_linear,
        .mclk_setting = {
            {
                .mclk_setting_en = K_FALSE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL0_CLK_DIV4,
                .setting.mclk_div = 16,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
};

static k_bool xs9950_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int xs9950_power_rest(k_s32 on)
{
    #define VICAP_XS9950_RST_GPIO     (0)  //24// 

    kd_pin_mode(VICAP_XS9950_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_XS9950_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_XS9950_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_XS9950_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(VICAP_XS9950_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}


static int xs9950_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 xs9950_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    xs9950_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, XS9950_REG_CHIP_ID_L, &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}



static k_s32 xs9950_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!xs9950_init_flag) {
            xs9950_power_rest(on);
            xs9950_i2c_init(&dev->i2c_info);
        // }
        ret = xs9950_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        xs9950_init_flag = K_FALSE;
        xs9950_power_rest(on);
    }

    return ret;
}

static k_s32 xs9950_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(xs9950_mode_info) / sizeof(k_sensor_mode); i++) {
            if (xs9950_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(xs9950_mode_info[i]);
                dev->sensor_mode = &(xs9950_mode_info[i]);
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
        current_mode->ae_info.gain_increment = XS9950_MIN_GAIN_STEP;

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
        if (!xs9950_init_flag) {
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
        current_mode->ae_info.gain_increment = XS9950_MIN_GAIN_STEP;

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

    k_u16 again_h;
    k_u16 again_l;
    k_u16 exp_time_h, exp_time_l;
    k_u16 exp_time;
    float again = 0, dgain = 0;

    // ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_LONG_AGAIN, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_LONG_AGAIN_H, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_LONG_AGAIN_L, &again_l);
    again = 1.0;// (float)again_h / 16.0f;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    // ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_LONG_EXP_TIME_H, &exp_time_h);
    // ret = sensor_reg_read(&dev->i2c_info, XS9950_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) | ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;
    xs9950_init_flag = K_TRUE;

    return ret;
}


static k_s32 xs9950_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(xs9950_mode_info) / sizeof(k_sensor_mode); i++) {
        if (xs9950_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &xs9950_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(xs9950_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

static k_s32 xs9950_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

static k_s32 xs9950_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 xs9950_sensor_set_stream(void *ctx, k_s32 enable)
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

static k_s32 xs9950_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

static k_s32 xs9950_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16 + 0.5);
        //if(current_mode->sensor_again !=again)
        {
	        // ret = sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN,(again & 0xff));
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/16.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS]* 16 + 0.5);
        // ret = sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN,(again & 0xff));
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

static k_s32 xs9950_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

static k_s32 xs9950_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_AGAIN_L,(again & 0xff));
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

static k_s32 xs9950_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

static k_s32 xs9950_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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
	        // ret |= sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_EXP_TIME_H, ( exp_line >> 4) & 0xff);
	        // ret |= sensor_reg_write(&dev->i2c_info, XS9950_REG_LONG_EXP_TIME_L, ( exp_line << 4) & 0xf0);
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

static k_s32 xs9950_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 xs9950_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

static k_s32 xs9950_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 xs9950_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 xs9950_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

static k_s32 xs9950_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 xs9950_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 xs9950_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev xs9950_csi0_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c3",   //"i2c0", //"i2c3",
        .slave_addr = 0x30,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "xs9950_csi0",
    .sensor_func = {
        .sensor_power = xs9950_sensor_power_on,
        .sensor_init = xs9950_sensor_init,
        .sensor_get_chip_id = xs9950_sensor_get_chip_id,
        .sensor_get_mode = xs9950_sensor_get_mode,
        .sensor_set_mode = xs9950_sensor_set_mode,
        .sensor_enum_mode = xs9950_sensor_enum_mode,
        .sensor_get_caps = xs9950_sensor_get_caps,
        .sensor_conn_check = xs9950_sensor_conn_check,
        .sensor_set_stream = xs9950_sensor_set_stream,
        .sensor_get_again = xs9950_sensor_get_again,
        .sensor_set_again = xs9950_sensor_set_again,
        .sensor_get_dgain = xs9950_sensor_get_dgain,
        .sensor_set_dgain = xs9950_sensor_set_dgain,
        .sensor_get_intg_time = xs9950_sensor_get_intg_time,
        .sensor_set_intg_time = xs9950_sensor_set_intg_time,
        .sensor_get_exp_parm = xs9950_sensor_get_exp_parm,
        .sensor_set_exp_parm = xs9950_sensor_set_exp_parm,
        .sensor_get_fps = xs9950_sensor_get_fps,
        .sensor_set_fps = xs9950_sensor_set_fps,
        .sensor_get_isp_status = xs9950_sensor_get_isp_status,
        .sensor_set_blc = xs9950_sensor_set_blc,
        .sensor_set_wb = xs9950_sensor_set_wb,
        .sensor_get_tpg = xs9950_sensor_get_tpg,
        .sensor_set_tpg = xs9950_sensor_set_tpg,
        .sensor_get_expand_curve = xs9950_sensor_get_expand_curve,
        .sensor_get_otp_data = xs9950_sensor_get_otp_data,
        .sensor_mirror_set = xs9950_sensor_mirror_set,
    },
};
