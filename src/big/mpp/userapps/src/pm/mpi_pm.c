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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "k_pm_ioctl.h"
#include "mpi_pm_api.h"

static int g_pm_fd = -1;
static pthread_mutex_t g_pm_mutex = PTHREAD_MUTEX_INITIALIZER;

static int pm_check_open(void)
{
    if (g_pm_fd >= 0)
        return 0;

    pthread_mutex_lock(&g_pm_mutex);
    if (g_pm_fd < 0) {
        g_pm_fd = open("/dev/pm", O_RDWR);
        if (g_pm_fd < 0) {
            perror("open err\n");
            pthread_mutex_unlock(&g_pm_mutex);
            return -errno;
        }
    }
    pthread_mutex_unlock(&g_pm_mutex);

    return 0;
}

int kd_mpi_pm_set_reg(uint32_t reg, uint32_t val)
{
    return 0;
}

int kd_mpi_pm_get_reg(uint32_t reg, uint32_t *pval)
{
    return 0;
}

int kd_mpi_pm_get_profiles(k_pm_domain domain, uint32_t *pcount,
    k_pm_profile *pprofile)
{
    int ret;
    _k_pm_domain_profiles *profiles;

    ret = pm_check_open();
    if (ret)
        return ret;

    if ((!pcount) || (*pcount && !pprofile))
        return K_ERR_PM_NULL_PTR;
    if (*pcount > 128)
        *pcount = 128;
    profiles = malloc(sizeof(_k_pm_domain_profiles) + *pcount *
        sizeof(k_pm_profile));
    if (!profiles)
        return K_ERR_PM_NOMEM;

    profiles->domain = domain;
    profiles->number = *pcount;

    ret = ioctl(g_pm_fd, K_IOC_PM_GET_PROFILES, profiles);
    if (ret) {
        free(profiles);
        return ret;
    }

    if (*pcount)
        memcpy(pprofile, profiles->profiles, profiles->number *
            sizeof(k_pm_profile));
    *pcount = profiles->number;
    free(profiles);

    return 0;
}

int kd_mpi_pm_get_stat(k_pm_domain domain)
{
    return 0;
}

int kd_mpi_pm_set_governor(k_pm_domain domain, k_pm_governor governor)
{
    int ret;
    _k_pm_domain_governor gov;

    ret = pm_check_open();
    if (ret)
        return ret;

    gov.domain = domain;
    gov.governor = governor;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_GOVERNOR, &gov);

    return ret;
}

int kd_mpi_pm_get_governor(k_pm_domain domain, k_pm_governor *pgovernor)
{
    int ret;
    _k_pm_domain_governor gov;

    ret = pm_check_open();
    if (ret)
        return ret;

    gov.domain = domain;

    ret = ioctl(g_pm_fd, K_IOC_PM_GET_GOVERNOR, &gov);
    *pgovernor = gov.governor;

    return ret;
}

int kd_mpi_pm_set_profile(k_pm_domain domain, int32_t index)
{
    int ret;
    _k_pm_domain_profile profile;

    ret = pm_check_open();
    if (ret)
        return ret;

    profile.domain = domain;
    profile.index = index;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_PROFILE, &profile);

    return ret;
}

int kd_mpi_pm_get_profile(k_pm_domain domain, int32_t *pindex)
{
    int ret;
    _k_pm_domain_profile profile;

    ret = pm_check_open();
    if (ret)
        return ret;

    profile.domain = domain;

    ret = ioctl(g_pm_fd, K_IOC_PM_GET_PROFILE, &profile);
    *pindex = profile.index;

    return ret;
}

int kd_mpi_pm_set_profile_lock(k_pm_domain domain, int32_t index)
{
    int ret;
    _k_pm_domain_profile profile;

    ret = pm_check_open();
    if (ret)
        return ret;

    profile.domain = domain;
    profile.index = index;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_PROFILE_LOCK, &profile);

    return ret;
}

int kd_mpi_pm_set_profile_unlock(k_pm_domain domain, int32_t index)
{
    int ret;
    _k_pm_domain_profile profile;

    ret = pm_check_open();
    if (ret)
        return ret;

    profile.domain = domain;
    profile.index = index;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_PROFILE_UNLOCK, &profile);

    return ret;
}

int kd_mpi_pm_set_thermal_protect(k_pm_domain domain, int32_t temp,
    int32_t index)
{
    int ret;
    _k_pm_domain_thermal_protect thermal;

    ret = pm_check_open();
    if (ret)
        return ret;

    thermal.domain = domain;
    thermal.temp = temp;
    thermal.index = index;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_THERMAL_PROTECT, &thermal);

    return ret;
}

int kd_mpi_pm_get_thermal_protect(k_pm_domain domain, int32_t *ptemp,
    int32_t *pindex)
{
    int ret;
    _k_pm_domain_thermal_protect thermal;

    ret = pm_check_open();
    if (ret)
        return ret;

    thermal.domain = domain;

    ret = ioctl(g_pm_fd, K_IOC_PM_GET_THERMAL_PROTECT, &thermal);
    *ptemp = thermal.temp;
    *pindex = thermal.index;

    return ret;
}

int kd_mpi_pm_set_thermal_shutdown(int32_t temp)
{
    int ret;

    ret = pm_check_open();
    if (ret)
        return ret;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_THERMAL_SHUTDOWN, &temp);

    return ret;
}

int kd_mpi_pm_get_thermal_shutdown(int32_t *ptemp)
{
    int ret;
    int32_t temp;

    ret = pm_check_open();
    if (ret)
        return ret;

    ret = ioctl(g_pm_fd, K_IOC_PM_GET_THERMAL_SHUTDOWN, &temp);
    *ptemp = temp;

    return ret;
}

int kd_mpi_pm_set_clock(k_pm_domain domain, bool enable)
{
    int ret;
    _k_pm_domain_bool domain_bool;

    ret = pm_check_open();
    if (ret)
        return ret;

    domain_bool.domain = domain;
    domain_bool.enable = enable;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_CLOCK, &domain_bool);

    return ret;
}

int kd_mpi_pm_set_power(k_pm_domain domain, bool enable)
{
    int ret;
    _k_pm_domain_bool domain_bool;

    ret = pm_check_open();
    if (ret)
        return ret;

    domain_bool.domain = domain;
    domain_bool.enable = enable;

    ret = ioctl(g_pm_fd, K_IOC_PM_SET_POWER, &domain_bool);

    return ret;
}

__attribute__((weak)) int kd_mpi_pm_runtime_runstage(k_runtimestage_id stage)
{
    int ret = 0;

    if (stage == RUNTIMESTAGE_ID_AI2D_START) {
        ret = kd_mpi_pm_set_clock(PM_DOMAIN_KPU, true);
        if (!ret)
            ret = kd_mpi_pm_set_power(PM_DOMAIN_KPU, true);
    } else if (stage == RUNTIMESTAGE_ID_AI2D_STOP) {
        ret = kd_mpi_pm_set_power(PM_DOMAIN_KPU, false);
        if (!ret)
            ret = kd_mpi_pm_set_clock(PM_DOMAIN_KPU, false);
    } else if (stage == RUNTIMESTAGE_ID_KPU_START) {
        ret = kd_mpi_pm_set_clock(PM_DOMAIN_KPU, true);
        if (!ret)
            ret = kd_mpi_pm_set_power(PM_DOMAIN_KPU, true);
    } else if (stage == RUNTIMESTAGE_ID_KPU_STOP) {
        ret = kd_mpi_pm_set_power(PM_DOMAIN_KPU, false);
        if (!ret)
            ret = kd_mpi_pm_set_clock(PM_DOMAIN_KPU, false);
    } else {
        ret = 0;
    }

    return ret;
}
