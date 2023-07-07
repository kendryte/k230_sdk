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
#include "mapi_sys_comm.h"
#include "mapi_sys_api.h"
#include "msg_server_dispatch.h"
#include "mpi_sys_api.h"
#include "mpi_vb_api.h"
#include "k_vb_comm.h"
#include "mpi_venc_api.h"
#include <stdlib.h>

#define CHECK_MAPI_SYS_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_sys_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_SYS_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

static k_bool g_media_init = K_FALSE;

static k_s32 mapi_media_config_init(k_mapi_media_config_t *media_config)
{
    k_s32 ret;

    /* vb pool config */
    ret = kd_mpi_vb_set_config(&media_config->vb_config);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_set_config failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    /* vb supplenet config */
    ret = kd_mpi_vb_set_supplement_config(&media_config->vb_supp);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_set_supplement_config failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    /* vb pool init */
    ret = kd_mpi_vb_init();
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_init failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }
    return ret;
}

static k_s32 mapi_media_config_deinit(void)
{
    k_s32 ret;

    ret = kd_mpi_vb_exit();
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_exit failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }
    return ret;
}

k_s32 kd_mapi_media_init(const k_mapi_media_attr_t *media_attr)
{

    k_s32 ret;

    if(g_media_init == K_TRUE) {
        mapi_sys_error_trace("media has already been inited\n");
        return K_SUCCESS;
    }

    ret = mapi_media_config_init((k_mapi_media_config_t *)&media_attr->media_config);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_media_config_init failed:0x%08x\n", ret);
        return ret;
    }

    g_media_init = K_TRUE;

    return ret;
}

k_s32 kd_mapi_media_deinit(void)
{

    k_s32 ret;

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media has not been inited\n");
        return K_SUCCESS;
    }
    ret = kd_mpi_venc_close_fd();
    if(ret)
        mapi_sys_error_trace("kd_mpi_venc_close_fd error ret:%d \n", ret);

    ret = mapi_media_config_deinit();
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("mapi_media_config_deinit failed:0x%08x\n", ret);
        return ret;
    }
    g_media_init = K_FALSE;

    return ret;
}

k_s32 kd_mapi_vb_create_pool(k_vb_pool_config *config)
{
    k_s32 ret;
    k_s32 pool_id;
    CHECK_MAPI_SYS_NULL_PTR("pool_config", config);

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_SUCCESS;
    }

    mapi_sys_error_trace("blk_cnt:%d,blk_size:%d,mode:%d\n",config->blk_cnt,config->blk_size,config->mode);
    pool_id = kd_mpi_vb_create_pool(config);
    if(pool_id == VB_INVALID_POOLID) {
        mapi_sys_error_trace("kd_mpi_vb_create_pool failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(pool_id));
        return SYS_RET_MPI_TO_MAPI(pool_id);
    }

    return pool_id;
}

k_s32 kd_mapi_vb_destory_pool(k_u32 pool_id)
{
    k_s32 ret;

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_SUCCESS;
    }

    ret = kd_mpi_vb_destory_pool(pool_id);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_destory_pool failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    return ret;
}

k_s32 kd_mapi_alloc_buffer(k_u64 *phys_addr, void **virt_addr, k_u32 len, const k_char *name)
{
    k_s32 ret;
    k_u64 buffer_phys_addr;
    void *buffer_virt_addr;

    CHECK_MAPI_SYS_NULL_PTR("phys_addr", phys_addr);
    CHECK_MAPI_SYS_NULL_PTR("virt_addr", virt_addr);
    CHECK_MAPI_SYS_NULL_PTR("name", name);

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_SUCCESS;
    }

    if(len <= 0) {
        mapi_sys_error_trace("Buffer len can not be less than zero\n");
        return K_MAPI_ERR_SYS_ILLEGAL_PARAM;
    }

    ret = kd_mpi_sys_mmz_alloc_cached(&buffer_phys_addr, &buffer_virt_addr, name, "anonymous", len);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_sys_mmz_alloc_cached failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    *phys_addr = buffer_phys_addr;
    *virt_addr = buffer_virt_addr;
    return ret;

}

k_s32 kd_mapi_free_buffer(k_u64 phys_addr, void *virt_addr, k_u32 len)
{
    k_s32 ret;
    CHECK_MAPI_SYS_NULL_PTR("virt_addr", virt_addr);

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_SUCCESS;
    }

    ret = kd_mpi_sys_mmz_free(phys_addr, virt_addr);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_sys_mmz_free failed:0x%08x\n",SYS_RET_MPI_TO_MAPI(ret));
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    return ret;
}

/* media server ipcmsg init */
k_s32 kd_mapi_sys_init(void)
{
    k_s32 ret = media_msg_server_init();

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("media_msg_server_init failed:0x%x\n", ret);
        return ret;
    }
    return ret;
}

/* media server ipcmsg deinit */
k_s32 kd_mapi_sys_deinit(void)
{
    k_s32 ret = media_msg_server_deinit();

    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("media_msg_server_deinit failed:0x%x\n", ret);
        return ret;
    }
    return ret;
}

k_s32 kd_mapi_sys_get_vb_block(k_u32 *pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name)
{
    k_vb_blk_handle handle;
    k_u64 get_phys_addr;
    k_u32 get_pool_id;

    CHECK_MAPI_SYS_NULL_PTR("phys_addr", phys_addr);

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, blk_size, mmz_name);
    if(handle == VB_INVALID_HANDLE) {
        mapi_sys_error_trace("kd_mpi_vb_get_block get failed\n");
        return K_MAPI_ERR_SYS_NOMEM;
    }

    get_phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if(get_phys_addr == 0) {
        mapi_sys_error_trace("kd_mpi_vb_handle_to_phyaddr failed\n");
        return K_MAPI_ERR_SYS_NOMEM;
    }

    get_pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if(get_pool_id == VB_INVALID_POOLID) {
        mapi_sys_error_trace("kd_mpi_vb_handle_to_pool_id failed\n");
        return K_MAPI_ERR_SYS_NOMEM;
    }
#if 0
    get_virt_addr = kd_mpi_sys_mmap(get_phys_addr, blk_size);
#endif
    *phys_addr = get_phys_addr;
    *pool_id = get_pool_id;

    return K_SUCCESS;
}

k_s32 kd_mapi_sys_get_vb_block_from_pool_id(k_u32 pool_id, k_u64 *phys_addr, k_u64 blk_size, const char* mmz_name)
{
    k_vb_blk_handle handle;
    k_u64 get_phys_addr;
    k_u32 get_pool_id;

    CHECK_MAPI_SYS_NULL_PTR("phys_addr", phys_addr);

    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    handle = kd_mpi_vb_get_block(pool_id, blk_size, mmz_name);
    if(handle == VB_INVALID_HANDLE) {
        mapi_sys_error_trace("kd_mpi_vb_get_block get failed\n");
        return K_MAPI_ERR_SYS_NOMEM;
    }

    get_phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if(get_phys_addr == 0) {
        mapi_sys_error_trace("kd_mpi_vb_handle_to_phyaddr failed\n");
        return K_MAPI_ERR_SYS_NOMEM;
    }

    *phys_addr = get_phys_addr;

    return K_SUCCESS;
}

k_s32 kd_mapi_sys_release_vb_block(k_u64 phys_addr, k_u64 blk_size)
{
    k_s32 ret;
    k_vb_blk_handle handle;


    if(g_media_init == K_FALSE) {
        mapi_sys_error_trace("media not init yet\n");
        return K_MAPI_ERR_SYS_NOTREADY;
    }

    handle = kd_mpi_vb_phyaddr_to_handle(phys_addr);
    if(handle == VB_INVALID_HANDLE) {
        mapi_sys_error_trace("kd_mpi_vb_phyaddr_to_handle failed\n");
        return K_MAPI_ERR_SYS_ILLEGAL_PARAM;
    }

    ret = kd_mpi_vb_release_block(handle);
    if(ret != K_SUCCESS) {
        mapi_sys_error_trace("kd_mpi_vb_release_block failed\n");
        return SYS_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}
