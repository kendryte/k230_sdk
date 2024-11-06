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
#include "k_datafifo.h"

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
    MSG_CMD_MEDIA_AI_BIND_AO,
    MSG_CMD_MEDIA_AI_UNBIND_AO,
    MSG_CMD_MEDIA_ACODEC_RESET,
    MSG_CMD_MEDIA_AI_SET_VEQ_ATTR,
    MSG_CMD_MEDIA_AI_GET_VEQ_ATTR,
    MSG_CMD_MEDIA_AI_SEND_FAR_ECHO_FRAME,
    MSG_CMD_MEDIA_AI_AEC_INIT_DATAFIFO,
    MSG_CMD_MEDIA_AI_AEC_DEINIT_DATAFIFO,
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

typedef struct
{
    k_handle ai_hdl;
    k_handle ao_hdl;
} k_msg_ai_bind_ao_t;

typedef struct
{
    k_handle ai_hdl;
    k_s32 gain;
}k_msg_ai_gain_info;

typedef struct
{
    k_handle ai_hdl;
    k_ai_vqe_enable vqe_enable;
}k_msg_ai_vqe_attr;

typedef struct
{
    k_handle ai_hdl;
    k_u64 phyAddr;
} k_msg_ai_aec_datafifo_t;

typedef void (*ai_aec_datafifo_release_func)(void* pStream);
typedef struct
{
    k_u32 item_count;//in
    k_u32 item_size;//in
    K_DATAFIFO_OPEN_MODE_E open_mode;//in
    ai_aec_datafifo_release_func release_func;//in

    k_datafifo_handle data_hdl;//out
}k_ai_aec_datafifo;

#define K_AI_AEC_DATAFIFO_ITEM_COUNT 5
#define K_AI_AEC_DATAFIFO_ITEM_SIZE  sizeof(k_msg_ai_frame_t)

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_AI_H__ */