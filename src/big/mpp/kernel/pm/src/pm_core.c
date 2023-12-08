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

#include <rtthread.h>
#include <rtdevice.h>
#include <ioremap.h>
#include <sys/mman.h>
#include <dfs_file.h>
#include <lwp_user_mm.h>
#include "k_pm_ioctl.h"
#include "pm_domain.h"
#define DBG_TAG  "pm_core"
#ifdef RT_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_WARNING
#endif
#define DBG_COLOR
#include <rtdbg.h>

#define COPY_FROM_USER(dst, src, size)                                      \
    (size != lwp_get_from_user(dst, src, size))
#define COPY_TO_USER(dst, src, size)                                        \
    (size != lwp_put_to_user(dst, src, size))

#define THERMAL_DETECT_POLL_TIME 5000 // ms

struct pm_dev {
    struct rt_device parent;
    struct rt_mutex mutex;
    struct pm_domain_dev *dev[PM_DOMAIN_NR];
    int32_t thermal_shutdown_temp;
};

static struct pm_dev pm_device = {
    .dev[PM_DOMAIN_CPU] = &pm_domain_cpu,
    .dev[PM_DOMAIN_KPU] = &pm_domain_kpu,
    .dev[PM_DOMAIN_DPU] = &pm_domain_dpu,
    .dev[PM_DOMAIN_VPU] = &pm_domain_vpu,
    .dev[PM_DOMAIN_DISPLAY] = &pm_domain_disp,
};

static int pm_core_default(void)
{
    pm_device.thermal_shutdown_temp = 20000;

    pm_device.dev[PM_DOMAIN_CPU]->expect_profile_idx = 0;
    pm_device.dev[PM_DOMAIN_CPU]->actual_profile_idx = 0;
    pm_device.dev[PM_DOMAIN_CPU]->governor = PM_GOVERNOR_MANUAL;
    pm_device.dev[PM_DOMAIN_CPU]->thermal_protect_temp = 20000;
    pm_device.dev[PM_DOMAIN_CPU]->thermal_protect_idx = 0;

    pm_device.dev[PM_DOMAIN_KPU]->expect_profile_idx = 0;
    pm_device.dev[PM_DOMAIN_KPU]->actual_profile_idx = 0;
    pm_device.dev[PM_DOMAIN_KPU]->governor = PM_GOVERNOR_MANUAL;
    pm_device.dev[PM_DOMAIN_KPU]->thermal_protect_temp = 20000;
    pm_device.dev[PM_DOMAIN_KPU]->thermal_protect_idx = 0;

    return 0;
}

static int pm_core_domain_check(k_pm_domain domain)
{
    if (domain >= PM_DOMAIN_NR)
        return K_ERR_PM_ILLEGAL_PARAM;

    if (!pm_device.dev[domain])
        return K_ERR_PM_UNEXIST;

    return 0;
}

static int pm_core_get_reg(void *args)
{
    return 0;
}

static int pm_core_set_reg(void *args)
{
    return 0;
}

static int pm_core_get_stat(void *args)
{
    return 0;
}

static int pm_core_get_profiles(void *args)
{
    int ret;
    _k_pm_domain_profiles profiles;

    if (COPY_FROM_USER(&profiles, args, sizeof(_k_pm_domain_profiles)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(profiles.domain);
    if (ret)
        return ret;

    if (profiles.number == 0) {
        profiles.number = pm_device.dev[profiles.domain]->profile_nr;
        if (COPY_TO_USER(args, &profiles, sizeof(_k_pm_domain_profiles)))
            return K_ERR_PM_ILLEGAL_PARAM;
    } else {
        if (profiles.number > pm_device.dev[profiles.domain]->profile_nr)
            profiles.number = pm_device.dev[profiles.domain]->profile_nr;
        if (COPY_TO_USER(args, &profiles, sizeof(_k_pm_domain_profiles)))
            return K_ERR_PM_ILLEGAL_PARAM;
        if (COPY_TO_USER(((_k_pm_domain_profiles *)args)->profiles,
                pm_device.dev[profiles.domain]->profiles,
                sizeof(k_pm_profile) * profiles.number))
            return K_ERR_PM_ILLEGAL_PARAM;
    }

    return 0;
}

static int pm_core_set_governor(void *args)
{
    int ret;
    _k_pm_domain_governor governor;

    if (COPY_FROM_USER(&governor, args, sizeof(_k_pm_domain_governor)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(governor.domain);
    if (ret)
        return ret;

    ret = pm_domain_set_governor(pm_device.dev[governor.domain],
        governor.governor);
    if (ret)
        return ret;

    ret = pm_domain_get_expect_profile(pm_device.dev[governor.domain]);
    if (ret)
        return ret;

    ret = pm_domain_set_actual_profile(pm_device.dev[governor.domain]);

    return ret;
}

static int pm_core_get_governor(void *args)
{
    int ret;
    _k_pm_domain_governor governor;

    if (COPY_FROM_USER(&governor, args, sizeof(_k_pm_domain_governor)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(governor.domain);
    if (ret)
        return ret;

    governor.governor = pm_device.dev[governor.domain]->governor;

    if (COPY_TO_USER(args, &governor, sizeof(_k_pm_domain_governor)))
        return K_ERR_PM_ILLEGAL_PARAM;

    return 0;
}

static int pm_core_set_profile(void *args)
{
    int ret;
    _k_pm_domain_profile profile;

    if (COPY_FROM_USER(&profile, args, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(profile.domain);
    if (ret)
        return ret;

    ret = pm_domain_set_expect_profile(pm_device.dev[profile.domain],
        profile.index);
    if (ret)
        return ret;

    ret = pm_domain_set_actual_profile(pm_device.dev[profile.domain]);
    if (ret)
        return ret;

    if (COPY_TO_USER(args, &profile, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    return 0;
}

static int pm_core_get_profile(void *args)
{
    int ret;
    _k_pm_domain_profile profile;

    if (COPY_FROM_USER(&profile, args, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(profile.domain);
    if (ret)
        return ret;

    ret = pm_domain_get_actual_profile(pm_device.dev[profile.domain],
        &profile.index);
    if (ret)
        return ret;

    if (COPY_TO_USER(args, &profile, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    return 0;
}

static int pm_core_set_profile_lock(void *args)
{
    int ret;
    _k_pm_domain_profile profile;

    if (COPY_FROM_USER(&profile, args, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(profile.domain);
    if (ret)
        return ret;

    ret = pm_domain_set_expect_profile_lock(pm_device.dev[profile.domain],
        profile.index);
    if (ret)
        return ret;

    ret = pm_domain_set_actual_profile(pm_device.dev[profile.domain]);

    return ret;
}

static int pm_core_set_profile_unlock(void *args)
{
    int ret;
    _k_pm_domain_profile profile;

    if (COPY_FROM_USER(&profile, args, sizeof(_k_pm_domain_profile)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(profile.domain);
    if (ret)
        return ret;

    ret = pm_domain_set_expect_profile_unlock(pm_device.dev[profile.domain],
        profile.index);
    if (ret)
        return ret;

    ret = pm_domain_set_actual_profile(pm_device.dev[profile.domain]);

    return ret;
}

static int pm_core_set_thermal_protect(void *args)
{
    int ret;
    _k_pm_domain_thermal_protect thermal;

    if (COPY_FROM_USER(&thermal, args, sizeof(_k_pm_domain_thermal_protect)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(thermal.domain);
    if (ret)
        return ret;

    if (thermal.domain != PM_DOMAIN_CPU && thermal.domain != PM_DOMAIN_KPU)
        return K_ERR_PM_NOT_SUPPORT;

    pm_device.dev[thermal.domain]->thermal_protect_temp = thermal.temp;
    pm_device.dev[thermal.domain]->thermal_protect_idx = thermal.index;

    return 0;
}

static int pm_core_get_thermal_protect(void *args)
{
    int ret;
    _k_pm_domain_thermal_protect thermal;

    if (COPY_FROM_USER(&thermal, args, sizeof(_k_pm_domain_thermal_protect)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_core_domain_check(thermal.domain);
    if (ret)
        return ret;

    if (thermal.domain != PM_DOMAIN_CPU && thermal.domain != PM_DOMAIN_KPU)
        return K_ERR_PM_NOT_SUPPORT;

    thermal.temp = pm_device.dev[thermal.domain]->thermal_protect_temp;
    thermal.index = pm_device.dev[thermal.domain]->thermal_protect_idx;

    if (COPY_TO_USER(args, &thermal, sizeof(_k_pm_domain_thermal_protect)))
        return K_ERR_PM_ILLEGAL_PARAM;

    return 0;
}

static int pm_core_set_thermal_shutdown(void *args)
{
    int ret;
    int32_t temp;

    if (COPY_FROM_USER(&temp, args, sizeof(int32_t)))
        return K_ERR_PM_ILLEGAL_PARAM;

    pm_device.thermal_shutdown_temp = temp;

    return 0;
}

static int pm_core_get_thermal_shutdown(void *args)
{
    int ret;
    int32_t temp;

    if (COPY_FROM_USER(&temp, args, sizeof(int32_t)))
        return K_ERR_PM_ILLEGAL_PARAM;

    temp = pm_device.thermal_shutdown_temp;

    if (COPY_TO_USER(args, &temp, sizeof(int32_t)))
        return K_ERR_PM_ILLEGAL_PARAM;

    return 0;
}

static int pm_core_set_clock(void *args)
{
    int ret;
    _k_pm_domain_bool enable;

    if (COPY_FROM_USER(&enable, args, sizeof(_k_pm_domain_bool)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_domain_enable_clock(pm_device.dev[enable.domain], enable.enable);

    return ret;
}

static int pm_core_set_power(void *args)
{
    int ret;
    _k_pm_domain_bool enable;

    if (COPY_FROM_USER(&enable, args, sizeof(_k_pm_domain_bool)))
        return K_ERR_PM_ILLEGAL_PARAM;

    ret = pm_domain_enable_power(pm_device.dev[enable.domain], enable.enable);

    return ret;
}

static int pm_core_open(struct dfs_fd *file)
{
    return 0;
}

static int pm_core_close(struct dfs_fd *file)
{
    return 0;
}

static int pm_core_ioctl(struct dfs_fd *file, int cmd, void *args)
{
    int ret;

    rt_mutex_take(&pm_device.mutex, RT_WAITING_FOREVER);
    switch (cmd) {
    case K_IOC_PM_SET_REG:
        ret = pm_core_get_reg(args);
        break;
    case K_IOC_PM_GET_REG:
        ret = pm_core_set_reg(args);
        break;
    case K_IOC_PM_GET_PROFILES:
        ret = pm_core_get_profiles(args);
        break;
    case K_IOC_PM_GET_STAT:
        ret = pm_core_get_stat(args);
        break;
    case K_IOC_PM_SET_GOVERNOR:
        ret = pm_core_set_governor(args);
        break;
    case K_IOC_PM_GET_GOVERNOR:
        ret = pm_core_get_governor(args);
        break;
    case K_IOC_PM_SET_PROFILE:
        ret = pm_core_set_profile(args);
        break;
    case K_IOC_PM_GET_PROFILE:
        ret = pm_core_get_profile(args);
        break;
    case K_IOC_PM_SET_PROFILE_LOCK:
        ret = pm_core_set_profile_lock(args);
        break;
    case K_IOC_PM_SET_PROFILE_UNLOCK:
        ret = pm_core_set_profile_unlock(args);
        break;
    case K_IOC_PM_SET_THERMAL_PROTECT:
        ret = pm_core_set_thermal_protect(args);
        break;
    case K_IOC_PM_GET_THERMAL_PROTECT:
        ret = pm_core_get_thermal_protect(args);
        break;
    case K_IOC_PM_SET_THERMAL_SHUTDOWN:
        ret = pm_core_set_thermal_shutdown(args);
        break;
    case K_IOC_PM_GET_THERMAL_SHUTDOWN:
        ret = pm_core_get_thermal_shutdown(args);
        break;
    case K_IOC_PM_SET_CLOCK:
        ret = pm_core_set_clock(args);
        break;
    case K_IOC_PM_SET_POWER:
        ret = pm_core_set_power(args);
        break;
    default:
        ret = -EPERM;
        break;
    }
    rt_mutex_release(&pm_device.mutex);

    return ret;
}

static const struct dfs_file_ops pm_core_fops = {
    .open = pm_core_open,
    .close = pm_core_close,
    .ioctl = pm_core_ioctl,
};

static int32_t get_temperature(void)
{
    // todo get temperature
    return 0;
}

static void shutdown(void)
{
    pm_domain_enable_power(pm_device.dev[PM_DOMAIN_KPU], false);
    pm_domain_enable_power(pm_device.dev[PM_DOMAIN_DPU], false);
    pm_domain_enable_power(pm_device.dev[PM_DOMAIN_VPU], false);
    // todo poweroff
}

static void thermal_detect_thread(void *arg)
{
    while (1) {
        rt_thread_mdelay(THERMAL_DETECT_POLL_TIME);

        int32_t temp = get_temperature();

        for (int i = PM_DOMAIN_CPU; i < PM_DOMAIN_NR; i++) {
            if (pm_device.dev[i] == NULL)
                continue;
            if (i != PM_DOMAIN_CPU && i != PM_DOMAIN_KPU)
                continue;
            pm_device.dev[i]->thermal_over =
                temp > pm_device.dev[i]->thermal_protect_temp ? true : false;
            pm_domain_set_actual_profile(pm_device.dev[i]);
        }

        if (temp > pm_device.thermal_shutdown_temp)
            shutdown();
    }
}

int pm_core_init(void)
{
    int ret;
    rt_thread_t tid;

    ret = pm_core_default();
    if (ret) {
        LOG_E("pm_core_default error\n");
        return K_ERR_PM_ILLEGAL_PARAM;
    }

    for (int i = PM_DOMAIN_CPU; i < PM_DOMAIN_NR; i++) {
        if (pm_device.dev[i] == NULL)
            continue;
        ret = pm_domain_init(pm_device.dev[i]);
        if (ret !=0) {
            LOG_E("domain %d init error\n", i);
            return ret;
        }
        rt_mutex_init(&pm_device.dev[i]->mutex, "pm_mutex", RT_IPC_FLAG_PRIO);
        if (pm_device.dev[i]->regulator_name) {
            pm_device.dev[i]->regulator = (struct regulator_dev *)
                rt_device_find(pm_device.dev[i]->regulator_name);
            if (pm_device.dev[i]->regulator == RT_NULL) {
                LOG_E("Can't find %s device\n",
                    pm_device.dev[i]->regulator_name);
                return K_ERR_PM_UNEXIST;
            }
        }
        ret = pm_domain_set_governor(pm_device.dev[i],
            pm_device.dev[i]->governor);
        if (ret !=0 && ret != K_ERR_PM_NOT_SUPPORT) {
            LOG_E("domain %d set_governor error\n", i);
            return ret;
        }
    }

    ret = rt_device_register(&pm_device.parent, "pm", RT_DEVICE_FLAG_RDWR);
    if (ret) {
        LOG_E("regulator register fail: %d\n", ret);
        return -1;
    }

    pm_device.parent.fops = &pm_core_fops;
    ret = rt_mutex_init(&pm_device.mutex, "pm_mutex", RT_IPC_FLAG_PRIO);

    tid = rt_thread_create("thermal_detect_thread", thermal_detect_thread,
        RT_NULL, 1024 * 10, RT_THREAD_PRIORITY_MAX / 2,
        rt_tick_from_millisecond(10));
    rt_thread_startup(tid);

    // temporary fix
    pm_domain_enable_clock(pm_device.dev[PM_DOMAIN_KPU], true);
    pm_domain_enable_power(pm_device.dev[PM_DOMAIN_KPU], true);

    return ret;
}
