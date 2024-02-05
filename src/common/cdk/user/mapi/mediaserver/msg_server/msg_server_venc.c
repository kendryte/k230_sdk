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
#include "msg_venc.h"
#include "msg_server_dispatch.h"
#include "mapi_venc_api.h"
#include "mapi_nonai_2d_api.h"
#include "mapi_venc_comm.h"
#include "k_datafifo.h"
#include "send_venc_data.h"
#include <stdio.h>

k_s32 msg_nonai_2d_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_nonai_2d_chn_attr_t *nonai_2d_chn_attr = msg->pBody;

    ret = kd_mapi_nonai_2d_init(nonai_2d_chn_attr->chn, &nonai_2d_chn_attr->attr);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_nonai_2d_init failed:0x%x\n", ret);
    }
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_nonai_2d_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_nonai_2d_chn_t *nonai_2d_chn = msg->pBody;

    ret = kd_mapi_nonai_2d_deinit(nonai_2d_chn->chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_nonai_2d_deinit failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_nonai_2d_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_nonai_2d_chn_t *nonai_2d_chn = msg->pBody;

    ret = kd_mapi_nonai_2d_start(nonai_2d_chn->chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_nonai_2d_start failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_nonai_2d_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_nonai_2d_chn_t *nonai_2d_chn = msg->pBody;

    ret = kd_mapi_nonai_2d_stop(nonai_2d_chn->chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_nonai_2d_stop failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_chn_attr_t *venc_chn_attr = msg->pBody;

    ret = kd_mapi_venc_init(venc_chn_attr->venc_chn, &venc_chn_attr->chn_attr);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_init failed:0x%x\n", ret);
    }
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}


k_s32 msg_venc_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_chn_t *venc_chn = msg->pBody;

    ret = kd_mapi_venc_deinit(venc_chn->venc_chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_deinit failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_start(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_start_t *venc_start = msg->pBody;

    ret = kd_mapi_venc_start(venc_start->venc_chn, venc_start->s32FrameCnt);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_start failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_stop(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_stop_t *venc_stop = msg->pBody;

    ret = kd_mapi_venc_stop(venc_stop->venc_chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_stop failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_bind_vi(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_bind_vi_t *bind_info = msg->pBody;

    ret = kd_mapi_venc_bind_vi(bind_info->src_dev, bind_info->src_chn, bind_info->venc_chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_bind_vi failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_venc_unbind_vi(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_bind_vi_t *bind_info = msg->pBody;

    ret = kd_mapi_venc_unbind_vi(bind_info->src_dev, bind_info->src_chn, bind_info->venc_chn);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_unbind_vi failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}





k_s32 msg_venc_register_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_callback_attr *venc_callback_msg = msg->pBody;

    kd_mapi_venc_registercallback(venc_callback_msg->venc_chn, &venc_callback_msg->callback_attr);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_mapi_venc_registercallback failed:%x\n", ret);
    }
    mapi_venc_error_trace("callback_attr set\n");

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}


k_s32 msg_venc_unregister_callback(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_callback_attr *venc_callback_msg = msg->pBody;

    kd_mapi_venc_unregistercallback(venc_callback_msg->venc_chn, &venc_callback_msg->callback_attr);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_mapi_venc_registercallback failed:%x\n", ret);
    }
    mapi_venc_error_trace("callback_attr set\n");

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}


k_s32 msg_venc_create_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_fifo_t *venc_fifo = msg->pBody;

    mapi_venc_error_trace("venc_chn:%d ###", venc_fifo->fifo_chn);

    venc_fifo->phyAddr = send_venc_data_init(venc_fifo->fifo_chn);
    mapi_venc_error_trace("phyAddr:%lx ###\n", venc_fifo->phyAddr);
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_venc_fifo_t));
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_delete_datafifo(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_fifo_t *venc_fifo = msg->pBody;

    send_venc_data_deinit(venc_fifo->fifo_chn);

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_venc_fifo_t));
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


k_s32 msg_venc_enable_idr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_venc_chn_idr_t *venc_chn_idr = msg->pBody;

    ret = kd_mapi_venc_enable_idr(venc_chn_idr->venc_chn, venc_chn_idr->idr_enable);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_request_idr failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}

k_s32 msg_venc_request_idr(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u32 *chn_num = msg->pBody;

    ret = kd_mapi_venc_request_idr(*chn_num);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_request_idr failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if (resp_msg == NULL)
    {
        mapi_venc_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return K_SUCCESS;
}


static msg_module_cmd_t g_module_cmd_table[] =
{
    {MSG_CMD_MEDIA_VENC_INIT,      msg_venc_init},
    {MSG_CMD_MEDIA_VENC_DEINIT,       msg_venc_deinit},
    {MSG_CMD_MEDIA_VENC_START,      msg_venc_start},
    {MSG_CMD_MEDIA_VENC_STOP,      msg_venc_stop},
    {MSG_CMD_MEDIA_VENC_BIND_VPROC,      msg_venc_bind_vi},
    {MSG_CMD_MEDIA_VENC_UBIND_VPROC,   msg_venc_unbind_vi},
    // {MSG_CMD_MEDIA_VENC_SEND_FRAME,        msg_venc_send_frame},
    // {MSG_CMD_MEDIA_VENC_QUERY_STATUS,      msg_venc_query_status},
    // {MSG_CMD_MEDIA_VENC_GET_STREAM,      msg_venc_get_stream},
    // {MSG_CMD_MEDIA_VENC_RELEASE_STREAM,      msg_venc_release_stream},
    {MSG_CMD_MEDIA_VENC_REGISTER_CALLBACK,      msg_venc_register_callback},
    {MSG_CMD_MEDIA_VENC_UNREGISTER_CALLBACK,      msg_venc_unregister_callback},
    {MSG_CMD_MEDIA_VENC_INIT_DATAFIFO,      msg_venc_create_datafifo},
    {MSG_CMD_MEDIA_VENC_DELETE_DATAFIFO,      msg_venc_delete_datafifo},
    {MSG_CMD_MEDIA_VENC_ENABLE_IDR,      msg_venc_enable_idr},
    {MSG_CMD_MEDIA_VENC_REQUEST_IDR,      msg_venc_request_idr},
    {MSG_CMD_MEDIA_VENC_2D_INIT, msg_nonai_2d_init},
    {MSG_CMD_MEDIA_VENC_2D_DEINIT, msg_nonai_2d_deinit},
    {MSG_CMD_MEDIA_VENC_2D_START, msg_nonai_2d_start},
    {MSG_CMD_MEDIA_VENC_2D_STOP, msg_nonai_2d_stop},
};

msg_server_module_t g_module_venc =
{
    K_MAPI_MOD_VENC,
    "venc",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_venc_mod(void)
{
    mapi_venc_error_trace("### \n");
    return &g_module_venc;
}