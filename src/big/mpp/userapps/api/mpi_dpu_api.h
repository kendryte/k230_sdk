/**
 * @file mpi_dpu_api.h
 * @author
 * @brief Defines APIs related to DPU device
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
#ifndef __MPI_DPU_API_H__
#define __MPI_DPU_API_H__

#include "k_type.h"
#include "k_dpu_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/** \addtogroup     DMA */
/** @{ */ /** <!-- [DMA] */

/**
 * @brief Initialize the DPU
 * @param [in] init
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_init(k_dpu_init_t *init);

/**
 * @brief Delete the DPU
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_delete();

/**
 * @brief Parse configuration parameters from binary file
 * @param [in] file_path File path
 * @param [out] param Parameters of DPU device
 * @param [out] image_check_templ_data Image check template
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */

/**
 * @brief Parse configuration parameters from binary file
 * @param [in] param_path File path
 * @param [out] param Parameters of DPU device
 * @param [out] lcn_param Parameters of LCN channel
 * @param [out] ir_param Parameters of IR channel
 * @param [out] g_temp_space Address of the parsed template image
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_parse_file(const k_char *param_path, k_dpu_dev_param_t *param,
    k_dpu_lcn_param_t *lcn_param, k_dpu_ir_param_t *ir_param, k_dpu_user_space_t *g_temp_space);

/**
 * @brief Set the device attributes of the DPU
 * @param [in] attr Pointer to the DPU device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_dev_attr(k_dpu_dev_attr_t *attr);

/**
 * @brief Get the device attributes of the DPU
 * @param [in] attr Pointer to the DPU device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_get_dev_attr(k_dpu_dev_attr_t *attr);

/**
 * @brief Set the channel attributes of the DPU
 * @param [in] lcn_attr Pointer of the speckle channel attribute
 * @param [in] ir_attr Pointer of the ir channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_chn_attr(k_dpu_chn_lcn_attr_t *lcn_attr, k_dpu_chn_ir_attr_t *ir_attr);

/**
 * @brief Get the channel attributes of the DPU
 * @param [out] lcn_attr Pointer of the speckle channel attribute
 * @param [out] ir_attr Pointer of the ir channel attribute
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_get_chn_attr(k_dpu_chn_lcn_attr_t *lcn_attr, k_dpu_chn_ir_attr_t *ir_attr);

/**
 * @brief Set the reference image address
 * @param [in] ref_path Path of the reference image
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_ref_image(const k_char *ref_path);

/**
 * @brief Set 64bit aligned reference image address
 * @param [in] ref_path Path of the reference image
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_aligned_ref_image(const k_char *ref_path);

/**
 * @brief Set the processed reference image address
 * @param [in] ref_path Path of the processed reference image
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_processed_ref_image(const k_char *ref_path);

/**
 * @brief Set the template iamge address
 * @param [in] temp_space Address of the parsed template image
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_set_template_image(k_dpu_user_space_t *temp_space);

/**
 * @brief Start the device of the DPU
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_start_dev();

/**
 * @brief Stop the device of the DPU
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_stop_dev();

/**
 * @brief Start the channel of the DPU
 * @param [in] chn_num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_start_chn(k_u32 chn_num);

/**
 * @brief Stop the channel of the DPU
 * @param [in] chn_num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_stop_chn(k_u32 chn_num);

/**
 * @brief Send a frame into the channel of the DPU
 * @param [in] chn_num channel number
 * @param [in] addr Physical address of the image to be sent
 * @param [in] s32_millisec Blocking time in milliseconds
 * @return k_s32
 * @retval k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_send_frame(k_u32 chn_num, k_u64 addr, k_s32 s32_millisec);

/**
 * @brief Get a frame from the channel of the DPU
 * @param [in] chn_num channel number
 * @param [out] result Store the results for this channel
 * @param [in] s32_millisec Blocking time in milliseconds
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_get_frame(k_u32 chn_num, k_dpu_chn_result_u *result, k_s32 s32_millisec);

/**
 * @brief Release the frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 */
k_s32 kd_mpi_dpu_release_frame();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif