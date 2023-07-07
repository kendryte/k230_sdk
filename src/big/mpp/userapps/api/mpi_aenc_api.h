/**
 * @file mpi_aenc_api.h
 * @author
 * @brief Defines APIs related to audio encode
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
#ifndef __MPI_AENC_API_H__
#define __MPI_AENC_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AENC */
/** @{ */ /** <!-- [AENC] */


#include "k_type.h"
#include "k_aenc_comm.h"

/**
 * @brief Create aenc channel
 *
 * @param [in] aenc_chn  aenc channel
 * @param [in] attr aenc channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_create_chn(k_aenc_chn aenc_chn, const k_aenc_chn_attr *attr);

/**
 * @brief Destroy aenc channel
 *
 * @param [in] aenc_chn  aenc channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_destroy_chn(k_aenc_chn aenc_chn);

/**
 * @brief Send aenc frame to encode
 *
 * @param [in] aenc_chn  aenc channel
 * @param [in] frame  frame to be encoded
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_send_frame(k_aenc_chn aenc_chn,const k_audio_frame *frame);

/**
 * @brief get encoded stream
 *
 * @param [in] aenc_chn  aenc channel
 * @param [out] stream  encoded data stream
 * @param [in] milli_sec  waiting for timeout time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_get_stream(k_aenc_chn aenc_chn, k_audio_stream *stream, k_s32 milli_sec);

/**
 * @brief release encoded stream
 *
 * @param [in] aenc_chn  aenc channel
 * @param [out] stream  encoded data stream
 * @param [in] milli_sec  waiting for timeout time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_release_stream(k_aenc_chn aenc_chn, const k_audio_stream *stream);

/**
 * @brief register encoder
 *
 * @param [out] handle encoder handle
 * @param [in] encoder aenc encoder info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_register_encoder(k_s32 *handle, const k_aenc_encoder *encoder);

/**
 * @brief unregister encoder
 *
 * @param [in] handle encoder handle
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_aenc_unregister_encoder(k_s32 handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

