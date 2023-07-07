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
#include "msg_vdec.h"
#include "msg_server_dispatch.h"
#include "mapi_vdec_api.h"
#include "mapi_vdec_comm.h"
#include "k_vdec_comm.h"

extern k_s32  vdec_datafifo_init(k_s32 vdec_chn,k_u64* phy_addr);
extern k_s32  vdec_datafifo_deinit(k_s32 vdec_chn);

k_s32 msg_vdec_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_pipe_attr_t* vdec_attr = msg->pBody;

    ret = kd_mapi_vdec_init(vdec_attr->vdec_chn,&vdec_attr->attr);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_s32* vdec_chn = msg->pBody;

    ret =  kd_mapi_vdec_deinit(*vdec_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_s32* vdec_chn = msg->pBody;

    ret =  kd_mapi_vdec_start(*vdec_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_s32* vdec_chn = msg->pBody;

    ret =  kd_mapi_vdec_stop(*vdec_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_bind_vo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_bind_vo_t* vdec_bind_vo_info = msg->pBody;

    ret = kd_mapi_vdec_bind_vo(vdec_bind_vo_info->vdec_chn,vdec_bind_vo_info->vo_dev,vdec_bind_vo_info->vo_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_unbind_vo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_bind_vo_t* vdec_bind_vo_info = msg->pBody;

    ret = kd_mapi_vdec_unbind_vo(vdec_bind_vo_info->vdec_chn,vdec_bind_vo_info->vo_dev,vdec_bind_vo_info->vo_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_send_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    return K_SUCCESS;
}

k_s32 msg_vdec_query_status(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_chn_status_t* vdec_chn_status = msg->pBody;

    ret = kd_mapi_vdec_query_status(vdec_chn_status->vdec_chn,&vdec_chn_status->status);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("%s failed:0x%x\n",__FUNCTION__ ,ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody,sizeof(k_msg_vdec_chn_status_t));
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_init_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_datafifo_t* vdec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = vdec_datafifo_init(vdec_datafifo->vdec_chn,&phy_addr);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("adec_datafifo_init failed:%x\n", ret);
    }

    vdec_datafifo->phyAddr = phy_addr;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(k_msg_vdec_datafifo_t));
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vdec_deinit_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_msg_vdec_datafifo_t* vdec_datafifo = msg->pBody;

    k_u64 phy_addr;
    ret = vdec_datafifo_deinit(vdec_datafifo->vdec_chn);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace("vdec_datafifo_deinit failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL,0);
    if(resp_msg == NULL) {
        mapi_vdec_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vdec_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_VDEC_INIT,      msg_vdec_init},
    {MSG_CMD_MEDIA_VDEC_DEINIT,       msg_vdec_deinit},
    {MSG_CMD_MEDIA_VDEC_START,      msg_vdec_start},
    {MSG_CMD_MEDIA_VDEC_STOP,      msg_vdec_stop},
    {MSG_CMD_MEDIA_VDEC_BIND_VO,      msg_vdec_bind_vo},
    {MSG_CMD_MEDIA_VDEC_UNBIND_VO,   msg_vdec_unbind_vo},
    {MSG_CMD_MEDIA_VDEC_SEND_STREAM,        msg_vdec_send_frame},
    {MSG_CMD_MEDIA_VDEC_QUERY_STATUS,      msg_vdec_query_status},
    {MSG_CMD_MEDIA_VDEC_INIT_DATAFIFO,      msg_vdec_init_datafifo},
    {MSG_CMD_MEDIA_VDEC_DEINIT_DATAFIFO,      msg_vdec_deinit_datafifo},

};

msg_server_module_t g_module_vdec = {
    K_MAPI_MOD_VDEC,
    "vdec",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_vdec_mod(void)
{
    return &g_module_vdec;
}