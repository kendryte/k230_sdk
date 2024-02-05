/**
 * @file mpi_vicap_api.h
 * @author
 * @brief Defines APIs related to virtual video input device
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
#ifndef __MPI_VICAP_API_H__
#define __MPI_VICAP_API_H__

#include "k_type.h"
#include "k_vicap_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VICAP */
/** @{ */ /** <!-- [VICAP] */

/**
 * @brief Get the sensor infomation of the the VICAP
 *
 * @param [in] sensor_type
 * @param [out] sensor_info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note APP need to call the API first to obtain the sensor config information.
 *
 */
k_s32 kd_mpi_vicap_get_sensor_info(k_vicap_sensor_type sensor_type, k_vicap_sensor_info *sensor_info);

k_s32 kd_mpi_vicap_get_sensor_type(k_vicap_sensor_type *sensor_type, const char *sensor_string);

const char *kd_mpi_vicap_get_sensor_string(k_vicap_sensor_type sensor_type);

/**
 * @brief Get the sensor device open fd of the the VICAP
 *
 * @param [in] sensor_info.dev_num
 * @param [out] sensor_info.sensor_fd
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note APP need use sensor reg read & write, need sensor fd to call the register operating function.
 *
 */
k_s32 kd_mpi_vicap_get_sensor_fd(k_vicap_sensor_attr *sensor_attr);

/**
 * @brief Set the device attributes of the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @param [in] dev_attr  vicap device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_vicap_set_dev_attr(k_vicap_dev dev_num, k_vicap_dev_attr dev_attr);

/**
 * @brief Set the device attributes of the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @param [out] dev_attr  vicap device attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 */
k_s32 kd_mpi_vicap_get_dev_attr(k_vicap_dev dev_num, k_vicap_dev_attr *dev_attr);

/**
 * @brief Set the channel attributes of the the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @param [in] chn_num  channel number
 * @param [in] chn_attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 *
 */
k_s32 kd_mpi_vicap_set_chn_attr(k_vicap_dev dev_num, k_vicap_chn chn_num, k_vicap_chn_attr chn_attr);

/**
 * @brief Get the channel attributes of the the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @param [in] chn_num  channel number
 * @param [out] chn_attr channel attributes
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Will not be effective
 *
 */
k_s32 kd_mpi_vicap_get_chn_attr(k_vicap_dev dev_num, k_vicap_chn chn_num, k_vicap_chn_attr *chn_attr);

/**
 * @brief Init VICAP
 *
 * @param [in] dev_num  vicap device num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note Before call the API, need to set the device & channel attribute.
 *
 */
k_s32 kd_mpi_vicap_init(k_vicap_dev dev_num);

/**
 * @brief Deinit VICAP
 *
 * @param [in] dev_num  vicap device num
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_vicap_deinit(k_vicap_dev dev_num);

/**
 * @brief Start the stream of the the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_vicap_start_stream(k_vicap_dev dev_num);

/**
 * @brief Stop the stream of the the VICAP
 *
 * @param [in] dev_num  vicap device number
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_vicap_stop_stream(k_vicap_dev dev_num);

/**
 * @brief dump frame from the VICAP
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
k_s32 kd_mpi_vicap_dump_frame(k_vicap_dev dev_num, k_vicap_chn chn_num, k_vicap_dump_format foramt,
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
k_s32 kd_mpi_vicap_dump_release(k_vicap_dev dev_num, k_vicap_chn chn_num, const k_video_frame_info *vf_info);

/**
 * @brief vicap set mclk
 *
 * @param [in] k_vicap_mclk_id mclk device number
 * @param [in] sel mclk clk select
 * @param [in] mclk_div mclk div
 * @param [in] mclk_en mclk enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_set_mclk(k_vicap_mclk_id id, k_vicap_mclk_sel sel, k_u8 mclk_div, k_u8 mclk_en);

/**
 * @brief vicap set vi drop frame
 *
 * @param [in] csi csi num
 * @param [in] k_vicap_drop_frame Description of lost frame information
 * @param [in] enable enable drop frame
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_set_vi_drop_frame(k_vicap_csi_num csi, k_vicap_drop_frame *frame, k_bool enable);

/**
 * @brief vicap set database parse mode
 *
 * @param [in] dev_num device number
 * @param [in] parse_mode parse mode, 0: xml & json mode, 1: header file mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_set_database_parse_mode(k_vicap_dev dev_num, k_vicap_database_parse_mode parse_mode);

/**
 * @brief vicap enable TPG(Test Pattern Generator)
 *
 * @param [in] enable 1: enable, 0: disable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_tpg_enable(k_bool enable);

/**
 * @brief vicap load user image data
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
k_s32 kd_mpi_vicap_load_image(k_vicap_dev dev_num, const void *image_data, k_u32 data_len);

k_s32 kd_mpi_vicap_dump_register(k_vicap_dev dev_num, const char *path);

/**
 * @brief vicap set dump reserved
 *
 * @param [in] dev_num device number
 * @param [in] chn_num channel number
 * @param [in] reserved enable reserved
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
void kd_mpi_vicap_set_dump_reserved(k_vicap_dev dev_num, k_vicap_chn chn_num, k_bool reserved);

/**
 * @brief vicap set slave mode
 *
 * @param [in] id slave dev id
 * @param [in] enable vsync and hsync enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_set_slave_enable(k_vicap_slave_id id, k_vicap_slave_enable *enable);

/**
 * @brief vicap set slave mode attr
 *
 * @param [in] id slave dev id
 * @param [in] info slave mode vsync andd hsync attr
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_set_slave_attr(k_vicap_slave_id id, k_vicap_slave_info *info);

/**
 * @brief vicap set 3d mode
 *
 * @param [in] enable 3d mode enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 */
k_s32 kd_mpi_vicap_3d_mode_crtl(k_bool enable);


/** @} */ /** <!-- ==== VICAP End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
