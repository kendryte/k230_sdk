/**
 * @file mapi_ai_api.h
 * @author  ()
 * @brief mapi of audio input module
 * @version 1.0
 * @date 2023-03-24
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
#ifndef __MAPI_AI_H__
#define __MAPI_AI_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_ai_api.h"
#include "k_ai_comm.h"

#define K_MAPI_ERR_AI_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_AI, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_AI*/
/** @{ */  /** <!-- [ MAPI_AI] */

/**
 * @brief Init ai device.
 *
 * @param [in] dev AI device id
 * @param [in] chn AI channel id
 * @param [in] dev_attr AI dev attribute
 * @param [out] ai_hdl  ai handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note device number of 0 indicates i2s, and a device number of 1 indicates pdm;
 *       I2s has a total of 2 channels, and pdm has a total of 4 channels.
 */
k_s32 kd_mapi_ai_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ai_hdl);

/**
 * @brief Deinit ai device.
 *
 * @param [in] ai_hdl AI handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note device number of 0 indicates i2s, and a device number of 1 indicates pdm;
 *       I2s has a total of 2 channels, and pdm has a total of 4 channels.
 */
k_s32 kd_mapi_ai_deinit(k_handle ai_hdl);

/**
 * @brief start ai device.
 *
 * @param [in] ai_hdl AI handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ai_start(k_handle ai_hdl);

/**
 * @brief stop ai device.
 *
 * @param [in] ai_hdl AI handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ai_stop(k_handle ai_hdl);

/**
 * @brief get ai frame.
 *
 * @param [in] ai_hdl AI handle
 * @param [in] frame  audio frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ai_get_frame(k_handle ai_hdl, k_audio_frame *frame);

/**
 * @brief release ai frame.
 *
 * @param [in] ai_hdl AI handle
 * @param [in] frame  audio frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ai_release_frame(k_handle ai_hdl, k_audio_frame *frame);


/** @} */ /** <!-- ==== MAPI_AI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_AI_API_H__ */