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

#include "pm_domain.h"

#define PROFILE_IS_UP(from, to) (from > to)

static int pm_domain_profile_index_transform(struct pm_domain_dev *pdev,
    int32_t index)
{
    if (index < 0)
        index = pdev->profile_nr + index;

    if (index < 0)
        index = 0;
    else if (index >= pdev->profile_nr)
        index = pdev->profile_nr - 1;

    return index;
}

int pm_domain_init(struct pm_domain_dev *pdev)
{
    if (pdev->ops && pdev->ops->init)
        return pdev->ops->init();

    return 0;
}

int pm_domain_set_governor(struct pm_domain_dev *pdev, k_pm_governor governor)
{
    int ret;

    if (pdev->governor == governor)
        return 0;

    if (!pdev->ops || !pdev->ops->set_governor)
        return K_ERR_PM_NOT_SUPPORT;

    ret = pdev->ops->set_governor(governor);

    return ret;
}

int pm_domain_set_expect_profile(struct pm_domain_dev *pdev, int32_t index)
{
    int ret;

    if (pdev->governor != PM_GOVERNOR_MANUAL)
        return K_ERR_PM_NOT_SUPPORT;

    pdev->expect_profile_idx = index;

    return 0;
}

int pm_domain_set_expect_profile_lock(struct pm_domain_dev *pdev,
    int32_t index)
{
    int ret;

    if (pdev->governor != PM_GOVERNOR_MANUAL)
        return K_ERR_PM_NOT_SUPPORT;

    index = pm_domain_profile_index_transform(pdev, index);

    if (pdev->profile_lock[index] >= UINT32_MAX)
        return K_ERR_PM_BUSY;

    pdev->profile_lock[index]++;
    for (int i = 0; i < pdev->profile_nr; i++) {
        if (pdev->profile_lock[i]) {
            pdev->expect_profile_idx = i;
            break;
        }
    }

    return 0;
}

int pm_domain_set_expect_profile_unlock(struct pm_domain_dev *pdev,
    int32_t index)
{
    int ret;

    if (pdev->governor != PM_GOVERNOR_MANUAL)
        return K_ERR_PM_NOT_SUPPORT;

    index = pm_domain_profile_index_transform(pdev, index);

    if (pdev->profile_lock[index] == 0)
        return K_ERR_PM_BUSY;

    pdev->profile_lock[index]--;
    for (int i = 0; i < pdev->profile_nr; i++) {
        if (pdev->profile_lock[i]) {
            pdev->expect_profile_idx = i;
            break;
        }
    }

    return 0;
}

int pm_domain_get_expect_profile(struct pm_domain_dev *pdev)
{
    if (pdev->governor == PM_GOVERNOR_PERFORMANCE)
        pdev->expect_profile_idx = 0;
    else if (pdev->governor == PM_GOVERNOR_ENERGYSAVING)
        pdev->expect_profile_idx = pdev->profile_nr - 1;

    if (pdev->ops && pdev->ops->get_expect_profile)
        pdev->ops->get_expect_profile();

    return 0;
}

int pm_domain_set_actual_profile(struct pm_domain_dev *pdev)
{
    int ret;
    int32_t from_idx_cvt;
    int32_t to_idx, to_idx_cvt;
    int32_t expect_profile_idx, expect_profile_idx_cvt;
    int32_t thermal_protect_idx, thermal_protect_idx_cvt;

    if (!pdev->ops || !pdev->ops->set_clock)
        return K_ERR_PM_NOT_SUPPORT;

    rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
    thermal_protect_idx = pdev->thermal_protect_idx;
    expect_profile_idx = pdev->expect_profile_idx;
    thermal_protect_idx_cvt = pm_domain_profile_index_transform(pdev,
        thermal_protect_idx);
    expect_profile_idx_cvt = pm_domain_profile_index_transform(pdev,
        expect_profile_idx);
    if (pdev->thermal_over &&
        PROFILE_IS_UP(thermal_protect_idx_cvt, expect_profile_idx_cvt)) {
        to_idx = thermal_protect_idx;
        to_idx_cvt = thermal_protect_idx_cvt;
    } else {
        to_idx = expect_profile_idx;
        to_idx_cvt = expect_profile_idx_cvt;
    }
    from_idx_cvt = pm_domain_profile_index_transform(pdev,
        pdev->actual_profile_idx);
    if (from_idx_cvt == to_idx_cvt) {
        rt_mutex_release(&pdev->mutex);
        return 0;
    }

    if (pdev->regulator == NULL) {
        ret = pdev->ops->set_clock(pdev->profiles[to_idx_cvt].freq);
    } else if (PROFILE_IS_UP(from_idx_cvt, to_idx_cvt)) {
        if (pdev->profiles[from_idx_cvt].volt ==
            pdev->profiles[to_idx_cvt].volt) {
            ret = 0;
        } else {
            ret = regulator_set_voltage(pdev->regulator,
            pdev->profiles[to_idx_cvt].volt);
            rt_thread_mdelay(1);
        }
        if (ret == 0)
            ret = pdev->ops->set_clock(pdev->profiles[to_idx_cvt].freq);
    } else {
        ret = pdev->ops->set_clock(pdev->profiles[to_idx_cvt].freq);
        if (ret == 0 && pdev->profiles[from_idx_cvt].volt !=
            pdev->profiles[to_idx_cvt].volt)
            ret = regulator_set_voltage(pdev->regulator,
                pdev->profiles[to_idx_cvt].volt);
    }

    if (ret == 0)
        pdev->actual_profile_idx = to_idx;

    rt_mutex_release(&pdev->mutex);

    return ret;
}

int pm_domain_get_actual_profile(struct pm_domain_dev *pdev, int32_t *pindex)
{
    *pindex = pm_domain_profile_index_transform(pdev, pdev->actual_profile_idx);

    return 0;
}

int pm_domain_enable_clock(struct pm_domain_dev *pdev, bool enable)
{
    if (!pdev->ops || !pdev->ops->enable_clock)
        return K_ERR_PM_NOT_SUPPORT;

    return pdev->ops->enable_clock(enable);
}

int pm_domain_enable_power(struct pm_domain_dev *pdev, bool enable)
{
    if (!pdev->ops || !pdev->ops->enable_power)
        return K_ERR_PM_NOT_SUPPORT;

    return pdev->ops->enable_power(enable);
}
