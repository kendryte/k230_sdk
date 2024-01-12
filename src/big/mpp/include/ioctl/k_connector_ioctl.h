/**
 * @file k_sensor_ioctl.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-03-20
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
 */

#ifndef __K_CONNECTOR_IOCTL_H__
#define __K_CONNECTOR_IOCTL_H__

#include "k_connector_comm.h"
#include "k_ioctl.h"
#include "k_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


/* connector ioctl cmd */
typedef enum {
    KD_IOC_NR_CONNECTOR_DEV_POWER,
    KD_IOC_NR_CONNECTOR_DEV_INIT,
    KD_IOC_NR_CONNECTOR_DEV_ID,
    KD_IOC_NR_CONNECTOR_DEV_GET_NEG_DATA,
    KD_IOC_NR_CONNECTOR_DEV_MIRROR,
} k_ioc_nr_connector;



#define KD_IOC_CONNECTOR_S_POWER           _IOW(K_IOC_TYPE_SENSOR, KD_IOC_NR_CONNECTOR_DEV_POWER, k_s32)
#define KD_IOC_CONNECTOR_S_INIT            _IOW(K_IOC_TYPE_SENSOR, KD_IOC_NR_CONNECTOR_DEV_INIT, k_connector_info)
#define KD_IOC_CONNECTOR_G_ID              _IOW(K_IOC_TYPE_SENSOR, KD_IOC_NR_CONNECTOR_DEV_ID, k_u32)
#define KD_IOC_CONNECTOR_G_NEG_DATA        _IOW(K_IOC_TYPE_SENSOR, KD_IOC_NR_CONNECTOR_DEV_GET_NEG_DATA, k_connector_negotiated_data)
#define KD_IOC_CONNECTOR_S_MIRROR           _IOW(K_IOC_TYPE_SENSOR, KD_IOC_NR_CONNECTOR_DEV_MIRROR, k_connector_mirror)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

