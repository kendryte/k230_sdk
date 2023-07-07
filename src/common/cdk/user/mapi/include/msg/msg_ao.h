/**
 * @file msg_ao.h
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
#ifndef __MSG_AO_H__
#define __MSG_AO_H__

#include "k_ao_comm.h"
#include "mpi_ao_api.h"
#include "mpi_sys_api.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_AO_INIT,
    MSG_CMD_MEDIA_AO_DEINIT,
    MSG_CMD_MEDIA_AO_START,
    MSG_CMD_MEDIA_AO_STOP,
    MSG_CMD_MEDIA_AO_SETVOLUME,
    MSG_CMD_MEDIA_AO_GETVOLUME,
    MSG_CMD_MEDIA_AO_MUTE,
    MSG_CMD_MEDIA_AO_UNMUTE,
    MSG_CMD_MEDIA_AO_SENDFRAME,
} k_msg_media_ao_cmd_t;

typedef struct
{
    k_u32 ao_dev;
    k_u32 ao_chn;
    k_aio_dev_attr  attr;
    k_handle ao_hdl;
} k_msg_ao_pipe_attr_t;

typedef struct
{
    k_handle ao_hdl;
    k_s32 milli_sec;
    k_audio_frame audio_frame;
} k_msg_ao_frame_t;

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_AO_H__ */