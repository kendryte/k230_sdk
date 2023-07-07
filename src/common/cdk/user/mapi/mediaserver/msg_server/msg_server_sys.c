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
#include "msg_sys.h"
#include "msg_dispatch.h"
#include "msg_server_dispatch.h"
#include "mapi_sys_api.h"
#include "mapi_sys_comm.h"

k_s32 msg_media_sys_init(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;

    ret = kd_mapi_media_init((const k_mapi_media_attr_t*)msg->pBody);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_media_init failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_deinit(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;

    ret = kd_mapi_media_deinit();
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_media_deinit failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_alloc_buffer(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_alloc_buffer_t *alloc_buffer = msg->pBody;
    k_u64 phys_addr;
    void *virt_addr;

    ret = kd_mapi_alloc_buffer(&phys_addr, &virt_addr, alloc_buffer->len,
                                    alloc_buffer->name);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_alloc_buffer failed:0x%x\n", ret);
    }

    alloc_buffer->phys_addr = phys_addr;
    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_alloc_buffer_t));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_free_buffer(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_free_buffer_t *free_buffer = msg->pBody;

    ret = kd_mapi_free_buffer(free_buffer->phys_addr, free_buffer->virt_addr, free_buffer->len);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_free_buffer failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_alloc_buffer_t));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_get_vb_block(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vb_info_t *vb_info = msg->pBody;
    k_u32 pool_id;
    k_u64 phys_addr;

    ret = kd_mapi_sys_get_vb_block(&pool_id, &phys_addr, vb_info->blk_size, vb_info->mmz_name);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_sys_get_vb_block failed:0x%x\n", ret);
    }

    vb_info->phys_addr = phys_addr;
    vb_info->pool_id = pool_id;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vb_info_t));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_get_vb_block_from_pool_id(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vb_info_t *vb_info = msg->pBody;
    k_u32 pool_id = vb_info->pool_id;
    k_u64 phys_addr;

    ret = kd_mapi_sys_get_vb_block_from_pool_id(pool_id, &phys_addr, vb_info->blk_size, vb_info->mmz_name);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_sys_get_vb_block failed:0x%x\n", ret);
    }

    vb_info->phys_addr = phys_addr;

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vb_info_t));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_release_vb_block(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    msg_vb_info_t *vb_info = msg->pBody;

    ret = kd_mapi_sys_release_vb_block(vb_info->phys_addr, vb_info->blk_size);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_sys_release_vb_block failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vb_info_t));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

k_s32 msg_media_sys_create_pool(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;

    msg_vb_config* vb_config = (msg_vb_config*)msg->pBody;

    k_s32 pool_id;

    pool_id = kd_mapi_vb_create_pool(&vb_config->vb_pool_config);
    if(pool_id == VB_INVALID_POOLID) {
        mapi_sys_error_trace("kd_mapi_vb_create_pool failed:0x%x\n", pool_id);
        ret = VB_INVALID_POOLID;
    }
    else
    {
        vb_config->pool_id = pool_id;
        ret = K_SUCCESS;
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, msg->pBody, sizeof(msg_vb_config));
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    mapi_sys_error_trace("msg_media_sys_create_pool ret:%d\n",ret);

    return ret;
}

k_s32 msg_media_sys_destory_pool(k_s32 id, k_ipcmsg_message_t *msg)
{
    k_s32 ret;
    k_ipcmsg_message_t *resp_msg;
    k_u32* pool_id = msg->pBody;

    ret = kd_mapi_vb_destory_pool(*pool_id);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mapi_vb_destory_pool failed:0x%x\n", ret);
    }

    resp_msg = kd_ipcmsg_create_resp_message(msg, ret, NULL, 0);
    if(resp_msg == NULL) {
        mapi_sys_error_trace("kd_ipcmsg_create_resp_message failed\n");
        return K_FAILED;
    }

    ret = kd_ipcmsg_send_async(id, resp_msg, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace(" kd_ipcmsg_send_async failed:%x\n", ret);
    }
    kd_ipcmsg_destroy_message(resp_msg);

    return ret;
}

static msg_module_cmd_t g_module_cmd_table[] = {
    {MSG_CMD_MEDIA_SYS_INIT,            msg_media_sys_init},
    {MSG_CMD_MEDIA_SYS_DEINIT,          msg_media_sys_deinit},
    {MSG_CMD_MEDIA_SYS_ALLOC_BUFFER,    msg_media_sys_alloc_buffer},
    {MSG_CMD_MEDIA_SYS_FREE_BUFFER,     msg_media_sys_free_buffer},
    {MSG_CMD_MEDIA_SYS_GET_VB,          msg_media_sys_get_vb_block},
    {MSG_CMD_MEDIA_SYS_RELEASE_VB,      msg_media_sys_release_vb_block},
    {MSG_CMD_MEDIA_SYS_GET_VB_FROM_POOL_ID, msg_media_sys_get_vb_block_from_pool_id},
    {MSG_CMD_MEDIA_SYS_CREATE_POOL,     msg_media_sys_create_pool},
    {MSG_CMD_MEDIA_SYS_DESTORY_POOL,    msg_media_sys_destory_pool},




};

msg_server_module_t g_module_sys = {
    K_MAPI_MOD_SYS,
    "sys",
    sizeof(g_module_cmd_table) / sizeof(msg_module_cmd_t),
    &g_module_cmd_table[0]
};

msg_server_module_t *mapi_msg_get_sys_mod(void)
{
    return &g_module_sys;
}