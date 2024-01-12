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
#include "msg_dpu.h"
#include "msg_server_dispatch.h"
#include "mapi_dpu_api.h"
#include "mapi_dpu_comm.h"
#include "k_datafifo.h"
#include "k_dpu_comm.h"
#include <stdio.h>

extern k_u64 send_dpu_data_init(int chn);
extern int send_dpu_data_deinit(int chn);
extern k_s32 kd_mapi_dpu_close();

k_s32 msg_dpu_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    k_dpu_info_t *init = msg->pBody;

    ret = kd_mapi_dpu_init(init);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("kd_mapi_dpu_init failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_register_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_message_t *resp_msg;
    msg_dpu_callback_attr *dpu_callback_msg = msg->pBody;

    ret = kd_mapi_dpu_registercallback(dpu_callback_msg->dpu_chn, &dpu_callback_msg->callback_attr);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_mapi_dpu_registercallback failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}


k_s32 msg_dpu_unregister_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_message_t *resp_msg;
    msg_dpu_callback_attr *dpu_callback_msg = msg->pBody;

    kd_mapi_dpu_unregistercallback(dpu_callback_msg->dpu_chn, &dpu_callback_msg->callback_attr);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_mapi_dpu_registercallback failed:%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}


k_s32 msg_dpu_create_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_dpu_fifo_t *dpu_fifo = msg->pBody;

    dpu_fifo->phyAddr = send_dpu_data_init(dpu_fifo->fifo_chn);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_dpu_fifo_t));
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_delete_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_dpu_fifo_t *dpu_fifo = msg->pBody;

    ret = send_dpu_data_deinit(dpu_fifo->fifo_chn);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_dpu_fifo_t));
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_start_grab(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    // msg_dpu_init_t *dpu_init = msg->pBody;

    ret = kd_mapi_dpu_start_grab();
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("kd_mapi_dpu_init failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_stop_grab(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    // msg_dpu_init_t *dpu_init = msg->pBody;

    ret = kd_mapi_dpu_stop_grab();
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("kd_mapi_dpu_init failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_close(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;

    ret = kd_mapi_dpu_close();
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace("kd_mapi_dpu_close failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_dpu_release_frame(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_dpu_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_dpu_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

static msg_module_cmd_t g_module_cmd_table[] =
{
    {MSG_CMD_MEDIA_DPU_INIT,                    msg_dpu_init},
    // {MSG_CMD_MEDIA_DPU_DELETE,                   msg_dpu_delete},
    // {MSG_CMD_MEDIA_DPU_PARSE_FILE,               msg_dpu_parse_file},
    // {MSG_CMD_MEDIA_DPU_SET_DEV_ATTR,         msg_dpu_set_dev_attr},
    // {MSG_CMD_MEDIA_DPU_GET_DEV_ATTR,         msg_dpu_get_dev_attr},
    // {MSG_CMD_MEDIA_DPU_SET_REF_IMAGE,            msg_dpu_set_ref_image},
    // {MSG_CMD_MEDIA_DPU_SET_PROCESSED_REF_IMAGE,  msg_dpu_set_processed_ref_image},
    // {MSG_CMD_MEDIA_DPU_SET_TEMPLATE_IMAGE,       msg_dpu_set_template_image},
    // {MSG_CMD_MEDIA_DPU_START_DEV,                msg_dpu_start_dev},
    // {MSG_CMD_MEDIA_DPU_SET_CHN_ATTR,         msg_dpu_set_chn_attr},
    // {MSG_CMD_MEDIA_DPU_GET_CHN_ATTR,         msg_dpu_get_chn_attr},
    // {MSG_CMD_MEDIA_DPU_START_CHN,                msg_dpu_start_chn},
    // {MSG_CMD_MEDIA_DPU_STOP_CHN,             msg_dpu_stop_chn},
    // {MSG_CMD_MEDIA_DPU_SEND_FRAME,               msg_dpu_send_frame},
    // {MSG_CMD_MEDIA_DPU_GET_FRAME,                msg_dpu_get_frame},
    {MSG_CMD_MEDIA_DPU_RELEASE_FRAME,            msg_dpu_release_frame},
    {MSG_CMD_MEDIA_DPU_REGISTER_CALLBACK,       msg_dpu_register_callback},
    {MSG_CMD_MEDIA_DPU_UNREGISTER_CALLBACK,     msg_dpu_unregister_callback},

    {MSG_CMD_MEDIA_DPU_INIT_DATAFIFO,           msg_dpu_create_datafifo},
    {MSG_CMD_MEDIA_DPU_DELETE_DATAFIFO,         msg_dpu_delete_datafifo},
    {MSG_CMD_MEDIA_DPU_START_GRAB,              msg_dpu_start_grab},
    {MSG_CMD_MEDIA_DPU_STOP_GRAB,               msg_dpu_stop_grab},
    {MSG_CMD_MEDIA_DPU_CLOSE,                   msg_dpu_close},
};

msg_server_module_t g_module_dpu =
{
    K_MAPI_MOD_DPU,
    "dpu",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_dpu_mod(void)
{
    mapi_dpu_error_trace("### \n");
    return &g_module_dpu;
}