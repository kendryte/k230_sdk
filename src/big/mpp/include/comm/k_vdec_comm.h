/**
 * @file k_vdec_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-01
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
#ifndef __K_VDEC_COMM_H__
#define __K_VDEC_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_payload_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VDEC */
/** @{ */ /** <!-- [VDEC] */

#define VDEC_MAX_DEV_NUMS        (1)
#define VDEC_MAX_CHN_NUMS        (4)

/**
 * @brief Defines the attributes of a VDEC channel
 *
 */

typedef enum
{
    K_VDEC_SEND_MODE_STREAM = 0,  /*streaming send mode*/
    K_VDEC_SEND_MODE_FRAME,       /*frame send mode*/
    K_VDEC_SEND_MODE_BUTT
} k_vdec_send_mode;

typedef struct
{
    k_payload_type type;        /*stream payload type*/
    k_vdec_send_mode mode;      /*stream sending mode*/
    k_u32 pic_width;            /*max decoded picture width*/
    k_u32 pic_height;           /*max decoded picture height*/
    k_u32 stream_buf_size;      /*single input vb size*/
    k_u32 frame_buf_size;       /*single output vb size*/
    k_u32 frame_buf_cnt;        /*output vb count*/
    k_u32 frame_buf_pool_id;
} k_vdec_chn_attr;

typedef struct
{
    k_payload_type type;            /*stream payload type*/
    k_bool is_valid_frame;          /*it is valid frame or not*/
    k_bool end_of_stream;           /*eos flag for this vb*/
} k_vdec_supplement_info;

typedef struct
{
    k_bool end_of_stream;     /*eos flag for this vb*/
    k_u64 pts;                /*pts for this vb*/
    k_u32 len;                /*length for this vb*/
    k_u64 phy_addr;           /*physics address for this vb*/
} k_vdec_stream;

typedef struct
{
    k_s32 set_pic_buf_size_err;  /*output buffer size error*/
    k_s32 format_err;            /*pixel format error*/
    k_s32 stream_unsupport;      /*unsupported input stream*/
} k_vdec_dec_err;

typedef struct
{
    k_payload_type type;         /*stream payload type*/
    k_bool is_started;           /*channel is started or not*/
    k_u32 recv_stream_frames;    /*reveived frames*/
    k_u32 dec_stream_frames;     /*decoded frames*/
    k_vdec_dec_err dec_err;      /*decode error*/
    k_u32 width;                 /*decoded picture width*/
    k_u32 height;                /*decoded picture height*/
    k_u64 latest_frame_pts;      /*latest pts for decoded frame*/
    k_bool end_of_stream;        /*eos for this channel */
} k_vdec_chn_status;

typedef enum
{
    K_VDEC_DSL_MODE_BY_SIZE,
    K_VDEC_DSL_MODE_BY_RATIO,
    K_VDEC_DSL_MODE_BUTT
} k_vdec_dsl_mode;

typedef struct
{
    k_u32 dsl_frame_width;  /*down scale width*/
    k_u32 dsl_frame_height; /*down scale height*/
} k_vdec_dsl_size;

typedef struct
{
    k_u8 dsl_ratio_hor;
    k_u8 dsl_ratio_ver;
} k_vdec_dsl_ratio;

typedef struct
{
    k_vdec_dsl_mode dsl_mode;
    union
    {
        k_vdec_dsl_size dsl_size;
        k_vdec_dsl_ratio dsl_ratio;
    };
} k_vdec_downscale;


#define K_ERR_VDEC_INVALID_DEVID     K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_VDEC_INVALID_CHNID     K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_VDEC_ILLEGAL_PARAM     K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VDEC_EXIST             K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_VDEC_UNEXIST           K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VDEC_NULL_PTR          K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VDEC_NOT_CONFIG        K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_VDEC_NOT_SUPPORT       K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_VDEC_NOT_PERM          K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VDEC_NOMEM             K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VDEC_NOBUF             K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VDEC_BUF_EMPTY         K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_VDEC_BUF_FULL          K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_VDEC_NOTREADY          K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VDEC_BADADDR           K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_VDEC_BUSY              K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_ERR_VDEC_NOT_PERM          K_DEF_ERR(K_ID_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)


/** @} */ /** <!-- ==== VDEC End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

