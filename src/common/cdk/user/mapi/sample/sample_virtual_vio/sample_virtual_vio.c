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
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vvi_api.h"

#define SAMPLE_VVO_DISPLAY_DEV_ID   1
#define SAMPLE_VVO_DISPLAY_CHN_ID   1
#define SAMPLE_VVI_PIPE_NUMS        2
#define SAMPLE_VVI_FRAME_RATE       1

#define COLOR_RED             0xFF0000FFU
#define COLOR_GREEN           0x00FF00FFU
#define COLOR_BLUE            0x0000FFFFU


typedef struct {
    k_u32 dev_num;
    k_u32 chn_num;
    k_u32 dev_height;
    k_u32 dev_width;
    k_pixel_format dev_format;
    k_u32 chn_height;
    k_u32 chn_width;
    k_pixel_format chn_format;
} sample_vvi_pipe_conf_t;

sample_vvi_pipe_conf_t g_pipe_conf[SAMPLE_VVI_PIPE_NUMS] =
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
    },
    {
        1,
        1,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        480,
        360,
        PIXEL_FORMAT_ARGB_8888,
    },
};

static k_video_frame_info insert_pic_info = {0};
static k_video_frame_info dump_pic_info = {0};
static k_mapi_media_attr_t media_attr = {0};
static k_s32 mmap_fd = -1;

static k_s32 sample_vvi_start_pipe(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 ret;
    k_s32 i;
    k_s32 j;
    k_vvi_chn_attr chn_attr = {0};
    k_vvi_dev_attr dev_attr = {0};

    for(i = 0; i < SAMPLE_VVI_PIPE_NUMS; i++) {
        dev_attr.format = pipe_conf[i].dev_format;
        dev_attr.height = pipe_conf[i].dev_height;
        dev_attr.width = pipe_conf[i].dev_width;
        chn_attr.format = pipe_conf[i].chn_format;
        chn_attr.height = pipe_conf[i].chn_height;
        chn_attr.width = pipe_conf[i].chn_width;
        chn_attr.frame_rate = SAMPLE_VVI_FRAME_RATE;

        ret = kd_mapi_vvi_start_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num, &dev_attr, &chn_attr);
        if(ret) {
            printf("kd_mpi_vvi_start");
            break;
        }
    }

    if(ret) {
        j = i;
        for(i = 0; i < j; i++) {
            kd_mapi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
        }
    }

    return ret;
}

static k_s32 sample_vvi_stop_pipe(sample_vvi_pipe_conf_t *pipe_conf)
{
    k_s32 ret;
    k_s32 i;

    for(i = 0; i < SAMPLE_VVI_PIPE_NUMS; i++) {
        ret |= kd_mapi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    }
    /*Allow one frame time for the virtual VO to release the VB block*/
#if 1
    k_u32 display_time = 1000 / SAMPLE_VVI_FRAME_RATE;
    usleep(display_time * 1000);
#endif
    return ret;
}

static k_s32 sample_vvi_prepare_insert_pic(k_video_frame_info *vf_info, k_u32 color, void **pic_vaddr)
{
    k_s32 ret;
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_s32 i = 0;
    k_s32 size;

    if(vf_info == NULL || pic_vaddr == NULL)
        return K_FAILED;

    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    ret = kd_mapi_sys_get_vb_block(&pool_id, &phys_addr, size, NULL);
    if(ret) {
        printf("kd_mapi_sys_get_vb_block error %x\n", ret);
    }

    printf("get vb block phys_addr:0x%lx\n", phys_addr);
#ifdef USE_RT_SMART
    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
#else
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u32 mmap_size = (((size) + (page_size) - 1) & ~((page_size) - 1));
    k_u64 page_mask = (page_size - 1);
    if(mmap_fd == -1)
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~page_mask);
    if(mmap_addr)
        virt_addr = mmap_addr + (phys_addr & page_mask);
#endif

    if(virt_addr == NULL) {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    for(i = 0; i < size / sizeof(k_u32); i++)
    {
       virt_addr[i] = color;
    }
    vf_info->mod_id = K_ID_V_VI;
    vf_info->pool_id = pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    vf_info->v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    *pic_vaddr = virt_addr;
    return 0;
}

static k_s32 sample_vvi_release_pic(const k_video_frame_info *vf_info, const void *virt_addr)
{
    k_vb_blk_handle handle;
    k_s32 ret;
    k_u32 size;

    if(vf_info == NULL)
        return K_FALSE;
    size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    ret = kd_mapi_sys_release_vb_block(vf_info->v_frame.phys_addr[0], size);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_release_vb_block error: %x\n", ret);
    }

#ifdef USE_RT_SMART
    ret = kd_mpi_sys_munmap((void *)virt_addr, size);
#else
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    size = (((size) + (page_size) - 1) & ~((page_size) - 1));
    munmap((void *)((k_u64)(virt_addr) & ~page_mask), size);
#endif
    return ret;
}

static void sample_vvi_display_frame(k_video_frame_info *vf_info)
{
    k_u32 *color = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    if(0 == vf_info->v_frame.phys_addr[0])
        return;
#ifdef USE_RT_SMART
    color = kd_mpi_sys_mmap(vf_info->v_frame.phys_addr[0], page_size);
#else
    void *mmap_addr = NULL;
    k_u64 page_mask = (page_size - 1);
    if(mmap_fd == -1)
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    mmap_addr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    mmap_fd, vf_info->v_frame.phys_addr[0] & ~page_mask);
    if(mmap_addr)
        color = mmap_addr + (vf_info->v_frame.phys_addr[0] & page_mask);
#endif
    if(!color) {
        printf("mmap color error\n");
        return;
    }
    switch(*color) {
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
#ifdef USE_RT_SMART
    kd_mpi_sys_munmap(color, page_size);
#else
    munmap(color, page_size);
#endif
    return;
}



int main(void)
{
    k_s32 ret;
    k_s32 i;
    char select;
    void* pic_vaddr = NULL;

    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error: %x\n", ret);
        goto exit;
    }

    media_attr.media_config.vb_config.max_pool_cnt = 64;
    for(i = 0; i < SAMPLE_VVI_PIPE_NUMS; i++) {
        media_attr.media_config.vb_config.comm_pool[i].blk_cnt = 5;
        media_attr.media_config.vb_config.comm_pool[i].blk_size = g_pipe_conf[i].chn_height * g_pipe_conf[i].chn_width * 4;
        media_attr.media_config.vb_config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
    }
    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
        goto sys_exit;
    }

    ret = kd_mapi_vvi_bind_vvo(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num,
                        SAMPLE_VVO_DISPLAY_DEV_ID, SAMPLE_VVO_DISPLAY_CHN_ID);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
        goto media_exit;
    }

    ret = kd_mapi_vvi_bind_vvo(g_pipe_conf[1].dev_num, g_pipe_conf[1].chn_num, 0, 0);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
        goto bind0_exit;
    }

    ret = sample_vvi_start_pipe(&g_pipe_conf[0]);
    if(ret != K_SUCCESS) {
        printf("sample_vvi_start_pipe error: %x\n", ret);
        goto bind1_exit;
    }

    memset(&insert_pic_info, 0, sizeof(insert_pic_info));
    insert_pic_info.v_frame.height = g_pipe_conf[0].chn_height;
    insert_pic_info.v_frame.width = g_pipe_conf[0].chn_width;
    insert_pic_info.v_frame.pixel_format = g_pipe_conf[0].chn_format;
    printf("Press r or g or b to insert a \"red\" \"green\" \"blue\" frame into vvi dev[0] chn[0]\n");
    while((select = (char)getchar()) != 'r' && select != 'g' && select != 'b');
    switch (select) {
    case 'r':
        ret = sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_RED, &pic_vaddr);
        printf("insert red to vvi\n");
        break;
    case 'g':
        ret = sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_GREEN, &pic_vaddr);
        printf("insert green to vvi\n");
        break;
    case 'b':
        ret = sample_vvi_prepare_insert_pic(&insert_pic_info, COLOR_BLUE, &pic_vaddr);
        printf("insert blue to vvi\n");
        break;
    default:
        break;
    }
    if(ret != K_SUCCESS) {
        printf("sample_vvi_prepare_insert_pic: %d\n", ret);
        goto bind1_exit;
    }

    ret = kd_mapi_vvi_insert_pic(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num, &insert_pic_info);
    if(ret != K_SUCCESS) {
        printf("sample_vvi_prepare_insert_pic: %d\n", ret);
        goto bind1_exit;
    }

    getchar();
    printf("Press Enter to remove pic\n");
    getchar();
    ret = kd_mapi_vvi_remove_pic(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_vvi_remove_pic: %d\n", ret);
        goto insert_exit;
    }

    if(pic_vaddr != 0) {
        sample_vvi_release_pic((const void*)&insert_pic_info, (const void*)pic_vaddr);
        pic_vaddr = NULL;
    }

    printf("Press Enter to dump pic from vvi\n");
    getchar();
    ret = kd_mapi_vvi_dump_frame(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num, &dump_pic_info, 2000);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_vvi_dump_frame: %x\n", ret);
        goto insert_exit;
    }
    printf("get frame phys addr:%lx\n", dump_pic_info.v_frame.phys_addr[0]);
    sample_vvi_display_frame(&dump_pic_info);

    printf("Press Enter to release dump pic from vvi\n");
    getchar();
    ret = kd_mapi_vvi_release_frame(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num, &dump_pic_info);
     if(ret != K_SUCCESS) {
        printf("kd_mapi_vvi_dump_frame: %x\n", ret);
        goto insert_exit;
    }

    printf("Press Enter to stop!!!!\n");
    getchar();

insert_exit:
    if(pic_vaddr != NULL)
        sample_vvi_release_pic(&insert_pic_info, pic_vaddr);
stop_exit:
    ret = sample_vvi_stop_pipe(&g_pipe_conf[0]);
bind1_exit:
    ret |= kd_mapi_vvi_unbind_vvo(g_pipe_conf[1].dev_num, g_pipe_conf[1].chn_num,0, 0);
bind0_exit:
    ret |= kd_mapi_vvi_unbind_vvo(g_pipe_conf[0].dev_num, g_pipe_conf[0].chn_num,
                        SAMPLE_VVO_DISPLAY_DEV_ID, SAMPLE_VVO_DISPLAY_CHN_ID);
media_exit:
    printf("Press Enter to deinit media!!!!\n");
    getchar();
    ret |= kd_mapi_media_deinit();
sys_exit:
    printf("Press Enter to exit!!!!\n");
    getchar();
    ret |= kd_mapi_sys_deinit();
exit:
    if(ret)
        printf("sample code run error\n");
    printf("Exit!!!!\n");
    return 0;
}