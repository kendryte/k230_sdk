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
#include <sys/mman.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"
#include "k_vvo_comm.h"

#define SAMPE_VVI_PIPE_NUMS   2
#define SAMPLE_VVI_FRAME_RATE 1

#define COLOR_RED             0xFF0000FFU
#define COLOR_GREEN           0x00FF00FFU
#define COLOR_BLUE            0x0000FFFFU


typedef struct
{
    k_u32 dev_num;
    k_u32 chn_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_pixel_format dev_format;
    k_u32 chn_height;
    k_u32 chn_width;
    k_pixel_format chn_format;
    k_bool is_pre_fill_color;
} sample_vvi_pipe_conf_t;

sample_vvi_pipe_conf_t g_pipe_conf[SAMPE_VVI_PIPE_NUMS] =
{
    {
        0,
        0,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        720,
        480,
        PIXEL_FORMAT_ARGB_8888,
        K_TRUE,
    },
    {
        1,
        1,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        64,
        32,
        PIXEL_FORMAT_ARGB_8888,
        K_FALSE,
    },
};


static k_s32 sample_vb_init(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 ret;
    k_vb_config config;
    k_s32 i;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
    {
        config.comm_pool[i].blk_cnt = 5;
        config.comm_pool[i].blk_size = pipe_conf[i].chn_height * pipe_conf[i].chn_width * 4;
        config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
    }
    ret = kd_mpi_vb_set_config(&config);
    printf("\n");
    printf("-----------virtual vi vo sample test------------------------\n");
    if (ret)
        printf("vb_set_config failed ret:%d\n", ret);

    k_vb_supplement_config supplement_config;
    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret)
        printf("vb_set_supplement_config failed ret:%d\n", ret);
    ret = kd_mpi_vb_init();
    if (ret)
        printf("vb_init failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_vvi_prepare_insert_pic(k_video_frame_info *vf_info, k_u32 color, void **pic_vaddr)
{
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_s32 i = 0;
    k_s32 size;

    if (vf_info == NULL || pic_vaddr == NULL)
        return K_FALSE;

    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if (pool_id == VB_INVALID_POOLID)
    {
        printf("%s get pool id error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }
    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);

    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    for (i = 0; i < size / sizeof(k_u32); i++)
    {
        virt_addr[i] = color;
    }
    vf_info->mod_id = K_ID_V_VI;
    vf_info->pool_id = pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    vf_info->v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    vf_info->v_frame.virt_addr[0] = 0;
    *pic_vaddr = virt_addr;
    return 0;
}

static k_s32 sample_vvi_release_pic(const k_video_frame_info *vf_info, const void *virt_addr)
{
    k_vb_blk_handle handle;
    k_s32 ret;
    k_u32 size;

    if (vf_info == NULL)
        return K_FALSE;
    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    handle = kd_mpi_vb_phyaddr_to_handle(vf_info->v_frame.phys_addr[0]);
    if (handle != VB_INVALID_HANDLE)
    {
        ret =  kd_mpi_vb_release_block(handle);
    }
    else
    {
        ret = K_FAILED;
    }
    if (virt_addr)
        kd_mpi_sys_munmap((void *)virt_addr, size);
    return ret;
}

static k_s32 sample_vvi_chn_dump_frame(k_s32 chn_num, k_video_frame_info *vf_info)
{
    k_s32 ret;
    if (!vf_info)
        return K_FAILED;
    ret = kd_mpi_vvi_chn_dump_request(chn_num);
    if (ret)
    {
        printf("vvi_dump_request failed ret:%d\n", ret);
        return ret;
    }

    ret = kd_mpi_vvi_chn_dump_frame(chn_num, vf_info, 1000);
    if (ret)
        printf("%s failed ret:%d\n", __func__, ret);
    return ret;
}

static void sample_vvi_display_frame(k_video_frame_info *vf_info)
{
    k_u32 *color;
    if (0 == vf_info->v_frame.phys_addr[0])
        return;
    color = kd_mpi_sys_mmap(vf_info->v_frame.phys_addr[0], 4096);
    if (!color)
    {
        printf("mmap color error\n");
        return;
    }
    switch (*color)
    {
    case COLOR_RED:
        printf("get frame color RED!\n");
        break;
    case COLOR_GREEN:
        printf("get frame color GREEN!\n");
        break;
    case COLOR_BLUE:
        printf("get frame color BLUE!\n");
        break;
    default:
        printf("unknow color %x\n", *color);
        break;
    }
    kd_mpi_sys_munmap(color, 4096);
    return;
}


static void sample_vvi_bind_vvo(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    if (!pipe_conf)
        return;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = VVO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = VVO_DISPLAY_CHN_ID;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = 0;
    vvo_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }
    return;
}

static void sample_vvi_unbind_vvo(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn vvo_mpp_chn;

    if (!pipe_conf)
        return;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = VVO_DISPLAY_DEV_ID;
    vvo_mpp_chn.chn_id = VVO_DISPLAY_CHN_ID;
    kd_mpi_sys_unbind(&vvi_mpp_chn, &vvo_mpp_chn);

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = pipe_conf[0].dev_num;
    vvi_mpp_chn.chn_id = pipe_conf[0].chn_num;
    vvo_mpp_chn.mod_id = K_ID_V_VO;
    vvo_mpp_chn.dev_id = 0;
    vvo_mpp_chn.chn_id = 0;
    kd_mpi_sys_unbind(&vvi_mpp_chn, &vvo_mpp_chn);

    return;
}


static void sample_vvi_attr_set(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;
    k_vvi_dev_attr dev_attr;
    k_vvi_chn_attr chn_attr;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
    {
        memset(&chn_attr, 0, sizeof(chn_attr));
        memset(&dev_attr, 0, sizeof(dev_attr));
        dev_attr.format = pipe_conf[i].dev_format;
        dev_attr.height = pipe_conf[i].dev_height;
        dev_attr.width = pipe_conf[i].dev_width;
        chn_attr.frame_rate = SAMPLE_VVI_FRAME_RATE;
        chn_attr.format = pipe_conf[i].chn_format;
        chn_attr.height = pipe_conf[i].chn_height;
        chn_attr.width = pipe_conf[i].chn_width;
        chn_attr.is_pre_fill_color = pipe_conf[i].is_pre_fill_color;
        printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", i,
               pipe_conf[i].dev_num, pipe_conf[i].dev_height, pipe_conf[i].dev_width,
               pipe_conf[i].chn_num, pipe_conf[i].chn_height, pipe_conf[i].chn_width);
        kd_mpi_vvi_set_dev_attr(pipe_conf[i].dev_num, &dev_attr);
        kd_mpi_vvi_set_chn_attr(pipe_conf[i].chn_num, &chn_attr);
    }
    return;
}

static void sample_vvi_start(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_start_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}

static void sample_vvi_stop(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 i;

    for (i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}

int main(void)
{
    k_video_frame_info insert_pic_info;
    k_video_frame_info dump_pic_info[2];
    k_bool is_frame_dumped[2] = {K_FALSE, K_FALSE};
    k_s32 ret;

    void *pic_vaddr = NULL;
    char select;

    if (sample_vb_init(g_pipe_conf))
    {
        return -1;
    }

    sample_vvi_bind_vvo(g_pipe_conf);

    sample_vvi_attr_set(g_pipe_conf);

    sample_vvi_start(g_pipe_conf);

    memset(&insert_pic_info, 0, sizeof(insert_pic_info));
    insert_pic_info.v_frame.height = g_pipe_conf[0].chn_height;
    insert_pic_info.v_frame.width = g_pipe_conf[0].chn_width;
    insert_pic_info.v_frame.pixel_format = g_pipe_conf[0].chn_format;
#if 1
    printf("Press r or g or b to insert a \"red\" \"green\" \"blue\" frame into vvi dev[0] chn[0]\n");
    while ((select = (char)getchar()) != 'r' && select != 'g' && select != 'b');
    switch (select)
    {
    case 'r':
        sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_RED, &pic_vaddr);
        printf("insert red to vvi\n");
        break;
    case 'g':
        sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_GREEN, &pic_vaddr);
        printf("insert green to vvi\n");
        break;
    case 'b':
        sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_BLUE, &pic_vaddr);
        printf("insert blue to vvi\n");
        break;
    default:
        break;
    }
#else
    sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_BLUE);
#endif
    kd_mpi_vvi_chn_insert_pic(g_pipe_conf[0].chn_num, &insert_pic_info);
    getchar();
    printf("Press Enter to remove pic\n");
    getchar();
    kd_mpi_vvi_chn_remove_pic(g_pipe_conf[0].chn_num);
    if (pic_vaddr != 0)
        sample_vvi_release_pic((const void *)&insert_pic_info, (const void *)pic_vaddr);
    printf("Press Enter to dump pic from vvi\n");
    getchar();

    ret = sample_vvi_chn_dump_frame(g_pipe_conf[0].chn_num, &dump_pic_info[0]);
    if (!ret)
    {
        sample_vvi_display_frame(&dump_pic_info[0]);
        is_frame_dumped[0] = K_TRUE;
    }
    ret = sample_vvi_chn_dump_frame(g_pipe_conf[0].chn_num, &dump_pic_info[1]);
    if (!ret)
    {
        sample_vvi_display_frame(&dump_pic_info[1]);
        is_frame_dumped[1] = K_TRUE;
    }

    if (is_frame_dumped[0])
    {
        ret = kd_mpi_vvi_chn_dump_release(g_pipe_conf[0].chn_num,
                                          (const k_video_frame_info *)&dump_pic_info[0]);
        if (ret)
            printf("dump_release failed ret:%d\n", ret);
    }
    if (is_frame_dumped[1])
    {
        ret = kd_mpi_vvi_chn_dump_release(g_pipe_conf[0].chn_num,
                                          (const k_video_frame_info *)&dump_pic_info[1]);
        if (ret)
            printf("dump_release failed ret:%d\n", ret);
    }

    printf("Press Enter to stop vvi\n");
    getchar();
    sample_vvi_stop(g_pipe_conf);
    sample_vvi_unbind_vvo(g_pipe_conf);
    printf("Press Enter to exit!!!!\n");
    getchar();

    /*Allow one frame time for the virtual VO to release the VB block*/
    k_u32 display_ms = 1000 / SAMPLE_VVI_FRAME_RATE;
    usleep(1000 * display_ms);

    sample_vb_exit();
    return 0;
}
