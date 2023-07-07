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

#ifndef __MSG_VDSS_H__
#define __MSG_VDSS_H__

#include "k_vdss_comm.h"
#include "mpi_vdss_api.h"
#include "mpi_sys_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef enum {
    MSG_CMD_MEDIA_VDSS_RST,
    MSG_CMD_MEDIA_VDSS_PIPE_START,
    MSG_CMD_MEDIA_VDSS_PIPE_STOP,
    MSG_CMD_MEDIA_VDSS_DUMP_FRAME,
    MSG_CMD_MEDIA_VDSS_RELEASE_FRAME,
    MSG_CMD_MEDIA_VDSS_SET_DEV_ATTR,
    MSG_CMD_MEDIA_VDSS_SET_CHN_ATTR,
} msg_media_vvi_cmd_t;


typedef struct
{
    k_u32 vdss_dev;
    k_u32 vdss_chn;
    k_s32 milli_sec;
    k_video_frame_info vf_info;
} msg_vdss_frame_t;

typedef struct {
    k_u32 chn_num;
    k_u32 dev_num;
    k_vicap_chn_attr attr;
} msg_vdss_chn_info;

typedef struct
{
    k_u32 vdss_dev;
    k_u32 vdss_chn;
} msg_vdss_pipe_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_VVI_H__ */