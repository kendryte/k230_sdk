/**
 * @file mpi_ai_api.h
 * @author
 * @brief Defines APIs related to ai input device
 * @version 1.0
 * @date 2022-10-21
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
#ifndef __MPI_AI_API_H__
#define __MPI_AI_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AI */
/** @{ */ /** <!-- [AI] */


#include "k_type.h"
#include "k_ai_comm.h"

/**
 * @brief Set device attributes of the ai
 *
 * @param [in] ai_dev  audio device
 * @param [in] attr device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_set_pub_attr(k_audio_dev ai_dev, const k_aio_dev_attr *attr);

/**
 * @brief Get device attributes of the ai
 *
 * @param [in] ai_dev  audio device
 * @param [out] attr device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_pub_attr(k_audio_dev ai_dev, k_aio_dev_attr *attr);

/**
 * @brief Enable ai device
 *
 * @param [in] ai_dev  audio device
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_enable(k_audio_dev ai_dev);

/**
 * @brief Disable ai device
 *
 * @param [in] ai_dev  audio device
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_disable(k_audio_dev ai_dev);

/**
 * @brief Enable ai device channel
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_enable_chn(k_audio_dev ai_dev, k_ai_chn ai_chn);

/**
 * @brief Disable ai device channel
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_disable_chn(k_audio_dev ai_dev, k_ai_chn ai_chn);

/**
 * @brief Set ai device channel param
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [in] chn_param  channel param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_set_chn_param(k_audio_dev ai_dev, k_ai_chn ai_chn, const k_ai_chn_param *chn_param);

/**
 * @brief Get ai device channel param
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [out] chn_param  channel param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_chn_param(k_audio_dev ai_dev, k_ai_chn ai_chn, k_ai_chn_param *chn_param);

/**
 * @brief Get ai device channel fd
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_fd(k_audio_dev ai_dev, k_ai_chn ai_chn);

/**
 * @brief Get ai device channel frame
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [in] frame  audio frame
 * @param [in] milli_sec  wait time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_frame(k_audio_dev ai_dev, k_ai_chn ai_chn, k_audio_frame *frame, k_u32  milli_sec);

/**
 * @brief Release ai device channel frame
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [in] frame  audio frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_release_frame(k_audio_dev ai_dev, k_ai_chn ai_chn, const k_audio_frame *frame);

/**
 * @brief Release ai device channel vqe
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [in] vqe_enable vqe enable flag
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_set_vqe_attr(k_audio_dev ai_dev, k_ai_chn ai_chn, const k_ai_vqe_enable vqe_enable);

/**
 * @brief Release ai device channel vqe
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [out] vqe_enable Pointer of vqe enable flag
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_vqe_attr(k_audio_dev ai_dev, k_ai_chn ai_chn, k_ai_vqe_enable *vqe_enable);


/**
 * @brief Sends a far echo frame to the specified audio input device and channel.
 *
 * @param ai_dev The audio input device identifier.
 * @param ai_chn The audio input channel identifier.
 * @param frame Pointer to the audio frame to be sent.
 * @param milli_sec Timeout in milliseconds for sending the frame.
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_ai_send_far_echo_frame(k_audio_dev ai_dev, k_ai_chn ai_chn, const k_audio_frame *frame, k_s32 milli_sec);

/**
 * @brief Set pitch shift attributes
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [in] k_ai_chn_pitch_shift_param pitch-shift param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_set_pitch_shift_attr(k_audio_dev ai_dev, k_ai_chn ai_chn, const k_ai_chn_pitch_shift_param *param);

/**
 * @brief Get pitch shift attributes
 *
 * @param [in] ai_dev  audio device
 * @param [in] ai_chn  audio channel
 * @param [out] k_ai_chn_pitch_shift_param pitch-shift param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ai_get_pitch_shift_attr(k_audio_dev ai_dev, k_ai_chn ai_chn, k_ai_chn_pitch_shift_param *param);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

