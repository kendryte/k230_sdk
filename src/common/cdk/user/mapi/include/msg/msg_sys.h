/**
 * @file msg_sys.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-06-12
 *
* @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
 *
 */
#ifndef __MSG_SYS_H__
#define __MSG_SYS_H__

#include "k_type.h"
#include "k_vb_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define SYS_NAME_LEN    128

typedef enum {
    MSG_CMD_MEDIA_SYS_INIT = 0,
    MSG_CMD_MEDIA_SYS_DEINIT,
    MSG_CMD_MEDIA_SYS_ALLOC_BUFFER,
    MSG_CMD_MEDIA_SYS_FREE_BUFFER,
    MSG_CMD_MEDIA_SYS_GET_VB,
    MSG_CMD_MEDIA_SYS_RELEASE_VB,
    MSG_CMD_MEDIA_SYS_GET_VB_FROM_POOL_ID,
    MSG_CMD_MEDIA_SYS_CREATE_POOL,
    MSG_CMD_MEDIA_SYS_DESTORY_POOL,
} msg_media_sys_cmd_t;

typedef struct
{
    k_u32 phys_addr;
    k_u32 len;
    k_char name[SYS_NAME_LEN];
} msg_alloc_buffer_t;

typedef struct {
    k_u32 phys_addr;
    void *virt_addr;
    k_u32 len;
} msg_free_buffer_t;

typedef struct {
    k_u32 pool_id;
    k_u64 blk_size;
    k_char mmz_name[SYS_NAME_LEN];
    k_u64 phys_addr;
}msg_vb_info_t;

typedef struct {
    k_u32 pool_id;
    k_vb_pool_config vb_pool_config;
} msg_vb_config;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_SYS_H__ */