/**
 * @file msg_adec.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-05-11
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
#ifndef __MSG_ADEC_H__
#define __MSG_ADEC_H__

#include "k_adec_comm.h"
#include "mpi_adec_api.h"
#include "mpi_sys_api.h"
#include "mapi_adec_comm.h"
#include "k_datafifo.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_ADEC_INIT,
    MSG_CMD_MEDIA_ADEC_DEINIT,
    MSG_CMD_MEDIA_ADEC_START,
    MSG_CMD_MEDIA_ADEC_STOP,
    MSG_CMD_MEDIA_ADEC_REGISTER_CALLBACK,
    MSG_CMD_MEDIA_ADEC_UNREGISTER_CALLBACK,
    MSG_CMD_MEDIA_ADEC_BIND_AO,
    MSG_CMD_MEDIA_ADEC_UNBIND_AO,
    MSG_CMD_MEDIA_ADEC_REGISTER_EXT_AUDIO_DECODER,
    MSG_CMD_MEDIA_ADEC_UNREGISTER_EXT_AUDIO_DECODER,
    MSG_CMD_MEDIA_ADEC_SEND_STREAM,
    MSG_CMD_MEDIA_ADEC_INIT_DATAFIFO,
    MSG_CMD_MEDIA_ADEC_DEINIT_DATAFIFO,
} k_msg_media_adec_cmd_t;

typedef struct
{
    k_handle adec_hdl;
    k_adec_chn_attr  attr;
} k_msg_adec_pipe_attr_t;

typedef struct
{
    k_handle adec_hdl;
    k_adec_callback_s callback_attr;
} k_msg_adec_callback_attr;

typedef struct
{
    k_handle ao_hdl;
    k_handle adec_hdl;
} k_msg_adec_bind_ao_t;

typedef struct
{
    k_adec_decoder decoder;
    k_handle decoder_hdl;
} k_msg_ext_audio_decoder_t;

typedef struct
{
    k_audio_stream stream;
    k_handle adec_hdl;
} k_msg_adec_stream_t;

typedef struct
{
    k_handle adec_hdl;
    k_u64 phyAddr;
} k_msg_adec_datafifo_t;

typedef void (*adec_datafifo_release_func)(void* pStream);
typedef struct
{
    k_u32 item_count;//in
    k_u32 item_size;//in
    K_DATAFIFO_OPEN_MODE_E open_mode;//in
    adec_datafifo_release_func release_func;//in

    k_datafifo_handle data_hdl;//out
}k_adec_datafifo;

#define K_ADEC_DATAFIFO_ITEM_COUNT 25
#define K_ADEC_DATAFIFO_ITEM_SIZE  sizeof(k_msg_adec_stream_t)

/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_ADEC_H__ */