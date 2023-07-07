/**
 * @file mpi_dma_api.h
 * @author
 * @brief Defines APIs related to DMA device
 * @version 1.0
 * @date 2022-09-22
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
#ifndef __MPI_DMA_API_H__
#define __MPI_DMA_API_H__

#include "k_type.h"
#include "k_dma_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     DMA */
/** @{ */ /** <!-- [DMA] */

/**
 * @brief Set the device attributes of the DMA
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_set_dev_attr(k_dma_dev_attr_t *attr);

/**
 * @brief Get the device attributes of the DMA
 * @param [in] attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_get_dev_attr(k_dma_dev_attr_t *attr);

/**
 * @brief Start the device of the DMA
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_start_dev();

/**
 * @brief Stop the device of the DMA
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_stop_dev();

/**
 * @brief Set the channel attributes of the DMA
 * @param [in] chn_num channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note After the pipe to which the channel belongs is started, the channel attributes cannot be set
 */
k_s32 kd_mpi_dma_set_chn_attr(k_u8 chn_num, k_dma_chn_attr_u *attr);

/**
 * @brief Get the channel attributes of the DMA
 * @param [in] chn_num channel number
 * @param [in] attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_get_chn_attr(k_u8 chn_num, k_dma_chn_attr_u *attr);

/**
 * @brief Start the channel of the DMA
 * @param [in] chn_num channel num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_start_chn(k_u8 chn_num);

/**
 * @brief Stop the channel of the DMA
 * @param [in] chn_num channel num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_stop_chn(k_u8 chn_num);

/**
 * @brief Send a frame into the channel of the DMA
 * @param [in] chn_num channel number
 * @param [in] df_info Information about the dma frame to be sent
 * @param [in] millisec Blocking time in milliseconds
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_send_frame(k_u8 chn_num, k_video_frame_info *df_info, k_s32 millisec);

/**
 * @brief Get a frame from the channel of the DMA
 * @param [in] chn_num channel number
 * @param [out] df_info Information about the dma frame to be got
 * @param [in] millisec Block time in milliseconds
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_get_frame(k_u8 chn_num, k_video_frame_info *df_info, k_s32 millisec);

/**
 * @brief Release the frame
 * @param [in] chn_num channel number
 * @param [in] df_info Information about the dma frame to be got
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dma_release_frame(k_u8 chn_num, k_video_frame_info *df_info);

/** @} */ /** <!-- ==== DMA End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif