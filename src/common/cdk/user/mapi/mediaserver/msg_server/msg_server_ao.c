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
#include "msg_ao.h"
#include "msg_server_dispatch.h"
#include "mapi_ao_api.h"
#include "mapi_ao_comm.h"

k_s32 msg_ao_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ao_pipe_attr_t *pipe_attr = msg->pBody;

    k_handle ao_hdl;
    ret = kd_mapi_ao_init(pipe_attr->ao_dev, pipe_attr->ao_chn,
                                    &pipe_attr->attr, &ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }
    pipe_attr->ao_hdl = ao_hdl;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_ao_pipe_attr_t));
    if(resp_msg == NULL) {
        mapi_ao_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ao_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ao_hdl = msg->pBody;

    ret =  kd_mapi_ao_deinit(*ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ao_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ao_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ao_hdl = msg->pBody;

    ret =  kd_mapi_ao_start(*ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ao_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ao_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ao_hdl = msg->pBody;

    ret =  kd_mapi_ao_stop(*ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ao_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ao_set_volume(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ao_gain_info*  gain_info = msg->pBody;

    ret =  kd_mapi_ao_set_volume(gain_info->ao_hdl,gain_info->gain);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ao_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ao_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ao_get_volume(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ao_mute(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ao_unmute(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ao_send_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}


static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_AO_INIT,      msg_ao_init},
    {MSG_CMD_MEDIA_AO_DEINIT,       msg_ao_deinit},
    {MSG_CMD_MEDIA_AO_START,      msg_ao_start},
    {MSG_CMD_MEDIA_AO_STOP,      msg_ao_stop},
    {MSG_CMD_MEDIA_AO_SETVOLUME,      msg_ao_set_volume},
    {MSG_CMD_MEDIA_AO_GETVOLUME,   msg_ao_get_volume},
    {MSG_CMD_MEDIA_AO_MUTE,        msg_ao_mute},
    {MSG_CMD_MEDIA_AO_UNMUTE,      msg_ao_unmute},
    {MSG_CMD_MEDIA_AO_SENDFRAME,      msg_ao_send_frame},

};

msg_server_module_t g_module_ao = {
    K_MAPI_MOD_AO,
    "ao",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_ao_mod(void)
{
    return &g_module_ao;
}