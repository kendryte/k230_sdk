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
#include <stdio.h>
#include "k_type.h"
#include "k_ipcmsg.h"
#include "k_comm_ipcmsg.h"
#include "msg_vicap.h"
#include "msg_server_dispatch.h"
#include "mapi_vicap_api.h"
#include "mapi_vicap_comm.h"

k_s32 msg_vicap_get_sensor_fd(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_sensor_attr_t *vicap_sensor_attr = msg->pBody;

    ret = kd_mapi_vicap_get_sensor_fd((k_vicap_sensor_attr *)vicap_sensor_attr);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("msg_vicap_get_sensor_fd failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vicap_sensor_attr_t));
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vicap_dump_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_frame_t *vicap_frame = msg->pBody;

    ret = kd_mapi_vicap_dump_frame(vicap_frame->vicap_dev, vicap_frame->vicap_chn, vicap_frame->dump_format, &vicap_frame->vf_info, vicap_frame->milli_sec);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vicap_frame_t));
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vicap_dump_release(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_frame_t *vicap_frame = msg->pBody;
    ret = kd_mapi_vicap_release_frame(vicap_frame->vicap_dev, vicap_frame->vicap_chn, &vicap_frame->vf_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vicap_get_sensor_info(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_sensor_info_t *sensor_info = msg->pBody;

    ret = kd_mapi_vicap_get_sensor_info((k_vicap_sensor_info *)sensor_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("msg_vicap_get_sensor_info failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vicap_sensor_info_t));
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vicap_set_dev_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_dev_set_info_t *meg_attr_info = msg->pBody;
    k_vicap_dev_set_info attr_info;
    memset(&attr_info, 0, sizeof(k_vicap_dev_set_info));
    memcpy(&attr_info, meg_attr_info, sizeof(k_vicap_dev_set_info));
    ret = kd_mapi_vicap_set_dev_attr(attr_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("msg_vicap_set_dev_attr failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vicap_set_chn_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vicap_chn_set_info_t *meg_attr_info = msg->pBody;
    k_vicap_chn_set_info attr_info;
    memset(&attr_info, 0, sizeof(k_vicap_chn_set_info));
    memcpy(&attr_info, meg_attr_info, sizeof(k_vicap_chn_set_info));
    ret = kd_mapi_vicap_set_chn_attr(attr_info);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("msg_vicap_set_chn_attr failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vicap_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vicap_dev *vicap_dev = msg->pBody;
    ret = kd_mapi_vicap_start(*vicap_dev);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vicap_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vicap_dev *vicap_dev = msg->pBody;
    ret = kd_mapi_vicap_stop(*vicap_dev);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mapi_vicap_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_VICAP_GET_SENSOR_FD,   msg_vicap_get_sensor_fd},
    {MSG_CMD_MEDIA_VICAP_DUMP_FRAME,      msg_vicap_dump_frame},
    {MSG_CMD_MEDIA_VICAP_RELEASE_FRAME,   msg_vicap_dump_release},
    {MSG_CMD_MEDIA_VICAP_GET_SENSOR_INFO, msg_vicap_get_sensor_info},
    {MSG_CMD_MEDIA_VICAP_SET_DEV_ATTR,    msg_vicap_set_dev_attr},
    {MSG_CMD_MEDIA_VICAP_SET_CHN_ATTR,    msg_vicap_set_chn_attr},
    {MSG_CMD_MEDIA_VICAP_START,           msg_vicap_start},
    {MSG_CMD_MEDIA_VICAP_STOP,            msg_vicap_stop},
};

msg_server_module_t g_module_vicap = {
    K_MAPI_MOD_VICAP,
    "vicap",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_vicap_mod(void)
{
    return &g_module_vicap;
}