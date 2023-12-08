/**
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

#ifndef __K_PM_IOCTL_H__
#define __K_PM_IOCTL_H__

#include "k_pm_comm.h"
#include "k_ioctl.h"
#include "k_type.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef struct {
    uint32_t reg;
    uint32_t val;
} _k_pm_reg;

typedef struct {
    k_pm_domain domain;
    uint32_t number;
    k_pm_profile profiles[0];
} _k_pm_domain_profiles;

typedef struct {
    k_pm_domain domain;
    k_pm_governor governor;
} _k_pm_domain_governor;

typedef struct {
    k_pm_domain domain;
    int32_t index;
} _k_pm_domain_profile;

typedef struct {
    k_pm_domain domain;
    int32_t temp;	// unit:0.01℃
    int32_t index;
} _k_pm_domain_thermal_protect;

typedef struct {
    k_pm_domain domain;
    bool enable;
} _k_pm_domain_bool;

enum {
    K_IOC_NR_PM_SET_REG,
    K_IOC_NR_PM_GET_REG,
    K_IOC_NR_PM_GET_PROFILES,
    K_IOC_NR_PM_GET_STAT,
    K_IOC_NR_PM_SET_GOVERNOR,
    K_IOC_NR_PM_GET_GOVERNOR,
    K_IOC_NR_PM_SET_PROFILE,
    K_IOC_NR_PM_GET_PROFILE,
    K_IOC_NR_PM_SET_PROFILE_LOCK,
    K_IOC_NR_PM_SET_PROFILE_UNLOCK,
    K_IOC_NR_PM_SET_THERMAL_PROTECT,
    K_IOC_NR_PM_GET_THERMAL_PROTECT,
    K_IOC_NR_PM_SET_THERMAL_SHUTDOWN,
    K_IOC_NR_PM_GET_THERMAL_SHUTDOWN,
    K_IOC_NR_PM_SET_CLOCK,
    K_IOC_NR_PM_SET_POWER,
};

#define K_IOC_PM_SET_REG                                                    \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_REG, _k_pm_reg)
#define K_IOC_PM_GET_REG                                                    \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_REG, _k_pm_reg)
#define K_IOC_PM_GET_PROFILES                                               \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_PROFILES, _k_pm_domain_profiles)
#define K_IOC_PM_GET_STAT                                                   \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_STAT, k_pm_domain)
#define K_IOC_PM_SET_GOVERNOR                                               \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_GOVERNOR, _k_pm_domain_governor)
#define K_IOC_PM_GET_GOVERNOR                                               \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_GOVERNOR, _k_pm_domain_governor)
#define K_IOC_PM_SET_PROFILE                                                \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_PROFILE, _k_pm_domain_profile)
#define K_IOC_PM_GET_PROFILE                                                \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_PROFILE, _k_pm_domain_profile)
#define K_IOC_PM_SET_PROFILE_LOCK                                           \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_PROFILE_LOCK, _k_pm_domain_profile)
#define K_IOC_PM_SET_PROFILE_UNLOCK                                         \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_PROFILE_UNLOCK,                     \
    _k_pm_domain_profile)
#define K_IOC_PM_SET_THERMAL_PROTECT                                        \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_THERMAL_PROTECT,                    \
    _k_pm_domain_thermal_protect)
#define K_IOC_PM_GET_THERMAL_PROTECT                                        \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_THERMAL_PROTECT,                   \
    _k_pm_domain_thermal_protect)
#define K_IOC_PM_SET_THERMAL_SHUTDOWN                                       \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_THERMAL_SHUTDOWN, int32_t)
#define K_IOC_PM_GET_THERMAL_SHUTDOWN                                       \
    _IOWR(K_IOC_TYPE_PM, K_IOC_NR_PM_GET_THERMAL_SHUTDOWN, int32_t)
#define K_IOC_PM_SET_CLOCK                                                  \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_CLOCK, _k_pm_domain_bool)
#define K_IOC_PM_SET_POWER                                                  \
    _IOW(K_IOC_TYPE_PM, K_IOC_NR_PM_SET_POWER, _k_pm_domain_bool)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
