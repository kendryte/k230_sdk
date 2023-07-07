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
#include "msg_adec.h"
#include "msg_server_dispatch.h"
#include "mapi_adec_api.h"
#include "mapi_adec_comm.h"
#include "k_adec_comm.h"

extern k_s32  adec_datafifo_init(k_handle adec_hdl,k_u64* phy_addr);
extern k_s32  adec_datafifo_deinit(k_handle adec_hdl);

k_s32 msg_adec_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_pipe_attr_t* adec_attr = msg->pBody;

    ret = kd_mapi_adec_init(adec_attr->adec_hdl,&adec_attr->attr);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* adec_hdl = msg->pBody;

    ret =  kd_mapi_adec_deinit(*adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* adec_hdl = msg->pBody;

    ret =  kd_mapi_adec_start(*adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* adec_hdl = msg->pBody;

    ret =  kd_mapi_adec_stop(*adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_register_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_callback_attr* adec_cb_attr = msg->pBody;

    ret = kd_mapi_adec_registercallback(adec_cb_attr->adec_hdl,&adec_cb_attr->callback_attr);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_unregister_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* adec_hdl = msg->pBody;

    ret = kd_mapi_adec_unregistercallback(*adec_hdl);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_bind_ao(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_bind_ao_t* adec_bind_ao_info = msg->pBody;

    ret = kd_mapi_adec_bind_ao(adec_bind_ao_info->ao_hdl,adec_bind_ao_info->adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_unbind_ao(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_bind_ao_t* adec_bind_ao_info = msg->pBody;

    ret = kd_mapi_adec_unbind_ao(adec_bind_ao_info->ao_hdl,adec_bind_ao_info->adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_register_ext_decoder(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ext_audio_decoder_t* ext_decoder_info = msg->pBody;

    k_handle decoder_hdl;
    ret = kd_mapi_register_ext_audio_decoder(&ext_decoder_info->decoder,&decoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    ext_decoder_info->decoder_hdl = decoder_hdl;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody,sizeof(k_msg_ext_audio_decoder_t));
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_unregister_ext_decoder(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ext_decoder_hdl = msg->pBody;

    ret =  kd_mapi_unregister_ext_audio_decoder(*ext_decoder_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_send_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_adec_init_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_datafifo_t* adec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = adec_datafifo_init(adec_datafifo->adec_hdl,&phy_addr);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("adec_datafifo_init failed:%x\n", ret);
    }

    adec_datafifo->phyAddr = phy_addr;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_adec_datafifo_t));
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_adec_deinit_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_adec_datafifo_t* adec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = adec_datafifo_deinit(adec_datafifo->adec_hdl);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace("adec_datafifo_deinit failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_adec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_adec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_ADEC_INIT,      msg_adec_init},
    {MSG_CMD_MEDIA_ADEC_DEINIT,       msg_adec_deinit},
    {MSG_CMD_MEDIA_ADEC_START,      msg_adec_start},
    {MSG_CMD_MEDIA_ADEC_STOP,      msg_adec_stop},
    {MSG_CMD_MEDIA_ADEC_REGISTER_CALLBACK,      msg_adec_register_callback},
    {MSG_CMD_MEDIA_ADEC_UNREGISTER_CALLBACK,   msg_adec_unregister_callback},
    {MSG_CMD_MEDIA_ADEC_BIND_AO,        msg_adec_bind_ao},
    {MSG_CMD_MEDIA_ADEC_UNBIND_AO,      msg_adec_unbind_ao},
    {MSG_CMD_MEDIA_ADEC_REGISTER_EXT_AUDIO_DECODER,      msg_adec_register_ext_decoder},
    {MSG_CMD_MEDIA_ADEC_UNREGISTER_EXT_AUDIO_DECODER,      msg_adec_unregister_ext_decoder},
    {MSG_CMD_MEDIA_ADEC_SEND_STREAM,      msg_adec_send_frame},
    {MSG_CMD_MEDIA_ADEC_INIT_DATAFIFO,      msg_adec_init_datafifo},
    {MSG_CMD_MEDIA_ADEC_DEINIT_DATAFIFO,      msg_adec_deinit_datafifo},

};

msg_server_module_t g_module_adec = {
    K_MAPI_MOD_ADEC,
    "adec",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_adec_mod(void)
{
    return &g_module_adec;
}