/**
 * @file mpi_vvi_api.h
 * @author
 * @brief Defines APIs related to virtual video input device
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
#ifndef __MPI_CONNECTOR_API_H__
#define __MPI_CONNECTOR_API_H__

#include "k_type.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     connector */
/** @{ */ /** <!-- [connector] */


k_s32 kd_mpi_connector_init(k_s32 fd, k_connector_info info);

k_s32 kd_mpi_connector_id_get(k_s32 fd, k_u32 *sensor_id);

k_s32 kd_mpi_connector_power_set(k_s32 fd, k_bool on);

k_s32 kd_mpi_connector_close(k_s32 fd);

k_s32 kd_mpi_connector_open(const char *connector_name);

k_s32 kd_mpi_connector_get_negotiated_data(k_s32 fd, k_connector_negotiated_data *negotiated_data);

k_s32 kd_mpi_connector_adapt_resolution(k_connector_type connector_type, k_connector_negotiated_data *negotiated_data);

k_s32 kd_mpi_get_connector_info(k_connector_type connector_type, k_connector_info *connector_info);

k_s32 kd_mpi_connector_set_mirror(k_s32 fd, k_connector_mirror mirror);


/** @} */ /** <!-- ==== connector End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
