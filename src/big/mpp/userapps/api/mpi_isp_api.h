/**
 * @file mpi_isp_api.h
 * @author
 * @brief Defines APIs related to isp deivce
 * @version 1.0
 * @date 2023-03-20
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
#ifndef __MPI_ISP_API_H__
#define __MPI_ISP_API_H__

#include "k_type.h"
#include "k_isp_comm.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ISP */
/** @{ */ /** <!-- [ISP] */


/**
 * @brief Set the device attributes of the ISP
 *
 * @param [in] dev_num  isp device number
 * @param [in] dev_attr  isp device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_isp_set_dev_attr(k_isp_dev dev_num, k_isp_dev_attr dev_attr);

/**
 * @brief Set the channel attributes of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @param [in] chn_num  channel number
 * @param [in] chn_attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 *
 */
k_s32 kd_mpi_isp_set_chn_attr(k_isp_dev dev_num, k_isp_chn chn_num, k_isp_chn_attr chn_attr);

/**
 * @brief Init ISP
 *
 * @param [in] dev_num  isp device num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Before call the API, need to set the device & channel attribute.
 *
 */
k_s32 kd_mpi_isp_init(k_isp_dev dev_num);

/**
 * @brief Deinit ISP
 *
 * @param [in] dev_num  isp device num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_deinit(k_isp_dev dev_num);

/**
 * @brief Connect of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_connect(k_isp_dev dev_num);

/**
 * @brief Disonnect of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_disconnect(k_isp_dev dev_num);

/**
 * @brief Register the 3a lib of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @param [in] usr_awb_lib user awb lib, if NULL, use the default awb lib
 * @param [in] usr_ae_lib user ae lib, if NULL, use the default ae lib
 * @param [in] usr_af_lib user af lib, if NULL, use the default af lib
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_register_3alib(k_isp_dev dev_num, void *usr_awb_lib, void *usr_ae_lib, void *usr_af_lib);

/**
 * @brief Unregister the 3a lib of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_unregister_3alib(k_isp_dev dev_num);

/**
 * @brief Start the stream of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_start_stream(k_isp_dev dev_num);

/**
 * @brief Stop the stream of the the ISP
 *
 * @param [in] dev_num  isp device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_stop_stream(k_isp_dev dev_num);

/**
 * @brief dump frame from the ISP
 *
 * @param [in] dev_num device number
 * @param [in] chn_num channel number
 * @param [in] foramt  dump data format
 * @param [out] vf_info Video frame information obtained
 * @param [in] milli_sec  timeout value
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_isp_dump_frame(k_isp_dev dev_num, k_isp_chn chn_num, k_vicap_dump_format foramt,
                              k_video_frame_info *vf_info, k_u32 milli_sec);

/**
 * @brief Release dumped video frame
 *
 * @param [in] dev_num device number
 * @param [in] chn_num channel number
 * @param [in] vf_info Video frame information obtained by ::kd_mpi_isp_dump_frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_isp_dump_release(k_isp_dev dev_num, k_isp_chn chn_num, const k_video_frame_info *vf_info);

k_s32 kd_mpi_submodule_control(k_isp_dev dev_num);

k_s32 kd_mpi_submodule_control_h(k_isp_dev dev_num, const void *auto_data, const void *manual_data);

/**
 * @brief set isp ae roi
 *
 * @param [in] dev_num device number
 * @param [in] user_roi roi param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Max support 8 windows
 */
k_s32 kd_mpi_isp_ae_set_roi(k_isp_dev dev_num, k_isp_ae_roi user_roi);

/**
 * @brief get isp ae roi
 *
 * @param [in] dev_num device number
 * @param [out] user_roi roi param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Max support 8 windows
 */
k_s32 kd_mpi_isp_ae_get_roi(k_isp_dev dev_num, k_isp_ae_roi *user_roi);

/**
 * @brief set isp ae roi enable 1/disable 0
 *
 * @param [in] dev_num device number
 * @param [in] enable enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Max support 8 windows
 */
k_s32 kd_mpi_isp_ae_roi_set_enable(k_isp_dev dev_num, k_bool enable);

/**
 * @brief set isp ae roi with chn when image crop or scale
 *
 * @param [in] dev_num device number
 * @param [in] chn_num channel number
 * @param [in] user_roi roi param
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Max support 8 windows
 */
k_s32 kd_mpi_isp_ae_set_roi_by_chn(k_isp_dev dev_num, k_isp_chn chn_num, k_isp_ae_roi user_roi);

/**
 * @brief isp load user image data
 *
 * @param [in] dev_num device number
 * @param [in] image_data
 * @param [in] data_len
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_isp_load_image(k_isp_dev dev_num, const void *image_data, k_u32 data_len);

k_s32 kd_mpi_isp_dump_register(k_isp_dev dev_num, FILE* file);

int kd_mpi_isp_tuning(const char* command, k_u32 command_size, char** response, k_u32* response_size);

k_s32 kd_mpi_isp_get_chn_buf_size(k_isp_dev dev_num, k_isp_chn chn_num);

/** @} */ /** <!-- ==== ISP End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
