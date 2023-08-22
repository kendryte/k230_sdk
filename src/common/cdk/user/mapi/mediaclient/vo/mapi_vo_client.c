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
#include <stdlib.h>
#include <pthread.h>
#include "mapi_vo_api.h"
#include "msg_client_dispatch.h"
#include "mapi_vo_comm.h"
#include "mpi_vo_api.h"
#include "msg_vo.h"
#include "k_type.h"
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <unistd.h>
#include "mpi_vo_api.h"
#include "k_vo_comm.h"


// rst display 
k_s32 kd_mapi_set_backlight(void)
{
    k_s32 ret;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_SET_BACKLIGHT,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


// rst display 
k_s32 kd_mapi_vo_reset(void)
{
    k_s32 ret;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_RST,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dsi_set_test_pattern(void)
{
    k_s32 ret;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_DSI_TEST_PATTERN,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dsi_set_attr(k_vo_dsi_attr *attr)
{
    k_s32 ret = 0;
    k_vo_dsi_attr dsi_attr;

    memcpy(&dsi_attr, attr, sizeof(k_vo_dsi_attr));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_DSI_SET_ATTR,
            &dsi_attr, sizeof(dsi_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dsi_enable(k_u32 enable)
{
    k_s32 ret = 0;
    k_s32 val = enable;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_DSI_ENABLE,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dsi_send_cmd(k_u8 *data, k_s32 cmd_len)
{
    k_s32 ret = 0;
    msg_dsi_cmd_t cmd;

    cmd.cmd_len = cmd_len;
    memcpy(cmd.data, data, cmd_len);

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_DSI_SEND_CMD,
            &cmd, sizeof(cmd), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_set_mipi_phy_attr(k_vo_mipi_phy_attr *attr)
{
    k_s32 ret = 0;
    k_vo_mipi_phy_attr phy_attr;

    memcpy(&phy_attr, attr, sizeof(k_vo_mipi_phy_attr));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_PHY_SET_ATTR,
            &phy_attr, sizeof(phy_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_dsi_read_pkg(k_u8 addr, k_u16 cmd_len, k_u32 *rv_data)
{
    k_s32 ret = 0;
    msg_dsi_read_cmd_t cmd_attr;

    cmd_attr.cmd_len = cmd_len;
    cmd_attr.addr = addr;
    memcpy(cmd_attr.rv_data, rv_data, cmd_len);

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_DSI_READ_PKG,
            &cmd_attr, sizeof(cmd_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_enable(void)
{
    k_s32 ret = 0;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_ENABLE,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_disable(void)
{
    k_s32 ret = 0;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_DISABLE,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_init(void)
{
    k_s32 ret = 0;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_INIT,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_set_dev_param(k_vo_pub_attr *attr)
{
    k_s32 ret = 0;
    // k_vo_pub_attr pub_attr;
    msg_vo_pub_attr pub_attr;

    pub_attr.bg_color = attr->bg_color;
    pub_attr.intf_type = attr->intf_type;
    pub_attr.intf_sync = attr->intf_sync;
    memcpy(&pub_attr.sync_info, attr->sync_info, sizeof(k_vo_display_resolution));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_DEV_ATTR,
            &pub_attr, sizeof(pub_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_set_user_sync_info(k_u32 pre_div, k_u32 clk_en)
{
    k_s32 ret = 0;
    msg_sync_info_t info;

    info.pre_div = pre_div;
    info.clk_en = clk_en;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_USER_SYNC_INFO,
            &info, sizeof(info), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_enable_video_layer(k_vo_layer layer)
{
    k_s32 ret = 0;
    k_s32 val = layer;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_ENABLE_VIDEO_LAYER,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_disable_video_layer(k_vo_layer layer)
{
    k_s32 ret = 0;
    k_s32 val = layer;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_DISABLE_VIDEO_LAYER,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_set_video_layer_attr(k_vo_layer layer, k_vo_video_layer_attr *attr)
{
    k_s32 ret = 0;
    msg_video_layer_attr_t layer_attr;

    layer_attr.layer = layer;
    memcpy(&layer_attr.attr, attr, sizeof(k_vo_video_layer_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_LAYER_ATTR,
            &layer_attr, sizeof(layer_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_set_layer_priority(k_vo_layer layer, k_s32 priority)
{
    k_s32 ret = 0;
    msg_layer_priority_t priority_t;

    priority_t.layer = layer;
    priority_t.priority = priority;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_LAYER_PRIORITY,
            &priority_t, sizeof(priority_t), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}

k_s32 kd_mapi_vo_set_video_osd_attr(k_vo_osd layer, k_vo_video_osd_attr *attr)
{
    k_s32 ret = 0;
    msg_video_osd_attr_t osd_attr;

    osd_attr.layer = layer;
    memcpy(&osd_attr.attr, attr, sizeof(k_vo_video_osd_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_OSD_ATTR,
            &osd_attr, sizeof(osd_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_osd_enable(k_vo_osd layer)
{
    k_s32 ret = 0;
    k_s32 val = layer;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_OSD_ENABLE,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_osd_disable(k_vo_osd layer)
{
    k_s32 ret = 0;
    k_s32 val = layer;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_OSD_DISABLE,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_draw_frame(k_vo_draw_frame *frame)
{
    k_s32 ret = 0;
    k_vo_draw_frame draw_fram;

    memcpy(&draw_fram, frame, sizeof(k_vo_draw_frame));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_DRAW_FRAME,
            &draw_fram, sizeof(draw_fram), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_enable_wbc(void)
{
    k_s32 ret = 0;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_ENABLE_WBC,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_disable_wbc(void)
{
    k_s32 ret = 0;
    k_s32 val = 0;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_DISABLE_WBC,
            &val, sizeof(val), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_set_wbc_attr(k_vo_wbc_attr *attr)
{
    k_s32 ret = 0;
    k_vo_wbc_attr wbc_attr;

    memcpy(&wbc_attr, attr, sizeof(k_vo_wbc_attr));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_SET_WBC_ATTR,
            &wbc_attr, sizeof(wbc_attr), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_chn_insert_frame(k_u32 chn_num, k_video_frame_info *vf_info)
{
    k_s32 ret = 0;
    msg_insert_frame_t frame;

    frame.chn_num = chn_num;
    memcpy(&frame.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_INSTALL_FRAME,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_chn_dump_frame(k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms)
{
    k_s32 ret = 0;
    msg_vo_frame_t frame;

    frame.chn_num = chn_num;
    frame.timeout_ms = timeout_ms;
    memcpy(&frame.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_DUMP_FRAME,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}


k_s32 kd_mapi_vo_chn_dump_release(k_u32 chn_num, const k_video_frame_info *vf_info)
{
    k_s32 ret = 0;
    msg_vo_frame_t frame;

    frame.chn_num = chn_num;
    memcpy(&frame.vf_info, vf_info, sizeof(k_video_frame_info));

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VO, 0, 0), MSG_CMD_MEDIA_VO_RELEASE_FRAME,
            &frame, sizeof(frame), NULL);

    if(ret != K_SUCCESS) {
        mapi_vo_error_trace("mapi_send_sync failed\n");
    }

    return ret;
}
