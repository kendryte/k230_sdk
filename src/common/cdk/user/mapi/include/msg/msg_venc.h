/**
 * @file msg_venc.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-03-22
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

#ifndef __MSG_VENC_H__
#define __MSG_VENC_H__

#include "k_venc_comm.h"
#include "k_nonai_2d_comm.h"
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "mapi_venc_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum
{
    MSG_CMD_MEDIA_VENC_INIT,
    MSG_CMD_MEDIA_VENC_DEINIT,
    MSG_CMD_MEDIA_VENC_START,
    MSG_CMD_MEDIA_VENC_STOP,
    MSG_CMD_MEDIA_VENC_BIND_VPROC,
    MSG_CMD_MEDIA_VENC_UBIND_VPROC,
    MSG_CMD_MEDIA_VENC_SEND_FRAME,
    MSG_CMD_MEDIA_VENC_QUERY_STATUS,
    MSG_CMD_MEDIA_VENC_GET_STREAM,
    MSG_CMD_MEDIA_VENC_RELEASE_STREAM,
    MSG_CMD_MEDIA_VENC_REGISTER_CALLBACK,
    MSG_CMD_MEDIA_VENC_UNREGISTER_CALLBACK,
    MSG_CMD_MEDIA_VENC_INIT_DATAFIFO,
    MSG_CMD_MEDIA_VENC_DELETE_DATAFIFO,
    MSG_CMD_MEDIA_VENC_ENABLE_IDR,
    MSG_CMD_MEDIA_VENC_REQUEST_IDR,
    MSG_CMD_MEDIA_VENC_2D_INIT,
    MSG_CMD_MEDIA_VENC_2D_DEINIT,
    MSG_CMD_MEDIA_VENC_2D_START,
    MSG_CMD_MEDIA_VENC_2D_STOP,
} msg_media_vvi_cmd_t;


typedef struct
{
    k_u32 venc_chn;
    k_venc_chn_attr chn_attr;
} msg_venc_chn_attr_t;

typedef struct
{
    k_u32 venc_chn;
} msg_venc_chn_t;

typedef struct
{
    k_u32 venc_chn;
    k_s32 s32FrameCnt;
} msg_venc_start_t;

typedef struct
{
    k_u32 venc_chn;
} msg_venc_stop_t;


typedef struct
{
    k_u32 src_dev;
    k_u32 src_chn;
    k_u32 venc_chn;
} msg_venc_bind_vi_t;


typedef struct
{
    k_u32 venc_chn;
    kd_venc_callback_s callback_attr;
} msg_venc_callback_attr;

typedef struct
{
    k_u32 fifo_chn;
    k_u64 phyAddr;
} msg_venc_fifo_t;


typedef struct
{
    k_u32 venc_chn;
    k_bool idr_enable;
} msg_venc_chn_idr_t;

typedef struct
{
    k_u32 chn;
    k_nonai_2d_chn_attr attr;
} msg_nonai_2d_chn_attr_t;

typedef struct
{
    k_u32 chn;
} msg_nonai_2d_chn_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VENC_H__ */