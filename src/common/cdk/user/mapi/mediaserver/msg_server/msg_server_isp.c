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
#include "msg_isp.h"
#include "msg_server_dispatch.h"
#include "mapi_isp_api.h"
#include "mapi_vicap_comm.h"

k_s32 msg_isp_ae_roi_get(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_isp_ae_roi_t *roi = msg->pBody;
    ret = kd_mapi_isp_ae_get_roi(roi->dev_num, &roi->ae_roi);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("msg_isp_ae_roi_get failed:0x%x\n", ret);
    }
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_isp_ae_roi_t));
    if(resp_msg == NULL)
    {
        mapi_vicap_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }
    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);
    return K_SUCCESS;
}

k_s32 msg_isp_ae_roi_set(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret = 0;
    k_ipcmsg_message_t *resp_msg;
    msg_isp_ae_roi_t *roi = msg->pBody;
    ret = kd_mapi_isp_ae_set_roi(roi->dev_num, roi->ae_roi);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("msg_isp_ae_roi_set failed:0x%x\n", ret);
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
    {MSG_CMD_MEDIA_ISP_AE_ROI_GET,      msg_isp_ae_roi_get},
    {MSG_CMD_MEDIA_ISP_AE_ROI_SET,      msg_isp_ae_roi_set},
};

msg_server_module_t g_module_isp = {
    K_MAPI_MOD_ISP,
    "isp",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_isp_mod(void)
{
    return &g_module_isp;
}