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
#include "msg_vdss.h"
#include "msg_server_dispatch.h"
#include "mapi_vdss_api.h"
#include "mapi_vvi_comm.h"
#include "stdio.h"
#include "k_vdss_comm.h"

k_s32 msg_vdss_pipe_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vdss_pipe_t *pipe = msg->pBody;

    ret = kd_mapi_vdss_stop_pipe(pipe->vdss_dev, pipe->vdss_chn);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_pipe_stop failed:0x%x\n", ret);
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

k_s32 msg_vdss_pipe_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vdss_pipe_t *pipe = msg->pBody;

    ret = kd_mapi_vdss_start_pipe(pipe->vdss_dev, pipe->vdss_chn);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_pipe_start failed:0x%x\n", ret);
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


k_s32 msg_vdss_pipe_rst(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u8 *val = msg->pBody;
    k_u8 val2 = *val;

    ret = kd_mapi_vdss_rst(val2);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_pipe_rst failed:0x%x\n", ret);
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

k_s32 msg_vdss_set_chn_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vdss_chn_info *pipe = msg->pBody;

    ret = kd_mapi_vdss_set_chn_attr(pipe->dev_num, pipe->chn_num, &pipe->attr);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_set_chn_attr failed:0x%x\n", ret);
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


k_s32 msg_vdss_set_dev_attr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_vicap_dev_attr *pipe = msg->pBody;

    ret = kd_mapi_vdss_set_dev_attr(pipe);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_set_dev_attr failed:0x%x\n", ret);
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

k_s32 msg_vdss_dump_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vdss_frame_t *pipe = msg->pBody;

    ret = kd_mapi_vdss_dump_frame(pipe->vdss_dev, pipe->vdss_chn, &pipe->vf_info, pipe->milli_sec);
    if(ret != K_SUCCESS) {
        printf("msg_vdss_dump_frame failed:0x%x\n", ret);
    }

    printf("#####---------------------- msg_vdss_dump_frame phy addr  is %x \n", pipe->vf_info.v_frame.phys_addr[0]);

    // resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vdss_frame_t));
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

k_s32 msg_vdss_release_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vdss_frame_t *pipe = msg->pBody;

    ret = kd_mapi_vdss_chn_release_frame(pipe->vdss_dev, pipe->vdss_chn, &pipe->vf_info);
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


static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_VDSS_RST,              msg_vdss_pipe_rst},
    {MSG_CMD_MEDIA_VDSS_PIPE_START,       msg_vdss_pipe_start},
    {MSG_CMD_MEDIA_VDSS_PIPE_STOP,        msg_vdss_pipe_stop},
    {MSG_CMD_MEDIA_VDSS_DUMP_FRAME,       msg_vdss_dump_frame},
    {MSG_CMD_MEDIA_VDSS_RELEASE_FRAME,    msg_vdss_release_frame},
    {MSG_CMD_MEDIA_VDSS_SET_DEV_ATTR,     msg_vdss_set_dev_attr},
    {MSG_CMD_MEDIA_VDSS_SET_CHN_ATTR,     msg_vdss_set_chn_attr},

};

msg_server_module_t g_module_vdss = {
    K_MAPI_MOD_VI,
    "vi",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_vi_mod(void)
{
    return &g_module_vdss;
}