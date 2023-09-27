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
#include "k_type.h"
#include "k_ipcmsg.h"
#include "k_comm_ipcmsg.h"
#include "msg_vo.h"
#include "msg_server_dispatch.h"
#include "mapi_vo_api.h"
#include "stdio.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

k_s32 msg_vo_set_backlight(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_set_backlight();
    if(ret != K_SUCCESS) {
        printf("msg_vo_reset failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_reset(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_reset();
    if(ret != K_SUCCESS) {
        printf("msg_vo_reset failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_dsi_set_test_pattern(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_dsi_set_test_pattern();
    if(ret != K_SUCCESS) {
        printf("msg_dsi_set_test_pattern failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_dsi_set_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vo_dsi_attr *attr = msg->pBody;

    ret = kd_mapi_dsi_set_attr(attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_set_attr failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_dsi_enable(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u32 enable = *val;

    ret = kd_mapi_dsi_enable(enable);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_dsi_send_cmd(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_dsi_cmd_t *cmd = msg->pBody;

    ret = kd_mapi_dsi_send_cmd(cmd->data, cmd->cmd_len);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_set_mipi_phy_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vo_mipi_phy_attr *attr = msg->pBody;

    ret = kd_mapi_set_mipi_phy_attr(attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_dsi_read_pkg(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_dsi_read_cmd_t *cmd_attr = msg->pBody;

    ret = kd_mapi_dsi_read_pkg(cmd_attr->addr, cmd_attr->cmd_len, cmd_attr->rv_data);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_enable(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_enable();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_vo_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_disable(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_disable();
    if(ret != K_SUCCESS) {
        printf("msg_vo_disable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_init();
    if(ret != K_SUCCESS) {
        printf("msg_vo_disable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_dev_param(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vo_pub_attr *attr = msg->pBody;
    k_vo_pub_attr pub_attr;
    k_vo_display_resolution sync_info;

    memcpy(&sync_info, &attr->sync_info, sizeof(k_vo_display_resolution));
    pub_attr.bg_color = attr->bg_color;
    pub_attr.intf_type = attr->intf_type;
    pub_attr.intf_sync = attr->intf_sync;
    pub_attr.sync_info = &sync_info;

    // printf("pub_attr.bg_color is %x \n",pub_attr.bg_color);
    // printf("sync_info is %d  sync_info.phyclk is %d \n", sync_info.pclk, sync_info.phyclk);

    ret = kd_mapi_vo_set_dev_param(&pub_attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_user_sync_info(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_sync_info_t *info = msg->pBody;

    ret = kd_mapi_vo_set_user_sync_info(info->pre_div, info->clk_en);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_enable_video_layer(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u8 enable = *val;

    ret = kd_mapi_vo_enable_video_layer(enable);
    if(ret != K_SUCCESS) {
        printf("msg_vo_disable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_disable_video_layer(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u8 enable = *val;

    ret = kd_mapi_vo_disable_video_layer(enable);
    if(ret != K_SUCCESS) {
        printf("msg_vo_disable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_video_layer_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_video_layer_attr_t *attr = msg->pBody;

    ret = kd_mapi_vo_set_video_layer_attr(attr->layer, &attr->attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_layer_priority(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_layer_priority_t *priority = msg->pBody;

    ret = kd_mapi_vo_set_layer_priority(priority->layer, priority->priority);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_video_osd_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_video_osd_attr_t *osd_attr = msg->pBody;

    ret = kd_mapi_vo_set_video_osd_attr(osd_attr->layer, &osd_attr->attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}



k_s32 msg_vo_osd_enable(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u8 layer = *val;

    ret = kd_mapi_vo_osd_enable(layer);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_osd_disable(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u8 layer = *val;

    ret = kd_mapi_vo_osd_disable(layer);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_draw_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vo_draw_frame *frame = msg->pBody;


    ret = kd_mapi_vo_draw_frame(frame);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}



k_s32 msg_vo_enable_wbc(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_enable_wbc();
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_disable_wbc(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;

    ret = kd_mapi_vo_disable_wbc();
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_set_wbc_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vo_wbc_attr *attr = msg->pBody;

    ret = kd_mapi_vo_set_wbc_attr(attr);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_chn_insert_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_insert_frame_t *frame = msg->pBody;

    ret = kd_mapi_vo_chn_insert_frame(frame->chn_num, &frame->vf_info);
    if(ret != K_SUCCESS) {
        printf("msg_dsi_enable failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_vo_chn_dump_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vo_frame_t *frame = msg->pBody;

    ret = kd_mapi_vo_chn_dump_frame(frame->chn_num, &frame->vf_info, frame->timeout_ms);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vo_frame_t));
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vo_chn_dump_release(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vo_frame_t *frame  = msg->pBody;

    ret = kd_mapi_vo_chn_dump_release(frame->chn_num, &frame->vf_info);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_release_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_get_connector_info(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_connector_info_t *info = msg->pBody;
    k_connector_info con_info;

    memset(&con_info, 0, sizeof(con_info));

    ret = kd_mapi_get_connector_info(info->connector_type, &con_info);
    if(ret != K_SUCCESS) {
        printf("msg_get_connector_info failed:0x%x\n", ret);
    }

    memcpy(info->connector_info.connector_name, con_info.connector_name, sizeof(con_info.connector_name));
    info->connector_info.screen_test_mode = con_info.screen_test_mode;
    info->connector_info.dsi_test_mode = con_info.dsi_test_mode;
    info->connector_info.bg_color = con_info.bg_color;
    info->connector_info.intr_line = con_info.intr_line;
    info->connector_info.lan_num = con_info.lan_num;
    info->connector_info.work_mode = con_info.work_mode;
    info->connector_info.cmd_mode = con_info.cmd_mode;

    memcpy(&info->connector_info.phy_attr, &con_info.phy_attr, sizeof(k_connectori_phy_attr));
    memcpy(&info->connector_info.resolution, &con_info.resolution, sizeof(k_vo_display_resolution));

    // printf(" 22 connector_type is %d connector_info.name is %s connector_info.bg_clolor is %x \n", info->connector_type, info->connector_info.connector_name, info->connector_info.bg_color);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_connector_info_t));
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_connector_open(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_s32 fd;
    k_ipcmsg_message_t *resp_msg;
    char *dev_name  = msg->pBody;
    char name[100];

    memcpy(name, dev_name, sizeof(dev_name));

    ret = kd_mapi_connector_open(name);
    if(ret < 0) {
        printf("msg_connector_open failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 connector_power_set(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_connector_power_t *power  = msg->pBody;

    ret = kd_mapi_connector_power_set(power->fd, power->on);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_release_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_connector_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_connector_init_t *info = msg->pBody;

    ret = kd_mapi_connector_init(info->fd, &info->info);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_connector_init_t));
    if(resp_msg == NULL) {
        printf("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        printf(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] = {    
    {MSG_CMD_MEDIA_SET_BACKLIGHT,               msg_vo_set_backlight},
    {MSG_CMD_MEDIA_VO_RST,                      msg_vo_reset},
    {MSG_CMD_MEDIA_DSI_TEST_PATTERN,            msg_dsi_set_test_pattern},
    {MSG_CMD_MEDIA_DSI_SET_ATTR,                msg_dsi_set_attr},
    {MSG_CMD_MEDIA_DSI_ENABLE,                  msg_dsi_enable},
    {MSG_CMD_MEDIA_DSI_SEND_CMD,                msg_dsi_send_cmd},
    {MSG_CMD_MEDIA_PHY_SET_ATTR,                msg_set_mipi_phy_attr},
    {MSG_CMD_MEDIA_DSI_READ_PKG,                msg_dsi_read_pkg},
    {MSG_CMD_MEDIA_VO_ENABLE,                   msg_vo_enable},
    {MSG_CMD_MEDIA_VO_DISABLE,                  msg_vo_disable},
    {MSG_CMD_MEDIA_VO_INIT,                     msg_vo_init},
    {MSG_CMD_MEDIA_VO_SET_DEV_ATTR,             msg_vo_set_dev_param},
    {MSG_CMD_MEDIA_VO_USER_SYNC_INFO,           msg_vo_set_user_sync_info},
    {MSG_CMD_MEDIA_VO_ENABLE_VIDEO_LAYER,       msg_vo_enable_video_layer},
    {MSG_CMD_MEDIA_VO_DISABLE_VIDEO_LAYER,      msg_vo_disable_video_layer},
    {MSG_CMD_MEDIA_VO_SET_LAYER_ATTR,           msg_vo_set_video_layer_attr},
    {MSG_CMD_MEDIA_VO_SET_LAYER_PRIORITY,       msg_vo_set_layer_priority},
    {MSG_CMD_MEDIA_VO_SET_OSD_ATTR,             msg_vo_set_video_osd_attr},
    {MSG_CMD_MEDIA_VO_SET_OSD_ENABLE,           msg_vo_osd_enable},
    {MSG_CMD_MEDIA_VO_SET_OSD_DISABLE,          msg_vo_osd_disable},
    {MSG_CMD_MEDIA_VO_DRAW_FRAME,               msg_vo_draw_frame},
    {MSG_CMD_MEDIA_VO_ENABLE_WBC,               msg_vo_enable_wbc},
    {MSG_CMD_MEDIA_VO_DISABLE_WBC,              msg_vo_disable_wbc},
    {MSG_CMD_MEDIA_VO_SET_WBC_ATTR,             msg_vo_set_wbc_attr},
    {MSG_CMD_MEDIA_VO_INSTALL_FRAME,            msg_vo_chn_insert_frame},
    {MSG_CMD_MEDIA_VO_DUMP_FRAME,               msg_vo_chn_dump_frame},
    {MSG_CMD_MEDIA_VO_RELEASE_FRAME,            msg_vo_chn_dump_release},
    {MSG_CMD_MEDIA_GET_CONNECTOR_INFO,          msg_get_connector_info},
    {MSG_CMD_MEDIA_OPEN_CONNECTOR,              msg_connector_open},
    {MSG_CMD_MEDIA_SET_CONNECTOR_POWER,         connector_power_set},
    {MSG_CMD_MEDIA_CONNECTOR_INIT,              msg_connector_init},

};


msg_server_module_t g_module_vo = {
    K_MAPI_MOD_VO,
    "vo",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_vo_mod(void)
{
    return &g_module_vo;
}