/**
 * @file mapi_aenc_api.h
 * @author  ()
 * @brief mapi of audio encode module
 * @version 1.0
 * @date 2023-05-11
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
#ifndef __MAPI_AENC_H__
#define __MAPI_AENC_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_aenc_api.h"
#include "k_aenc_comm.h"
#include "mapi_aenc_comm.h"


#define K_MAPI_ERR_AENC_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_AENC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_AENC_INVALID_HANDLE   K_MAPI_DEF_ERR(K_MAPI_MOD_AENC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_AENC*/
/** @{ */  /** <!-- [ MAPI_AENC] */

/**
 * @brief Init aenc device.
 *
 * @param [in] aenc_hdl aenc handle
 * @param [in] attr aenc channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_init(k_handle aenc_hdl,const k_aenc_chn_attr *attr);

/**
 * @brief deinit aenc device.
 *
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_deinit(k_handle aenc_hdl);

/**
 * @brief  aenc start
 *
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_start(k_handle aenc_hdl);

/**
 * @brief  aenc stop
 *
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_stop(k_handle aenc_hdl);

/**
 * @brief register aenc callback
 *
 * @param [in] aenc_hdl aenc handle
 * @param [in] aenc_cb aenc data callback
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_registercallback(k_handle aenc_hdl,k_aenc_callback_s *aenc_cb);

/**
 * @brief unregister aenc callback
 *
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_unregistercallback(k_handle aenc_hdl);

/**
 * @brief aenc bind ai
 *
 * @param [in] ai_hdl ai handle
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_bind_ai(k_handle ai_hdl,k_handle aenc_hdl);

/**
 * @brief aenc unbind ai
 *
 * @param [in] ai_hdl ai handle
 * @param [in] aenc_hdl aenc handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_unbind_ai(k_handle ai_hdl,k_handle aenc_hdl);

/**
 * @brief aenc register ext encoder
 *
 * @param [in] encoder_hdl ext encoder handle
 * @param [in] encoder extern audio encoder
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_register_ext_audio_encoder(const k_aenc_encoder *encoder,k_handle* encoder_hdl);


/**
 * @brief aenc unregister ext encoder
 *
 * @param [in] encoder_hdl ext encoder handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_unregister_ext_audio_encoder( k_handle encoder_hdl);

/**
 * @brief encode frame
 *
 * @param [in] aenc_hdl aenc handle
 * @param [in] frame encode frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_aenc_send_frame(k_handle aenc_hdl,const k_audio_frame *frame);


/** @} */ /** <!-- ==== MAPI_AI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_AENC_API_H__ */