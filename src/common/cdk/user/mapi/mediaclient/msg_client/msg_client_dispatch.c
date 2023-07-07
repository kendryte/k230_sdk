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

#include <pthread.h>
#include <stdlib.h>
#include "k_ipcmsg.h"
#include "k_comm_ipcmsg.h"
#include "mapi_sys_comm.h"
#include "mapi_sys_api.h"
#include "msg_client_dispatch.h"

#define IPCMSG_MEDIA_CLIENT_NAME    "kd_mapi_msg"
#define IPCMSG_PORT_MPP             (1)

static k_s32 g_media_msg_id = -1;
static k_bool g_msg_start_flag = K_FALSE;
static pthread_t g_client_receive_thread = -1;

k_s32 mapi_media_msg_get_id(void)
{
    return g_media_msg_id;
}

k_s32 mapi_send_sync(k_u32 module, k_u32 cmd, void *body, k_u32 body_len,
        mapi_priv_data_t *priv_data)
{
    k_s32 ret;
    k_ipcmsg_message_t *req = NULL;
    k_ipcmsg_message_t *resp = NULL;

    req = kd_ipcmsg_create_message(module, cmd, body, body_len);
    if(req == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_message failed\n");
        return K_MAPI_ERR_SYS_NULL_PTR;
    }

    if(priv_data != NULL) {
        memcpy(req->as32PrivData, priv_data->priv_data, sizeof(k_s32) * K_IPCMSG_PRIVDATA_NUM);
    }

    ret = kd_ipcmsg_send_sync(mapi_media_msg_get_id(), req, &resp, K_IPCMSG_SEND_SYNC_TIMEOUT);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_send_sync failed:0x%x\n", ret);
        kd_ipcmsg_destroy_message(req);
        kd_ipcmsg_destroy_message(resp);
        return ret;
    }

    ret = resp->s32RetVal;
    if(ret == K_SUCCESS && (resp->u32BodyLen > 0)) {
        memcpy(body, resp->pBody, resp->u32BodyLen);
        if(priv_data != NULL) {
            memcpy(priv_data->priv_data, resp->as32PrivData, sizeof(k_s32) * K_IPCMSG_PRIVDATA_NUM);
        }
    }
    kd_ipcmsg_destroy_message(req);
    kd_ipcmsg_destroy_message(resp);

    return ret;
}

k_s32 mapi_send_sync_with_resp(k_u32 module, k_u32 cmd, void *req_body, void* resp_body,
        k_u32 req_body_len, mapi_priv_data_t *priv_data)
{
    k_s32 ret;
    k_ipcmsg_message_t *req = NULL;
    k_ipcmsg_message_t *resp = NULL;
    req = kd_ipcmsg_create_message(module, cmd, req_body, req_body_len);
    if(req == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_message failed\n");
        return K_MAPI_ERR_SYS_NULL_PTR;
    }

    if(priv_data != NULL) {
        memcpy(req->as32PrivData, priv_data->priv_data, sizeof(k_s32) * K_IPCMSG_PRIVDATA_NUM);
    }

    ret = kd_ipcmsg_send_sync(mapi_media_msg_get_id(), req, &resp, K_IPCMSG_SEND_SYNC_TIMEOUT);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_send_sync failed:0x%x\n", ret);
        kd_ipcmsg_destroy_message(req);
        kd_ipcmsg_destroy_message(resp);
        return ret;
    }

    ret = resp->s32RetVal;
    if(ret == K_SUCCESS && (resp->u32BodyLen > 0)) {
        memcpy(resp_body, resp->pBody, resp->u32BodyLen);
        if(priv_data != NULL) {
            memcpy(priv_data->priv_data, resp->as32PrivData, sizeof(k_s32) * K_IPCMSG_PRIVDATA_NUM);
        }
    }
    kd_ipcmsg_destroy_message(req);
    kd_ipcmsg_destroy_message(resp);

    return ret;
    return ret;
}

static void meida_msg_receive_proc(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_u32 mod_id;
    k_ipcmsg_message_t *resp_msg;
    k_s32 ret;

    mod_id = GET_MOD_ID(msg->u32Module);

    resp_msg = kd_ipcmsg_create_resp_message(msg, K_SUCCESS, NULL, 0);
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_send_async failed :0x%x\n", ret);
    }

    kd_ipcmsg_destroy_message(resp_msg);
    return;
}

static void *media_msg_client_receive_thread(void *arg)
{
    mapi_sys_info_trace("Run\n");
    kd_ipcmsg_run(g_media_msg_id);
    mapi_sys_info_trace("After run\n");
    return NULL;
}

k_s32 media_msg_client_init(void)
{
    k_s32 ret = K_SUCCESS;
    k_ipcmsg_connect_t conn_attr ={ 1, IPCMSG_PORT_MPP, 1 };

    ret = kd_ipcmsg_add_service(IPCMSG_MEDIA_CLIENT_NAME, &conn_attr);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_ipcmsg_add_service failed:0x%08x\n", ret);
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    ret = kd_ipcmsg_connect(&g_media_msg_id, IPCMSG_MEDIA_CLIENT_NAME, meida_msg_receive_proc);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("connect failed\n");
        return ret;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&g_client_receive_thread, &attr, media_msg_client_receive_thread, &g_media_msg_id);
    if(K_SUCCESS != ret) {
        pthread_attr_destroy(&attr);
        mapi_sys_error_trace("pthread_create failed\n");
        return ret;
    }

    pthread_attr_destroy(&attr);
    mapi_sys_info_trace("msg init success!\n");
    return ret;
}

k_s32 media_msg_client_deinit(void)
{
    k_s32 ret;

    ret = kd_ipcmsg_disconnect(g_media_msg_id);

    pthread_join(g_client_receive_thread, NULL);

    kd_ipcmsg_del_service(IPCMSG_MEDIA_CLIENT_NAME);
    return ret;
}

