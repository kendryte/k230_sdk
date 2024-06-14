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
#include "msg_sensor.h"
#include "mapi_sensor_api.h"
#include "mapi_sensor_comm.h"
#include "mapi_sys_api.h"
#include "mpi_sensor_api.h"
#include "k_sensor_comm.h"

#define CHECK_MAPI_SENSOR_NULL_PTR(paraname, ptr)                      \
    do {                                                              \
        if ((ptr) == NULL) {                                          \
            mapi_sensor_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_SENSOR_NULL_PTR;                         \
        }                                                             \
    } while (0)

#define MAPI_SENSOR_UNUSED(x)    ((x)=(x))

k_s32 kd_mapi_sensor_reg_read(k_s32 fd, k_u32 reg_addr, k_u32 *reg_val)
{
    k_s32 ret = 0;
    CHECK_MAPI_SENSOR_NULL_PTR("reg_val", reg_val);
    ret = kd_mpi_sensor_reg_read(fd, reg_addr, reg_val);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_reg_read failed:0x%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_sensor_reg_write(k_s32 fd, k_u32 reg_addr, k_u32 reg_val)
{
    k_s32 ret;
    ret = kd_mpi_sensor_reg_write(fd, reg_addr, reg_val);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_reg_write failed:0x:%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_sensor_again_set(k_s32 fd, k_sensor_gain gain)
{
    k_s32 ret;
    ret = kd_mpi_sensor_again_set(fd, gain);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_again_set failed:0x:%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_sensor_again_get(k_s32 fd, k_sensor_gain *gain)
{
    k_s32 ret = 0;
    CHECK_MAPI_SENSOR_NULL_PTR("gain", gain);
    ret = kd_mpi_sensor_again_get(fd, gain);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_again_get failed:0x%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}

k_s32 kd_mapi_sensor_exposure_time_set(k_s32 fd, k_sensor_intg_time exp_time)
{
    k_s32 ret;
    ret = kd_mpi_sensor_intg_time_set(fd, exp_time);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_exposure_time_set failed:0x:%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }

    return K_SUCCESS;
}

k_s32 kd_mapi_sensor_exposure_time_get(k_s32 fd, k_sensor_intg_time *exp_time)
{
    k_s32 ret = 0;
    CHECK_MAPI_SENSOR_NULL_PTR("exp_time", exp_time);
    ret = kd_mpi_sensor_intg_time_get(fd, exp_time);
    if(ret != K_SUCCESS)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_exposure_time_get failed:0x%x\n", ret);
        return SENSOR_RET_MPI_TO_MAPI(ret);
    }
    return K_SUCCESS;
}


k_s32 kd_mapi_sensor_otpdata_get(k_s32 sensor_type, k_sensor_otp_date *otp_data)
{
    k_s32 ret = 0;
    k_s32 sensor_fd = -1;
    k_sensor_mode mode;
    k_vicap_sensor_info sensor_info;
    k_sensor_otp_date read_otp_data;
    
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    memcpy(&read_otp_data, otp_data, sizeof(k_sensor_otp_date));

    CHECK_MAPI_SENSOR_NULL_PTR("otp_data", otp_data);

    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);

    sensor_fd = kd_mpi_sensor_open(sensor_info.sensor_name);
    if (sensor_fd < 0) {
        printf("%s, sensor open failed.\n", __func__);
        return -1;
    }

    ret = kd_mpi_sensor_otpdata_get(sensor_fd, &read_otp_data);
    if (ret) {
        printf("%s, sensor stream on failed.\n", __func__);
        return -1;
    }

    memcpy(otp_data, &read_otp_data, sizeof(k_sensor_otp_date));

    return 0;
}