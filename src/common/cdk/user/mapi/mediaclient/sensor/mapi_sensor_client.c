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
#include <stdlib.h>
#include <pthread.h>
#include "mapi_sensor_api.h"
#include "msg_client_dispatch.h"
#include "mapi_sensor_comm.h"
#include "mpi_sensor_api.h"
#include "msg_sensor.h"
#include "k_type.h"
#include "k_sensor_comm.h"

#include <stdio.h>

k_s32 kd_mapi_sensor_reg_read(k_s32 fd, k_u32 reg_addr, k_u32 *reg_val)
{
    msg_sensor_reg_opt_t reg_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }

    reg_opt.sensor_fd = fd;
    reg_opt.reg_addr = reg_addr;
    reg_opt.reg_val = 0;
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_REG_READ,
            &reg_opt, sizeof(reg_opt), NULL);

    if(ret != K_SUCCESS) {
        printf("kd_mapi_sensor_reg_read failed\n");
    }
    memcpy(reg_val, &reg_opt.reg_val, sizeof(k_u32));
    return ret;
}

k_s32 kd_mapi_sensor_reg_write(k_s32 fd, k_u32 reg_addr, k_u32 reg_val)
{
    msg_sensor_reg_opt_t reg_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }

    reg_opt.sensor_fd = fd;
    reg_opt.reg_addr = reg_addr;
    reg_opt.reg_val = reg_val;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_REG_WRITE,
            &reg_opt, sizeof(reg_opt), NULL);

    if(ret != K_SUCCESS) {
        mapi_sensor_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_sensor_again_set(k_s32 fd, k_sensor_gain gain)
{
    msg_sensor_gain_opt_t gain_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_again_set, sensor fd %d error!\n", fd);
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }
    gain_opt.sensor_fd = fd;
    memcpy(&gain_opt.gain, &gain, sizeof(k_sensor_gain));
#ifdef MAPI_SENSOR_DEBUG
    for(k_u8 i = 0; i < SENSOR_EXPO_FRAME_TYPE_MAX; i++)
    {
        printf("@@@@ sensor set again[%d] = %f\n", i, gain_opt.gain.gain[i]);
    }
#endif
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_AGAIN_SET,
            &gain_opt, sizeof(gain_opt), NULL);

    if(ret != K_SUCCESS) {
        mapi_sensor_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_sensor_again_get(k_s32 fd, k_sensor_gain *gain)
{
    msg_sensor_gain_opt_t gain_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }

    gain_opt.sensor_fd = fd;
    memcpy(&gain_opt.gain, gain, sizeof(k_sensor_gain));
    memset(&gain_opt.gain.gain, 0, sizeof(gain_opt.gain.gain));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_AGAIN_GET,
            &gain_opt, sizeof(gain_opt), NULL);
#ifdef MAPI_SENSOR_DEBUG
    for(k_u8 i = 0; i < SENSOR_EXPO_FRAME_TYPE_MAX; i++)
    {
        printf("@@@@ sensor get again[%d] = %f\n", i, gain_opt.gain.gain[i]);
    }
#endif
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sensor_again_get failed\n");
    }
    memcpy(gain, &gain_opt.gain, sizeof(k_sensor_gain));
    return ret;
}

k_s32 kd_mapi_sensor_exposure_time_set(k_s32 fd, k_sensor_intg_time exp_time)
{
    msg_sensor_exposure_time_opt_t exp_time_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        mapi_sensor_error_trace("kd_mapi_sensor_exposure_time_set, sensor fd %d error!\n", fd);
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }
    exp_time_opt.sensor_fd = fd;
    memcpy(&exp_time_opt.exp_time, &exp_time, sizeof(k_sensor_intg_time));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_EXPOSURE_SET,
            &exp_time_opt, sizeof(exp_time_opt), NULL);

    if(ret != K_SUCCESS) {
        mapi_sensor_error_trace("mapi_send_sync failed\n");
    }
    return ret;
}

k_s32 kd_mapi_sensor_exposure_time_get(k_s32 fd, k_sensor_intg_time *exp_time)
{
    msg_sensor_exposure_time_opt_t exp_opt;
    k_s32 ret = 0;

    if(fd < 0)
    {
        return K_MAPI_ERR_SENSOR_ILLEGAL_PARAM;
    }

    exp_opt.sensor_fd = fd;
    memcpy(&exp_opt.exp_time, exp_time, sizeof(k_sensor_intg_time));
    memset(&exp_opt.exp_time.intg_time, 0, sizeof(exp_opt.exp_time.intg_time));
    ret = mapi_send_sync(MODFD(K_MAPI_MOD_SENSOR, 0, 0), MSG_CMD_MEDIA_SENSOR_EXPOSURE_GET,
            &exp_opt, sizeof(exp_opt), NULL);
#ifdef MAPI_SENSOR_DEBUG
    for(k_u8 i = 0; i < SENSOR_EXPO_FRAME_TYPE_MAX; i++)
    {
        printf("@@@@ sensor get exp time[%d] = %f\n", i, exp_opt.exp_time.intg_time[i]);
    }
#endif
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sensor_exposure_time_get failed\n");
    }
    memcpy(exp_time, &exp_opt.exp_time, sizeof(k_sensor_intg_time));
    return ret;
}