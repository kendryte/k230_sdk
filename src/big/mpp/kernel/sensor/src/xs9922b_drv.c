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

#define pr_info(...) rt_kprintf(__VA_ARGS__)
#define pr_debug(...) //rt_kprintf(__VA_ARGS__)
#define pr_warn(...)    //rt_kprintf(__VA_ARGS__)
#define pr_err(...)    rt_kprintf(__VA_ARGS__)

#define XS9922B_REG_CHIP_ID_H    0x40f0
#define XS9922B_REG_CHIP_ID_L    0x40f1

#define XS9922B_REG_LONG_EXP_TIME_H    0x3501
#define XS9922B_REG_LONG_EXP_TIME_L    0x3502

#define XS9922B_REG_LONG_AGAIN	0x3509
//#define XS9922B_REG_LONG_AGAIN_H    0x350a
//#define XS9922B_REG_LONG_AGAIN_L    0x350b

#define XS9922B_MIN_GAIN_STEP    (1.0f/16.0f)


/*
iic ：
slave addr :  0x30
iic 电压： k230 1.8 , xs9922 : 3.3 

mipi：
line num :  4
mipi rate : 1.2G
mipi 连续模式：  连续模式

通道1： 手动模式（0x010c）,     default 关闭 (0x0e08)
通道2： 手动模式（0x110c）,     default 关闭 (0x1e08)
通道3： 手动模式（0x210c）,     default 关闭 (0x2e08)
通道4： 手动模式（0x310c）,     default 关闭 (0x3e08)

彩条模式 ： default ： open 0x06。  不出彩条设置成0x0

帧号发送方式 :  1、 2、 3、 4轮流发送
备注：
rst 需要拉低100ms，在拉高
*/
#if 1
static const k_sensor_reg xs9922b_mipi4lane_720p_30fps_linear[] = {

    // {0x4082, 0x03}, iic burst模式
    {0x0e08, 0x00},
    {0x1e08, 0x00},
    {0x2e08, 0x00},
    {0x3e08, 0x00},
    //{0x4500, 0x02},
    {0x4f00, 0x01},
    {0x4030, 0x3f},
    {0x0e02, 0x00},
    {0x1e02, 0x00},
    {0x2e02, 0x00},
    {0x3e02, 0x00},
    {0x0803, 0x00},
    {0x1803, 0x00},
    {0x2803, 0x00},
    {0x3803, 0x00},
    {0x4020, 0x00},
    {0x080e, 0x00},
    {0x080e, 0x20},
    {0x080e, 0x28},
    {0x180e, 0x00},
    {0x180e, 0x20},
    {0x180e, 0x28},
    {0x280e, 0x00},
    {0x280e, 0x20},
    {0x280e, 0x28},
    {0x380e, 0x00},
    {0x380e, 0x20},
    {0x380e, 0x28},
    {0x4020, 0x03},
    {0x0803, 0x0f},
    {0x1803, 0x0f},
    {0x2803, 0x0f},
    {0x3803, 0x0f},
#if RM_STRIPES
    {0x0800, 0x16},
    {0x1800, 0x16},
    {0x2800, 0x16},
    {0x3800, 0x16},
    {0x4205, 0x36},
    {0x4215, 0x36},
    {0x4225, 0x36},
    {0x4235, 0x36},//wait(0.05)
    {0x4205, 0x26},
    {0x4215, 0x26},
    {0x4225, 0x26},
    {0x4235, 0x26},
    {0x0800, 0x17},
    {0x1800, 0x17},
    {0x2800, 0x17},
    {0x3800, 0x17},
    #endif
    {0x4340, 0x65},
    {0x4204, 0x02},
    {0x4214, 0x02},
    {0x4224, 0x02},
    {0x4234, 0x02},
    {0x4080, 0x07},
    {0x4119, 0x01},
    {0x0501, 0x84},
#if USE_YUVGAIN
    {0x010e, Y_GAIN},
    {0x010f, U_GAIN},
    {0x0110, V_GAIN},
#endif
    {0x0111, 0x40},
    {0x1501, 0x81},
#if USE_YUVGAIN
    {0x110e, Y_GAIN},
    {0x110f, U_GAIN},
    {0x1110, V_GAIN},
#endif
    {0x1111, 0x40},
    {0x2501, 0x8e},
#if USE_YUVGAIN
    {0x210e, Y_GAIN},
    {0x210f, U_GAIN},
    {0x2110, V_GAIN},
#endif
    {0x2111, 0x40},
    {0x3501, 0x8b},
#if USE_YUVGAIN
    {0x310e, Y_GAIN},
    {0x310f, U_GAIN},
    {0x3110, V_GAIN},
#endif
    {0x3111, 0x40},
    {0x4141, 0x22},
    {0x4140, 0x22},
    {0x413f, 0x22},
    {0x413e, 0x22},
    {0x030c, 0x03},
    {0x0300, 0x3f},
    {0x0333, 0x09},
    {0x0305, 0xe0},
    {0x011c, 0x32},
    {0x0105, 0xe1},
    //{0x0106, 0x80}, // contrast
    //{0x0107, 0x00}, // brightness
    //{0x0108, 0x80}, // staturation
    //{0x0109, 0x00}, // hue
    {0x01bf, 0x4e},
    {0x0b7c, 0x02},
    {0x0b55, 0x80},
    {0x0b56, 0x00},
    {0x0b59, 0x04},
    {0x0b5a, 0x01},
    {0x0b5c, 0x07},
    {0x0b5e, 0x05},
    {0x0b31, 0x18},
    {0x0b36, 0x40},
    {0x0b37, 0x1f},
    {0x0b4b, 0x10},
    {0x0b4e, 0x05},
    {0x0b51, 0x21},
    {0x0b15, 0x03},
    {0x0b16, 0x03},
    {0x0b17, 0x03},
    {0x0b07, 0x03},
    {0x0b08, 0x05},
    {0x0b1a, 0x10},
    {0x0b1e, 0xb8},
    {0x0b1f, 0x08},
    {0x0b5f, 0x64},
    {0x4200, 0x3f},
    {0x130c, 0x03},
    {0x1300, 0x3f},
    {0x1333, 0x09},
    {0x111c, 0x32},
    {0x11bf, 0x4e},
    {0x1b7c, 0x02},
    {0x1b55, 0x80},
    {0x1b56, 0x00},
    {0x1b59, 0x04},
    {0x1b5a, 0x01},
    {0x1b5c, 0x07},
    {0x1b5e, 0x05},
    {0x1b31, 0x18},
    {0x1b36, 0x40},
    {0x1b37, 0x1f},
    {0x1b4b, 0x10},
    {0x1b4e, 0x05},
    {0x1b51, 0x21},
    {0x1b15, 0x03},
    {0x1b16, 0x03},
    {0x1b17, 0x03},
    {0x1b07, 0x03},
    {0x1b08, 0x05},
    {0x1b1a, 0x10},
    {0x1b1e, 0xb8},
    {0x1b1f, 0x08},
    {0x1b5f, 0x64},
    {0x1105, 0xe1},
    //{0x1106, 0x80}
    //{0x1107, 0x00},
    //{0x1108, 0x80},
    //{0x1109, 0x00},
    {0x4210, 0x33},
    {0x230c, 0x03},
    {0x2300, 0x3f},
    {0x2333, 0x09},
    {0x211c, 0x32},
    {0x21bf, 0x4e},
    {0x2b7c, 0x02},
    {0x2b55, 0x80},
    {0x2b56, 0x00},
    {0x2b59, 0x04},
    {0x2b5a, 0x01},
    {0x2b5c, 0x07},
    {0x2b5e, 0x05},
    {0x2b31, 0x18},
    {0x2b36, 0x40},
    {0x2b37, 0x1f},
    {0x2b4b, 0x10},
    {0x2b4e, 0x05},
    {0x2b51, 0x21},
    {0x2b15, 0x03},
    {0x2b16, 0x03},
    {0x2b17, 0x03},
    {0x2b07, 0x03},
    {0x2b08, 0x05},
    {0x2b1a, 0x10},
    {0x2b1e, 0xb8},
    {0x2b1f, 0x08},
    {0x2b5f, 0x64},
    {0x2105, 0xe1},
    //{0x2106, 0x80},
    //{0x2107, 0x00},
    //{0x2108, 0x80},
    //{0x2109, 0x00},
    {0x4220, 0x33},
    {0x330c, 0x03},
    {0x3300, 0x3f},
    {0x3333, 0x09},
    {0x311c, 0x32},
    {0x31bf, 0x4e},
    {0x3b7c, 0x02},
    {0x3b55, 0x80},
    {0x3b56, 0x00},
    {0x3b59, 0x04},
    {0x3b5a, 0x01},
    {0x3b5c, 0x07},
    {0x3b5e, 0x05},
    {0x3b31, 0x18},
    {0x3b36, 0x40},
    {0x3b37, 0x1f},
    {0x3b4b, 0x10},
    {0x3b4e, 0x05},
    {0x3b51, 0x21},
    {0x3b15, 0x03},
    {0x3b16, 0x03},
    {0x3b17, 0x03},
    {0x3b07, 0x03},
    {0x3b08, 0x05},
    {0x3b1a, 0x10},
    {0x3b1e, 0xb8},
    {0x3b1f, 0x08},
    {0x3b5f, 0x64},
    {0x3105, 0xe1},
    //{0x3106, 0x80},
    //{0x3107, 0x00},
    //{0x3108, 0x80},
    //{0x3109, 0x00},
    {0x4230, 0x33},
    {0x4800, 0x81},
    {0x4802, 0x01},
    {0x4030, 0x15},
    {0x50fc, 0x00},
    {0x50fd, 0x00},
    {0x50fe, 0x0d},
    {0x50ff, 0x59},
    {0x50e4, 0x00},//32位寄存器， 0x50e4是高字节， 0x50e7低8位字节
    {0x50e5, 0x00},
    {0x50e6, 0x2f},
    {0x50e7, 0x01},//0=1lane,1=2lane,2=3lane,3=4lane
    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x32},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x00},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x44},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x36},     //0x78=1.5G,0x38 =1.4G,0x76=1.3G, 0x36=1.2G,0x34=1G,0x32=800M,0x2c=500M
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x30},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x40},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x50},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x80},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x90},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x20},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x45},             //0x45
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
#if MIPILANESWAP
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x55},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x01},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x85},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x01},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
#endif
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x04},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x05},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x07},
    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},//MIPI 1:连续模式 ,0:非连续模式
    {0x4801, 0x81},
    {0x4031, 0x01},
    {0x4138, 0x1e},
    {0x413b, 0x1e},
    {0x4135, 0x1e},
    {0x412f, 0x1e},
    {0x412c, 0x1e},
    {0x4126, 0x1e},
    {0x4129, 0x1e},
    {0x4123, 0x1e},
    {0x4f00, 0x01},
    //xs9922_720p_4lanes_25fps_1500M
    {0x0803, 0x03}, /////////////////////////cam0
    {0x080e, 0x0f},
    {0x0803, 0x0f},
    {0x080e, 0x3f},
    {0x080e, 0x3f},
    {0x012d, 0x3f},
    {0x012f, 0xcc},
    {0x01e2, 0x03},
    {0x0158, 0x01},
    {0x0130, 0x10},
    {0x010c, 0x00},//通道0 1： 手动模式， 0： 自动模式
    {0x010d, 0x00},//40:AHD 720@25F 41:AHD 720@30F
    {0x0805, 0x05},
    {0x0e11, 0x08},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x0e12, 0x01},
    {0x060b, 0x00},
    {0x0627, 0x14},
    {0x061c, 0x00},
    {0x061d, 0x5a},
    {0x061e, 0xa5},
    {0x0640, 0x04},
    {0x0616, 0x24},
    {0x0617, 0x00},
    {0x0618, 0x04},
    {0x060a, 0x07},
    {0x010a, 0x05},
    {0x0100, 0x30},
    {0x0104, 0x48},
    {0x0802, 0x21},
    {0x0502, 0x0c},
    {0x050e, 0x1c},
    {0x1803, 0x03}, ////////////////////////cam1
    {0x180e, 0x0f},
    {0x1803, 0x0f},
    {0x180e, 0x3f},
    {0x180e, 0x3f},
    {0x112d, 0x3f},
    {0x112f, 0xcc},
    {0x11e2, 0x03},
    {0x1158, 0x01},
    {0x1130, 0x10},
    {0x110c, 0x01},//通道1 1： 手动模式， 0： 自动模式
    {0x110d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x1805, 0x05},
    {0x1e11, 0x08},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x1e12, 0x01},
    {0x160b, 0x00},
    {0x1627, 0x14},
    {0x161c, 0x00},
    {0x161d, 0x5a},
    {0x161e, 0xa5},
    {0x1640, 0x04},
    {0x1616, 0x24},
    {0x1617, 0x00},
    {0x1618, 0x04},
    {0x160a, 0x07},
    {0x110a, 0x05},
    {0x1100, 0x30},
    {0x1104, 0x48},
    {0x1802, 0x21},
    {0x1502, 0x0d},
    {0x150e, 0x1d},
    {0x2803, 0x03}, ////////////////////////cam2
    {0x280e, 0x0f},
    {0x2803, 0x0f},
    {0x280e, 0x3f},
    {0x280e, 0x3f},
    {0x212d, 0x3f},
    {0x212f, 0xcc},
    {0x21e2, 0x03},
    {0x2158, 0x01},
    {0x2130, 0x10},
    {0x210c, 0x01},//通道2 1： 手动模式， 0： 自动模式
    {0x210d, 0x41},//40:AHD 720@25F 41:AHD 720@30F
    {0x2805, 0x05},
    {0x2e11, 0x08},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x2e12, 0x01},
    {0x260b, 0x00},
    {0x2627, 0x14},
    {0x261c, 0x00},
    {0x261d, 0x5a},
    {0x261e, 0xa5},
    {0x2640, 0x04},
    {0x2616, 0x24},
    {0x2617, 0x00},
    {0x2618, 0x04},
    {0x260a, 0x07},
    {0x210a, 0x05},
    {0x2100, 0x30},
    {0x2104, 0x48},
    {0x2802, 0x21},
    {0x2502, 0x0e},
    {0x250e, 0x1e},
    {0x3803, 0x03}, ////////////////////////cam3
    {0x380e, 0x0f},
    {0x3803, 0x0f},
    {0x380e, 0x3f},
    {0x380e, 0x3f},
    {0x312d, 0x3f},
    {0x312f, 0xcc},
    {0x31e2, 0x03},
    {0x3158, 0x01},
    {0x3130, 0x10},
    {0x310c, 0x01},//通道3 1： 手动模式， 0： 自动模式
    {0x310d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x3805, 0x05},
    {0x3e11, 0x08},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x3e12, 0x01},
    {0x360b, 0x00},
    {0x3627, 0x14},
    {0x361c, 0x00},
    {0x361d, 0x5a},
    {0x361e, 0xa5},
    {0x3640, 0x04},
    {0x3616, 0x24},
    {0x3617, 0x00},
    {0x3618, 0x04},
    {0x360a, 0x07},
    {0x310a, 0x05},
    {0x3100, 0x38},
    {0x3104, 0x48},
    {0x3802, 0x21},
    {0x3502, 0x0f},
    {0x350e, 0x1f},
    //adc rst
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},
    {0x180e, 0x08},
    {0x1803, 0x0e},
    {0x1803, 0x0f},
    {0x180e, 0x28},
    {0x280e, 0x08},
    {0x2803, 0x0e},
    {0x2803, 0x0f},
    {0x280e, 0x28},
    {0x380e, 0x08},
    {0x3803, 0x0e},
    {0x3803, 0x0f},
    {0x380e, 0x28},
    {0x0e08, 0x01},//通道0 1： 开启， 0： 关闭
    {0x1e08, 0x00},//通道1 1： 开启， 0： 关闭
    {0x2e08, 0x00},//通道2 1： 开启， 0： 关闭
    {0x3e08, 0x00},//通道3 1： 开启， 0： 关闭                  id == 3
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x00},
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},

    //adc rst ------------------- 
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},
    //adc rst
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},

    {REG_NULL, 0x0,},
};

#else

static const k_sensor_reg xs9922b_mipi4lane_720p_30fps_linear[] = {

    // {0x4082, 0x03}, iic burst模式
    {0x0e08, 0x01},
    {0x1e08, 0x00},
    {0x2e08, 0x00},
    {0x3e08, 0x00},
    //{0x4500, 0x02},
    {0x4f00, 0x01},
    {0x4030, 0x3f},
    {0x0e02, 0x00},
    {0x1e02, 0x00},
    {0x2e02, 0x00},
    {0x3e02, 0x00},
    {0x0803, 0x00},
    {0x1803, 0x00},
    {0x2803, 0x00},
    {0x3803, 0x00},
    {0x4020, 0x00},
    {0x080e, 0x00},
    {0x080e, 0x20},
    {0x080e, 0x28},
    {0x180e, 0x00},
    {0x180e, 0x20},
    {0x180e, 0x28},
    {0x280e, 0x00},
    {0x280e, 0x20},
    {0x280e, 0x28},
    {0x380e, 0x00},
    {0x380e, 0x20},
    {0x380e, 0x28},
    {0x4020, 0x03},
    {0x0803, 0x0f},
    {0x1803, 0x0f},
    {0x2803, 0x0f},
    {0x3803, 0x0f},
#if RM_STRIPES
    {0x0800, 0x16},
    {0x1800, 0x16},
    {0x2800, 0x16},
    {0x3800, 0x16},
    {0x4205, 0x36},
    {0x4215, 0x36},
    {0x4225, 0x36},
    {0x4235, 0x36},//wait(0.05)
    {0x4205, 0x26},
    {0x4215, 0x26},
    {0x4225, 0x26},
    {0x4235, 0x26},
    {0x0800, 0x17},
    {0x1800, 0x17},
    {0x2800, 0x17},
    {0x3800, 0x17},
    #endif
    {0x4340, 0x65},
    {0x4204, 0x02},
    {0x4214, 0x02},
    {0x4224, 0x02},
    {0x4234, 0x02},
    {0x4080, 0x07},
    {0x4119, 0x01},
    {0x0501, 0x84},
#if USE_YUVGAIN
    {0x010e, Y_GAIN},
    {0x010f, U_GAIN},
    {0x0110, V_GAIN},
#endif
    {0x0111, 0x40},
    {0x1501, 0x81},
#if USE_YUVGAIN
    {0x110e, Y_GAIN},
    {0x110f, U_GAIN},
    {0x1110, V_GAIN},
#endif
    {0x1111, 0x40},
    {0x2501, 0x8e},
#if USE_YUVGAIN
    {0x210e, Y_GAIN},
    {0x210f, U_GAIN},
    {0x2110, V_GAIN},
#endif
    {0x2111, 0x40},
    {0x3501, 0x8b},
#if USE_YUVGAIN
    {0x310e, Y_GAIN},
    {0x310f, U_GAIN},
    {0x3110, V_GAIN},
#endif
    {0x3111, 0x40},
    {0x4141, 0x22},
    {0x4140, 0x22},
    {0x413f, 0x22},
    {0x413e, 0x22},
    {0x030c, 0x03},
    {0x0300, 0x3f},
    {0x0333, 0x09},
    {0x0305, 0xe0},
    {0x011c, 0x32},
    {0x0105, 0xe1},
    //{0x0106, 0x80}, // contrast
    //{0x0107, 0x00}, // brightness
    //{0x0108, 0x80}, // staturation
    //{0x0109, 0x00}, // hue
    {0x01bf, 0x4e},
    {0x0b7c, 0x02},
    {0x0b55, 0x80},
    {0x0b56, 0x00},
    {0x0b59, 0x04},
    {0x0b5a, 0x01},
    {0x0b5c, 0x07},
    {0x0b5e, 0x05},
    {0x0b31, 0x18},
    {0x0b36, 0x40},
    {0x0b37, 0x1f},
    {0x0b4b, 0x10},
    {0x0b4e, 0x05},
    {0x0b51, 0x21},
    {0x0b15, 0x03},
    {0x0b16, 0x03},
    {0x0b17, 0x03},
    {0x0b07, 0x03},
    {0x0b08, 0x05},
    {0x0b1a, 0x10},
    {0x0b1e, 0xb8},
    {0x0b1f, 0x08},
    {0x0b5f, 0x64},
    {0x4200, 0x3f},
    {0x130c, 0x03},
    {0x1300, 0x3f},
    {0x1333, 0x09},
    {0x111c, 0x32},
    {0x11bf, 0x4e},
    {0x1b7c, 0x02},
    {0x1b55, 0x80},
    {0x1b56, 0x00},
    {0x1b59, 0x04},
    {0x1b5a, 0x01},
    {0x1b5c, 0x07},
    {0x1b5e, 0x05},
    {0x1b31, 0x18},
    {0x1b36, 0x40},
    {0x1b37, 0x1f},
    {0x1b4b, 0x10},
    {0x1b4e, 0x05},
    {0x1b51, 0x21},
    {0x1b15, 0x03},
    {0x1b16, 0x03},
    {0x1b17, 0x03},
    {0x1b07, 0x03},
    {0x1b08, 0x05},
    {0x1b1a, 0x10},
    {0x1b1e, 0xb8},
    {0x1b1f, 0x08},
    {0x1b5f, 0x64},
    {0x1105, 0xe1},
    //{0x1106, 0x80}
    //{0x1107, 0x00},
    //{0x1108, 0x80},
    //{0x1109, 0x00},
    {0x4210, 0x33},
    {0x230c, 0x03},
    {0x2300, 0x3f},
    {0x2333, 0x09},
    {0x211c, 0x32},
    {0x21bf, 0x4e},
    {0x2b7c, 0x02},
    {0x2b55, 0x80},
    {0x2b56, 0x00},
    {0x2b59, 0x04},
    {0x2b5a, 0x01},
    {0x2b5c, 0x07},
    {0x2b5e, 0x05},
    {0x2b31, 0x18},
    {0x2b36, 0x40},
    {0x2b37, 0x1f},
    {0x2b4b, 0x10},
    {0x2b4e, 0x05},
    {0x2b51, 0x21},
    {0x2b15, 0x03},
    {0x2b16, 0x03},
    {0x2b17, 0x03},
    {0x2b07, 0x03},
    {0x2b08, 0x05},
    {0x2b1a, 0x10},
    {0x2b1e, 0xb8},
    {0x2b1f, 0x08},
    {0x2b5f, 0x64},
    {0x2105, 0xe1},
    //{0x2106, 0x80},
    //{0x2107, 0x00},
    //{0x2108, 0x80},
    //{0x2109, 0x00},
    {0x4220, 0x33},
    {0x330c, 0x03},
    {0x3300, 0x3f},
    {0x3333, 0x09},
    {0x311c, 0x32},
    {0x31bf, 0x4e},
    {0x3b7c, 0x02},
    {0x3b55, 0x80},
    {0x3b56, 0x00},
    {0x3b59, 0x04},
    {0x3b5a, 0x01},
    {0x3b5c, 0x07},
    {0x3b5e, 0x05},
    {0x3b31, 0x18},
    {0x3b36, 0x40},
    {0x3b37, 0x1f},
    {0x3b4b, 0x10},
    {0x3b4e, 0x05},
    {0x3b51, 0x21},
    {0x3b15, 0x03},
    {0x3b16, 0x03},
    {0x3b17, 0x03},
    {0x3b07, 0x03},
    {0x3b08, 0x05},
    {0x3b1a, 0x10},
    {0x3b1e, 0xb8},
    {0x3b1f, 0x08},
    {0x3b5f, 0x64},
    {0x3105, 0xe1},
    //{0x3106, 0x80},
    //{0x3107, 0x00},
    //{0x3108, 0x80},
    //{0x3109, 0x00},
    {0x4230, 0x33},
    {0x4800, 0x81},
    {0x4802, 0x01},
    {0x4030, 0x15},
    {0x50fc, 0x00},
    {0x50fd, 0x00},
    {0x50fe, 0x0d},
    {0x50ff, 0x59},
    {0x50e4, 0x00},//32位寄存器， 0x50e4是高字节， 0x50e7低8位字节
    {0x50e5, 0x00},
    {0x50e6, 0x2f},
    {0x50e7, 0x03},//0=1lane,1=2lane,2=3lane,3=4lane
    {0x50f0, 0x00},
    {0x50f1, 0x00},
    {0x50f2, 0x00},
    {0x50f3, 0x32},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x00},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x44},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x78},//0x78=1.5G,0x38 =1.4G,0x76=1.3G, 0x36=1.2G,0x34=1G,0x32=800M,0x2c=500M
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x30},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x40},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x50},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x80},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x90},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x03},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x20},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x45},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    #if MIPILANESWAP
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x55},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x01},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    {0x5118, 0x00},
    {0x5119, 0x01},
    {0x511a, 0x00},
    {0x511b, 0x85},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x00},
    {0x5118, 0x00},
    {0x5119, 0x00},
    {0x511a, 0x00},
    {0x511b, 0x01},
    {0x5114, 0x00},
    {0x5115, 0x00},
    {0x5116, 0x00},
    {0x5117, 0x02},
    #endif
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x04},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x05},
    {0x50e0, 0x00},
    {0x50e1, 0x00},
    {0x50e2, 0x00},
    {0x50e3, 0x07},
    {0x50e8, 0x00},
    {0x50e9, 0x00},
    {0x50ea, 0x00},
    {0x50eb, 0x01},//MIPI 1:连续模式 ,0:非连续模式
    {0x4801, 0x81},
    {0x4031, 0x01},
    {0x4138, 0x1e},
    {0x413b, 0x1e},
    {0x4135, 0x1e},
    {0x412f, 0x1e},
    {0x412c, 0x1e},
    {0x4126, 0x1e},
    {0x4129, 0x1e},
    {0x4123, 0x1e},
    {0x4f00, 0x01},
    //xs9922_720p_4lanes_25fps_1500M
    {0x0803, 0x03}, /////////////////////////cam0
    {0x080e, 0x0f},
    {0x0803, 0x0f},
    {0x080e, 0x3f},
    {0x080e, 0x3f},
    {0x012d, 0x3f},
    {0x012f, 0xcc},
    {0x01e2, 0x03},
    {0x0158, 0x01},
    {0x0130, 0x10},
    {0x010c, 0x01},//通道0 1： 手动模式， 0： 自动模式
    {0x010d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x0805, 0x05},
    {0x0e11, 0x06},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x0e12, 0x01},
    {0x060b, 0x00},
    {0x0627, 0x14},
    {0x061c, 0x00},
    {0x061d, 0x5a},
    {0x061e, 0xa5},
    {0x0640, 0x04},
    {0x0616, 0x24},
    {0x0617, 0x00},
    {0x0618, 0x04},
    {0x060a, 0x07},
    {0x010a, 0x05},
    {0x0100, 0x30},
    {0x0104, 0x48},
    {0x0802, 0x21},
    {0x0502, 0x0c},
    {0x050e, 0x1c},
    {0x1803, 0x03}, ////////////////////////cam1
    {0x180e, 0x0f},
    {0x1803, 0x0f},
    {0x180e, 0x3f},
    {0x180e, 0x3f},
    {0x112d, 0x3f},
    {0x112f, 0xcc},
    {0x11e2, 0x03},
    {0x1158, 0x01},
    {0x1130, 0x10},
    {0x110c, 0x01},//通道1 1： 手动模式， 0： 自动模式
    {0x110d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x1805, 0x05},
    {0x1e11, 0x06},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x1e12, 0x01},
    {0x160b, 0x00},
    {0x1627, 0x14},
    {0x161c, 0x00},
    {0x161d, 0x5a},
    {0x161e, 0xa5},
    {0x1640, 0x04},
    {0x1616, 0x24},
    {0x1617, 0x00},
    {0x1618, 0x04},
    {0x160a, 0x07},
    {0x110a, 0x05},
    {0x1100, 0x30},
    {0x1104, 0x48},
    {0x1802, 0x21},
    {0x1502, 0x0d},
    {0x150e, 0x1d},
    {0x2803, 0x03}, ////////////////////////cam2
    {0x280e, 0x0f},
    {0x2803, 0x0f},
    {0x280e, 0x3f},
    {0x280e, 0x3f},
    {0x212d, 0x3f},
    {0x212f, 0xcc},
    {0x21e2, 0x03},
    {0x2158, 0x01},
    {0x2130, 0x10},
    {0x210c, 0x01},//通道2 1： 手动模式， 0： 自动模式
    {0x210d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x2805, 0x05},
    {0x2e11, 0x06},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x2e12, 0x01},
    {0x260b, 0x00},
    {0x2627, 0x14},
    {0x261c, 0x00},
    {0x261d, 0x5a},
    {0x261e, 0xa5},
    {0x2640, 0x04},
    {0x2616, 0x24},
    {0x2617, 0x00},
    {0x2618, 0x04},
    {0x260a, 0x07},
    {0x210a, 0x05},
    {0x2100, 0x30},
    {0x2104, 0x48},
    {0x2802, 0x21},
    {0x2502, 0x0e},
    {0x250e, 0x1e},
    {0x3803, 0x03}, ////////////////////////cam3
    {0x380e, 0x0f},
    {0x3803, 0x0f},
    {0x380e, 0x3f},
    {0x380e, 0x3f},
    {0x312d, 0x3f},
    {0x312f, 0xcc},
    {0x31e2, 0x03},
    {0x3158, 0x01},
    {0x3130, 0x10},
    {0x310c, 0x01},//通道3 1： 手动模式， 0： 自动模式
    {0x310d, 0x40},//40:AHD 720@25F 41:AHD 720@30F
    {0x3805, 0x05},
    {0x3e11, 0x06},//彩条模式， 00-08， 白黄青绿粉红蓝黑彩条
    {0x3e12, 0x01},
    {0x360b, 0x00},
    {0x3627, 0x14},
    {0x361c, 0x00},
    {0x361d, 0x5a},
    {0x361e, 0xa5},
    {0x3640, 0x04},
    {0x3616, 0x24},
    {0x3617, 0x00},
    {0x3618, 0x04},
    {0x360a, 0x07},
    {0x310a, 0x05},
    {0x3100, 0x38},
    {0x3104, 0x48},
    {0x3802, 0x21},
    {0x3502, 0x0f},
    {0x350e, 0x1f},
    //adc rst
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},
    {0x180e, 0x08},
    {0x1803, 0x0e},
    {0x1803, 0x0f},
    {0x180e, 0x28},
    {0x280e, 0x08},
    {0x2803, 0x0e},
    {0x2803, 0x0f},
    {0x280e, 0x28},
    {0x380e, 0x08},
    {0x3803, 0x0e},
    {0x3803, 0x0f},
    {0x380e, 0x28},
    {0x0e08, 0x01},//通道0 1： 开启， 0： 关闭
    {0x1e08, 0x01},//通道1 1： 开启， 0： 关闭
    {0x2e08, 0x01},//通道2 1： 开启， 0： 关闭
    {0x3e08, 0x01},//通道3 1： 开启， 0： 关闭
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x00},
    {0x5004, 0x00},
    {0x5005, 0x00},
    {0x5006, 0x00},
    {0x5007, 0x01},

    //adc rst ------------------- 
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},
    //adc rst
    {0x080e, 0x08},
    {0x0803, 0x0e},
    {0x0803, 0x0f},
    {0x080e, 0x28},

    {REG_NULL, 0x0,},
};
#endif



static k_sensor_mode xs9922b_mode_info[] = {
    {
        .index = 0,
        .sensor_type = XS9950_MIPI_CSI2_1280X720_30FPS_YUV422, //XS9922B_MIPI_CSI0_1280X720_30FPS_YUV422_DOL3,
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
        .reg_list = xs9922b_mipi4lane_720p_30fps_linear,
        .mclk_setting = {{K_FALSE}, {K_FALSE}, {K_FALSE}},
    },
};

static k_bool xs9922b_init_flag = K_FALSE;
static k_sensor_mode *current_mode = NULL;

static int xs9922b_power_rest(k_s32 on)
{
    #define VICAP_XS9922B_RST_GPIO     (0)  //24// 

    kd_pin_mode(VICAP_XS9922B_RST_GPIO, GPIO_DM_OUTPUT);

    if (on) {
        kd_pin_write(VICAP_XS9922B_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_XS9922B_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
        rt_thread_mdelay(100);
        kd_pin_write(VICAP_XS9922B_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_LOW  GPIO_PV_HIGH
    } else {
        kd_pin_write(VICAP_XS9922B_RST_GPIO, GPIO_PV_LOW); // GPIO_PV_LOW  GPIO_PV_HIGH
    }
    rt_thread_mdelay(1);

    return 0;
}


static int xs9922b_i2c_init(k_sensor_i2c_info *i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL)
    {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}


static k_s32 xs9922b_sensor_get_chip_id(void *ctx, k_u32 *chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter\n", __func__);

    xs9922b_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_CHIP_ID_H, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, XS9922B_REG_CHIP_ID_L, &id_low);
    if (ret) {
        // pr_err("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    rt_kprintf("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}


static k_s32 xs9922b_sensor_power_on(void *ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev *dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter\n", __func__);
    if (on) {
        // if (!xs9922b_init_flag) {
            xs9922b_power_rest(on);
            xs9922b_i2c_init(&dev->i2c_info);
        // }
        ret = xs9922b_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        xs9922b_init_flag = K_FALSE;
        xs9922b_power_rest(on);
    }

    return ret;
}

static k_s32 xs9922b_sensor_init(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev *dev = ctx;
    pr_info("%s enter, sensor_type:%d\n", __func__, mode.sensor_type);

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(xs9922b_mode_info) / sizeof(k_sensor_mode); i++) {
            if (xs9922b_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(xs9922b_mode_info[i]);
                dev->sensor_mode = &(xs9922b_mode_info[i]);
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
        // if (!xs9922b_init_flag) {
            ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);
        // }
        k_u16 reg_val = 0;
        k_u32 reg_adr = 0x511b;
        ret = sensor_reg_read(&dev->i2c_info, reg_adr, &reg_val);
        printf("write reg success ----------0x511b----mipi rata is %x----------\n", reg_val);

        reg_adr = 0x0e08;
        ret = sensor_reg_read(&dev->i2c_info, reg_adr, &reg_val);
        printf(" reg --0x0e08 is %x----------\n", reg_val);

        reg_adr = 0x0e02;
        ret = sensor_reg_read(&dev->i2c_info, reg_adr, &reg_val);
        printf(" reg --0x0e02 is %x----------\n", reg_val);

        reg_adr = 0x1e02;
        ret = sensor_reg_read(&dev->i2c_info, reg_adr, &reg_val);
        printf(" reg --0x1e02 is %x----------\n", reg_val);

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
        current_mode->ae_info.gain_increment = XS9922B_MIN_GAIN_STEP;

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
        if (!xs9922b_init_flag) {
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
        current_mode->ae_info.gain_increment = XS9922B_MIN_GAIN_STEP;

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

    // ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_LONG_AGAIN, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_H, &again_h);
    //ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_L, &again_l);
    again = 1.0;// (float)again_h / 16.0f;

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

    // ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_LONG_EXP_TIME_H, &exp_time_h);
    // ret = sensor_reg_read(&dev->i2c_info, XS9922B_REG_LONG_EXP_TIME_L, &exp_time_l);
    exp_time = (exp_time_h << 4) | ((exp_time_l >> 4) & 0x0F);

    current_mode->ae_info.cur_integration_time = exp_time * current_mode->ae_info.one_line_exp_time;
    xs9922b_init_flag = K_TRUE;

    return ret;
}


k_s32 xs9922b_sensor_get_mode(void *ctx, k_sensor_mode *mode)
{
    k_s32 ret = -1;

    pr_info("%s enter, sensor_type(%d)\n", __func__, mode->sensor_type);

    for (k_s32 i = 0; i < sizeof(xs9922b_mode_info) / sizeof(k_sensor_mode); i++) {
        if (xs9922b_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &xs9922b_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(xs9922b_mode_info[i]);
            return 0;
        }
    }
    pr_info("%s, the mode not exit.\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_set_mode(void *ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_enum_mode(void *ctx, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(enum_mode, 0, sizeof(k_sensor_enum_mode));

    return ret;
}

k_s32 xs9922b_sensor_get_caps(void *ctx, k_sensor_caps *caps)
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

k_s32 xs9922b_sensor_conn_check(void *ctx, k_s32 *conn)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

k_s32 xs9922b_sensor_set_stream(void *ctx, k_s32 enable)
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

k_s32 xs9922b_sensor_get_again(void *ctx, k_sensor_gain *gain)
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

k_s32 xs9922b_sensor_set_again(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    struct sensor_driver_dev *dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(gain.gain[SENSOR_LINEAR_PARAS] * 16 + 0.5);
        //if(current_mode->sensor_again !=again)
        {
	        // ret = sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN,(again & 0xff));
	        current_mode->sensor_again = again;
        }
        current_mode->ae_info.cur_again = (float)current_mode->sensor_again/16.0f;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(gain.gain[SENSOR_DUAL_EXP_L_PARAS]* 16 + 0.5);
        // ret = sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN,(again & 0xff));
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

k_s32 xs9922b_sensor_get_dgain(void *ctx, k_sensor_gain *gain)
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

k_s32 xs9922b_sensor_set_dgain(void *ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev *dev = ctx;

    pr_info("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_L,(again & 0xff));
        current_mode->ae_info.cur_dgain = (float)dgain/1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        //ret = sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_H,(again & 0x0300)>>8);
        //ret |= sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_AGAIN_L,(again & 0xff));
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

k_s32 xs9922b_sensor_get_intg_time(void *ctx, k_sensor_intg_time *time)
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

k_s32 xs9922b_sensor_set_intg_time(void *ctx, k_sensor_intg_time time)
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
	        // ret |= sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_EXP_TIME_H, ( exp_line >> 4) & 0xff);
	        // ret |= sensor_reg_write(&dev->i2c_info, XS9922B_REG_LONG_EXP_TIME_L, ( exp_line << 4) & 0xf0);
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

k_s32 xs9922b_sensor_get_exp_parm(void *ctx, k_sensor_exposure_param *exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

k_s32 xs9922b_sensor_set_exp_parm(void *ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_get_fps(void *ctx, k_u32 *fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    *fps = 30000;

    return ret;
}

k_s32 xs9922b_sensor_set_fps(void *ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_get_isp_status(void *ctx, k_sensor_isp_status *staus)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

k_s32 xs9922b_sensor_set_blc(void *ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_set_wb(void *ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_get_tpg(void *ctx, k_sensor_test_pattern *tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

k_s32 xs9922b_sensor_set_tpg(void *ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);

    return ret;
}

k_s32 xs9922b_sensor_get_expand_curve(void *ctx, k_sensor_compand_curve *curve)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

k_s32 xs9922b_sensor_get_otp_data(void *ctx, void *data)
{
    k_s32 ret = 0;

    pr_info("%s enter\n", __func__);
    memset(data, 0, sizeof(void *));

    return ret;
}

static k_s32 xs9922b_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev xs9922b_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c3",   //"i2c0", //"i2c3",
        .slave_addr = 0x30,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "xs9922b",
    .sensor_func = {
        .sensor_power = xs9922b_sensor_power_on,
        .sensor_init = xs9922b_sensor_init,
        .sensor_get_chip_id = xs9922b_sensor_get_chip_id,
        .sensor_get_mode = xs9922b_sensor_get_mode,
        .sensor_set_mode = xs9922b_sensor_set_mode,
        .sensor_enum_mode = xs9922b_sensor_enum_mode,
        .sensor_get_caps = xs9922b_sensor_get_caps,
        .sensor_conn_check = xs9922b_sensor_conn_check,
        .sensor_set_stream = xs9922b_sensor_set_stream,
        .sensor_get_again = xs9922b_sensor_get_again,
        .sensor_set_again = xs9922b_sensor_set_again,
        .sensor_get_dgain = xs9922b_sensor_get_dgain,
        .sensor_set_dgain = xs9922b_sensor_set_dgain,
        .sensor_get_intg_time = xs9922b_sensor_get_intg_time,
        .sensor_set_intg_time = xs9922b_sensor_set_intg_time,
        .sensor_get_exp_parm = xs9922b_sensor_get_exp_parm,
        .sensor_set_exp_parm = xs9922b_sensor_set_exp_parm,
        .sensor_get_fps = xs9922b_sensor_get_fps,
        .sensor_set_fps = xs9922b_sensor_set_fps,
        .sensor_get_isp_status = xs9922b_sensor_get_isp_status,
        .sensor_set_blc = xs9922b_sensor_set_blc,
        .sensor_set_wb = xs9922b_sensor_set_wb,
        .sensor_get_tpg = xs9922b_sensor_get_tpg,
        .sensor_set_tpg = xs9922b_sensor_set_tpg,
        .sensor_get_expand_curve = xs9922b_sensor_get_expand_curve,
        .sensor_get_otp_data = xs9922b_sensor_get_otp_data,
        .sensor_mirror_set = xs9922b_sensor_mirror_set,
    },
};
