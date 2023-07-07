/**
 * @file mapi_venc_api.h
 * @author  ()
 * @brief mapi of venc module
 * @version 1.0
 * @date 2023-03-22
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
#ifndef __MAPI_VENC_H__
#define __MAPI_VENC_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_venc_api.h"
#include "k_venc_comm.h"
#include "mapi_venc_comm.h"

#define K_MAPI_ERR_VENC_ILLEGAL_PARAM    K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_MAPI_ERR_VENC_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_VENC_NOT_PERM         K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_MAPI_ERR_VENC_NOTREADY         K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_MAPI_ERR_VENC_BUSY             K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_BUSY)
#define K_MAPI_ERR_VENC_NOMEM            K_MAPI_DEF_ERR(K_MAPI_MOD_VENC, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_VENC*/
/** @{ */  /** <!-- [ MAPI_VENC] */

/**
 * @brief Create encode channel
 *
 * @param [in] chn_num  channel number
 * @param [in] pst_venc_attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note MJPEG rc attributes are different from h264 and h265
 *
 */

k_s32 kd_mapi_venc_init(k_u32 chn_num, k_venc_chn_attr *pst_venc_attr);

/**
 * @brief Destory channel
 *
 * @param [in] chn_num channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create/stop channel
 */
k_s32 kd_mapi_venc_deinit(k_u32 chn_num);


/**
 * @brief register  callback
 *
 * @param [in] chn_num  channel number
 * @param [in] pst_venc_cb  get video enc data from  kd_venc_callback_s
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after venc init
 */
k_s32 kd_mapi_venc_registercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb);


/**
 * @brief unregister  callback
 *
 * @param [in] chn_num  channel number
 * @param [in] pst_venc_cb
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after registercallback
 */
k_s32 kd_mapi_venc_unregistercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb);

/**
 * @brief Start channel
 *
 * @param [in] chn_num channel number
 * @param [in] s32_frame_cnt  get frame num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after create channel
 */
k_s32 kd_mapi_venc_start(k_s32 chn_num, k_s32 s32_frame_cnt);

/**
 * @brief Stop channel
 *
 * @param [in] chn_num  channel number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after start channel
 */
k_s32 kd_mapi_venc_stop(k_s32 chn_num);


/**
 * @brief venc bind src
 *
 * @param [in] src_dev  vi dev channel
 * @param [in] src_chn  vi chn channel
 * @param [in] chn_num  enc channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after init vi model
 */
k_s32 kd_mapi_venc_bind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num);

/**
 * @brief venc unbind src
 *
 * @param [in] src_dev  vi dev channel
 * @param [in] src_chn  vi chn channel
 * @param [in] chn_num  enc channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after venc_bind_vi
 */
k_s32 kd_mapi_venc_unbind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num);


/**
 * @brief Venc enable IDR frame, generate IDR frame according to GOP interval
 *
 * @param [in] chn_num  enc channel
 * @param [in] idr_enable  1:enable 0:not enable
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after creat channel, before start channel
 */
k_s32 kd_mapi_venc_enable_idr(k_s32 chn_num, k_bool idr_enable);

/**
 * @brief Venc request IDR frame, insert IDR frame immediately
 *
 * @param [in] chn_num  enc channel
 *
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note It should be called after creat channel, before start channel
 */
k_s32 kd_mapi_venc_request_idr(k_s32 chn_num);


/** @} */ /** <!-- ==== MAPI_VENC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_VENC_API_H__ */

