/**
 * @file mapi_ao_api.h
 * @author  ()
 * @brief mapi of audio output module
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
#ifndef __MAPI_AO_H__
#define __MAPI_AO_H__

#include "k_type.h"
#include "k_mapi_errno.h"
#include "k_mapi_module.h"
#include "mpi_ao_api.h"
#include "k_ao_comm.h"

#define K_MAPI_ERR_AO_NULL_PTR         K_MAPI_DEF_ERR(K_MAPI_MOD_AO, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** \addtogroup     MAPI_AO*/
/** @{ */  /** <!-- [ MAPI_AO] */

/**
 * @brief Init ao device.
 *
 * @param [in] dev AO device id
 * @param [in] chn AO channel id
 * @param [in] dev_attr AO dev attribute
 * @param [out] ao_hdl AO handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note device number of 0 indicates i2s,other values are invalid.
 *       I2s has a total of 2 channels.
 */
k_s32 kd_mapi_ao_init(k_u32 dev, k_u32 chn, const k_aio_dev_attr *dev_attr,k_handle* ao_hdl);

/**
 * @brief Deinit ai device.
 *
 * @param [out] ao_hdl AO handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 * @note device number of 0 indicates i2s,other values are invalid.
 *       I2s has a total of 2 channels.
 */
k_s32 kd_mapi_ao_deinit(k_handle ao_hdl);

/**
 * @brief start ao device.
 *
 * @param [out] ao_hdl AO handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ao_start(k_handle ao_hdl);

/**
 * @brief stop ao device.
 *
 * @param [out] ao_hdl AO handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ao_stop(k_handle ao_hdl);

/**
 * @brief send ao frame.
 *
 * @param [out] ao_hdl AO handle
 * @param [in] frame  audio frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ao_send_frame(k_handle ao_hdl, const k_audio_frame *frame);

/**
 * @brief ao set volume
 *
 * @param [in] ao_hdl ao handle
 * @param [in] volume ao set volume value,[-39,6],step 1.5
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see error code
 */
k_s32 kd_mapi_ao_set_volume(k_handle ao_hdl,float volume);

/** @} */ /** <!-- ==== MAPI_AI End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __K_MAPI_AO_API_H__ */