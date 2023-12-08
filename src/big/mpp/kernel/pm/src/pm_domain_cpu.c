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
#include <rtthread.h>
#include "pm_domain.h"

#define AUTO_GOVERNOR_POLL_TIME 100 // ms

static k_pm_profile profiles[] = {
    {1.6e9, 0.8e6},
    {1.188e9, 0.7e6},
    {0.8e9, 0.68e6},
    {0.594e9, 0.66e6},
    {0.4e9, 0.64e6},
    {0.2e9, 0.62e6},
};

static uint32_t profile_lock[ARRAY_SIZE(profiles)];
static bool auto_load_enable;
static uint8_t load_table[ARRAY_SIZE(profiles)][ARRAY_SIZE(profiles)] = {
    {64, 43, 31, 20, 10},
    {90, 58, 43, 28, 14},
    {90, 99, 64, 43, 20},
    {90, 99, 99, 58, 28},
    {90, 99, 99, 99, 43},
    {90, 99, 99, 99, 99},
};

static int get_expect_profile(void);

extern rt_uint8_t sys_cpu_usage(rt_uint8_t cpu_id);
static int32_t auto_governor_get_profile(void)
{
    int load;
    int32_t index;
    uint8_t *table;

    load = sys_cpu_usage(0);
    pm_domain_get_actual_profile(&pm_domain_cpu, &index);

    table = load_table[index];

    index = -1;
    for (int i = 0; i < ARRAY_SIZE(profiles); i++) {
        if (load > table[i]) {
            index = i;
            break;
        }
    }

    return index;
}

static void auto_load_thread(void *arg)
{
    while (1) {
        rt_thread_mdelay(AUTO_GOVERNOR_POLL_TIME);
        if (!auto_load_enable)
            continue;
        get_expect_profile();
        pm_domain_set_actual_profile(&pm_domain_cpu);
    }
}

static int init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("auto_load_thread", auto_load_thread,
        RT_NULL, 1024 * 10, RT_THREAD_PRIORITY_MAX / 2,
        rt_tick_from_millisecond(10));
    rt_thread_startup(tid);

    return 0;
}

static int set_governor(k_pm_governor governor)
{
    if (governor == PM_GOVERNOR_MANUAL ||
        governor == PM_GOVERNOR_PERFORMANCE ||
        governor == PM_GOVERNOR_ENERGYSAVING ||
        governor == PM_GOVERNOR_AUTO) {
        pm_domain_cpu.governor = governor;

        auto_load_enable = governor == PM_GOVERNOR_AUTO ? true : false;

        if (governor == PM_GOVERNOR_MANUAL)
            memset(profile_lock, 0, sizeof(profile_lock));

        return 0;
    }

    return K_ERR_PM_ILLEGAL_PARAM;
}

static int get_expect_profile(void)
{
    if (pm_domain_cpu.governor != PM_GOVERNOR_AUTO)
        return -1;

    pm_domain_cpu.expect_profile_idx = auto_governor_get_profile();

    return 0;
}

static int set_clock(int32_t freq)
{
    int ret;

    switch (freq) {
    case (int)1.6e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL0, 1, 1);
        break;
    case (int)1.188e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL1_DIV_2, 1, 1);
        break;
    case (int)0.8e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL0, 1, 2);
        break;
    case (int)0.594e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL1_DIV_2, 1, 2);
        break;
    case (int)0.4e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL0, 1, 4);
        break;
    case (int)0.2e9:
        ret = sysctl_media_clk_set_leaf_parent_div(SYSCTL_CLK_CPU_1_SRC,
            SYSCTL_CLK_ROOT_PLL0, 1, 8);
        break;
    default:
        ret = K_ERR_PM_ILLEGAL_PARAM;
        break;
    }

    return ret ? 0 : K_ERR_PM_ILLEGAL_PARAM;
}

static int get_stat(void)
{

}

static struct pm_domain_ops ops = {
    .init = init,
    .set_governor = set_governor,
    .get_expect_profile = get_expect_profile,
    .set_clock = set_clock,
    .get_stat = get_stat,
};

struct pm_domain_dev pm_domain_cpu = {
    .profiles = profiles,
    .profile_nr = ARRAY_SIZE(profiles),
    .profile_lock = profile_lock,
    .regulator_name = "regulator_cpu",
    .ops = &ops,
};
