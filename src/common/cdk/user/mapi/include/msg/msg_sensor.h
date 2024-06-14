/**
 * @file msg_sensor.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-04-11
 *
* @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
 *
 */

#ifndef __MSG_SENSOR_H__
#define __MSG_SENSOR_H__

#include "k_sensor_comm.h"
#include "k_vicap_comm.h"
#include "mpi_sensor_api.h"
#include "mpi_sys_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum {
    MSG_CMD_MEDIA_SENSOR_REG_READ,
    MSG_CMD_MEDIA_SENSOR_REG_WRITE,
    MSG_CMD_MEDIA_SENSOR_EXPOSURE_SET,
    MSG_CMD_MEDIA_SENSOR_EXPOSURE_GET,
    MSG_CMD_MEDIA_SENSOR_AGAIN_SET,
    MSG_CMD_MEDIA_SENSOR_AGAIN_GET,
    MSG_CMD_MEDIA_SENSOR_OTP_GET,
} msg_media_sensor_cmd_t;

typedef struct {
    k_s32 sensor_fd;
    k_u32 reg_addr;
    k_u32 reg_val;
} msg_sensor_reg_opt_t;

typedef struct {
    k_s32 sensor_fd;
    k_sensor_gain gain;
} msg_sensor_gain_opt_t;

typedef struct {
    k_s32 sensor_fd;
    k_sensor_intg_time exp_time;
} msg_sensor_exposure_time_opt_t;


typedef struct {
    k_s32 sensor_type;
    k_sensor_otp_date otp_data;
} msg_sensor_otp_opt_t;



/** @}*/  /** <!-- ==== COMM End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_SENSOR_H__ */