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

#include "k_type.h"
#include "k_mmz_comm.h"
#include "mpi_sys_api.h"

int main(void)
{
    k_sys_virmem_info info;
    k_u64 phyaddr0;
    k_u64 phyaddr1;
    k_u64 phyaddr2;
    void *virtaddr0 = NULL;
    void *virtaddr1 = NULL;
    void *virtaddr2 = NULL;
    k_s32 ret;
    k_s32 i;

    k_char test_str[16] = "\"hello world\"";

    ret = kd_mpi_sys_mmz_alloc(&phyaddr0, &virtaddr0, "test0", "anonymous", 4096 * 513);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_alloc return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }
    else
    {
        printf("\nalloc result phyaddr:0x%lx virtaddr:%p\n", phyaddr0, virtaddr0);
    }

    ret = kd_mpi_sys_mmz_alloc(&phyaddr1, &virtaddr1, "test1", "anonymous", 4096 * 2);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_alloc return 0x%08x\n", ret);
        kd_mpi_sys_mmz_free(phyaddr0, NULL);
        goto sample_mmz_failed;
    }
    else
    {
        printf("alloc result phyaddr:0x%lx virtaddr:%p\n", phyaddr1, virtaddr1);
    }

    memcpy(virtaddr1, test_str, 16);
    printf("write test string %s in virt addr:%p \n", (k_char *)virtaddr1, virtaddr1);

    ret = kd_mpi_sys_mmz_free(phyaddr1, NULL);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_free return 0x%08x\n", ret);
        kd_mpi_sys_mmz_free(phyaddr0, NULL);
        goto sample_mmz_failed;
    }
    printf("free phys addr:0x%lx \n", phyaddr1);

    ret = kd_mpi_sys_mmz_free(phyaddr0, NULL);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_free return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }
    printf("free phys addr:0x%lx \n", phyaddr0);

    ret = kd_mpi_sys_mmz_alloc(&phyaddr2, &virtaddr2, "test2", "anonymous", 4096 * 4096);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_alloc return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }
    printf("alloc result phyaddr:0x%lx virtaddr:%p\n", phyaddr2, virtaddr2);

    virtaddr2 += (phyaddr1 - phyaddr0);
    ret = kd_mpi_sys_get_virmem_info(virtaddr2, &info);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_get_virmem_info return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }
    else
    {
        printf("get memory info by virtaddr:%p phy addr:0x0x%lx icached:%d \n", virtaddr2, info.phy_addr, info.cached);
        printf("get test string %s in virtaddr:%p \n", (k_char *)virtaddr2,  virtaddr2);
    }
    ret = kd_mpi_sys_mmz_free(phyaddr2, NULL);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_free return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }

    ret = kd_mpi_sys_mmz_alloc_cached(&phyaddr2, &virtaddr2, "test2", "anonymous", 4096 * 4096);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_alloc_cached return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }
    printf("alloc cached result phyaddr:0x%lx virtaddr:%p\n", phyaddr2, virtaddr2);

    k_u32 *write_addr = (k_u32 *)virtaddr2;
    for (i = 0; i < 4096 * 4096 / 4; i++)
    {
        write_addr[i] = i * 4;
    }

    ret = kd_mpi_sys_mmz_flush_cache(phyaddr2, virtaddr2, 4096 * 4096);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_flush_cache return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }

    ret = kd_mpi_sys_mmz_free(phyaddr2, virtaddr2);
    if (ret != K_ERR_OK)
    {
        printf("[ERROR]: kd_mpi_sys_mmz_free return 0x%08x\n", ret);
        goto sample_mmz_failed;
    }

    printf("sample mmz test success\n");
    return 0;
sample_mmz_failed:
    printf("sample mmz test failed\n");

}
