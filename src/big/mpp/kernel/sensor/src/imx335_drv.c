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
#define IMX335_REG_MODE_SELECT 0x3000
#define IMX335_MODE_STANDBY 0x01
#define IMX335_MODE_STREAMING 0x00

/* Lines per frame */
#define IMX335_REG_LPFR 0x3030

/* Chip ID */
#define IMX335_REG_ID 0x3912
#define IMX335_ID 0x00

/* Exposure control */
#define IMX335_REG_SHR0_L 0x3058
#define IMX335_REG_SHR0_M 0x3059
#define IMX335_REG_SHR1_L 0x305c
#define IMX335_REG_SHR1_M 0x305d
#define IMX335_REG_SHR2_L 0x3060
#define IMX335_REG_SHR2_M 0x3061
#define IMX335_REG_RHS1_L 0x3068
#define IMX335_REG_RHS1_M 0x3069
#define IMX335_REG_RHS2_L 0x306c
#define IMX335_REG_RHS2_M 0x306d
#define IMX335_VMAX_LINEAR 4500
#define IMX335_VMAX_DOL2 3980
#define IMX335_VMAX_DOL3 4500

/* Analog gain control */
#define IMX335_REG_AGAIN_L 0x30e8
#define IMX335_REG_AGAIN_H 0x30e9
#define IMX335_AGAIN_STEP (1.0f/256.0f)

/* Group hold register */
#define IMX335_REG_HOLD 0x3001

/* Input clock rate */
#define IMX335_INCLK_RATE 24000000

/* CSI2 HW configuration */
#define IMX335_LINK_FREQ 594000000
#define IMX335_NUM_DATA_LANES 4

#define IMX335_REG_MIN 0x00
#define IMX335_REG_MAX 0xfffff

#define DOL2_RHS1 482
#define DOL3_RHS1 986
#define DOL3_RHS2 2608//1072

#define DOL2_ratio 16.0
#define DOL3_LS_ratio 16.0
#define DOL3_VS_ratio 16.0


static const k_sensor_reg imx335_mipi_2lane_raw10_1920x1080_30fps_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0xc6 },
    { 0x314d, 0x00 },
    { 0x315a, 0x02 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3050, 0x00 }, // AD bit, 12bit ==> 0x00, 10bit
    { 0x319d, 0x00 }, // output bit, 12bit ==>0x00, 10bit
    { 0x341c, 0xff }, // AD bit, 12bit ==>0xff, 10bit
    { 0x341d, 0x01 }, // AD bit, 12bit ==>0x01, 10bit
    { 0x3018, 0x04 }, // window mode
    { 0x3300, 0x00 }, // scan mode or binning
    { 0x3302, 0x10 }, //black level
    { 0x3199, 0x00 }, // HADD VADD
    { 0x3030, 0x94 }, // V span max, VMAX = 4500
    { 0x3031, 0x11 },
    { 0x3032, 0x00 },
    { 0x3034, 0x26 }, // H span max, HMAX = 550
    { 0x3035, 0x02 },
    { 0x304c, 0x14 }, // V OB width
    { 0x304e, 0x00 }, // H dir inv
    { 0x304f, 0x00 }, // V dir inv
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0x3056, 0x48 }, // effective pixel line, y output size = 1096 line = 8 + 1080 +8
    { 0x3057, 0x04 },
    { 0x302c, 0x80 }, // crop mode H start, 672/2 + 48 = 384
    { 0x302d, 0x01 },
    { 0x302e, 0x98 }, // crop mode H size, 1944 = 12 + 1920 +12
    { 0x302f, 0x07 },
    { 0x3072, 0x28 }, // crop mode OB V size
    { 0x3073, 0x00 },
    { 0x3074, 0x60 }, // crop mode UL V start
    { 0x3075, 0x02 },
    { 0x3076, 0x90 }, // crop mode V size,
    { 0x3077, 0x08 },
    { 0x30c6, 0x12 }, // crop mode black offset
    { 0x30c7, 0x00 },
    { 0x30ce, 0x64 }, // crop mode UNRD_LINE_MAX
    { 0x30cf, 0x00 },
    { 0x30d8, 0xc0 }, // crop mode UNREAD_ED_ADR
    { 0x30d9, 0x0b },
    { REG_NULL, 0x00 }
};

static const k_sensor_reg imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_24m_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0xc6 },
    { 0x314d, 0x00 },
    { 0x315a, 0x02 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3018, 0x04 }, // window mode
    { 0x3300, 0x00 }, // scan mode or binning
    { 0x3302, 0x10 }, //black level
    { 0x3199, 0x00 }, // HADD VADD
    { 0x3030, 0x94 }, // V span max
    { 0x3031, 0x11 },
    { 0x3032, 0x00 },
    { 0x3034, 0x26 }, // H span max
    { 0x3035, 0x02 },
    { 0x304c, 0x14 }, // V OB width
    { 0x304e, 0x00 }, // H dir inv
    { 0x304f, 0x00 }, // V dir inv
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0x3056, 0x48 }, // effective pixel line
    { 0x3057, 0x04 },
    { 0x302c, 0x80 }, // crop mode H start
    { 0x302d, 0x01 },
    { 0x302e, 0x98 }, // crop mode H size, 1944
    { 0x302f, 0x07 },
    { 0x3072, 0x28 }, // crop mode OB V size
    { 0x3073, 0x00 },
    { 0x3074, 0x60 }, // crop mode UL V start
    { 0x3075, 0x02 },
    { 0x3076, 0x90 }, // crop mode V size, 2192
    { 0x3077, 0x08 },
    { 0x30c6, 0x12 }, // crop mode black offset
    { 0x30c7, 0x00 },
    { 0x30ce, 0x64 }, // crop mode UNRD_LINE_MAX
    { 0x30cf, 0x00 },
    { 0x30d8, 0xc0 }, // crop mode UNREAD_ED_ADR
    { 0x30d9, 0x0b },
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_74_25_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0xb6 }, // lane 24MHz
    { 0x300d, 0x7f },
    { 0x314c, 0x80 },
    { 0x314d, 0x00 },
    { 0x315a, 0x03 },
    { 0x3168, 0x68 },
    { 0x316a, 0x7f },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3018, 0x04 }, // window mode
    { 0x3300, 0x00 }, // scan mode or binning
    { 0x3302, 0x10 }, //black level
    { 0x3199, 0x00 }, // HADD VADD
    { 0x3030, 0x94 }, // V span max
    { 0x3031, 0x11 },
    { 0x3032, 0x00 },
    { 0x3034, 0x26 }, // H span max
    { 0x3035, 0x02 },
    { 0x304c, 0x14 }, // V OB width
    { 0x304e, 0x00 }, // H dir inv
    { 0x304f, 0x00 }, // V dir inv
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { 0x3056, 0x48 }, // effective pixel line
    { 0x3057, 0x04 },
    { 0x302c, 0x80 }, // crop mode H start
    { 0x302d, 0x01 },
    { 0x302e, 0x98 }, // crop mode H size, 1944
    { 0x302f, 0x07 },
    { 0x3072, 0x28 }, // crop mode OB V size
    { 0x3073, 0x00 },
    { 0x3074, 0x60 }, // crop mode UL V start
    { 0x3075, 0x02 },
    { 0x3076, 0x90 }, // crop mode V size, 2192
    { 0x3077, 0x08 },
    { 0x30c6, 0x12 }, // crop mode black offset
    { 0x30c7, 0x00 },
    { 0x30ce, 0x64 }, // crop mode UNRD_LINE_MAX
    { 0x30cf, 0x00 },
    { 0x30d8, 0xc0 }, // crop mode UNREAD_ED_ADR
    { 0x30d9, 0x0b },
    { REG_NULL, 0x00 }
};

static const k_sensor_reg imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_24m_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0xc6 },
    { 0x314d, 0x00 },
    { 0x315a, 0x02 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_74_25_regs[] = {
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x01 }, // 2 lane
    { 0x300c, 0xb6 }, // lane 24MHz
    { 0x300d, 0x7f },
    { 0x314c, 0x80 },
    { 0x314d, 0x00 },
    { 0x315a, 0x03 },
    { 0x3168, 0x68 },
    { 0x316a, 0x7f },
    { 0x319e, 0x01 },    //1188Mbps
    { 0x3a18, 0x8f }, // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x4f },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x47 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0x37 },
    { 0x3a1f, 0x01 },
    { 0x3a20, 0x4f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x87 },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x4f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x7f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x3f },
    { 0x3a29, 0x00 },
    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { REG_NULL, 0x00 }
};

static const k_sensor_reg imx335_mipi_4lane_raw10_2592x1940_30fps_regs[] = {
    { 0x3000, 0x01},
    { 0x3002, 0x01},
    { 0x3004, 0x04},
    { 0x3004, 0x00},
    //All pixel scan mode, A/D Conversion 10bit / Output 10 bit / 891 Mbps /30fps
    { 0x3018, 0x00},
    { 0x3030, 0x94},
    { 0x3031, 0x11},
    { 0x3032, 0x00},
    { 0x3034, 0x26},
    { 0x3035, 0x02},
    { 0x304c, 0x14},
    { 0x304e, 0x00},
    { 0x304f, 0x00},
    { 0x3050, 0x00},
    { 0x3056, 0xac},
    { 0x3057, 0x07},
    { 0x3072, 0x28},
    { 0x3073, 0x00},
    { 0x3074, 0xb0},
    { 0x3075, 0x00},
    { 0x3076, 0x58},
    { 0x3077, 0x0f},
    //All pixel
    { 0x3078, 0x01},
    { 0x3079, 0x02},
    { 0x307a, 0xff},
    { 0x307b, 0x02},
    { 0x307c, 0x00},
    { 0x307d, 0x00},
    { 0x307e, 0x00},
    { 0x307f, 0x00},
    { 0x3080, 0x01},
    { 0x3081, 0x02},
    { 0x3082, 0xff},
    { 0x3083, 0x02},
    { 0x3084, 0x00},
    { 0x3085, 0x00},
    { 0x3086, 0x00},
    { 0x3087, 0x00},
    { 0x30a4, 0x33},
    { 0x30a8, 0x10},
    { 0x30a9, 0x04},
    { 0x30ac, 0x00},
    { 0x30ad, 0x00},
    { 0x30b0, 0x10},
    { 0x30b1, 0x08},
    { 0x30b4, 0x00},
    { 0x30b5, 0x00},
    { 0x30b6, 0x00},
    { 0x30b7, 0x00},
    { 0x3112, 0x08},
    { 0x3113, 0x00},
    { 0x3116, 0x08},
    { 0x3117, 0x00},
    { 0x3199, 0x00},
    { 0x319d, 0x00},
    { 0x3300, 0x00},
    { 0x341c, 0xff},
    { 0x341d, 0x01},
    { 0x3a01, 0x03},
    { 0x3a18, 0x7f},
    { 0x3a19, 0x00},
    { 0x3a1a, 0x37},
    { 0x3a1b, 0x00},
    { 0x3a1c, 0x37},
    { 0x3a1d, 0x00},
    { 0x3a1e, 0xf7},
    { 0x3a1f, 0x00},
    { 0x3a20, 0x3f},
    { 0x3a21, 0x00},
    { 0x3a22, 0x6f},
    { 0x3a23, 0x00},
    { 0x3a24, 0x3f},
    { 0x3a25, 0x00},
    { 0x3a26, 0x5f},
    { 0x3a27, 0x00},
    { 0x3a28, 0x2f},
    { 0x3a29, 0x00},
    //----891Mbps/lane 24MHz
    { 0x300c, 0x3b},
    { 0x300d, 0x2a},
    { 0x314c, 0x29},
    { 0x314d, 0x01},
    { 0x315a, 0x06},
    { 0x3168, 0xa0},
    { 0x316a, 0x7e},
    { 0x319e, 0x02},
    { 0x3000, 0x00},
    { 0x3002, 0x00},
    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x305a, 0x00 },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_4lane_raw12_2592x1944_30fps_mclk_74_25_regs[] = {    //891Mbps
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x03 }, // 4 lane
    { 0x300c, 0xb6 }, // lane 24MHz
    { 0x300d, 0x7f },
    { 0x314c, 0xC0 },
    { 0x314d, 0x00 },
    { 0x315a, 0x07 },
    { 0x3168, 0x68 },
    { 0x316a, 0x7F },
    { 0x319e, 0x02 },    //891Mbps
    { 0x3a18, 0x7f },     // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x37 },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x37 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0xf7 },
    { 0x3a1f, 0x00 },
    { 0x3a20, 0x3f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x6f },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x3f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x5f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x2f },
    { 0x3a29, 0x00 },
    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_4lane_raw12_2592x1944_30fps_mclk_24m_regs[] = {    //891Mbps
    { 0x3000, 0x01 },
    { 0x3002, 0x00 },
    { 0x3a01, 0x03 }, // 4 lane
    { 0x300c, 0x3b }, // lane 24MHz
    { 0x300d, 0x2a },
    { 0x314c, 0x19 },
    { 0x314d, 0x01 },
    { 0x315a, 0x06 },
    { 0x3168, 0xa0 },
    { 0x316a, 0x7e },
    { 0x319e, 0x02 },    //891Mbps
    { 0x3a18, 0x7f },     // mipi phy
    { 0x3a19, 0x00 },
    { 0x3a1a, 0x37 },
    { 0x3a1b, 0x00 },
    { 0x3a1c, 0x37 },
    { 0x3a1d, 0x00 },
    { 0x3a1e, 0xf7 },
    { 0x3a1f, 0x00 },
    { 0x3a20, 0x3f },
    { 0x3a21, 0x00 },
    { 0x3a22, 0x6f },
    { 0x3a23, 0x00 },
    { 0x3a24, 0x3f },
    { 0x3a25, 0x00 },
    { 0x3a26, 0x5f },
    { 0x3a27, 0x00 },
    { 0x3a28, 0x2f },
    { 0x3a29, 0x00 },
    { 0x3302, 0x10 }, //black level
    { 0x3058, 0xac }, // shutter sweep time, 1000 ET Line
    { 0x3059, 0x0d },
    { 0x30e8, 0x00 },
    { 0x30e9, 0x00 },
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_4lane_raw10_dol_2x_regs[] = {
    { 0x3000, 0x01},
    { 0x3002, 0x01},
    { 0x3004, 0x04},
    { 0x3004, 0x00},
    //All pixel scan mode, A/D Conversion 10bit / Output 10 bit / 1188 Mbps /60fps
    { 0x3018, 0x00},
    { 0x3030, 0x8c}, //VMAX = 3980 = 0xf8c
    { 0x3031, 0x0f},	//0x0f
    { 0x3032, 0x00},

    //{ 0x3034, 0x13}, //30fps
    //{ 0x3035, 0x01},
    { 0x3034, 0x75}, //25fps, 0x175 = 373
    { 0x3035, 0x01},

    { 0x304c, 0x14},
    { 0x304e, 0x00},
    { 0x304f, 0x00},
    { 0x3050, 0x00},
    { 0x3056, 0xac},
    { 0x3057, 0x07},
    { 0x3072, 0x28},
    { 0x3073, 0x00},
    { 0x3074, 0xb0},
    { 0x3075, 0x00},
    { 0x3076, 0x58},
    { 0x3077, 0x0f},
    //All pixel
    //normal
    { 0x3078, 0x01},
    { 0x3079, 0x02},
    { 0x307a, 0xff},
    { 0x307b, 0x02},
    { 0x307c, 0x00},
    { 0x307d, 0x00},
    { 0x307e, 0x00},
    { 0x307f, 0x00},
    { 0x3080, 0x01},
    { 0x3081, 0x02},
    { 0x3082, 0xff},
    { 0x3083, 0x02},
    { 0x3084, 0x00},
    { 0x3085, 0x00},
    { 0x3086, 0x00},
    { 0x3087, 0x00},
    { 0x30a4, 0x33},
    { 0x30a8, 0x10},
    { 0x30a9, 0x04},
    { 0x30ac, 0x00},
    { 0x30ad, 0x00},
    { 0x30b0, 0x10},
    { 0x30b1, 0x08},
    { 0x30b4, 0x00},
    { 0x30b5, 0x00},
    { 0x30b6, 0x00},
    { 0x30b7, 0x00},
    { 0x3112, 0x08},
    { 0x3113, 0x00},
    { 0x3116, 0x08},
    { 0x3117, 0x00},
    //normal end
    { 0x3199, 0x00},
    { 0x319d, 0x00},
    { 0x3300, 0x00},
    { 0x341c, 0xff},
    { 0x341d, 0x01},
    { 0x3a01, 0x03},
    //phy timing
    { 0x3a18, 0x8f},
    { 0x3a19, 0x00},
    { 0x3a1a, 0x4f},
    { 0x3a1b, 0x00},
    { 0x3a1c, 0x47},
    { 0x3a1d, 0x00},
    { 0x3a1e, 0x37},
    { 0x3a1f, 0x01},
    { 0x3a20, 0x4f},
    { 0x3a21, 0x00},
    { 0x3a22, 0x87},
    { 0x3a23, 0x00},
    { 0x3a24, 0x4f},
    { 0x3a25, 0x00},
    { 0x3a26, 0x7f},
    { 0x3a27, 0x00},
    { 0x3a28, 0x3f},
    { 0x3a29, 0x00},
    //----1188Mbps/lane 24MHz inck
    { 0x300c, 0x3b},
    { 0x300d, 0x2a},
    { 0x314c, 0xc6},
    { 0x314d, 0x00},
    { 0x315a, 0x02},
    { 0x3168, 0xa0},
    { 0x316a, 0x7e},
    { 0x319e, 0x01},
    //start
    { 0x3000, 0x00},
    { 0x3002, 0x00},
    { 0x3302, 0x10}, //black level
    { 0x3058, (IMX335_VMAX_DOL2*2 - 1024)&0xFF}, //SHR0, 1000 ET Line, 3980*2 - 1024 = 0x1b18
    { 0x3059, (IMX335_VMAX_DOL2*2 - 1024)>>8},
    { 0x305a, 0x00},
    { 0x305c, (DOL2_RHS1 - (1024>>4))&0xFF}, //SHR1, 1024/16 ET Line, RHS1 - 64
    { 0x305d, (DOL2_RHS1 - (1024>>4))>>8},
    { 0x305e, 0x00},
    { 0x3068, (DOL2_RHS1&0xFF)}, //RHS1
    { 0x3069, (DOL2_RHS1>>8)},
    { 0x30e8, 0x00}, //gain
    { 0x30e9, 0x00},

    /* DOL related */
    { 0x319f, 0x03}, //VC
    { 0x3048, 0x01}, //DOL mode
    { 0x3049, 0x01}, //DOL 2 frame
    { 0x304c, 0x13}, //DOL HDR VC mode
    { 0x31d7, 0x01}, //master mode, 2 DOL
    { 0x304a, 0x04}, //exposure operation: DOL 2 frame
    { 0x304b, 0x03}, //exposure operation: DOL 2 frame/3 frame
    { REG_NULL, 0x00 }
};


static const k_sensor_reg imx335_mipi_4lane_raw10_3x_regs[] = {
    { 0x3000, 0x01},
    { 0x3002, 0x01},
    { 0x3004, 0x04},
    { 0x3004, 0x00},
    //All pixel scan mode, A/D Conversion 10bit / Output 10 bit / 1188 Mbps /60fps
    { 0x3018, 0x00}, //WINMODE, all-pixel scan mode
    { 0x3030, 0x94}, //VMAX = 4500
    { 0x3031, 0x11},
    { 0x3032, 0x00},

    //{ 0x3034, 0x13}, //15fps
    //{ 0x3035, 0x01},
    { 0x3034, 0x26}, //7.5fps, 0x226(550)
    { 0x3035, 0x02},

    { 0x304c, 0x14}, //vertical direction OB width = 20
    { 0x304e, 0x00}, //no mirror
    { 0x304f, 0x00}, //no flip
    { 0x3050, 0x00}, //AD 10bit
    { 0x3056, 0xac},
    { 0x3057, 0x07},
    { 0x3072, 0x28},
    { 0x3073, 0x00},
    { 0x3074, 0xb0},
    { 0x3075, 0x00},
    { 0x3076, 0x58},
    { 0x3077, 0x0f},
    //All pixel
    //normal
    { 0x3078, 0x01},
    { 0x3079, 0x02},
    { 0x307a, 0xff},
    { 0x307b, 0x02},
    { 0x307c, 0x00},
    { 0x307d, 0x00},
    { 0x307e, 0x00},
    { 0x307f, 0x00},
    { 0x3080, 0x01},
    { 0x3081, 0x02},
    { 0x3082, 0xff},
    { 0x3083, 0x02},
    { 0x3084, 0x00},
    { 0x3085, 0x00},
    { 0x3086, 0x00},
    { 0x3087, 0x00},
    { 0x30a4, 0x33},
    { 0x30a8, 0x10},
    { 0x30a9, 0x04},
    { 0x30ac, 0x00},
    { 0x30ad, 0x00},
    { 0x30b0, 0x10},
    { 0x30b1, 0x08},
    { 0x30b4, 0x00},
    { 0x30b5, 0x00},
    { 0x30b6, 0x00},
    { 0x30b7, 0x00},
    { 0x3112, 0x08},
    { 0x3113, 0x00},
    { 0x3116, 0x08},
    { 0x3117, 0x00},
    //normal end
    { 0x3199, 0x00},
    { 0x319d, 0x00}, //MDBIT, 10bit
    { 0x3300, 0x00},
    { 0x341c, 0xff}, //ADBIT1, 10bit
    { 0x341d, 0x01},
    { 0x3a01, 0x03}, //CSI-2 4lane
    //phy timing
    { 0x3a18, 0x8f},
    { 0x3a19, 0x00},
    { 0x3a1a, 0x4f},
    { 0x3a1b, 0x00},
    { 0x3a1c, 0x47},
    { 0x3a1d, 0x00},
    { 0x3a1e, 0x37},
    { 0x3a1f, 0x01},
    { 0x3a20, 0x4f},
    { 0x3a21, 0x00},
    { 0x3a22, 0x87},
    { 0x3a23, 0x00},
    { 0x3a24, 0x4f},
    { 0x3a25, 0x00},
    { 0x3a26, 0x7f},
    { 0x3a27, 0x00},
    { 0x3a28, 0x3f},
    { 0x3a29, 0x00},
    //----1188Mbps/lane 24MHz inck
    { 0x300c, 0x3b},
    { 0x300d, 0x2a},
    { 0x314c, 0xc6},
    { 0x314d, 0x00},
    { 0x315a, 0x02},
    { 0x3168, 0xa0},
    { 0x316a, 0x7e},
    { 0x319e, 0x01},
    //start
    { 0x3000, 0x00},
    { 0x3002, 0x00},
    { 0x3302, 0x10}, //black level
    { 0x3058, (IMX335_VMAX_DOL3*4 - 3072)&0xFF}, //SHR0, 3072 ET Line,  4500*4 - 3072
    { 0x3059, (IMX335_VMAX_DOL3*4 - 3072)>>8},
    { 0x305a, 0x00},
    { 0x305c, (DOL3_RHS1 - (3072>>4))&0xFF}, //SHR1, 3072/16 ET Line, RHS1 - 128
    { 0x305d, (DOL3_RHS1 - (3072>>4))>>8},
    { 0x305e, 0x00},
    { 0x3060, (DOL3_RHS2 - (3072>>8))&0xFF}, //SHR2, 3072/256 ET Line, RHS2 - 8
    { 0x3061, ((DOL3_RHS2 - (3072>>8))>>8)},
    { 0x3062, 0x00},
    { 0x3068, (DOL3_RHS1&0xFF)}, //RHS1
    { 0x3069, (DOL3_RHS1>>8)},
    { 0x306c, (DOL3_RHS2&0xFF)}, //RHS2
    { 0x306d, (DOL3_RHS2>>8)},
    { 0x30e8, 0x00}, //gain
    { 0x30e9, 0x00},

    /* DOL related */
    { 0x319f, 0x03}, //VC
    { 0x3048, 0x01}, //DOL mode
    { 0x3049, 0x02}, //DOL 3 frame
    { 0x304c, 0x13}, //DOL HDR VC mode
    { 0x31d7, 0x03}, //master mode, 3 DOL
    { 0x304a, 0x05}, //exposure operation: DOL 3 frame
    { 0x304b, 0x03}, //exposure operation: DOL 2 frame/3 frame
    { REG_NULL, 0x00 }
};

static k_sensor_mode imx335_mode_info[] = {
    {
        .index = 0,
        .sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR,
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
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_24m_regs,
    },
    {
        .index = 1,
        .sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_24m_regs,
    },
    {
        .index = 2,
        .sensor_type = IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_4lane_raw12_2592x1944_30fps_mclk_24m_regs,
    },

    {
        .index = 3,
        .sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR,
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
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_2lane_raw12_1920x1080_30fps_mclk_74_25_regs,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 8,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 4,
        .sensor_type = IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 2,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_2lane_raw12_2592x1944_30fps_mclk_74_25_regs,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 8,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 5,
        .sensor_type = IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_LINEAR,
        .bit_width = 12,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2C,
        },
        .reg_list = imx335_mipi_4lane_raw12_2592x1944_30fps_mclk_74_25_regs,
        .mclk_setting = {
            {
                .mclk_setting_en = K_TRUE,
                .setting.id = SENSOR_MCLK0,
                .setting.mclk_sel = SENSOR_PLL1_CLK_DIV4,
                .setting.mclk_div = 8,
            },
            {K_FALSE},
            {K_FALSE},
        },
    },
    {
        .index = 6,
        .sensor_type = IMX335_MIPI_4LANE_RAW10_2XDOL,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .stitching_mode = SENSOR_STITCHING_2DOL,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2B,
        },
        .reg_list = imx335_mipi_4lane_raw10_dol_2x_regs,
    },
    {
        .index = 7,
        .sensor_type = IMX335_MIPI_4LANE_RAW10_3XDOL,
        .size = {
            .bounds_width = 2592,
            .bounds_height = 1944,
            .top = 0,
            .left = 0,
            .width = 2592,
            .height = 1944,
        },
        .fps = 30000,
        .hdr_mode = SENSOR_MODE_HDR_STITCH,
        .stitching_mode = SENSOR_STITCHING_3DOL,
        .bit_width = 10,
        .bayer_pattern = BAYER_PAT_RGGB,
        .mipi_info = {
            .csi_id = 0,
            .mipi_lanes = 4,
            .data_type = 0x2B,
        },
        .reg_list = imx335_mipi_4lane_raw10_3x_regs,
    },
};

static k_sensor_mode* current_mode = NULL;

static int imx335_power_reset(k_s32 on)
{
    // #define IMX335_RST_PIN                  46
    // #define IMX335_MASTER_PIN               28

    k_u8 rst_gpio, master_gpio;

    rst_gpio = VICAP_IMX335_RST_GPIO;
    if(VICAP_IMX335_MASTER_GPIO != 255) {
        master_gpio = VICAP_IMX335_MASTER_GPIO;

        kd_pin_mode(rst_gpio, GPIO_DM_OUTPUT);
        kd_pin_mode(master_gpio, GPIO_DM_OUTPUT);

        kd_pin_write(master_gpio, GPIO_PV_LOW);
    }

    if (on) {
        kd_pin_write(rst_gpio, GPIO_PV_HIGH); // GPIO_PV_HIGH
        rt_thread_mdelay(1);
        kd_pin_write(rst_gpio, GPIO_PV_LOW); // GPIO_PV_LOW
        rt_thread_mdelay(1);
        kd_pin_write(rst_gpio, GPIO_PV_HIGH); // GPIO_PV_HIGH
    } else {
        kd_pin_write(rst_gpio, GPIO_PV_LOW); // GPIO_PV_LOW
    }

    rt_thread_mdelay(1);

    return 0;
}

static int imx335_i2c_init(k_sensor_i2c_info* i2c_info)
{
    i2c_info->i2c_bus = rt_i2c_bus_device_find(i2c_info->i2c_name);
    if (i2c_info->i2c_bus == RT_NULL) {
        pr_err("can't find %s deivce", i2c_info->i2c_name);
        return RT_ERROR;
    }

    return 0;
}

static k_s32 imx335_sensor_get_chip_id(void* ctx, k_u32* chip_id)
{
    k_s32 ret = 0;
    k_u16 id_high = 0;
    k_u16 id_low = 0;
    struct sensor_driver_dev* dev = ctx;
    pr_info("%s enter\n", __func__);

    kd_pin_mode(VICAP_IMX335_RST_GPIO, GPIO_DM_OUTPUT);
    kd_pin_write(VICAP_IMX335_RST_GPIO, GPIO_PV_HIGH); // GPIO_PV_HIGH

    imx335_i2c_init(&dev->i2c_info);

    ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_ID, &id_high);
    ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_ID + 1, &id_low);
    if (ret) {
        // rt_kprintf("%s error\n", __func__);
        return -1;
    }

    *chip_id = (id_high << 8) | id_low;
    pr_info("%s chip_id[0x%08X]\n", __func__, *chip_id);
    return ret;
}

static k_s32 imx335_sensor_power_on(void* ctx, k_s32 on)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;
    k_u32 chip_id = 0;
    pr_info("%s enter, %d\n", __func__, on);
    if (on) {
        imx335_power_reset(0);
        imx335_i2c_init(&dev->i2c_info);
        imx335_power_reset(1);
        ret = imx335_sensor_get_chip_id(ctx, &chip_id);
        if(ret < 0)
        {
            pr_err("%s, iic read chip id err \n", __func__);
        }
    } else {
        imx335_power_reset(on);
    }

    return ret;
}

static k_s32 imx335_sensor_init(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;
    k_s32 i = 0;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode == NULL) {
        for (i = 0; i < sizeof(imx335_mode_info) / sizeof(k_sensor_mode); i++) {
            if (imx335_mode_info[i].sensor_type == mode.sensor_type) {
                current_mode = &(imx335_mode_info[i]);
                dev->sensor_mode = &(imx335_mode_info[i]);
                break;
            }
        }
    }

    if (current_mode == NULL) {
        pr_err("%s, current mode not exit.\n", __func__);
        return -1;
    }

    switch (current_mode->index) {

    case 6:	// HDR 2DOL
        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        current_mode->ae_info.frame_length = IMX335_VMAX_DOL2<<1;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000005025; // s, one_line_exp_time
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 50.0;//powf(10, 0.015*113.0);//

        current_mode->ae_info.int_time_delay_frame = 1;
        current_mode->ae_info.gain_delay_frame = 1;

        current_mode->ae_info.color_type = SENSOR_COLOR;    //color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time * 4;
        current_mode->ae_info.gain_increment = IMX335_AGAIN_STEP;

        current_mode->ae_info.max_long_integraion_line = 7460;  // 116*64 = 7424
        current_mode->ae_info.min_long_integraion_line = 64;	//1 * 64

        current_mode->ae_info.max_integraion_line = 464;	//116 * 4
        current_mode->ae_info.min_integraion_line = 4;	//1*4

        current_mode->ae_info.max_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_long_integraion_line;
        current_mode->ae_info.min_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_integraion_line;
        current_mode->ae_info.min_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;

        current_mode->ae_info.cur_long_again = 0.0;
        current_mode->ae_info.cur_long_dgain = 0.0;

        current_mode->ae_info.cur_again = 1.0;
        current_mode->ae_info.cur_dgain = 1.0;

        current_mode->ae_info.a_long_gain.min = 1.0;
        current_mode->ae_info.a_long_gain.max = 100.0;
        current_mode->ae_info.a_long_gain.step = (1.0f / 256.0f);

        current_mode->ae_info.a_gain.min = 1.0;
        current_mode->ae_info.a_gain.max = 100.0;
        current_mode->ae_info.a_gain.step = (1.0f / 256.0f);

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

        case 7:	// HDR 3DOL
        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        current_mode->ae_info.frame_length = IMX335_VMAX_DOL3<<2;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000007407;	 // s, one_line_exp_time
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 50.0;//powf(10, 0.015*113.0);//

        current_mode->ae_info.int_time_delay_frame = 1;
        current_mode->ae_info.gain_delay_frame = 1;

        current_mode->ae_info.color_type = SENSOR_COLOR;    //color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time * 6;
        current_mode->ae_info.gain_increment = IMX335_AGAIN_STEP;

        current_mode->ae_info.max_long_integraion_line = 15360;	// * 6 * 256
        current_mode->ae_info.min_long_integraion_line = 1536;	// * 6 * 256

        current_mode->ae_info.max_integraion_line = 960; // * 6 * 16
        current_mode->ae_info.min_integraion_line = 96; // * 6 * 16

        current_mode->ae_info.max_vs_integraion_line = 60;	//  * 6
        current_mode->ae_info.min_vs_integraion_line = 6;	//  * 6

        current_mode->ae_info.max_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_long_integraion_line;
        current_mode->ae_info.min_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_integraion_line;
        current_mode->ae_info.min_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.max_vs_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_vs_integraion_line;
        current_mode->ae_info.min_vs_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_vs_integraion_line;

        current_mode->ae_info.cur_long_integration_time = 0.0;
        current_mode->ae_info.cur_integration_time = 0.0;
        current_mode->ae_info.cur_vs_integration_time = 0.0;

        current_mode->ae_info.cur_long_again = 1.0;
        current_mode->ae_info.cur_long_dgain = 1.0;

        current_mode->ae_info.cur_again = 1.0;
        current_mode->ae_info.cur_dgain = 1.0;

        current_mode->ae_info.cur_vs_again = 1.0;
        current_mode->ae_info.cur_vs_dgain = 1.0;

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

    default:
        ret = sensor_reg_list_write(&dev->i2c_info, current_mode->reg_list);

        current_mode->ae_info.frame_length = IMX335_VMAX_LINEAR;
        current_mode->ae_info.cur_frame_length = current_mode->ae_info.frame_length;
        current_mode->ae_info.one_line_exp_time = 0.000007407; // s
        current_mode->ae_info.gain_accuracy = 1024;

        current_mode->ae_info.min_gain = 1.0;
        current_mode->ae_info.max_gain = 50.0;//powf(10, 0.015*113.0);//

        current_mode->ae_info.int_time_delay_frame = 1;
        current_mode->ae_info.gain_delay_frame = 1;
        //current_mode->ae_info.ae_min_interval_frame = 2.5;
        current_mode->ae_info.color_type = SENSOR_COLOR;    //color sensor

        current_mode->ae_info.integration_time_increment = current_mode->ae_info.one_line_exp_time;
        current_mode->ae_info.gain_increment = IMX335_AGAIN_STEP;

        current_mode->ae_info.max_long_integraion_line = current_mode->ae_info.frame_length - 9;
        current_mode->ae_info.min_long_integraion_line = 1;

        current_mode->ae_info.max_integraion_line = current_mode->ae_info.frame_length - 9;
        current_mode->ae_info.min_integraion_line = 1;

        current_mode->ae_info.max_vs_integraion_line = current_mode->ae_info.frame_length - 9;
        current_mode->ae_info.min_vs_integraion_line = 1;

        current_mode->ae_info.max_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_long_integraion_line;
        current_mode->ae_info.min_long_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_long_integraion_line;

        current_mode->ae_info.max_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_integraion_line;
        current_mode->ae_info.min_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_integraion_line;

        current_mode->ae_info.max_vs_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.max_vs_integraion_line;
        current_mode->ae_info.min_vs_integraion_time = current_mode->ae_info.one_line_exp_time * current_mode->ae_info.min_vs_integraion_line;

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

    k_u16 again_h, again_l;
    float again = 0, dgain = 0;

    ret = sensor_reg_read(&dev->i2c_info,  IMX335_REG_AGAIN_L, &again_l);
    ret = sensor_reg_read(&dev->i2c_info,  IMX335_REG_AGAIN_H, &again_h);
    //again = (float)(((again_h & 0x07) << 8) | again_l) * 0.015f;    //db value/20, (RegVal * 3/10)/20
    again = (float)((again_h & 0x07)<<8 | again_l) * 0.015f;
    again = powf(10, again);    //times value

    dgain = 1.0;
    current_mode->ae_info.cur_gain = again * dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;

	if(current_mode->hdr_mode == SENSOR_MODE_LINEAR)
	{
	    k_u16 SHR0_m, SHR0_l;
	    k_u32 exp_time;
	    ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_L, &SHR0_l);
	    ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_M, &SHR0_m);
	    exp_time =IMX335_VMAX_LINEAR - ((SHR0_m <<8) | SHR0_l);

	    current_mode->ae_info.cur_integration_time =  current_mode->ae_info.one_line_exp_time * exp_time;
  	}
  	else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH)
  	{
  		if(current_mode->index ==6)	//HDR 2DOL
  		{
  			k_u16 SHR0_m, SHR0_l;
  			k_u16 SHR1_m, SHR1_l;
  			k_u16 RHS1_m, RHS1_l;	//default setting is 522
  			k_u32 exp_time, exp_shorttime;

  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_L, &SHR0_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_M, &SHR0_m);
  			exp_time = current_mode->ae_info.frame_length - ((SHR0_m <<8) | SHR0_l);
  			current_mode->ae_info.cur_long_integration_time = current_mode->ae_info.one_line_exp_time * exp_time;

  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR1_L, &SHR1_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR1_M, &SHR1_m);
  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS1_L, &RHS1_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS1_M, &RHS1_m);
  			exp_shorttime = ((RHS1_m <<8) | RHS1_l) - ((SHR1_m <<8) | SHR1_l);
  			current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time * exp_shorttime;

  		}
  		else if(current_mode->index ==7)	//HDR 3DOL
  		{
  			k_u16 SHR0_m, SHR0_l;
  			k_u16 SHR1_m, SHR1_l;
  			k_u16 SHR2_m, SHR2_l;
  			k_u16 RHS1_m, RHS1_l;
  			k_u16 RHS2_m, RHS2_l;
  			k_u32 exp_time, exp_shorttime, exp_vstime;

  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_L, &SHR0_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR0_M, &SHR0_m);
  			exp_time = current_mode->ae_info.frame_length - ((SHR0_m <<8) | SHR0_l);
  			current_mode->ae_info.cur_long_integration_time = current_mode->ae_info.one_line_exp_time * exp_time;

  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR1_L, &SHR1_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR1_M, &SHR1_m);
  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS1_L, &RHS1_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS1_M, &RHS1_m);
  			exp_shorttime = ((RHS1_m <<8) | RHS1_l) - ((SHR1_m <<8) | SHR1_l);
  			current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time * exp_shorttime;

  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR2_L, &SHR2_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_SHR2_M, &SHR2_m);
  			ret = sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS2_L, &RHS2_l);
  			ret |= sensor_reg_read(&dev->i2c_info, IMX335_REG_RHS2_M, &RHS2_m);
  			exp_vstime = ((RHS2_m <<8) | RHS2_l) - ((SHR2_m <<8) | SHR2_l);
  			current_mode->ae_info.cur_vs_integration_time = current_mode->ae_info.one_line_exp_time * exp_vstime;

  		}
  	}

    pr_info("%s exit, sensor_type:%d\n", __func__, mode.sensor_type);
    return ret;
}

static k_s32 imx335_sensor_get_mode(void* ctx, k_sensor_mode* mode)
{
    k_s32 ret = -1;

    for (k_s32 i = 0; i < sizeof(imx335_mode_info) / sizeof(k_sensor_mode); i++) {
        if (imx335_mode_info[i].sensor_type == mode->sensor_type) {
            memcpy(mode, &imx335_mode_info[i], sizeof(k_sensor_mode));
            current_mode = &(imx335_mode_info[i]);
            return 0;
        }
    }
    pr_debug("%s, the mode not exit.\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_set_mode(void* ctx, k_sensor_mode mode)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_enum_mode(void* ctx, k_sensor_enum_mode* enum_mode)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    if (enum_mode->index >= (sizeof(imx335_mode_info) / sizeof(k_sensor_mode))) {
        pr_err("%s, invalid mode index.\n", __func__);
        return -1;
    }

    for (k_s32 i = 0; i < sizeof(imx335_mode_info) / sizeof(k_sensor_mode); i++) {
        if (imx335_mode_info[i].index == enum_mode->index) {
            memcpy(&enum_mode->mode, &imx335_mode_info[i], sizeof(k_sensor_mode));
            return 0;
        }
    }
    return ret;
}

static k_s32 imx335_sensor_get_caps(void* ctx, k_sensor_caps* caps)
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

static k_s32 imx335_sensor_conn_check(void* ctx, k_s32* conn)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    *conn = 1;

    return ret;
}

static k_s32 imx335_sensor_set_stream(void* ctx, k_s32 enable)
{
    k_s32 ret = 0;
    struct sensor_driver_dev* dev = ctx;

    pr_info("%s enter, enable(%d)\n", __func__, enable);
    if (enable) {
        ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_MODE_SELECT, IMX335_MODE_STREAMING);
    } else {
        ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_MODE_SELECT, IMX335_MODE_STANDBY);
    }
    pr_info("%s exit, ret(%d)\n", __func__, ret);

    return ret;
}

static k_s32 imx335_sensor_get_again(void* ctx, k_sensor_gain* gain)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        gain->gain[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_again;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    	if(current_mode->index == 6)
    	{
	        gain->gain[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_long_again;
	        gain->gain[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_again;
      	}
      	else if(current_mode->index == 7)
      	{
      		gain->gain[SENSOR_TRI_EXP_L_PARAS] = current_mode->ae_info.cur_long_again;
	        gain->gain[SENSOR_TRI_EXP_S_PARAS] = current_mode->ae_info.cur_again;
        	gain->gain[SENSOR_TRI_EXP_VS_PARAS] = current_mode->ae_info.cur_vs_again;
      	}
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }

    return ret;
}

static k_s32 imx335_sensor_set_again(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u16 again;
    float SensorGain;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        again = (k_u16)(log10f(gain.gain[SENSOR_LINEAR_PARAS])*200.0f/3.0f + 0.5f);     //20*log(gain)*10/3
        if(current_mode->sensor_again !=again)
        {
            ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_AGAIN_L,(again & 0xff));
            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_AGAIN_H,(again & 0x0700)>>8);
            current_mode->sensor_again = again;
        }
        SensorGain = (float)(current_mode->sensor_again) * 0.015f;    //db value/20,(RegVal * 3/10)/20
        current_mode->ae_info.cur_again = powf(10, SensorGain);
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        again = (k_u16)(log10f(gain.gain[SENSOR_DUAL_EXP_L_PARAS])*200.0f/3.0f + 0.5f);     //20*log(gain)*10/3
        ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_AGAIN_L,(again & 0xff));
        ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_AGAIN_H,(again & 0x0700)>>8);


        SensorGain = (float)(again) * 0.015f;    //db value/20,(RegVal * 3/10)/20
        current_mode->ae_info.cur_long_again = powf(10, SensorGain);

        //again = (k_u32)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 16);
        // TODO
        //current_mode->ae_info.cur_vs_again = again / 16.0f;
        current_mode->ae_info.cur_again = current_mode->ae_info.cur_long_again;
        current_mode->ae_info.cur_vs_again = current_mode->ae_info.cur_long_again;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s, hdr_mode(%d), cur_again(%u)\n", __func__, current_mode->hdr_mode, (k_u32)(current_mode->ae_info.cur_again * 1000));

    return ret;
}

static k_s32 imx335_sensor_get_dgain(void* ctx, k_sensor_gain* gain)
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

static k_s32 imx335_sensor_set_dgain(void* ctx, k_sensor_gain gain)
{
    k_s32 ret = 0;
    k_u32 dgain;
    struct sensor_driver_dev* dev = ctx;

    pr_debug("%s enter hdr_mode(%d)\n", __func__, current_mode->hdr_mode);
    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        dgain = (k_u32)(gain.gain[SENSOR_LINEAR_PARAS] * 1024);
        current_mode->ae_info.cur_dgain = dgain / 1024.0f;

    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
        dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_L_PARAS] * 1024);
        current_mode->ae_info.cur_long_dgain = dgain / 1024.0f;

        //dgain = (k_u32)(gain.gain[SENSOR_DUAL_EXP_S_PARAS] * 1024);
        // TODO wirte vs gain register
        current_mode->ae_info.cur_dgain = current_mode->ae_info.cur_long_dgain;
        current_mode->ae_info.cur_vs_dgain = current_mode->ae_info.cur_long_dgain;
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    current_mode->ae_info.cur_gain = current_mode->ae_info.cur_again * current_mode->ae_info.cur_dgain;
    current_mode->ae_info.cur_long_gain = current_mode->ae_info.cur_gain;
    current_mode->ae_info.cur_vs_gain = current_mode->ae_info.cur_gain;
    pr_debug("%s,cur_gain(%d)\n", __func__, (k_u32)(current_mode->ae_info.cur_gain * 10000));

    return ret;
}

static k_s32 imx335_sensor_get_intg_time(void* ctx, k_sensor_intg_time* time)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR) {
        time->intg_time[SENSOR_LINEAR_PARAS] = current_mode->ae_info.cur_integration_time;
    } else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH) {
    	if(current_mode->index == 6)
    	{
	        time->intg_time[SENSOR_DUAL_EXP_L_PARAS] = current_mode->ae_info.cur_long_integration_time;
	        time->intg_time[SENSOR_DUAL_EXP_S_PARAS] = current_mode->ae_info.cur_integration_time;
    	}
    	else if(current_mode->index == 7)
    	{
	        time->intg_time[SENSOR_TRI_EXP_L_PARAS] = current_mode->ae_info.cur_long_integration_time;
	        time->intg_time[SENSOR_TRI_EXP_S_PARAS] = current_mode->ae_info.cur_integration_time;
	        time->intg_time[SENSOR_TRI_EXP_VS_PARAS] = current_mode->ae_info.cur_vs_integration_time;
    	}
    } else {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }

    return ret;
}

static k_s32 imx335_sensor_set_intg_time(void* ctx, k_sensor_intg_time time)
{
    k_s32 ret = 0;
    k_u32 exp_line = 0;
    float integraion_time = 0;
    struct sensor_driver_dev* dev = ctx;

    if (current_mode->hdr_mode == SENSOR_MODE_LINEAR)
    {
        integraion_time = time.intg_time[SENSOR_LINEAR_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_line));
        if (current_mode->et_line != exp_line)
        {
            k_u16 SHR0 = IMX335_VMAX_LINEAR - exp_line;
            ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_L, SHR0 & 0xff);
            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_M, (SHR0 >> 8) & 0xff);
            current_mode->et_line = exp_line;
        }
        current_mode->ae_info.cur_integration_time = (float)current_mode->et_line * current_mode->ae_info.one_line_exp_time;
    }
    else if (current_mode->hdr_mode == SENSOR_MODE_HDR_STITCH)
    {
        integraion_time = time.intg_time[SENSOR_DUAL_EXP_L_PARAS];
        exp_line = integraion_time / current_mode->ae_info.one_line_exp_time;
        exp_line = MIN(current_mode->ae_info.max_long_integraion_line, MAX(current_mode->ae_info.min_long_integraion_line, exp_line));
        if(current_mode->index ==6)
        {
        	exp_line = ((exp_line + 2)>>2)<<2;
        	float temp = (float)exp_line/(4.0*DOL2_ratio);
        	k_u16 exp_s_line =(int)( temp+0.5)<<2;
        	exp_s_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_s_line));
	        if (current_mode->et_line != exp_line)
	        {
	            k_u16 SHR0 = current_mode->ae_info.frame_length - exp_line;
	            k_u16 RHS1 = DOL2_RHS1;
	            k_u16 SHR1 = RHS1 - exp_s_line;
	            ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_L, SHR0 & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_M, (SHR0 >> 8) & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_L, SHR1 & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_M, (SHR1 >> 8) & 0xff);
	            current_mode->et_line = exp_line;
	        }
	        current_mode->ae_info.cur_long_integration_time = current_mode->ae_info.one_line_exp_time * exp_line;
	        current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time * exp_s_line;
        }
        else if(current_mode->index ==7)
        {
        	float temp = (float)exp_line/6;
        	exp_line = (int)(temp+0.5)*6;
        	temp = (float)exp_line/(6.0*DOL3_LS_ratio);
        	k_u16 exp_s_line = (int)(temp+0.5)*6;
        	exp_s_line = MIN(current_mode->ae_info.max_integraion_line, MAX(current_mode->ae_info.min_integraion_line, exp_s_line));
        	temp = (float)exp_line/(6.0*DOL3_LS_ratio*DOL3_VS_ratio);
        	k_u16 exp_vs_line = (int)(temp+0.5)*6;
        	exp_vs_line = MIN(current_mode->ae_info.max_vs_integraion_line, MAX(current_mode->ae_info.min_vs_integraion_line, exp_vs_line));
        	if (current_mode->et_line != exp_line)
	        {
	            k_u16 SHR0 = current_mode->ae_info.frame_length - exp_line;
	            k_u16 RHS1 = DOL3_RHS1;
	            k_u16 SHR1 = RHS1 - exp_s_line;
	            k_u16 RHS2 = DOL3_RHS2;
	            k_u16 SHR2 = RHS2 - exp_vs_line;
	            ret = sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_L, SHR0 & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR0_M, (SHR0 >> 8) & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_L, SHR1 & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_M, (SHR1 >> 8) & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_L, SHR2 & 0xff);
	            ret |= sensor_reg_write(&dev->i2c_info, IMX335_REG_SHR1_M, (SHR2 >> 8) & 0xff);
	            current_mode->et_line = exp_line;
	        }
	        current_mode->ae_info.cur_long_integration_time = current_mode->ae_info.one_line_exp_time * exp_line;
	        current_mode->ae_info.cur_integration_time = current_mode->ae_info.one_line_exp_time * exp_s_line;
	        current_mode->ae_info.cur_vs_integration_time = current_mode->ae_info.one_line_exp_time * exp_vs_line;
        }
    }
    else
    {
        pr_err("%s, unsupport exposure frame.\n", __func__);
        return -1;
    }
    pr_debug("%s hdr_mode(%d), exp_line(%d), integraion_time(%u)\n",
        __func__, current_mode->hdr_mode, exp_line, (k_u32)(integraion_time * 1000000000));

    return ret;
}

static k_s32 imx335_sensor_get_exp_parm(void* ctx, k_sensor_exposure_param* exp_parm)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(exp_parm, 0, sizeof(k_sensor_exposure_param));

    return ret;
}

static k_s32 imx335_sensor_set_exp_parm(void* ctx, k_sensor_exposure_param exp_parm)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_get_fps(void* ctx, k_u32* fps)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    *fps = current_mode->fps;

    return ret;
}

static k_s32 imx335_sensor_set_fps(void* ctx, k_u32 fps)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_get_isp_status(void* ctx, k_sensor_isp_status* staus)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(staus, 0, sizeof(k_sensor_isp_status));

    return ret;
}

static k_s32 imx335_sensor_set_blc(void* ctx, k_sensor_blc blc)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_set_wb(void* ctx, k_sensor_white_balance wb)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_get_tpg(void* ctx, k_sensor_test_pattern* tpg)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(tpg, 0, sizeof(k_sensor_test_pattern));

    return ret;
}

static k_s32 imx335_sensor_set_tpg(void* ctx, k_sensor_test_pattern tpg)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);

    return ret;
}

static k_s32 imx335_sensor_get_expand_curve(void* ctx, k_sensor_compand_curve* curve)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(curve, 0, sizeof(k_sensor_compand_curve));

    return ret;
}

static k_s32 imx335_sensor_get_otp_data(void* ctx, void* data)
{
    k_s32 ret = 0;

    pr_debug("%s enter\n", __func__);
    memset(data, 0, sizeof(void*));

    return ret;
}

static k_s32 imx335_sensor_mirror_set(void *ctx, k_vicap_mirror_mode mirror)
{
    return 0;
}

struct sensor_driver_dev imx335_sensor_drv = {
    .i2c_info = {
        .i2c_bus = NULL,
        .i2c_name = "i2c0",
        .slave_addr = 0x1a,
        .reg_addr_size = SENSOR_REG_VALUE_16BIT,
        .reg_val_size = SENSOR_REG_VALUE_8BIT,
    },
    .sensor_name = "imx335",
    .sensor_func = {
        .sensor_power = imx335_sensor_power_on,
        .sensor_init = imx335_sensor_init,
        .sensor_get_chip_id = imx335_sensor_get_chip_id,
        .sensor_get_mode = imx335_sensor_get_mode,
        .sensor_set_mode = imx335_sensor_set_mode,
        .sensor_enum_mode = imx335_sensor_enum_mode,
        .sensor_get_caps = imx335_sensor_get_caps,
        .sensor_conn_check = imx335_sensor_conn_check,
        .sensor_set_stream = imx335_sensor_set_stream,
        .sensor_get_again = imx335_sensor_get_again,
        .sensor_set_again = imx335_sensor_set_again,
        .sensor_get_dgain = imx335_sensor_get_dgain,
        .sensor_set_dgain = imx335_sensor_set_dgain,
        .sensor_get_intg_time = imx335_sensor_get_intg_time,
        .sensor_set_intg_time = imx335_sensor_set_intg_time,
        .sensor_get_exp_parm = imx335_sensor_get_exp_parm,
        .sensor_set_exp_parm = imx335_sensor_set_exp_parm,
        .sensor_get_fps = imx335_sensor_get_fps,
        .sensor_set_fps = imx335_sensor_set_fps,
        .sensor_get_isp_status = imx335_sensor_get_isp_status,
        .sensor_set_blc = imx335_sensor_set_blc,
        .sensor_set_wb = imx335_sensor_set_wb,
        .sensor_get_tpg = imx335_sensor_get_tpg,
        .sensor_set_tpg = imx335_sensor_set_tpg,
        .sensor_get_expand_curve = imx335_sensor_get_expand_curve,
        .sensor_get_otp_data = imx335_sensor_get_otp_data,
        .sensor_mirror_set = imx335_sensor_mirror_set,
    },
};
