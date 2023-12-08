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

#ifndef __PM_DOMAIN_H__
#define __PM_DOMAIN_H__

#include "k_pm_comm.h"
#include "regulator.h"
#include "sysctl_media_clk.h"
#include "sysctl_pwr.h"

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

struct pm_domain_ops {
    int (*init)(void);
    int (*set_governor)(k_pm_governor governor);
    int (*get_expect_profile)(void);
    int (*enable_clock)(bool enable);
    int (*set_clock)(int32_t freq);
    int (*enable_power)(bool enable);
    int (*get_stat)(void);
};

struct pm_domain_dev {
    struct rt_mutex mutex;
    k_pm_profile *profiles;
    uint32_t *profile_lock;
    uint32_t profile_nr;
    int32_t expect_profile_idx;
    int32_t actual_profile_idx;
    k_pm_governor governor;
    int32_t thermal_protect_temp;
    int32_t thermal_protect_idx;
    bool thermal_over;
    const char *regulator_name;
    struct regulator_dev *regulator;
    struct pm_domain_ops *ops;
};

extern struct pm_domain_dev pm_domain_cpu;
extern struct pm_domain_dev pm_domain_kpu;
extern struct pm_domain_dev pm_domain_vpu;
extern struct pm_domain_dev pm_domain_dpu;
extern struct pm_domain_dev pm_domain_disp;

int pm_domain_init(struct pm_domain_dev *pdev);
int pm_domain_set_governor(struct pm_domain_dev *pdev, k_pm_governor governor);
int pm_domain_set_expect_profile(struct pm_domain_dev *pdev, int32_t index);
int pm_domain_set_expect_profile_lock(struct pm_domain_dev *pdev,
    int32_t index);
int pm_domain_set_expect_profile_unlock(struct pm_domain_dev *pdev,
    int32_t index);
int pm_domain_get_expect_profile(struct pm_domain_dev *pdev);
int pm_domain_set_actual_profile(struct pm_domain_dev *pdev);
int pm_domain_get_actual_profile(struct pm_domain_dev *pdev, int32_t *pindex);
int pm_domain_enable_clock(struct pm_domain_dev *pdev, bool enable);
int pm_domain_enable_power(struct pm_domain_dev *pdev, bool enable);

#endif /* __PM_DOMAIN_H__ */
