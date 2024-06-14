/**
* @file k_ao_comm.h
* @author
* @sxp
* @version 1.0
* @date 2022-10-21
*
* @copyright
* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#ifndef __K_BOARD_CONFIG_COMM_H__
#define __K_BOARD_CONFIG_COMM_H__


#include "k_autoconf_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


// sip gpio board config 
#if defined(CONFIG_BOARD_K230D)
// display gpio 
#define DISPLAY_LCD_RST_GPIO                            21
#define DISPLAY_LCD_BACKLIGHT_EN                        20

// imx335 gpio config 
#define VICAP_IMX335_RST_GPIO                           37
#define VICAP_IMX335_MASTER_GPIO                        33
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           35
#define OV5647_IIC "i2c1"
#define OV5647_CAM_PIN                                  20
#define OV5647_CAM_PIN_CSI2                             (24)
#define OV5647_CAM_PIN_CSI1                             (23)

#define OV5647_CSI2_IIC "i2c1"
#define OV5647_CSI1_IIC "i2c0"
#define OV9732_RST_PIN                                  (28)

#define LT9611_RESET_GPIO   42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c4"

#elif defined(CONFIG_BOARD_K230_EVB)
// usip evb gpio config 
// display gpio 
#define DISPLAY_LCD_RST_GPIO                            9
#define DISPLAY_LCD_BACKLIGHT_EN                        31

// imx335 gpio config 
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           49
#define OV5647_IIC "i2c1"
#define OV5647_CAM_PIN                                  20
#define OV5647_CAM_PIN_CSI2                             (24)
#define OV5647_CAM_PIN_CSI1                             (23)

#define OV5647_CSI2_IIC "i2c1"
#define OV5647_CSI1_IIC "i2c0"
#define OV9732_RST_PIN                                  (28)

#define LT9611_RESET_GPIO   42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c4"

#elif defined(CONFIG_BOARD_K230_CANMV)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            20
#define DISPLAY_LCD_BACKLIGHT_EN                        25

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           23
#define OV5647_IIC "i2c3"
#define OV5647_CAM_PIN                                  0
#define OV5647_CAM_PIN_CSI2                             (24)
#define OV5647_CAM_PIN_CSI1                             (23)

#define OV5647_CSI2_IIC "i2c1"
#define OV5647_CSI1_IIC "i2c0"
#define OV9732_RST_PIN                                  (24)

#define LT9611_RESET_GPIO   42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c4"

#elif defined(CONFIG_BOARD_K230D_CANMV)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            24
#define DISPLAY_LCD_BACKLIGHT_EN                        25

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           20
#define OV5647_IIC "i2c4"
#define OV5647_CAM_PIN                                  20
#define OV5647_CAM_PIN_CSI2                             (24)
#define OV5647_CAM_PIN_CSI1                             (23)
#define OV9732_RST_PIN                                  (24)

#define OV5647_CSI2_IIC "i2c1"
#define OV5647_CSI1_IIC "i2c0"

#define LT9611_RESET_GPIO   42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c4"

#elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            24
#define DISPLAY_LCD_BACKLIGHT_EN                        25

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           20
#define OV5647_CSI2_IIC "i2c4"
#define OV5647_CSI1_IIC "i2c1"
#define OV5647_IIC "i2c0"
#define OV5647_CAM_PIN                                  45
#define OV5647_CAM_PIN_CSI2                             (62)
#define OV5647_CAM_PIN_CSI1                             (10)
#define OV9732_RST_PIN                                  (28)


#define LT9611_RESET_GPIO                               22
#define LT9611_SLAVE_ADDR                               0x3b
#define LT9611_I2C_BUS                                  "i2c3"

#elif defined(CONFIG_BOARD_K230_CANMV_V2)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            22
#define DISPLAY_LCD_BACKLIGHT_EN                        25

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           20

#define OV5647_IIC "i2c0"
#define OV5647_CAM_PIN                                  61

#define OV5647_CSI1_IIC  "i2c1"
#define OV5647_CSI2_IIC "i2c4"

#define OV5647_CAM_PIN_CSI2                             (21)
#define OV5647_CAM_PIN_CSI1                             (60)

#define OV9732_RST_PIN                                  (28)

#define LT9611_RESET_GPIO   24 //42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c3"//"i2c4"

#elif defined(CONFIG_BOARD_K230_CANMV_DONGSHANPI)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            22
#define DISPLAY_LCD_BACKLIGHT_EN                        25

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           20

#define OV5647_IIC "i2c0"
#define OV5647_CAM_PIN                                  61

#define OV5647_CSI1_IIC  "i2c1"
#define OV5647_CSI2_IIC "i2c4"

#define OV5647_CAM_PIN_CSI2                             (21)
#define OV5647_CAM_PIN_CSI1                             (60)

#define OV9732_RST_PIN                                  (28)

#define LT9611_RESET_GPIO   24 //42
#define LT9611_SLAVE_ADDR   0x3b
#define LT9611_I2C_BUS      "i2c3"//"i2c4"

#else

#define DISPLAY_LCD_RST_GPIO                            9
#define DISPLAY_LCD_BACKLIGHT_EN                        31

// imx335 gpio config 
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           49
#define OV5647_IIC "i2c1"
#define OV5647_CAM_PIN                                  20
#define OV5647_CSI2_IIC "i2c1"
#define OV5647_CSI1_IIC "i2c0"
#define OV5647_CAM_PIN_CSI2                             (24)
#define OV5647_CAM_PIN_CSI1                             (23)
#define OV9732_RST_PIN                                  (24)

#define LT9611_RESET_GPIO                               42
#define LT9611_SLAVE_ADDR                               0x3b
#define LT9611_I2C_BUS                                  "i2c4"

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
