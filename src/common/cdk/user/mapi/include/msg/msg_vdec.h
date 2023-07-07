/**
 * @file msg_vdec.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-06-20
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
 */
#ifndef __MSG_VDEC_H__
#define __MSG_VDEC_H__

#include "k_vdec_comm.h"
#include "mpi_vdec_api.h"
#include "mpi_sys_api.h"
#include "mapi_vdec_comm.h"
#include "k_datafifo.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_VDEC_INIT,
    MSG_CMD_MEDIA_VDEC_DEINIT,
    MSG_CMD_MEDIA_VDEC_START,
    MSG_CMD_MEDIA_VDEC_STOP,
    MSG_CMD_MEDIA_VDEC_BIND_VO,
    MSG_CMD_MEDIA_VDEC_UNBIND_VO,
    MSG_CMD_MEDIA_VDEC_SEND_STREAM,
    MSG_CMD_MEDIA_VDEC_QUERY_STATUS,
    MSG_CMD_MEDIA_VDEC_INIT_DATAFIFO,
    MSG_CMD_MEDIA_VDEC_DEINIT_DATAFIFO,
} k_msg_media_vdec_cmd_t;

typedef struct
{
    k_u32 vdec_chn;
    k_vdec_chn_attr  attr;
} k_msg_vdec_pipe_attr_t;

typedef struct
{
    k_u32 vdec_chn;
    k_u32 vo_dev;
    k_u32 vo_chn;
} k_msg_vdec_bind_vo_t;

typedef struct
{
    k_vdec_stream stream;
    k_u32 vdec_chn;
} k_msg_vdec_stream_t;

typedef struct
{
    k_u32 vdec_chn;
    k_u64 phyAddr;
} k_msg_vdec_datafifo_t;

typedef struct
{
    k_u32 vdec_chn;
    k_vdec_chn_status status;
} k_msg_vdec_chn_status_t;

typedef void (*vdec_datafifo_release_func)(void* pStream);
typedef struct
{
    k_u32 item_count;//in
    k_u32 item_size;//in
    K_DATAFIFO_OPEN_MODE_E open_mode;//in
    vdec_datafifo_release_func release_func;//in

    k_datafifo_handle data_hdl;//out
}k_vdec_datafifo;

#define K_VDEC_DATAFIFO_ITEM_COUNT 25
#define K_VDEC_DATAFIFO_ITEM_SIZE  sizeof(k_msg_vdec_stream_t)

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VDEC_H__ */