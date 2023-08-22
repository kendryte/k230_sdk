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
#include <unistd.h>
#include <string.h>
#include "mpi_pm_api.h"

static const char* domain_name[] = {
    "CPU_DOMAIN",
    "KPU_DOMAIN",
    "DPU_DOMAIN",
    "VPU_DOMAIN",
    "DISPLAY_DOMAIN",
    "MEDIA_DOMAIN",
};

static const char* governor_name[] = {
    "MANUAL_GOVERNOR",
    "PERFORMANCE_GOVERNOR",
    "ENERGYSAVING_GOVERNOR",
    "AUTO_GOVERNOR",
};

static int sample_pm_set_reg(int argc, char **argv)
{
    return 0;
}

static int sample_pm_get_reg(int argc, char **argv)
{
    return 0;
}

static int sample_pm_get_profiles(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    uint32_t count = strtoul(argv[1], NULL, 0);
    count = count > 128 ? 128 : count;

    if (count) {
        k_pm_profile *profiles = malloc(sizeof(k_pm_profile) * count);
        ret = kd_mpi_pm_get_profiles(domain, &count, profiles);
        if (ret) {
            printf("kd_mpi_pm_get_profiles error: %#x\n", ret);
            return ret;
        }
        printf("%s profiles info:\n", domain_name[domain]);
        for (int i = 0; i < count; i++)
            printf("profile: %d, freq: %10d Hz, volt: %7d uV\n", i,
                profiles[i].freq, profiles[i].volt);
    } else {
        ret = kd_mpi_pm_get_profiles(domain, &count, NULL);
        if (ret) {
            printf("kd_mpi_pm_get_profiles error: %#x\n", ret);
            return ret;
        }
        printf("%s profile_count: %u\n", domain_name[domain], count);
    }

    return 0;
}

static int sample_pm_get_stat(int argc, char **argv)
{
    return 0;
}

static int sample_pm_set_governor(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    k_pm_governor governor = strtoul(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_governor(domain, governor);
    if (ret) {
        printf("kd_mpi_pm_set_governor error: %#x\n", ret);
        return ret;
    }
    printf("%s set %s\n", domain_name[domain], governor_name[governor]);

    return 0;
}

static int sample_pm_get_governor(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    k_pm_governor governor;

    ret = kd_mpi_pm_get_governor(domain, &governor);
    if (ret) {
        printf("kd_mpi_pm_get_governor error: %#x\n", ret);
        return ret;
    }
    printf("%s current governor is %s\n", domain_name[domain],
        governor_name[governor]);

    return 0;
}

static int sample_pm_set_profile(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t index = strtol(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_profile(domain, index);
    if (ret) {
        printf("kd_mpi_pm_set_profile error: %#x\n", ret);
        return ret;
    }
    printf("%s set profile %d\n", domain_name[domain], index);

    return 0;
}

static int sample_pm_get_profile(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t index;

    ret = kd_mpi_pm_get_profile(domain, &index);
    if (ret) {
        printf("kd_mpi_pm_get_profile error: %#x\n", ret);
        return ret;
    }
    printf("%s current profile is %d\n", domain_name[domain], index);

    return 0;
}

static int sample_pm_set_profile_lock(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t index = strtol(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_profile_lock(domain, index);
    if (ret) {
        printf("kd_mpi_pm_set_profile_lock error: %#x\n", ret);
        return ret;
    }
    printf("%s set profile %d\n", domain_name[domain], index);

    return 0;
}

static int sample_pm_set_profile_unlock(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t index = strtol(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_profile_unlock(domain, index);
    if (ret) {
        printf("kd_mpi_pm_set_profile_unlock error: %#x\n", ret);
        return ret;
    }
    printf("%s set profile %d\n", domain_name[domain], index);

    return 0;
}

static int sample_pm_set_thermal_protect(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t temp = strtol(argv[1], NULL, 0);
    int32_t index = strtol(argv[2], NULL, 0);

    ret = kd_mpi_pm_set_thermal_protect(domain, temp, index);
    if (ret) {
        printf("kd_mpi_pm_set_thermal_protect error: %#x\n", ret);
        return ret;
    }
    printf("%s set thermal protect %.2f 'C limit profile %d\n",
        domain_name[domain], temp * 0.01, index);

    return 0;
}

static int sample_pm_get_thermal_protect(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    int32_t temp;
    int32_t index;

    ret = kd_mpi_pm_get_thermal_protect(domain, &temp, &index);
    if (ret) {
        printf("kd_mpi_pm_get_thermal_protect error: %#x\n", ret);
        return ret;
    }
    printf("%s current thermal protect is %.2f 'C limit profile %d\n",
        domain_name[domain], temp * 0.01, index);

    return 0;
}

static int sample_pm_set_thermal_shutdown(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    int32_t temp = strtol(argv[0], NULL, 0);

    ret = kd_mpi_pm_set_thermal_shutdown(temp);
    if (ret) {
        printf("kd_mpi_pm_set_thermal_shutdown error: %#x\n", ret);
        return ret;
    }
    printf("CHIP set thermal shutdown %.2f 'C\n", temp * 0.01);

    return 0;
}

static int sample_pm_get_thermal_shutdown(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    int32_t temp;

    ret = kd_mpi_pm_get_thermal_shutdown(&temp);
    if (ret) {
        printf("kd_mpi_pm_get_thermal_shutdown error: %#x\n", ret);
        return ret;
    }
    printf("CHIP current thermal shutdown is %.2f 'C\n", temp * 0.01);

    return 0;
}

static int sample_pm_set_clock(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    bool enable = !!strtoul(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_clock(domain, enable);
    if (ret) {
        printf("kd_mpi_pm_set_clock error: %#x\n", ret);
        return ret;
    }
    printf("%s %s clock\n", domain_name[domain], enable ? "enable" : "disable");

    return 0;
}

static int sample_pm_set_power(int argc, char **argv)
{
    printf("%s\n", __func__);

    int ret;
    k_pm_domain domain = strtoul(argv[0], NULL, 0);
    bool enable = !!strtoul(argv[1], NULL, 0);

    ret = kd_mpi_pm_set_power(domain, enable);
    if (ret) {
        printf("kd_mpi_pm_set_power error: %#x\n", ret);
        return ret;
    }
    printf("%s %s power\n", domain_name[domain], enable ? "enable" : "disable");

    return 0;
}

struct {
    int (*func)(int argc, char **argv);
    int argc;
    const char *name;
    const char *desc;
} sample_func_list[] = {
    {sample_pm_set_reg, 2, "sample_pm_set_reg", "addr data"},
    {sample_pm_get_reg, 1, "sample_pm_get_reg", "addr"},
    {sample_pm_get_profiles, 2, "sample_pm_get_profiles", "domain count"},
    {sample_pm_get_stat, 0, "sample_pm_get_stat", ""},
    {sample_pm_set_governor, 2, "sample_pm_set_governor", "domain gov"},
    {sample_pm_get_governor, 1, "sample_pm_get_governor", "domain"},
    {sample_pm_set_profile, 2, "sample_pm_set_profile", "domain profile"},
    {sample_pm_get_profile, 1, "sample_pm_get_profile", "domain"},
    {sample_pm_set_profile_lock, 2, "sample_pm_set_profile_lock",
        "domain profile"},
    {sample_pm_set_profile_unlock, 2, "sample_pm_set_profile_unlock",
        "domain profile"},
    {sample_pm_set_thermal_protect, 3, "sample_pm_set_thermal_protect",
        "domain temp profile"},
    {sample_pm_get_thermal_protect, 1, "sample_pm_get_thermal_protect",
        "domain"},
    {sample_pm_set_thermal_shutdown, 1, "sample_pm_set_thermal_shutdown",
        "temp"},
    {sample_pm_get_thermal_shutdown, 0, "sample_pm_get_thermal_shutdown", ""},
    {sample_pm_set_clock, 2, "sample_pm_set_clock", "domain enable"},
    {sample_pm_set_power, 2, "sample_pm_set_power", "domain enable"},
};

static void show_help(void)
{
    printf("Usage: sample_pm func_index [opt]\n");
    for (int i = 0; i < sizeof(sample_func_list) / sizeof(sample_func_list[0]);
        i++)
        printf("\t%2d: %s, opt: %s\n", i, sample_func_list[i].name,
            sample_func_list[i].desc);
    printf("\tother: %s\n", sample_func_list[0].name);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        show_help();
        return -1;
    }

    uint32_t func_index = strtoul(argv[1], NULL, 0);
    if (func_index > sizeof(sample_func_list) / sizeof(sample_func_list[0]))
        func_index = 0;
    argc -= 2;
    if (argc != sample_func_list[func_index].argc) {
        show_help();
        return -1;
    }
    sample_func_list[func_index].func(argc, &argv[2]);

    return 0;
}
