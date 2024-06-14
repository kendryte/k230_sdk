/**
 * @file vo_test_case.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-01
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
#ifndef __VO_TEST_CASE_H__
#define __VO_TEST_CASE_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_vo_comm.h"
#include "k_type.h"
#include "k_connector_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


#define LAYER_TEST                              1
// #define OSD_TEST                                1

#if LAYER_TEST

#define PRIVATE_POLL_SZE                        (1920 * 1080 * 3 / 2) + (4096 * 2)
#define PRIVATE_POLL_NUM                        (4)

#else

#define PRIVATE_POLL_SZE                        (320 * 240 * 4)
#define PRIVATE_POLL_NUM                        (4)

#endif


#define COLOR_ARGB_RED                           0xFFFF0000U
#define COLOR_ARGB_GREEN                         0xFF00FF00U
#define COLOR_ARGB_BLUE                          0xFF0000FFU

#define COLOR_ABGR_RED                           0xFF0000FFU
#define COLOR_ABGR_GREEN                         0xFF00FF00U
#define COLOR_ABGR_BLUE                          0xFFFF0000U

#define COLOR_RGB565_RED                         0xf800
#define COLOR_RGB565_GREEN                       0x400
#define COLOR_RGB565_BLUE                        0x1f


#define TXPHY_216_M                              (106)                       //108 M phy clk  
#define TXPHY_216_N                              (5)
#define TXPHY_216_VOC                            (0x27)                      // 100111 = 0x27
#define TXPHY_216_HS_FREQ                        (0x93)                      //1001 0011 = 0x93


#define TXPHY_297_M                              (196)                       //148.5 M phy clk  ok
#define TXPHY_297_N                              (7)
#define TXPHY_297_VOC                            (0x27)                      // 100111 = 0x27
#define TXPHY_297_HS_FREQ                        (0xa5)                      //10100101 = 0x82

typedef enum
{
    DISPLAY_DSI_LP_MODE_TEST,
    DISPLAY_DSI_HS_MODE_TEST,
    DISPLAY_DSI_TEST_PATTERN,
    DISPALY_VO_BACKGROUND_TEST,
    DISPALY_VO_WRITEBACK_TEST,
    DISPALY_VO_OSD0_TEST,
    DISPALY_VO_INSERT_MULTI_FRAME_OSD0_TEST,
    DISPALY_VO_LAYER_INSERT_FRAME_TEST,
    DISPALY_VVI_BING_VO_LAYER_TEST,
    DISPALY_VVI_BING_VO_OSD_TEST,
    DISPALY_VVI_BING_VO_OSD_DUMP_FRAME_TEST,
    DISPALY_VO_1LAN_CASE_TEST,
    DISPALY_VO_DSI_READ_ID,
    DISPALY_VO_LAYER0_ROTATION,
    DISPALY_VO_CONNECTOR_TEST,
    DISPALY_VO_LT9611_TEST,
    DISPALY_VO_ST7701_480x854_TEST,
    DISPALY_VO_ST7701_480x800_TEST,
    DISPALY_VO_ILI9806_480x800_TEST,
    // DISPALY_VO_LAYER_FUNCTION_TEST,
} display_test_case;


typedef struct
{
    k_u64 osd_phy_addr;
    void *osd_virt_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
} osd_info;

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;

    //only layer0、layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;

} layer_info;

typedef struct
{
    k_u32 dev_num;
    k_u32 chn_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_pixel_format dev_format;
    k_u32 chn_height;
    k_u32 chn_width;
    k_pixel_format chn_format;
} sample_vvi_pipe_conf_t;

typedef struct
{
    k_u32 w;
    k_u32 h;
    k_vo_layer ch;
    k_vo_rotation ro;
} layer_bind_config;

void dwc_dsi_lpmode_test(void);
void dwc_dsi_hsmode_test(void);
void dwc_dsi_init_with_test_pattern(void);
void dwc_dsi_init(void);
void vo_background_init(void);
k_s32 vo_osd_insert_frame_test(void);
k_s32 vo_layer_insert_frame_test(void);
k_s32 vo_layer_bind_test(void);
k_s32 vo_osd_bind_test(void);
k_s32 vo_osd_insert_multi_frame_test(void);
k_s32 vo_writeback_test(void);
k_s32 vo_layer_insert_frame_change_function_test(void);

k_u32 vo_creat_osd_test(k_vo_osd osd, osd_info *info);
int vo_creat_layer_test(k_vo_layer chn_id, layer_info *info);
k_s32 vo_osd_bind_dump_frame_test(void);

k_s32 vo_layer_bind_config(layer_bind_config *config);

k_s32 vdss_bind_vo_config();

void dwc_dsi_1lan_init(void);
void vo_1lan_background_init(void);

void dwc_dsi_read_hx8399_id(void);

void dwc_dsi_layer0_init(void);
k_s32 vo_layer0_scaler_test(void);
k_s32 sample_connector_osd_install_frame(k_connector_type type);
k_s32 sample_connector_wbc_dump_frame(k_connector_type type);

/** @} */ /** <!-- ==== VVO End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
