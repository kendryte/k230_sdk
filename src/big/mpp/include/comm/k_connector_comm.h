/**
 * @file k_sensor_comm.h
 * @author
 * @sxp
 * @version 1.0
 * @date 2023-03-20
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
#ifndef __K_CONNECTOR_COMM_H__
#define __K_CONNECTOR_COMM_H__

#include "k_type.h"
#include "k_errno.h"
#include "k_module.h"
#include "k_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define CONNECTOR_NUM_MAX                                  128


#define BACKGROUND_BLACK_COLOR                            (0x808000)
#define BACKGROUND_PINK_COLOR                             (0xffffff)


typedef enum {
    HX8377_V2_MIPI_4LAN_1080X1920_30FPS = 0,
    LT9611_MIPI_4LAN_1920X1080_30FPS = 1,
    ST7701_V1_MIPI_2LAN_480X800_30FPS = 2,
    ST7701_V1_MIPI_2LAN_480X854_30FPS = 3,
    ILI9806_MIPI_2LAN_480X800_30FPS = 4,
    LT9611_MIPI_ADAPT_RESOLUTION = 100,
    LT9611_MIPI_4LAN_1920X1080_60FPS,
    LT9611_MIPI_4LAN_1920X1080_50FPS,
    LT9611_MIPI_4LAN_1920X1080_25FPS,
    LT9611_MIPI_4LAN_1920X1080_24FPS,
    LT9611_MIPI_4LAN_1280X720_60FPS = 110,
    LT9611_MIPI_4LAN_1280X720_50FPS,
    LT9611_MIPI_4LAN_1280X720_30FPS,
    LT9611_MIPI_4LAN_640X480_60FPS = 120,
    VIRTUAL_DISPLAY_DEVICE = 200,
} k_connector_type;

typedef struct
{
    k_u32 n;
    k_u32 m;
    k_u32 voc;
    k_u32 hs_freq;
} k_connectori_phy_attr;


typedef struct {
    const char *connector_name;
    k_u32 screen_test_mode;
    k_u32 dsi_test_mode;
    k_u32 bg_color;
    k_u32 intr_line;
    k_u32 pixclk_div;
    k_dsi_lan_num lan_num;
    k_dsi_work_mode work_mode;
    k_vo_dsi_cmd_mode cmd_mode;
    k_connectori_phy_attr phy_attr;
    k_vo_display_resolution resolution;
    k_connector_type type;
} k_connector_info;

typedef enum
{
    K_CONNECTOR_MIRROR_HOR = 1, 
    K_CONNECTOR_MIRROR_VER,
    K_CONNECTOR_MIRROR_BOTH,
}k_connector_mirror;


typedef struct
{
    k_u32 connection_status;
    k_u32 negotiated_count;
    k_connector_type negotiated_types[256];
} k_connector_negotiated_data;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
