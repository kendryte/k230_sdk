/**
 * @file mpi_sensor_api.h
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
#ifndef __MPI_SENSOR_API_H__
#define __MPI_SENSOR_API_H__

#include "k_type.h"
#include "k_sensor_comm.h"
#include "k_vicap_comm.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     SENSOR */
/** @{ */ /** <!-- [SENSOR] */

/**
 * @brief open the device by the sensor name
 *
 * @param [in] sensor_name
 * @return k_s32
 * @retval fd success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_open(const char *sensor_name);

/**
 * @brief close the sensor device
 *
 * @param [in] fd
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_close(k_s32 fd);

/**
 * @brief Get the sensor infomation of the the VICAP
 *
 * @param [in] fd
 * @param [in] on true: power on, false; power off
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_power_set(k_s32 fd, k_bool on);

/**
 * @brief Get the sensor chid id
 *
 * @param [in] fd
 * @param [out] sensor_id
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_id_get(k_s32 fd, k_u32 *sensor_id);

/**
 * @brief Init the sensor by the mode
 *
 * @param [in] fd
 * @param [in] mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_init(k_s32 fd, k_sensor_mode mode);

/**
 * @brief Read the sensor register
 *
 * @param [in] fd
 * @param [in] reg_addr
 * @param [out] reg_val
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_reg_read(k_s32 fd, k_u32 reg_addr, k_u32 *reg_val);

/**
 * @brief Write the sensor register
 *
 * @param [in] fd
 * @param [in] reg_addr
 * @param [in] reg_val
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */

k_s32 kd_mpi_sensor_reg_write(k_s32 fd, k_u32 reg_addr, k_u32 reg_val);

/**
 * @brief Get the sensor mode by the sensor type of the mode
 *
 * @param [in] fd
 * @param [out] mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_mode_get(k_s32 fd, k_sensor_mode *mode);

/**
 * @brief Set the sensor mode
 *
 * @param [in] fd
 * @param [in] mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_mode_set(k_s32 fd, k_sensor_mode mode);

/**
 * @brief Enum the sensor mode by the mode index
 *
 * @param [in] fd
 * @param [out] enum_mode
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_mode_enum(k_s32 fd, k_sensor_enum_mode *enum_mode);

/**
 * @brief Get the sensor capabilities
 *
 * @param [in] sensor_type
 * @param [out] sensor_info
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_caps_get(k_s32 fd, k_sensor_caps *caps);

/**
 * @brief Check the sensor connect status.
 *
 * @param [in] fd
 * @param [out] connection
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_connection_check(k_s32 fd, k_s32 *connection);

/**
 * @brief Set the sensor stream out enable/disable
 *
 * @param [in] fd
 * @param [in] enable  0: disable, non-zero; enable
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_stream_enable(k_s32 fd, k_s32 enable);

/**
 * @brief Set the sensor analog gain
 *
 * @param [in] fd
 * @param [in] gain
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_again_set(k_s32 fd, k_sensor_gain gain);

/**
 * @brief Get the sensor analog gain
 *
 * @param [in] fd
 * @param [out] gain
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_again_get(k_s32 fd, k_sensor_gain *gain);

/**
 * @brief Set the sensor digital gain
 *
 * @param [in] fd
 * @param [in] gain
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_dgain_set(k_s32 fd, k_sensor_gain gain);

/**
 * @brief Get the sensor digital gain
 *
 * @param [in] fd
 * @param [out] gain
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_dgain_get(k_s32 fd, k_sensor_gain *gain);

/**
 * @brief Set the sensor integration time
 *
 * @param [in] fd
 * @param [in] time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_intg_time_set(k_s32 fd, k_sensor_intg_time time);

/**
 * @brief Get the sensor integration time
 *
 * @param [in] fd
 * @param [out] time
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_intg_time_get(k_s32 fd, k_sensor_intg_time *time);

/**
 * @brief Get the sensor fps
 *
 * @param [in] fd
 * @param [out] fps
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_fps_get(k_s32 fd, k_u32 *fps);

/**
 * @brief Set the sensor fps
 *
 * @param [in] fd
 * @param [out] fps
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_fps_set(k_s32 fd, k_u32 fps);

/**
 * @brief Get the sensor isp status
 *
 * @param [in] fd
 * @param [out] isp_status
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_isp_status_get(k_s32 fd, k_sensor_isp_status *isp_status);

/**
 * @brief Set the sensor blc param
 *
 * @param [in] fd
 * @param [in] blc
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_blc_set(k_s32 fd, k_sensor_blc blc);

/**
 * @brief Set the sensor wb param
 *
 * @param [in] fd
 * @param [in] wb
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_wb_set(k_s32 fd, k_sensor_white_balance wb);

/**
 * @brief Get the sensor test pattern status.
 *
 * @param [in] fd
 * @param [out] tpg
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_tpg_get(k_s32 fd, k_sensor_test_pattern *tpg);

/**
 * @brief Set the sensor test pattern mode
 *
 * @param [in] fd
 * @param [in] tpg
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_tpg_set(k_s32 fd, k_sensor_test_pattern tpg);

/**
 * @brief Get the sensor expand curve
 *
 * @param [in] fd
 * @param [out] curve
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_expand_curve_get(k_s32 fd, k_sensor_compand_curve *curve);

/**
 * @brief Get the sensor otp data.
 *
 * @param [in] fd
 * @param [out] ota_data
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_otpdata_get(k_s32 fd, void *ota_data);


/**
 * @brief Set the sensor otp data.
 *
 * @param [in] fd
 * @param [out] ota_data
 * @return k_s32
 * @retval 0 success
 * @retval "not 0" see err code
 * @see K_ERR_CODE_E
 * @note
 *
 */
k_s32 kd_mpi_sensor_otpdata_set(k_s32 fd, void *ota_data);


k_s32 kd_mpi_sensor_mirror_set(k_s32 fd, k_vicap_mirror_mode mirror);

/** @} */ /** <!-- ==== SENSOR End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
