/**
 * @file mpi_adec_api.h
 * @author
 * @brief Defines APIs related to audio decode
 * @version 1.0
 * @date 2023-4-6
 *
 * @copyright
 * Copyright (c), Canaan Bright Sight Co., Ltd
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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.HI_MPI_AO_GetFd
 */
#ifndef __MPI_ADEC_API_H__
#define __MPI_ADEC_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ADEC */
/** @{ */ /** <!-- [ADEC] */


#include "k_type.h"
#include "k_adec_comm.h"

/**
 * @brief Create adec channel
 *
 * @param [in] adec_chn  adec channel
 * @param [in] attr adec channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_create_chn(k_adec_chn adec_chn, const k_adec_chn_attr *attr);

/**
 * @brief Destroy adec channel
 *
 * @param [in] adec_chn  adec channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_destroy_chn(k_adec_chn adec_chn);

/**
 * @brief Send adec frame to decode
 *
 * @param [in] adec_chn  adec channel
 * @param [in] stream  stream to be decoded
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_send_stream(k_adec_chn adec_chn,const k_audio_stream *stream,k_bool block);

/**
 * @brief Clear decoding channel cache
 *
 * @param [in] adec_chn  adec channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_clr_chn_buf(k_adec_chn adec_chn);

/**
 * @brief get decoded frame
 *
 * @param [in] adec_chn  adec channel
 * @param [out] frame  decoded data frame
 * @param [in] milli_sec  waiting for timeout time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_get_frame(k_adec_chn adec_chn, k_audio_frame *frame, k_s32 milli_sec);

/**
 * @brief release encoded stream
 *
 * @param [in] adec_chn  adec channel
 * @param [out] frame  decoded data frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_release_frame(k_adec_chn adec_chn, const k_audio_frame *frame);

/**
 * @brief register decoder
 *
 * @param [out] handle decoder handle
 * @param [in] decoder adec decoder info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_register_decoder(k_s32 *handle, const k_adec_decoder *decoder);

/**
 * @brief unregister decoder
 *
 * @param [in] handle decoder handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_adec_unregister_decoder(k_s32 handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

