/**
 * @file mapi_isp_api.h
 * @author  ()
 * @brief mapi of virtual video inputdeo input module
 * @version 1.0
 * @date 2023-04-21
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
#ifndef __MAPI_ISP_OPT_H__
#define __MAPI_ISP_OPT_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_isp_api.h"
#include "k_isp_comm.h"

#define K_MAPI_ERR_ISP_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_ISP_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_ISP_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_ISP_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_ISP_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_ISP_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_MAPI_ERR_ISP_INVALID_DEVID    K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_MAPI_ERR_ISP_INVALID_CHNID    K_MAPI_DEF_ERR(K_MAPI_MOD_ISP, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_ISP_OPT*/
/** @{ */  /** <!-- [ MAPI_ISP_OPT] */

/**
 * @brief Get isp ae roi
 *
 * @param [in] dev vicap device id
 * @param [out] ae_roi Roi info
 * - -1 indicates blocking mode.
 * - 0 indicates non-blocking mode.
 * - Greater than 0 indicates timeout mode, and the timeout time is measured in milliseconds(ms).
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Before calling ::kd_mapi_isp_ae_get_roi, calling isp submodule ctrl before use this function
 */
k_s32 kd_mapi_isp_ae_get_roi(k_vicap_dev dev_num, k_isp_ae_roi *ae_roi);

/**
 * @brief Set isp ae roi
 *
 * @param [in] dev vicap device id
 * @param [in] ae_roi user roi
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note
 * - If the channel id or device id does not belong to a started pipe, the mpi returns failure
 * - Before calling ::kd_mapi_isp_ae_get_roi, calling isp submodule ctrl before use this function
 */
k_s32 kd_mapi_isp_ae_set_roi(k_vicap_dev dev_num, k_isp_ae_roi ae_roi);

/** @} */ /** <!-- ==== MAPI_ISP_OPT End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_ISP_OPT_API_H__ */
