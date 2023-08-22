/**
 * @file mpi_pm_api.h
 * @author
 * @brief Defines APIs related to pm
 * @version 1.0
 * @date 2023-06-25
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
#ifndef __MPI_PM_API_H__
#define __MPI_PM_API_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PM */
/** @{ */ /** <!-- [PM] */

#include "k_pm_comm.h"

int kd_mpi_pm_set_reg(uint32_t reg, uint32_t val);
int kd_mpi_pm_get_reg(uint32_t reg, uint32_t *pval);
int kd_mpi_pm_get_profiles(k_pm_domain domain, uint32_t *pcount,
    k_pm_profile *pprofile);
int kd_mpi_pm_get_stat(k_pm_domain domain);
int kd_mpi_pm_set_governor(k_pm_domain domain, k_pm_governor governor);
int kd_mpi_pm_get_governor(k_pm_domain domain, k_pm_governor *pgovernor);
int kd_mpi_pm_set_profile(k_pm_domain domain, int32_t index);
int kd_mpi_pm_get_profile(k_pm_domain domain, int32_t *pindex);
int kd_mpi_pm_set_profile_lock(k_pm_domain domain, int32_t index);
int kd_mpi_pm_set_profile_unlock(k_pm_domain domain, int32_t index);
int kd_mpi_pm_set_thermal_protect(k_pm_domain domain, int32_t temp,
    int32_t index);
int kd_mpi_pm_get_thermal_protect(k_pm_domain domain, int32_t *ptemp,
    int32_t *pindex);
int kd_mpi_pm_set_thermal_shutdown(int32_t temp);
int kd_mpi_pm_get_thermal_shutdown(int32_t *ptemp);
int kd_mpi_pm_set_clock(k_pm_domain domain, bool enable);
int kd_mpi_pm_set_power(k_pm_domain domain, bool enable);

typedef enum {
    RUNTIMESTAGE_ID_AI2D_START,
    RUNTIMESTAGE_ID_AI2D_STOP,
    RUNTIMESTAGE_ID_KPU_START,
    RUNTIMESTAGE_ID_KPU_STOP,
    RUNTIMESTAGE_ID_CPU_START,
    RUNTIMESTAGE_ID_CPU_STOP,
} k_runtimestage_id;

int kd_mpi_pm_runtime_runstage(k_runtimestage_id stage);

/** @} */ /** <!-- ==== PM End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
