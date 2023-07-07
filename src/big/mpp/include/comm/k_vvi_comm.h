/**
 * @file k_vvi_comm.h
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
#ifndef __K_VVI_COMM_H__
#define __K_VVI_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VVI */
/** @{ */ /** <!-- [VVI] */

#define VVI_MAX_DEV_NUMS        (2)
#define VVI_MAX_CHN_NUMS        (2)
#define VVI_MAX_FRAME_COUNT     (3)

/**
 * @brief Defines the attributes of a VVI channel
 *
 */
typedef struct
{
    k_u32 height;
    k_u32 width;
    k_pixel_format format;  /**< Currently only ARGB is supported*/
    k_u32 frame_rate;   /**< Cannot be 0*/
    k_bool is_pre_fill_color;   /*Whether to prepare the fill color in advance*/
} k_vvi_chn_attr;

/**
 * @brief Defines the attributes of a VVI device
 *
 * @note This configuration does not actually take effect
 */
typedef struct
{
    k_u32 height;
    k_u32 width;
    k_pixel_format format;
} k_vvi_dev_attr;

#define K_ERR_VVI_INVALID_DEVID     K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_VVI_INVALID_CHNID     K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_VVI_ILLEGAL_PARAM     K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_VVI_EXIST             K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_VVI_UNEXIST           K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_VVI_NULL_PTR          K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_VVI_NOT_CONFIG        K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_VVI_NOT_SUPPORT       K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_VVI_NOT_PERM          K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_VVI_NOMEM             K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_VVI_NOBUF             K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_VVI_BUF_EMPTY         K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_VVI_BUF_FULL          K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_VVI_NOTREADY          K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_VVI_BADADDR           K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_VVI_BUSY              K_DEF_ERR(K_ID_V_VI, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== VVI End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
