/**
 * @file msg_vicap.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-04-11
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
 *
 */

#ifndef __MSG_VICAP_H__
#define __MSG_VICAP_H__

#include "k_vicap_comm.h"
#include "mpi_vicap_api.h"
#include "mpi_sys_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_VICAP_GET_SENSOR_FD,
    MSG_CMD_MEDIA_VICAP_DUMP_FRAME,
    MSG_CMD_MEDIA_VICAP_RELEASE_FRAME,
    MSG_CMD_MEDIA_VICAP_SET_DEV_ATTR,
    MSG_CMD_MEDIA_VICAP_SET_CHN_ATTR,
    MSG_CMD_MEDIA_VICAP_GET_SENSOR_INFO,
    MSG_CMD_MEDIA_VICAP_START,
    MSG_CMD_MEDIA_VICAP_STOP,
    MSG_CMD_MEDIA_VICAP_DROP_FRAME,
    MSG_CMD_MEDIA_VICAP_SET_MCLK,
    MSG_CMD_MEDIA_VICAP_TUNING
} msg_media_vicap_cmd_t;

typedef struct
{
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_dump_format dump_format;
    k_u32 milli_sec;
    k_video_frame_info vf_info;
} msg_vicap_frame_t;

typedef struct {
    k_vicap_dev dev_num;
    k_vicap_chn chn_num;
    k_s32 sensor_fd;
} msg_vicap_sensor_attr_t;

typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_sensor_type sensor_type;
    k_u32 out_width;
    k_u32 out_height;
    k_pixel_format pixel_format;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_bool dw_en;
    k_bool crop_en;
    k_u32 buf_size;
} msg_vicap_attr_info_t;

typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_work_mode mode;
    k_vicap_sensor_type sensor_type;
    k_vicap_isp_pipe_ctrl pipe_ctrl;
    k_bool dw_en;
    k_u32 buffer_num;
    k_u32 buffer_size;
} msg_vicap_dev_set_info_t;

typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_bool crop_en;
    k_bool scale_en;
    k_bool chn_en;
    k_u32 out_width;
    k_u32 out_height;
    k_u32 crop_h_start;
    k_u32 crop_v_start;
    k_pixel_format pixel_format;
    k_u32 buf_size;
    k_u8 alignment;
    k_u8 fps;
} msg_vicap_chn_set_info_t;

typedef struct {
    const char *sensor_name;
    k_u16 width;
    k_u16 height;
    k_vicap_csi_num csi_num;  /**< CSI NUM that the sensor connects to*/
    k_vicap_mipi_lanes mipi_lanes;  /**< MIPI lanes that the sensor connects to*/
    k_vicap_data_source source_id; /**<source id that the sensor used to*/
    k_bool is_3d_sensor;

    k_vicap_mipi_phy_freq phy_freq;
    k_vicap_csi_data_type data_type;
    k_vicap_hdr_mode hdr_mode;
    k_vicap_vi_flash_mode flash_mode;
    k_vicap_vi_first_frame_sel first_frame;
    k_u16 glitch_filter;
    k_vicap_sensor_type sensor_type;
} msg_vicap_sensor_info_t;

typedef struct
{
    k_vicap_csi_num csi;
    k_bool enable;
    k_vicap_drop_frame frame;
} msg_vicap_drop_frame_info_t;

typedef struct
{
    k_vicap_mclk_id id;
    k_vicap_mclk_sel sel;
    k_u8 mclk_div;
    k_u8 mclk_en;
} msg_vicap_mclk_set_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VICAP_H__ */