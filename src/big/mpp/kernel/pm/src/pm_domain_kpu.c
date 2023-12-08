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

#include <string.h>
#include "pm_domain.h"

static k_pm_profile profiles[] = {
    {0.8e9, 0.8e6},
    {0.4e9, 0.8e6},
    {0.2e9, 0.8e6},
    {0.1e9, 0.8e6},
};

static uint32_t profile_lock[ARRAY_SIZE(profiles)];

static int set_governor(k_pm_governor governor)
{
    if (governor == PM_GOVERNOR_MANUAL ||
        governor == PM_GOVERNOR_PERFORMANCE ||
        governor == PM_GOVERNOR_ENERGYSAVING) {
        pm_domain_kpu.governor = governor;

        if (governor == PM_GOVERNOR_MANUAL)
            memset(profile_lock, 0, sizeof(profile_lock));

        return 0;
    }

    return K_ERR_PM_ILLEGAL_PARAM;
}

static int enable_clock(bool enable)
{
    sysctl_media_clk_set_leaf_en_multi(SYSCTL_CLK_AI_SRC, enable);

    return 0;
}

static int set_clock(int32_t freq)
{
    int ret;

    switch (freq) {
    case (int)0.8e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_AI_SRC,
            SYSCTL_CLK_ROOT_PLL3_DIV_2, 1, 1);
        break;
    case (int)0.4e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_AI_SRC,
            SYSCTL_CLK_ROOT_PLL3_DIV_2, 1, 2);
        break;
    case (int)0.2e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_AI_SRC,
            SYSCTL_CLK_ROOT_PLL3_DIV_2, 1, 4);
        break;
    case (int)0.1e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_AI_SRC,
            SYSCTL_CLK_ROOT_PLL3_DIV_2, 1, 8);
        break;
    default:
        ret = K_ERR_PM_ILLEGAL_PARAM;
        break;
    }

    return ret ? 0 : K_ERR_PM_ILLEGAL_PARAM;
}

static int enable_power(bool enable)
{
    int ret;

    if (enable)
        ret = sysctl_pwr_up(SYSCTL_PD_AI);
    else
        ret = sysctl_pwr_off(SYSCTL_PD_AI);

    return ret ? 0 : K_ERR_PM_BUSY;
}

static struct pm_domain_ops ops = {
    .set_governor = set_governor,
    .enable_clock = enable_clock,
    .set_clock = set_clock,
    .enable_power = enable_power,
};

struct pm_domain_dev pm_domain_kpu = {
    .profiles = profiles,
    .profile_nr = ARRAY_SIZE(profiles),
    .profile_lock = profile_lock,
    .regulator_name = "regulator_kpu",
    .ops = &ops,
};
