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
#include "msg_aenc.h"
#include "msg_server_dispatch.h"
#include "mapi_aenc_api.h"
#include "mapi_aenc_comm.h"
#include "k_aenc_comm.h"

extern k_s32  aenc_datafifo_init(k_handle aenc_hdl,k_u64* phy_addr);
extern k_s32  aenc_datafifo_deinit(k_handle aenc_hdl);

k_s32 msg_aenc_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_pipe_attr_t* aenc_attr = msg->pBody;


    ret = kd_mapi_aenc_init(aenc_attr->aenc_hdl,&aenc_attr->attr);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* aenc_hdl = msg->pBody;

    ret =  kd_mapi_aenc_deinit(*aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* aenc_hdl = msg->pBody;

    ret =  kd_mapi_aenc_start(*aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* aenc_hdl = msg->pBody;

    ret =  kd_mapi_aenc_stop(*aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_register_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_callback_attr* aenc_cb_attr = msg->pBody;

    ret = kd_mapi_aenc_registercallback(aenc_cb_attr->aenc_hdl,&aenc_cb_attr->callback_attr);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_unregister_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* aenc_hdl = msg->pBody;

    ret = kd_mapi_aenc_unregistercallback(*aenc_hdl);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_bind_ai(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_bind_ai_t* aenc_bind_ai_info = msg->pBody;

    ret = kd_mapi_aenc_bind_ai(aenc_bind_ai_info->ai_hdl,aenc_bind_ai_info->aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_unbind_ai(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_bind_ai_t* aenc_bind_ai_info = msg->pBody;

    ret = kd_mapi_aenc_unbind_ai(aenc_bind_ai_info->ai_hdl,aenc_bind_ai_info->aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_register_ext_encoder(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ext_audio_encoder_t* ext_encoder_info = msg->pBody;

    k_handle encoder_hdl;
    ret = kd_mapi_register_ext_audio_encoder(&ext_encoder_info->encoder,&encoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    ext_encoder_info->encoder_hdl = encoder_hdl;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody,sizeof(k_msg_ext_audio_encoder_t));
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_unregister_ext_encoder(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ext_encoder_hdl = msg->pBody;

    ret =  kd_mapi_unregister_ext_audio_encoder(*ext_encoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_send_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_aenc_init_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_datafifo_t* aenc_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = aenc_datafifo_init(aenc_datafifo->aenc_hdl,&phy_addr);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("aenc_datafifo_init failed:%x\n", ret);
    }

    aenc_datafifo->phyAddr = phy_addr;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_aenc_datafifo_t));
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_aenc_deinit_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_aenc_datafifo_t* aenc_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = aenc_datafifo_deinit(aenc_datafifo->aenc_hdl);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace("aenc_datafifo_deinit failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_aenc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_aenc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_AENC_INIT,      msg_aenc_init},
    {MSG_CMD_MEDIA_AENC_DEINIT,       msg_aenc_deinit},
    {MSG_CMD_MEDIA_AENC_START,      msg_aenc_start},
    {MSG_CMD_MEDIA_AENC_STOP,      msg_aenc_stop},
    {MSG_CMD_MEDIA_AENC_REGISTER_CALLBACK,      msg_aenc_register_callback},
    {MSG_CMD_MEDIA_AENC_UNREGISTER_CALLBACK,   msg_aenc_unregister_callback},
    {MSG_CMD_MEDIA_AENC_BIND_AI,        msg_aenc_bind_ai},
    {MSG_CMD_MEDIA_AENC_UNBIND_AI,      msg_aenc_unbind_ai},
    {MSG_CMD_MEDIA_AENC_REGISTER_EXT_AUDIO_ENCODER,      msg_aenc_register_ext_encoder},
    {MSG_CMD_MEDIA_AENC_UNREGISTER_EXT_AUDIO_ENCODER,      msg_aenc_unregister_ext_encoder},
    {MSG_CMD_MEDIA_AENC_SEND_FRAME,      msg_aenc_send_frame},
    {MSG_CMD_MEDIA_AENC_INIT_DATAFIFO,      msg_aenc_init_datafifo},
    {MSG_CMD_MEDIA_AENC_DEINIT_DATAFIFO,      msg_aenc_deinit_datafifo},

};

msg_server_module_t g_module_aenc = {
    K_MAPI_MOD_AENC,
    "aenc",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_aenc_mod(void)
{
    return &g_module_aenc;
}