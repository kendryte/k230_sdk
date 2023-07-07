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
#include <time.h>

#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"

#define VB_TEST_COUNT   2
#define VB_TEST_PIC_SIZE   (1080 * 1920 * 4)


int main(void)
{
    k_s32 ret = 0;
    int i = 0;
    int test_count = 0;
loop_test:
    if (test_count == VB_TEST_COUNT)
    {
        printf("vb test success!\n");
        return 0;
    }
    test_count++;
    k_vb_config config;
    k_vb_config get_config;
    memset(&config, 0, sizeof(config));
    memset(&get_config, 0, sizeof(get_config));

    config.max_pool_cnt = 10;
    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].blk_size = VB_TEST_PIC_SIZE;
    config.comm_pool[0].mode = VB_REMAP_MODE_CACHED;
    config.comm_pool[1].blk_cnt = 3;
    config.comm_pool[1].blk_size = 0x2000;
    config.comm_pool[1].mode = VB_REMAP_MODE_NONE;
    config.comm_pool[2].blk_cnt = 5;
    config.comm_pool[2].blk_size = 0x1000;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    ret = kd_mpi_vb_set_config(&config);
    printf("\n");
    printf("----------------------vb sample test---------------------------\n");
    printf("vb_set_config ret:0x%x\n", ret);

    ret = kd_mpi_vb_get_config(&get_config);
    printf("vb_get_config ret:0x%x\n", ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    if (memcmp(&get_config, &config, sizeof(k_vb_config)))
        goto vb_test_failed;

    printf("config.max_pool_cnt:%d\n", get_config.max_pool_cnt);
    for (i = 0; i < VB_MAX_COMM_POOLS; i++)
    {
        if (get_config.comm_pool[i].blk_cnt)
        {
            printf("config.comm_pool[%d].blk_cnt:%u\n", i, get_config.comm_pool[i].blk_cnt);
            printf("config.comm_pool[%d].blk_size:%lx\n", i,  get_config.comm_pool[i].blk_size);
            printf("config.comm_pool[%d].mode:%u\n", i,  get_config.comm_pool[i].mode);
        }
    }

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    printf("vb_set_supplement_conf ret:0x%x\n", ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    k_vb_supplement_config get_supplement_config;
    memset(&get_supplement_config, 0, sizeof(get_supplement_config));
    ret = kd_mpi_vb_get_supplement_config(&get_supplement_config);
    printf("vb_get_supplement_config ret:0x%x config:%08x\n", ret, get_supplement_config.supplement_config);
    if (K_FAILED == ret)
        goto vb_test_failed;
    if (memcmp(&get_supplement_config, &supplement_config, sizeof(k_vb_supplement_config)))
        goto vb_test_failed;

    ret = kd_mpi_vb_init();
    printf("vb_do_init ret:0x%x\n", ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    k_vb_mod_config mod_config;
    memset(&mod_config, 0, sizeof(mod_config));
    mod_config.uid = VB_UID_VDEC;
    mod_config.mod_config.max_pool_cnt = 2;
    mod_config.mod_config.comm_pool[0].blk_cnt = 5;
    mod_config.mod_config.comm_pool[0].blk_size = 0x1000;
    mod_config.mod_config.comm_pool[0].mode = VB_REMAP_MODE_CACHED;
    mod_config.mod_config.comm_pool[1].blk_cnt = 3;
    mod_config.mod_config.comm_pool[1].blk_size = 0x2000;
    mod_config.mod_config.comm_pool[1].mode = VB_REMAP_MODE_NONE;
    ret = kd_mpi_vb_set_mod_pool_config(mod_config.uid, &(mod_config.mod_config));
    printf("vb_set_mod_pool_config uid:%d ret:0x%x\n", mod_config.uid, ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    k_vb_mod_config get_mod_config;
    memset(&get_mod_config, 0, sizeof(get_mod_config));
    ret = kd_mpi_vb_get_mod_pool_config(mod_config.uid, &(get_mod_config.mod_config));
    printf("vb_get_mod_pool_config uid:%d ret:0x%x\n", mod_config.uid, ret);
    if (K_FAILED == ret)
        goto vb_test_failed;
    if (memcmp(&(mod_config.mod_config), &(get_mod_config.mod_config), sizeof(k_vb_config)))
        goto vb_test_failed;

    ret = kd_mpi_vb_init_mod_common_pool(VB_UID_VDEC);
    printf("vb_init_mod_common_pool ret:0x%x\n", ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    k_vb_pool_config pool_config;
    k_u32 pool_id;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = 4;
    pool_config.blk_size = 0x2000;
    pool_config.mode = VB_REMAP_MODE_CACHED;
    pool_id = kd_mpi_vb_create_pool(&pool_config);
    printf("vb_create_pool pool_id:%d\n", pool_id);
    if (pool_id == VB_INVALID_POOLID)
        goto vb_test_failed;

    k_vb_blk_handle handle;
    handle = kd_mpi_vb_get_block(pool_id, 0x800, NULL);
    printf("vb_get_block id:%d size:0x800 handle:%08x\n",
           pool_id, handle);
    if (handle == VB_INVALID_HANDLE)
        goto vb_test_failed;

    k_u32 pool_id_new;
    pool_id_new = kd_mpi_vb_handle_to_pool_id(handle);
    if (pool_id != pool_id_new)
    {
        printf("kd_mpi_vb_handle_to_pool_id failed!\n");
        goto vb_test_failed;
    }

    /*Proactively generate an error*/
    ret = kd_mpi_vb_destory_pool(pool_id);
    printf("vb_destory_pool pool_id:%d ret:0x%x\n", pool_id, ret);
    if (!ret)
        goto vb_test_failed;


    k_u64 phys_addr;
    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    printf("vb_handle_to_phyaddr handle:%08x addr:%lx\n", handle, phys_addr);
    if (0 == phys_addr)
    {
        goto vb_test_failed;
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    printf("vb_handle_to_pool_id handle:%08x pool_id:%d\n", handle, pool_id);
    if (pool_id == VB_INVALID_POOLID)
        goto vb_test_failed;

    handle = kd_mpi_vb_phyaddr_to_handle(phys_addr);
    printf("vb_phyaddr_to_handle handle:%08x phys_addr:%lx\n", handle, phys_addr);
    if (handle == VB_INVALID_HANDLE)
        goto vb_test_failed;

    k_video_supplement video_supp;
    memset(&video_supp, 0, sizeof(video_supp));
    ret = kd_mpi_vb_get_supplement_attr(handle, &video_supp);
    printf("vb_get_supplement_attr ret:0x%x \n", ret);
    printf("vb_get_supplement_attr jpeg_dcf_phy_addr:%lx\n", video_supp.jpeg_dcf_phy_addr);
    printf("vb_get_supplement_attr jpeg_dcf_kvirt_addr:%p\n", video_supp.jpeg_dcf_kvirt_addr);
    printf("vb_get_supplement_attr isp_info_phy_addr:%lx\n", video_supp.isp_info_phy_addr);
    printf("vb_get_supplement_attr isp_info_kvrit_addr:%p\n", video_supp.isp_info_kvirt_addr);
    if (ret)
        goto vb_test_failed;

    ret = kd_mpi_vb_inquire_user_cnt(handle);
    printf("vb_inquire_user_cnt handle:%08x ret:0x%x\n", handle, ret);
    if (K_FAILED == ret)
        goto vb_test_failed;

    ret = kd_mpi_vb_release_block(handle);
    printf("vb_release_block handle:%08x ret:0x%x\n", handle, ret);
    if (ret)
        goto vb_test_failed;

    ret = kd_mpi_vb_destory_pool(pool_id);
    printf("vb_destory_pool pool_id:%d ret:0x%x\n", pool_id, ret);
    if (ret)
        goto vb_test_failed;

    k_vb_blk_handle handle_arry[VB_MAX_POOLS];
    k_u32 *map_addr[VB_MAX_POOLS] = {NULL};
    k_s32 write_index = 0;
    struct timespec write_begain;
    struct timespec write_end;
    k_s32 read_index = 0;
    struct timespec read_begain;
    struct timespec read_end;
    k_u64 time_use, time_use_us, time_use_sec;
    volatile k_u32 read_val;

    memset(handle_arry, VB_INVALID_HANDLE, sizeof(handle_arry));
    i = 0;
    while (1)
    {
        handle_arry[i] = kd_mpi_vb_get_block(VB_INVALID_POOLID, VB_TEST_PIC_SIZE, NULL);
        if (handle_arry[i] == VB_INVALID_HANDLE)
        {
            break;
        }
        else
        {
            printf("kd_mpi_vb_get_block size:%d handle:%08x\n", VB_TEST_PIC_SIZE, handle_arry[i]);
            phys_addr = kd_mpi_vb_handle_to_phyaddr(handle_arry[i]);
            printf("vb_handle_to_phyaddr handle:%08x addr:%lx\n", handle_arry[i], phys_addr);
            if (0 == phys_addr)
            {
                printf("kd_mpi_vb_handle_to_phyaddr error\n");
                goto vb_test_failed;
            }
            map_addr[i] = kd_mpi_sys_mmap_cached(phys_addr, VB_TEST_PIC_SIZE);
            if (!map_addr[i])
            {
                printf("kd_mpi_sys_mmap_cached error\n");
                goto vb_test_failed;
            }
            printf("kd_mpi_sys_mmap_cache mmap addr:%p\n", map_addr[i]);

            clock_gettime(CLOCK_MONOTONIC, &write_begain);
            for (write_index = 0; write_index < (VB_TEST_PIC_SIZE / 4); write_index++)
            {
                map_addr[i][write_index] = write_index;
            }
            clock_gettime(CLOCK_MONOTONIC, &write_end);
            if (write_end.tv_nsec > write_begain.tv_nsec)
                time_use = write_end.tv_nsec - write_begain.tv_nsec;
            else
            {
                time_use = 999999999 - write_begain.tv_nsec + write_end.tv_nsec;
            }
            time_use_sec = write_end.tv_sec - write_begain.tv_sec;
            time_use_us = time_use / 1000 % 1000;
            printf("write cached memory size:%d time use:%ld.%ldms\n",  \
                   VB_TEST_PIC_SIZE, time_use / 1000000 + (time_use_sec * 1000), time_use_us);

            clock_gettime(CLOCK_MONOTONIC, &read_begain);
            for (read_index = 0; read_index < (VB_TEST_PIC_SIZE / 4); read_index++)
            {
                read_val = map_addr[i][read_index];
                if (read_val != read_index)
                {
                    printf("read cache memcoy error\n");
                    goto vb_test_failed;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &read_end);
            if (read_end.tv_nsec > read_begain.tv_nsec)
                time_use = read_end.tv_nsec - read_begain.tv_nsec;
            else
            {
                time_use = 999999999 - read_begain.tv_nsec + read_end.tv_nsec;
            }
            time_use_sec = read_end.tv_sec - read_begain.tv_sec;
            time_use_us = time_use / 1000 % 1000;
            printf("read cached memory size:%d time use:%ld.%ldms\n", \
                   VB_TEST_PIC_SIZE, time_use / 1000000 + (time_use_sec * 1000), time_use_us);

            ret = kd_mpi_sys_mmz_flush_cache(phys_addr, map_addr[i], VB_TEST_PIC_SIZE);
            if (ret)
            {
                printf("kd_mpi_sys_mmz_flush_cache error\n");
                goto vb_test_failed;
            }
            printf("kd_mpi_sys_mmz_flush_cache virtaddr:%p\n", map_addr[i]);

            ret = kd_mpi_sys_munmap(map_addr[i], VB_TEST_PIC_SIZE);
            if (ret)
            {
                printf("kd_mpi_sys_munmap error\n");
                goto vb_test_failed;
            }

            map_addr[i] = kd_mpi_sys_mmap(phys_addr, VB_TEST_PIC_SIZE);
            if (!map_addr[i])
            {
                printf("kd_mpi_sys_mmap_cached error\n");
                goto vb_test_failed;
            }
            printf("kd_mpi_sys_mmap_cache mmap addr:%p\n", map_addr[i]);

            clock_gettime(CLOCK_MONOTONIC, &write_begain);
            for (write_index = 0; write_index < (VB_TEST_PIC_SIZE / 4); write_index++)
            {
                map_addr[i][write_index] = (write_index + 1);
            }
            clock_gettime(CLOCK_MONOTONIC, &write_end);
            if (write_end.tv_nsec > write_begain.tv_nsec)
                time_use = write_end.tv_nsec - write_begain.tv_nsec;
            else
            {
                time_use = 999999999 - write_begain.tv_nsec + write_end.tv_nsec;
            }
            time_use_sec = write_end.tv_sec - write_begain.tv_sec;
            time_use_us = time_use / 1000 % 1000;
            printf("write noncache memory size:%d time use:%ld.%ldms\n",  \
                   VB_TEST_PIC_SIZE, time_use / 1000000 + (time_use_sec * 1000), time_use_us);

            clock_gettime(CLOCK_MONOTONIC, &read_begain);
            for (read_index = 0; read_index < (VB_TEST_PIC_SIZE / 4); read_index++)
            {
                read_val = map_addr[i][read_index];
                if (read_val != (read_index + 1))
                {
                    printf("read noncache memcoy error\n");
                    goto vb_test_failed;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &read_end);
            if (read_end.tv_nsec > read_begain.tv_nsec)
                time_use = read_end.tv_nsec - read_begain.tv_nsec;
            else
            {
                time_use = 999999999 - read_begain.tv_nsec + read_end.tv_nsec;
            }
            time_use_sec = read_end.tv_sec - read_begain.tv_sec;
            time_use_us = time_use / 1000 % 1000;
            printf("read noncache memory size:%d time use:%ld.%ldms\n", \
                   VB_TEST_PIC_SIZE, time_use / 1000000 + (time_use_sec * 1000), time_use_us);

            ret = kd_mpi_sys_munmap(map_addr[i], VB_TEST_PIC_SIZE);
            if (ret)
            {
                printf("kd_mpi_sys_munmap error\n");
                goto vb_test_failed;
            }
            printf("kd_mpi_sys_munmap virtaddr:%p size:%d\n", map_addr[i], VB_TEST_PIC_SIZE);

        }
        i++;
    }
#if 1
    i = 0;
    while (1)
    {
        if (handle_arry[i] == VB_INVALID_HANDLE)
        {
            break;
        }
        else
        {
            ret = kd_mpi_vb_release_block(handle_arry[i]);
            printf("kd_mpi_vb_release_block handle:%08x ret:%d\n", handle_arry[i], ret);
            if (ret)
                goto vb_test_failed;
        }
        i++;
    }
#endif
    ret = kd_mpi_vb_exit_mod_common_pool(VB_UID_VDEC);
    printf("vb_exit_mod_common_pool ret:0x%x\n", ret);
    if (ret)
        goto vb_test_failed;

    ret = kd_mpi_vb_exit();
    printf("vb_exit ret:0x%x\n", ret);
    if (ret)
        goto vb_test_failed;

    goto loop_test;
vb_test_failed:
    printf("vb test failed\n");
    return -1;
}
