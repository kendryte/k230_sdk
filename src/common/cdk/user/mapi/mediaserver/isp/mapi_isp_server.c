/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#include <stdio.h>
#include <unistd.h>
#include "msg_isp.h"
#include "mapi_isp_api.h"
#include "mapi_vicap_comm.h"
#include "mapi_sys_api.h"
#include "mpi_isp_api.h"
#include "k_isp_comm.h"

#define CHECK_MAPI_ISP_NULL_PTR(paraname, ptr)                      \
    do {                                                              \
        if ((ptr) == NULL) {                                          \
            mapi_vicap_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_ISP_NULL_PTR;                         \
        }                                                             \
    } while (0)

k_s32 kd_mapi_isp_ae_get_roi(k_vicap_dev dev_num, k_isp_ae_roi *ae_roi)
{
    k_s32 ret = 0;
    CHECK_MAPI_ISP_NULL_PTR("ae_roi", ae_roi);
    ret = kd_mpi_isp_ae_get_roi(dev_num, ae_roi);
    if(ret != K_SUCCESS)
    {
        mapi_vicap_error_trace("kd_mapi_isp_ae_get_roi failed:0x%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_isp_ae_set_roi(k_vicap_dev dev_num, k_isp_ae_roi ae_roi)
{
    k_s32 ret;
    ret = kd_mpi_isp_ae_set_roi(dev_num, ae_roi);
    if(ret != K_SUCCESS) {
        mapi_vicap_error_trace("kd_mpi_isp_ae_set_roi failed:0x:%x\n", ret);
        return VICAP_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}