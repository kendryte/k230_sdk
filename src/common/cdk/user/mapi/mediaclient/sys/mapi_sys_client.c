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

#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include "mapi_sys_api.h"
#include "msg_client_dispatch.h"
#include "mapi_sys_comm.h"
#include "mpi_sys_api.h"
#include "msg_sys.h"
#include "k_type.h"


#define CHECK_MAPI_SYS_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_sys_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_SYS_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

static k_bool g_sys_msg_init = K_FALSE;
static k_bool g_media_init = K_FALSE;
static pthread_mutex_t g_media_init_lock;

/* media client ipcmsg init */
k_s32 kd_mapi_sys_init(void)
{

    if(g_sys_msg_init == K_TRUE) {
        mapi_sys_error_trace("sys has already been inited\n");
        return K_SUCCESS;
    }

    k_s32 ret = media_msg_client_init();

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("media_msg_client_init failed:0x%x\n", ret);
        return ret;
    }
    g_sys_msg_init = K_TRUE;
    pthread_mutex_init(&g_media_init_lock, NULL);
    return ret;
}

/* media client ipcmsg deinit */
k_s32 kd_mapi_sys_deinit(void)
{
    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys has already been deinited\n");
        return K_SUCCESS;
    }

    k_s32 ret = media_msg_client_deinit();

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("media_msg_client_deinit failed:0x%x\n", ret);
        return ret;
    }
    g_sys_msg_init = K_FALSE;
    pthread_mutex_destroy(&g_media_init_lock);
    return ret;
}

k_s32 kd_mapi_media_init(const k_mapi_media_attr_t *media_attr)
{
    k_s32 ret;

    CHECK_MAPI_SYS_NULL_PTR("media_attr", media_attr);

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_TRUE) {
        mapi_sys_error_trace("media has already been inited\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_INIT,
                        (void *)media_attr, sizeof(k_mapi_media_attr_t), NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    g_media_init = K_TRUE;
    pthread_mutex_unlock(&g_media_init_lock);

    return ret;
}

k_s32 kd_mapi_media_deinit(void)
{
    k_s32 ret;

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media has already been deinited\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_DEINIT, NULL, 0, NULL);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    g_media_init = K_FALSE;
    pthread_mutex_unlock(&g_media_init_lock);

    return ret;
}

k_s32 kd_mapi_vb_create_pool(k_vb_pool_config *config)
{
    k_s32 ret;
    CHECK_MAPI_SYS_NULL_PTR("pool_config", config);

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    msg_vb_config vb_config;
    vb_config.vb_pool_config = *config;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_CREATE_POOL,
                    &vb_config, sizeof(vb_config), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    pthread_mutex_unlock(&g_media_init_lock);

    return vb_config.pool_id;
}

k_s32 kd_mapi_vb_destory_pool(k_u32 pool_id)
{
    k_s32 ret;
    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_DESTORY_POOL,
                    &pool_id, sizeof(pool_id), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    pthread_mutex_unlock(&g_media_init_lock);

    return ret;
}

k_s32 kd_mapi_alloc_buffer(k_u64 *phys_addr, void **virt_addr, k_u32 len, const k_char *name)
{
    k_s32 ret;
    msg_alloc_buffer_t alloc_buffer;
    void* mmap_addr;

    CHECK_MAPI_SYS_NULL_PTR("phys_addr", phys_addr);
    CHECK_MAPI_SYS_NULL_PTR("virt_addr", virt_addr);
    CHECK_MAPI_SYS_NULL_PTR("name", name);

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    if(len <= 0) {
        mapi_sys_error_trace("Buffer len can not be less than zero\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_MAPI_ERR_SYS_ILLEGAL_PARAM;
    }

    alloc_buffer.len = len;
    strcpy(alloc_buffer.name, name);

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_ALLOC_BUFFER,
                    &alloc_buffer, sizeof(msg_alloc_buffer_t), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }
    *phys_addr = alloc_buffer.phys_addr;
#if 0
    mmap_addr = kd_mpi_sys_mmap(alloc_buffer.phys_addr, alloc_buffer.len);
#else
    mmap_addr = NULL;
#endif
    *virt_addr = mmap_addr;

    pthread_mutex_unlock(&g_media_init_lock);

    return ret;

}

k_s32 kd_mapi_free_buffer(k_u64 phys_addr, void *virt_addr, k_u32 len)
{
    k_s32 ret;
    msg_free_buffer_t free_buffer;

    CHECK_MAPI_SYS_NULL_PTR("virt_addr", virt_addr);

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    if(len <= 0) {
        mapi_sys_error_trace("Buffer len can not be less than zero\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_MAPI_ERR_SYS_ILLEGAL_PARAM;
    }

    free_buffer.phys_addr = phys_addr;
    free_buffer.len = len;

    ret = munmap(virt_addr, len);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("munmap fail virtaddr:%p\n", virt_addr);
    }

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_FREE_BUFFER,
                    &free_buffer, sizeof(msg_free_buffer_t), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    pthread_mutex_unlock(&g_media_init_lock);
    return ret;
}

k_s32 kd_mapi_sys_get_vb_block(k_u32 *pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name)
{
    k_s32 ret;
    msg_vb_info_t vb_info;

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    memset(&vb_info, 0, sizeof(vb_info));
    vb_info.blk_size = blk_size;
    if(mmz_name)
        strcpy(vb_info.mmz_name, mmz_name);

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_GET_VB,
                &vb_info, sizeof(vb_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    *phys_addr = vb_info.phys_addr;
    *pool_id = vb_info.pool_id;
    pthread_mutex_unlock(&g_media_init_lock);

    return ret;
}

k_s32 kd_mapi_sys_get_vb_block_from_pool_id(k_u32 pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name)
{
    k_s32 ret;
    msg_vb_info_t vb_info;

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    memset(&vb_info, 0, sizeof(vb_info));
    vb_info.blk_size = blk_size;
    vb_info.pool_id = pool_id;
    if(mmz_name)
        strcpy(vb_info.mmz_name, mmz_name);

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_GET_VB_FROM_POOL_ID,
                &vb_info, sizeof(vb_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }

    *phys_addr = vb_info.phys_addr;
    pthread_mutex_unlock(&g_media_init_lock);

    return ret;
}

k_s32 kd_mapi_sys_release_vb_block(k_u64 phys_addr,  k_u64 blk_size)
{    k_s32 ret;
    msg_vb_info_t vb_info;

    if(g_sys_msg_init == K_FALSE) {
        mapi_sys_error_trace("sys not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    pthread_mutex_lock(&g_media_init_lock);\
    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return K_SUCCESS;
    }

    memset(&vb_info, 0, sizeof(vb_info));
    vb_info.blk_size = blk_size;
    vb_info.phys_addr = phys_addr;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_RELEASE_VB,
                &vb_info, sizeof(vb_info), NULL);

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_send_sync failed\n");
        pthread_mutex_unlock(&g_media_init_lock);
        return ret;
    }
    pthread_mutex_unlock(&g_media_init_lock);
    return ret;
}