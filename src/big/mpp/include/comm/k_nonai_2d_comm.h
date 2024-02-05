/**
 * @file k_nonai_2d_comm.h
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
#ifndef __K_NONAI_2D_COMM_H__
#define __K_NONAI_2D_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include "k_video_comm.h"
#include "k_payload_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     NONAI_2D */
/** @{ */ /** <!-- [NONAI_2D] */

#define K_NONAI_2D_MAX_DEV_NUMS        (1)
#define K_NONAI_2D_MAX_CHN_NUMS        (24)

/**
 * @brief Defines the attributes of a NONAI_2D channel
 *
 */

typedef enum
{
    K_NONAI_2D_CALC_MODE_CSC = 0,       /* Color space conversion */
    K_NONAI_2D_CALC_MODE_OSD,           /* On Screen Display */
    K_NONAI_2D_CALC_MODE_BORDER,        /* Draw border */
    K_NONAI_2D_CALC_MODE_OSD_BORDER,    /* OSD first, then draw border */
    K_NONAI_2D_CALC_MODE_BUTT
} k_nonai_2d_calc_mode;

typedef enum
{
    NONAI_2D_COLOR_GAMUT_BT601 = 0,
    NONAI_2D_COLOR_GAMUT_BT709,
    NONAI_2D_COLOR_GAMUT_BT2020,
    NONAI_2D_COLOR_GAMUT_BUTT
} k_nonai_2d_color_gamut;

typedef struct
{
    k_pixel_format dst_fmt;         /* Format of output image */
    k_nonai_2d_calc_mode mode;
} k_nonai_2d_chn_attr;

typedef struct
{
    k_s16 coef[12];                /* Pointer of coefficent */
} k_nonai_2d_coef_attr;

#define K_ERR_NONAI_2D_INVALID_DEVID     K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_NONAI_2D_INVALID_CHNID     K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_NONAI_2D_ILLEGAL_PARAM     K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_NONAI_2D_EXIST             K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_NONAI_2D_UNEXIST           K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_NONAI_2D_NULL_PTR          K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_NONAI_2D_NOT_CONFIG        K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_NONAI_2D_NOT_SUPPORT       K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_NONAI_2D_NOT_PERM          K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_NONAI_2D_NOMEM             K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_NONAI_2D_NOBUF             K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_NONAI_2D_BUF_EMPTY         K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_NONAI_2D_BUF_FULL          K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_NONAI_2D_NOTREADY          K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_NONAI_2D_BADADDR           K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_NONAI_2D_BUSY              K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_ERR_NONAI_2D_NOT_PERM          K_DEF_ERR(K_ID_NONAI_2D, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)


/** @} */ /** <!-- ==== NONAI_2D End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
