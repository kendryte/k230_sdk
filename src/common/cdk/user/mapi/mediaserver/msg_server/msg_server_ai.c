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
#include "msg_ai.h"
#include "msg_server_dispatch.h"
#include "mapi_ai_api.h"
#include "mapi_ai_comm.h"

extern k_s32  ai_aec_datafifo_init(k_handle ai_hdl,k_u64* phy_addr);
extern k_s32  ai_aec_datafifo_deinit(k_handle ai_hdl);
k_s32 msg_ai_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_pipe_attr_t *pipe_attr = msg->pBody;

    k_handle ai_hdl;
    ret = kd_mapi_ai_init(pipe_attr->ai_dev, pipe_attr->ai_chn,
                                    &pipe_attr->attr, &ai_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    pipe_attr->ai_hdl = ai_hdl;


    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_ai_pipe_attr_t));
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ai_hdl = msg->pBody;

    ret =  kd_mapi_ai_deinit(*ai_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ai_hdl = msg->pBody;

    ret =  kd_mapi_ai_start(*ai_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_handle* ai_hdl = msg->pBody;

    ret =  kd_mapi_ai_stop(*ai_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

/*k_s32 msg_ai_set_volume(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ai_get_volume(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ai_mute(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ai_unmute(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}*/

k_s32 msg_ai_get_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ai_release_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_ai_set_pitch_shift_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_pitch_shift_attr_t* attr = msg->pBody;

    ret =  kd_mapi_ai_set_pitch_shift_attr(attr->ai_hdl, &attr->param);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
        return ret;
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }
    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
        kd_ipcmsg_destroy_message(resp_msg);
        return ret;
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_get_pitch_shift_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_pitch_shift_attr_t* attr = msg->pBody;

    ret =  kd_mapi_ai_get_pitch_shift_attr(attr->ai_hdl, &attr->param);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
        return ret;
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }
    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
        kd_ipcmsg_destroy_message(resp_msg);
        return ret;
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_bind_ao(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_bind_ao_t* ai_bind_ao_info = msg->pBody;

    ret = kd_mapi_ai_bind_ao(ai_bind_ao_info->ai_hdl,ai_bind_ao_info->ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_unbind_ao(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_bind_ao_t* ai_bind_ao_info = msg->pBody;

    ret = kd_mapi_ai_unbind_ao(ai_bind_ao_info->ai_hdl,ai_bind_ao_info->ao_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_set_volume(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_gain_info * gain_info = msg->pBody;

    ret = kd_mapi_ai_set_volume(gain_info->ai_hdl,gain_info->gain);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_set_vqe_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_vqe_attr * vqe_attr = msg->pBody;

    ret = kd_mapi_ai_set_vqe_attr(vqe_attr->ai_hdl,vqe_attr->vqe_enable);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_get_vqe_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_vqe_attr * vqe_attr = msg->pBody;

    ret = kd_mapi_ai_get_vqe_attr(vqe_attr->ai_hdl,&vqe_attr->vqe_enable);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, vqe_attr, sizeof(k_msg_ai_vqe_attr));
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_send_far_echo_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_frame_t * far_echo_frame = msg->pBody;

    ret = kd_mapi_ai_send_far_echo_frame(far_echo_frame->ai_hdl,&far_echo_frame->audio_frame,far_echo_frame->milli_sec);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_acodec_reset(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_gain_info * gain_info = msg->pBody;

    ret = kd_mapi_acodec_reset();
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_aec_init_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_aec_datafifo_t* ai_aec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = ai_aec_datafifo_init(ai_aec_datafifo->ai_hdl,&phy_addr);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("ai_aec_datafifo_init failed:%x\n", ret);
    }

    ai_aec_datafifo->phyAddr = phy_addr;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_ai_aec_datafifo_t));
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_ai_aec_deinit_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_ai_aec_datafifo_t* ai_aec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = ai_aec_datafifo_deinit(ai_aec_datafifo->ai_hdl);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace("ai_aec_datafifo_deinit failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_ai_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_ai_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_AI_INIT,      msg_ai_init},
    {MSG_CMD_MEDIA_AI_DEINIT,       msg_ai_deinit},
    {MSG_CMD_MEDIA_AI_START,      msg_ai_start},
    {MSG_CMD_MEDIA_AI_STOP,      msg_ai_stop},
    {MSG_CMD_MEDIA_AI_SETVOLUME,      msg_ai_set_volume},
   // {MSG_CMD_MEDIA_AI_GETVOLUME,   msg_ai_get_volume},
   // {MSG_CMD_MEDIA_AI_MUTE,        msg_ai_mute},
   // {MSG_CMD_MEDIA_AI_UNMUTE,      msg_ai_unmute},
    {MSG_CMD_MEDIA_AI_GETFRAME,      msg_ai_get_frame},
    {MSG_CMD_MEDIA_AI_RELEASEFRAME,      msg_ai_release_frame},
    {MSG_CMD_MEDIA_AI_SET_PITCH_SHIFT_ATTR, msg_ai_set_pitch_shift_attr},
    {MSG_CMD_MEDIA_AI_GET_PITCH_SHIFT_ATTR, msg_ai_get_pitch_shift_attr},
    {MSG_CMD_MEDIA_AI_BIND_AO,msg_ai_bind_ao},
    {MSG_CMD_MEDIA_AI_UNBIND_AO,msg_ai_unbind_ao},
    {MSG_CMD_MEDIA_ACODEC_RESET,msg_acodec_reset},
    {MSG_CMD_MEDIA_AI_SET_VEQ_ATTR,msg_ai_set_vqe_attr},
    {MSG_CMD_MEDIA_AI_GET_VEQ_ATTR,msg_ai_get_vqe_attr},
    {MSG_CMD_MEDIA_AI_SEND_FAR_ECHO_FRAME,msg_ai_send_far_echo_frame},
    {MSG_CMD_MEDIA_AI_AEC_INIT_DATAFIFO,      msg_ai_aec_init_datafifo},
    {MSG_CMD_MEDIA_AI_AEC_DEINIT_DATAFIFO,      msg_ai_aec_deinit_datafifo},
};

msg_server_module_t g_module_ai = {
    K_MAPI_MOD_AI,
    "ai",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_ai_mod(void)
{
    return &g_module_ai;
}