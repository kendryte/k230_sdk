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

#elif defined(CONFIG_BOARD_K230_CANMV)
// usip evb gpio config
// display gpio
#define DISPLAY_LCD_RST_GPIO                            9
#define DISPLAY_LCD_BACKLIGHT_EN                        31

// imx335 gpio config
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           49
#define OV5647_IIC "i2c3"


#else

#define DISPLAY_LCD_RST_GPIO                            9
#define DISPLAY_LCD_BACKLIGHT_EN                        31

// imx335 gpio config 
#define VICAP_IMX335_RST_GPIO                           46
#define VICAP_IMX335_MASTER_GPIO                        28
//OV9286 gpio cinfig
#define VICAP_OV9286_RST_GPIO                           49
#define OV5647_IIC "i2c1"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
