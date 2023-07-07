/**
 * @file mapi_vvi_api.h
 * @author  ()
 * @brief mapi of virtual video inputdeo input module
 * @version 1.0
 * @date 2022-10-18
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
#ifndef __MAPI_VVI_H__
#define __MAPI_VVI_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_vdss_api.h"
#include "k_vdss_comm.h"


#define K_MAPI_ERR_VDSS_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VDSS_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VDSS_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VDSS_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VDSS_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VDSS_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_MAPI_ERR_VDSS_INVALID_DEVID    K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_MAPI_ERR_VDSS_INVALID_CHNID    K_MAPI_DEF_ERR(K_MAPI_MOD_VI, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VVI*/
/** @{ */  /** <!-- [ MAPI_VVI] */


k_s32 kd_mapi_vdss_chn_release_frame(k_u32 dev_num, k_u32 chn_num, const k_video_frame_info *vf_info);
k_s32 kd_mapi_vdss_dump_frame(k_u32 dev_num, k_u32 chn_num, k_video_frame_info *vf_info, k_u32 timeout_ms);
k_s32 kd_mapi_vdss_set_dev_attr(k_vicap_dev_attr *attr);
k_s32 kd_mapi_vdss_set_chn_attr(k_u32 dev_num, k_u32 chn_num, k_vicap_chn_attr *attr);
k_s32 kd_mapi_vdss_stop_pipe(k_u32 dev_num, k_u32 chn_num);
k_s32 kd_mapi_vdss_start_pipe(k_u32 dev_num, k_u32 chn_num);
k_s32 kd_mapi_vdss_rst(k_u8 val);

/** @} */ /** <!-- ==== MAPI_VVI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VVI_API_H__ */

