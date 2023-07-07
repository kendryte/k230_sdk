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
#include "msg_vvi.h"
#include "msg_server_dispatch.h"
#include "mapi_vvi_api.h"
#include "mapi_vvi_comm.h"


k_s32 msg_vvi_pipe_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_pipe_attr_t *pipe_attr = msg->pBody;

    ret = kd_mapi_vvi_start_pipe(pipe_attr->vvi_dev, pipe_attr->vvi_chn,
                                    &pipe_attr->dev_attr, &pipe_attr->chn_attr);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_start_pipe failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_vvi_pipe_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_pipe_t *pipe = msg->pBody;

    ret = kd_mapi_vvi_stop_pipe(pipe->vvi_dev, pipe->vvi_chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_stop_pipe failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vvi_insert_pic(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_frame_t *vvi_frame = msg->pBody;

    ret = kd_mapi_vvi_insert_pic(vvi_frame->vvi_dev, vvi_frame->vvi_chn, &vvi_frame->vf_info);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_insert_pic failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vvi_remove_pic(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_pipe_t *pipe = msg->pBody;

    ret = kd_mapi_vvi_remove_pic(pipe->vvi_dev, pipe->vvi_chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_remove_pic failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vvi_dump_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_frame_t *vvi_frame = msg->pBody;

    ret = kd_mapi_vvi_dump_frame(vvi_frame->vvi_dev, vvi_frame->vvi_chn, &vvi_frame->vf_info, vvi_frame->milli_sec);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vvi_frame_t));
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_vvi_dump_release(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_frame_t *vvi_frame = msg->pBody;

    ret = kd_mapi_vvi_release_frame(vvi_frame->vvi_dev, vvi_frame->vvi_chn, &vvi_frame->vf_info);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_dump_frame failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vvi_bind_vvo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_bind_vvo_t *bind_info = msg->pBody;

    ret = kd_mapi_vvi_bind_vvo(bind_info->vvi_dev, bind_info->vvi_chn, bind_info->vvo_dev, bind_info->vvo_chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_bind_vvo failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_vvi_unbind_vvo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vvi_bind_vvo_t *unbind_info = msg->pBody;

    ret = kd_mapi_vvi_unbind_vvo(unbind_info->vvi_dev, unbind_info->vvi_chn, unbind_info->vvo_dev, unbind_info->vvo_chn);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace("kd_mapi_vvi_bind_vvo failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_vvi_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_vvi_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_VVI_PIPE_START,      msg_vvi_pipe_start},
    {MSG_CMD_MEDIA_VVI_PIPE_STOP,       msg_vvi_pipe_stop},
    {MSG_CMD_MEDIA_VVI_INSERT_PIC,      msg_vvi_insert_pic},
    {MSG_CMD_MEDIA_VVI_REMOVE_PIC,      msg_vvi_remove_pic},
    {MSG_CMD_MEDIA_VVI_DUMP_FRAME,      msg_vvi_dump_frame},
    {MSG_CMD_MEDIA_VVI_RELEASE_FRAME,   msg_vvi_dump_release},
    {MSG_CMD_MEDIA_VVI_BIND_VVO,        msg_vvi_bind_vvo},
    {MSG_CMD_MEDIA_VVI_UNBIND_VVO,      msg_vvi_unbind_vvo},

};

msg_server_module_t g_module_vvi = {
    K_MAPI_MOD_VVI,
    "vvi",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_vvi_mod(void)
{
    return &g_module_vvi;
}