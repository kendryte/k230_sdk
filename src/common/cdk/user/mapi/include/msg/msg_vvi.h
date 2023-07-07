/**
 * @file msg_vvi.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2022-10-14
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
#ifndef __MSG_VVI_H__
#define __MSG_VVI_H__

#include "k_vvi_comm.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_VVI_PIPE_START,
    MSG_CMD_MEDIA_VVI_PIPE_STOP,
    MSG_CMD_MEDIA_VVI_INSERT_PIC,
    MSG_CMD_MEDIA_VVI_REMOVE_PIC,
    MSG_CMD_MEDIA_VVI_DUMP_FRAME,
    MSG_CMD_MEDIA_VVI_RELEASE_FRAME,
    MSG_CMD_MEDIA_VVI_BIND_VVO,
    MSG_CMD_MEDIA_VVI_UNBIND_VVO,
} msg_media_vvi_cmd_t;


typedef struct
{
    k_u32 vvi_dev;
    k_u32 vvi_chn;
    k_vvi_dev_attr dev_attr;
    k_vvi_chn_attr chn_attr;
} msg_vvi_pipe_attr_t;

typedef struct
{
    k_u32 vvi_dev;
    k_u32 vvi_chn;
} msg_vvi_pipe_t;

typedef struct
{
    k_u32 vvi_dev;
    k_u32 vvi_chn;
    k_s32 milli_sec;
    k_video_frame_info vf_info;
} msg_vvi_frame_t;

typedef struct
{
    k_u32 vvi_dev;
    k_u32 vvi_chn;
    k_u32 vvo_dev;
    k_u32 vvo_chn;
} msg_vvi_bind_vvo_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VVI_H__ */