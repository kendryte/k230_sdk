/**
 * @file k_pm_comm.h
 * @author
 * @brief PM user device error code and public data types
 * @version 1.0
 * @date 2023-06-25
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
#ifndef __K_PM_COMM_H__
#define __K_PM_COMM_H__

#include "k_errno.h"
#include "k_module.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

/** \addtogroup     PM */
/** @{ */ /** <!-- [PM] */

/**
 * @brief Defines the pm control domain
 *
 * @note The type list is maintained by the driver developer
 */
typedef enum {
    PM_DOMAIN_CPU,
    PM_DOMAIN_KPU,
    PM_DOMAIN_DPU,
    PM_DOMAIN_VPU,
    PM_DOMAIN_DISPLAY,
    PM_DOMAIN_MEDIA,
    PM_DOMAIN_NR,
} k_pm_domain;

/**
 * @brief Defines the pm control governor
 *
 * @note The type list is maintained by the driver developer
 */
typedef enum {
    PM_GOVERNOR_MANUAL,
    PM_GOVERNOR_PERFORMANCE,
    PM_GOVERNOR_ENERGYSAVING,
    PM_GOVERNOR_AUTO,
} k_pm_governor;

typedef struct {
    int32_t freq;	/**< unit:Hz */
    int32_t volt;	/**< unit:uV */
} k_pm_profile;

#define K_ERR_PM_ILLEGAL_PARAM                                              \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_PM_EXIST                                                      \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_PM_UNEXIST                                                    \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_PM_NULL_PTR                                                   \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_PM_NOT_CONFIG                                                 \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_PM_NOT_SUPPORT                                                \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_PM_NOT_PERM                                                   \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_PM_NOMEM                                                      \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_PM_NOTREADY                                                   \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_PM_BADADDR                                                    \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_PM_BUSY                                                       \
    K_DEF_ERR(K_ID_PM, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== PM End ==== */

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */
#endif
