/**
* @file k_aenc_comm.h
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
#ifndef __K_AENC_COMM_H__
#define __K_AENC_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_audio_comm.h"
#include "k_payload_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef k_u32 k_aenc_chn;
#define AENC_MAX_DEV_NUMS          (1)
#define AENC_MAX_CHN_NUMS          (8)
#define AENC_MAX_COUNT             (20)
#define K_MAX_ENCODER_NAME_LEN 25

typedef struct {
    k_payload_type type;
    k_u32 point_num_per_frame;
    k_u32 buf_size; /* buf size[2~K_MAX_AUDIO_FRAME_NUM] */
 //void ATTRIBUTE *value;
} k_aenc_chn_attr;

 typedef struct {
    k_payload_type type;
    k_u32 max_frame_len;
    k_char name[K_MAX_ENCODER_NAME_LEN];
    k_s32 (*func_open_encoder)(void *encoder_attr,void **encoder);
    k_s32 (*func_enc_frame)(void *encoder,const k_audio_frame *data,k_u8 *outbuf, k_u32 *out_len);
    k_s32 (*func_close_encoder)(void *encoder);
} k_aenc_encoder;

#define K_ERR_AENC_NOTREADY          K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_AENC_INVALID_DEVID     K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_AENC_INVALID_CHNID     K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_AENC_ILLEGAL_PARAM     K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_AENC_EXIST             K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_AENC_UNEXIST           K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_AENC_NULL_PTR          K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_AENC_NOT_CONFIG        K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_AENC_NOT_SUPPORT       K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_AENC_NOT_PERM          K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_AENC_NOMEM             K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_AENC_NOBUF             K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_AENC_BUF_EMPTY         K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_AENC_BUF_FULL          K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_AENC_NOTREADY          K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_AENC_BADADDR           K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_AENC_BUSY              K_DEF_ERR(K_ID_AENC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
