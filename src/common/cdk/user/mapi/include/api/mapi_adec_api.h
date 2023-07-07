/**
 * @file mapi_adec_api.h
 * @author  ()
 * @brief mapi of audio decode module
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
#ifndef __MAPI_ADEC_H__
#define __MAPI_ADEC_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_adec_api.h"
#include "k_adec_comm.h"
#include "mapi_adec_comm.h"

#define K_MAPI_ERR_ADEC_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_ADEC, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_MAPI_ERR_ADEC_INVALID_HANDLE   K_MAPI_DEF_ERR(K_MAPI_MOD_ADEC, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_ADEC*/
/** @{ */  /** <!-- [ MAPI_ADEC] */

/**
 * @brief Init adec device.
 *
 * @param [in] adec_hdl adec handle
 * @param [in] attr adec channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_init(k_handle adec_hdl,const k_adec_chn_attr *attr);

/**
 * @brief deinit adec device.
 *
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_deinit(k_handle adec_hdl);

/**
 * @brief adec start
 *
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_start(k_handle adec_hdl);

/**
 * @brief  adec stop
 *
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_stop(k_handle adec_hdl);

/**
 * @brief register adec callback
 *
 * @param [in] adec_hdl adec handle
 * @param [in] adec_cb adec data callback
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_registercallback(k_handle adec_hdl,k_adec_callback_s *adec_cb);

/**
 * @brief unregister adec callback
 *
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_unregistercallback(k_handle adec_hdl);

/**
 * @brief adec bind ao
 *
 * @param [in] ao_hdl ao handle
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_bind_ao(k_handle ao_hdl,k_handle adec_hdl);

/**
 * @brief adec unbind ao
 *
 * @param [in] ao_hdl ao handle
 * @param [in] adec_hdl adec handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_unbind_ao(k_handle ao_hdl,k_handle adec_hdl);

/**
 * @brief adec register ext decoder
 *
 * @param [in] decoder_hdl ext decoder handle
 * @param [in] decoder extern audio decoder
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_register_ext_audio_decoder(const k_adec_decoder *decoder,k_handle* decoder_hdl);

/**
 * @brief adec unregister ext decoder
 *
 * @param [in] decoder_hdl ext decoder handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_unregister_ext_audio_decoder( k_handle decoder_hdl);

/**
 * @brief decode stream
 *
 * @param [in] adec_hdl adec handle
 * @param [in] stream decode stream
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_adec_send_stream(k_handle adec_hdl,const k_audio_stream *stream);

/** @} */ /** <!-- ==== MAPI_ADEC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_ADEC_API_H__ */