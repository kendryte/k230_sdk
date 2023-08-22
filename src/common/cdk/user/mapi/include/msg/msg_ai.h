/**
 * @file msg_ai.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-03-24
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
#ifndef __MSG_AI_H__
#define __MSG_AI_H__

#include "k_ai_comm.h"
#include "mpi_ai_api.h"
#include "mpi_sys_api.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_AI_INIT,
    MSG_CMD_MEDIA_AI_DEINIT,
    MSG_CMD_MEDIA_AI_START,
    MSG_CMD_MEDIA_AI_STOP,
    MSG_CMD_MEDIA_AI_SETVOLUME,
    MSG_CMD_MEDIA_AI_GETVOLUME,
    MSG_CMD_MEDIA_AI_MUTE,
    MSG_CMD_MEDIA_AI_UNMUTE,
    MSG_CMD_MEDIA_AI_GETFRAME,
    MSG_CMD_MEDIA_AI_RELEASEFRAME,
    MSG_CMD_MEDIA_AI_SET_PITCH_SHIFT_ATTR,
    MSG_CMD_MEDIA_AI_GET_PITCH_SHIFT_ATTR,
} k_msg_media_ai_cmd_t;

typedef struct
{
    k_u32 ai_dev;
    k_u32 ai_chn;
    k_aio_dev_attr  attr;
    k_handle ai_hdl;
} k_msg_ai_pipe_attr_t;


typedef struct
{
    k_handle ai_hdl;
    k_s32 milli_sec;
    k_audio_frame audio_frame;
} k_msg_ai_frame_t;

typedef struct
{
    k_handle ai_hdl;
    k_s32 milli_sec;
    k_ai_chn_pitch_shift_param  param;
} k_msg_ai_pitch_shift_attr_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_AI_H__ */