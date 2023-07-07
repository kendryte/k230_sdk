/**
 * @file mpi_ao_api.h
 * @author
 * @brief Defines APIs related to ao output device
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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MPI_AO_API_H__
#define __MPI_AO_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AO */
/** @{ */ /** <!-- [AO] */


#include "k_type.h"
#include "k_ao_comm.h"


/**
 * @brief Set device attributes of the AO
 *
 * @param [in] ao_dev  audio device
 * @param [in] attr device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_set_pub_attr(k_audio_dev ao_dev, const k_aio_dev_attr *attr);

/**
 * @brief Get device attributes of the AO
 *
 * @param [in] ao_dev  audio device
 * @param [out] attr device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_get_pub_attr(k_audio_dev ao_dev, k_aio_dev_attr *attr);

/**
 * @brief Enable ao device
 *
 * @param [in] ao_dev  audio device
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_enable(k_audio_dev ao_dev);

/**
 * @brief Disable ao device
 *
 * @param [in] ao_dev  audio device
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_disable(k_audio_dev ao_dev);

/**
 * @brief Enable ao device channel
 *
 * @param [in] ao_dev  audio device
 * @param [in] ao_chn  audio channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_enable_chn(k_audio_dev ao_dev, k_ao_chn ao_chn);

/**
 * @brief Disable ao device channel
 *
 * @param [in] ao_dev  audio device
 * @param [in] ao_chn  audio channel
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_disable_chn(k_audio_dev ao_dev, k_ao_chn ao_chn);

/**
 * @brief Send ao channel frame
 *
 * @param [in] ao_dev  audio device
 * @param [in] ao_chn  audio channel
 * @param [in] frame  audio frame
 * @param [in] milli_sec  wait time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 *
 */
k_s32 kd_mpi_ao_send_frame(k_audio_dev ao_dev, k_ao_chn ao_chn, const k_audio_frame *frame, k_s32 milli_sec);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

