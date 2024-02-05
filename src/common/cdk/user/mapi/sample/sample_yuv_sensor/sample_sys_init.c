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
#include "mapi_sys_api.h"
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"
#include "mpi_nonai_2d_api.h"
#include "k_nonai_2d_comm.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"
#include "mpi_venc_api.h"
#include "k_venc_comm.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"

#define TOTAL_ENABLE_2D_CH_NUMS     6
#define NONAI_2D_RGB_CH             4
#define NONAI_2D_BIND_CH_0          0
#define NONAI_2D_BIND_CH_1          1
#define NONAI_2D_BIND_CH_2          2

#define ISP_CHN0_HEIGHT             (720)
#define ISP_CHN0_WIDTH              (1280)

#define USE_HDMI_INIT               1

#define OSD_ID                      K_VO_OSD3
#define OSD_WIDTH                   (1280)
#define OSD_HEIGHT                  (720)

static void *osd_virt_addr;
static k_u32 osd_size;

static k_vb_blk_handle sample_vo_install_frame(k_video_frame_info *vf_info, void **pic_vaddr)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size = 0;
    k_s32 pool_id;

    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_8888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_8888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_565 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_565)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_4444 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_4444)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_1555 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_1555)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3 / 2;

    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        // printf("%s get vb block error\n", __func__);
        return -1;
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if(pool_id == VB_INVALID_POOLID) {
        printf("%s, get pool id error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    // virt_addr = (k_u32 *)kd_mpi_sys_mmap_cached(phys_addr, size);

    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    osd_virt_addr = virt_addr;
    osd_size = size;

    return handle;
}

static k_vb_blk_handle osd_init(k_video_frame_info *vf_info)
{

    // setvp install frame
    k_vb_blk_handle block;
    void *pic_vaddr = NULL;

    memset(&block, 0, sizeof(k_vb_blk_handle));
    return block;
    memset(vf_info, 0 , sizeof(k_video_frame_info));
    if(USE_HDMI_INIT == 1)
    {
        vf_info->v_frame.width = OSD_HEIGHT;
        vf_info->v_frame.height = OSD_WIDTH;
    }
    else
    {
        vf_info->v_frame.width = OSD_WIDTH;
        vf_info->v_frame.height = OSD_HEIGHT;
    }
    vf_info->v_frame.stride[0] =vf_info->v_frame.width;
    vf_info->v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;

    block = sample_vo_install_frame(vf_info, &pic_vaddr);
    printf("sample_vo_install_frame------------ success \n");

    k_u32 *temp_addr = (k_u32 *)pic_vaddr;
    for (int i = 0; i < ((OSD_HEIGHT * OSD_WIDTH * 4) / sizeof(k_u32)) ; i++)
    {
        temp_addr[i] = 0xFF0000FFU;
    }

    return block;
}

static void osd_deinit(k_vb_blk_handle osd_block)
{
    k_s32 ret;

    ret = kd_mpi_sys_munmap(osd_virt_addr, osd_size);
    if (ret) {
        printf("kd_mpi_sys_munmap failed. %d\n", ret);
    }

    ret = kd_mpi_vb_release_block(osd_block);
    if (ret) {
        printf("kd_mpi_vb_release_block failed. %d\n", ret);
    }
}


int main(void)
{
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_video_frame_info dump_info;
    k_video_frame_info rgb_vf_info;
    k_video_frame_info vf_info;
    k_bool is_osd_init = K_FALSE;
    k_vb_blk_handle osd_block;

    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS)
        printf("kd_mapi_sys_init error:%d\n", ret);

    printf("big core start\n");

    while(1)
    {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        memset(&rgb_vf_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_0, VICAP_DUMP_YUV444, &dump_info, 1000);
        if (ret) {
            //little core is stopped
            if(is_osd_init)
            {
                osd_deinit(osd_block);
                is_osd_init = K_FALSE;
            }

            continue;
        }

        //dump frame successful means little core is ready.

        if(is_osd_init == K_FALSE)
        {
            osd_block = osd_init(&vf_info);
            is_osd_init = K_TRUE;
        }

        //yuv444 to rgb
        ret = kd_mpi_nonai_2d_send_frame(NONAI_2D_RGB_CH, &dump_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_send_frame ch 4 failed. %d\n", ret);
            goto vicap_release;
        }
        ret = kd_mpi_nonai_2d_get_frame(NONAI_2D_RGB_CH, &rgb_vf_info, 1000);
        if (ret) {
            printf("kd_mpi_nonai_2d_get_frame ch 4 failed. %d\n", ret);
            goto vicap_release;
        }
        if(ret == K_SUCCESS)
        {
            //TODO: AI
            //kd_mpi_vo_chn_insert_frame(K_VO_OSD3 + 3, &vf_info);  //insert osd
        }
        ret = kd_mpi_nonai_2d_release_frame(NONAI_2D_RGB_CH, &rgb_vf_info);
        if (ret) {
            printf("kd_mpi_nonai_2d_release_frame ch 4 failed. %d\n", ret);
        }

vicap_release:
        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_0, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }

    }

    ret = kd_mapi_sys_deinit();
    if(ret != K_SUCCESS)
        printf("kd_mapi_sys_deinit error:%d\n", ret);

    printf("big core exit\n");
    return 0;
}
