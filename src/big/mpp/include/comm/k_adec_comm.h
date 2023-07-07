/**
* @file k_adec_comm.h
* @author
* @version 1.0
* @date 2023-4-3
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
#ifndef __K_ADEC_COMM_H__
#define __K_ADEC_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_audio_comm.h"
#include "k_payload_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef k_u32 k_adec_chn;
#define K_MAX_DECODER_NAME_LEN 25
#define ADEC_MAX_DEV_NUMS          (1)
#define ADEC_MAX_CHN_NUMS          (8)
#define ADEC_MAX_COUNT             (20)

typedef struct {
 k_payload_type type;
 k_char name[K_MAX_DECODER_NAME_LEN];
 k_s32 (*func_open_decoder)(void *decoder_attr, void **decoder);
 k_s32 (*func_dec_stream)(void *decoder,const k_audio_stream *data,k_u8 *outbuf, k_u32 *out_len);
 k_s32 (*func_get_frame_info)(void *decoder, void *info);
 k_s32 (*func_close_decoder)(void *decoder);
 k_s32 (*func_reset_decoder)(void *decoder);
} k_adec_decoder;

typedef enum
{
    K_ADEC_MODE_PACK = 0, /* require input is valid dec pack(a
                             complete frame encode result),
                             e.g.the stream get from AENC is a
                             valid dec pack, the stream know actually
                             pack len from fiÃ is also a dec pack.
                             this mode is high-performative */
    K_ADEC_MODE_STREAM = 1,  /* input is stream, low-performative,
                             if you couldn't fin out whether a stream is
                             vaild dec pack,you could use this mode */
} k_adec_mode;

typedef struct {
 k_payload_type type;
 k_adec_mode mode;
 k_u32 point_num_per_frame;//must be the same as the ao attribute
 k_u32 buf_size; /* buf size[2~K_MAX_AUDIO_FRAME_NUM] */
} k_adec_chn_attr;

#define K_ERR_ADEC_NOTREADY          K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_ADEC_INVALID_DEVID     K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_ADEC_INVALID_CHNID     K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_ADEC_ILLEGAL_PARAM     K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_ADEC_EXIST             K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_ADEC_UNEXIST           K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_ADEC_NULL_PTR          K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_ADEC_NOT_CONFIG        K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_ADEC_NOT_SUPPORT       K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_ADEC_NOT_PERM          K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_ADEC_NOMEM             K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_ADEC_NOBUF             K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_ADEC_BUF_EMPTY         K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_ADEC_BUF_FULL          K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_ADEC_NOTREADY          K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_ADEC_BADADDR           K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_ADEC_BUSY              K_DEF_ERR(K_ID_ADEC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
