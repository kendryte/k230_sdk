/**
 * @file mapi_vdec_api.h
 * @author  ()
 * @brief mapi of video decode module
 * @version 1.0
 * @date 2023-06-19
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
#ifndef __MAPI_VDEC_H__
#define __MAPI_VDEC_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_vdec_api.h"
#include "k_vdec_comm.h"
#include "mapi_vdec_comm.h"

#define K_MAPI_ERR_VDEC_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VDEC_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VDEC_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VDEC_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VDEC_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VDEC_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_MAPI_ERR_VDEC_INVALID_HANDLE   K_MAPI_DEF_ERR(K_MAPI_MOD_VDEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VDEC*/
/** @{ */  /** <!-- [ MAPI_VDEC] */

/**
 * @brief Create video decode channel
 *
 * @param [in] chn_num  channel number
 * @param [in] attr vdec channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_init(k_u32 chn_num,const k_vdec_chn_attr *attr);

/**
 * @brief Destory video decode channel
 *
 * @param [in] chn_num  channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_deinit(k_u32 chn_num);

/**
 * @brief Start video decode channel
 *
 * @param [in] chn_num  channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_start(k_u32 chn_num);

/**
 * @brief  Stop video decode channel
 *
 * @param [in] chn_num  channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_stop(k_u32 chn_num);

/**
 * @brief Vdec bind vo
 *
 * @param [in] chn_num vdec channel
 * @param [in] vo_dev vo device
 * @param [in] vo_chn vo channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_bind_vo(k_u32 chn_num,k_u32 vo_dev, k_u32 vo_chn);

/**
 * @brief Vdec unbind vo
 *
 * @param [in] chn_num vdec channel
 * @param [in] vo_dev vo device
 * @param [in] vo_chn vo channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_unbind_vo(k_u32 chn_num,k_u32 vo_dev, k_u32 vo_chn);

/**
 * @brief Decode stream
 *
 * @param [in] chn_num Channel number
 * @param [in] stream Input stream
 * @param [in] milli_sec wait time, -1 means wait forever
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_vdec_send_stream(k_u32 chn_num, k_vdec_stream *stream, k_s32 milli_sec);

/**
 * @brief Query decode status
 *
 * @param [in] chn_num Channel number
 * @param [in] status Decode channel status
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mapi_vdec_query_status(k_u32 chn_num, k_vdec_chn_status *status);

/** @} */ /** <!-- ==== MAPI_VDEC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VDEC_API_H__ */