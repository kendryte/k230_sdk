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
#include <pthread.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_dma_comm.h"
#include "k_vvi_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_dma_api.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"

#define BLOCK_TIME          100
#define DMA_BUFF_NUM        3

#define SAMPE_VVI_PIPE_NUMS   2
#define SAMPLE_VVI_FRAME_RATE 1

#define DMA_CHN0                0
// #define DMA_CHN0_WIDTH          1920
// #define DMA_CHN0_HEIGHT         1080
// #define DMA_CHN0_SRC_STRIDE     1920
// #define DMA_CHN0_DST_STRIDE     1280
// #define DMA_CHN0_WIDTH          128
// #define DMA_CHN0_HEIGHT         64
// #define DMA_CHN0_SRC_STRIDE     128
// #define DMA_CHN0_DST_STRIDE     128
#define DMA_CHN0_WIDTH          640
#define DMA_CHN0_HEIGHT         320
#define DMA_CHN0_SRC_STRIDE     640
#define DMA_CHN0_DST_STRIDE     320

#define DMA_CHN1                1
// #define DMA_CHN1_WIDTH          1280
// #define DMA_CHN1_HEIGHT         720
// #define DMA_CHN1_SRC_STRIDE     1280
// #define DMA_CHN1_DST_STRIDE     1280
// #define DMA_CHN1_WIDTH          128
// #define DMA_CHN1_HEIGHT         64
// #define DMA_CHN1_SRC_STRIDE     128
// #define DMA_CHN1_DST_STRIDE     128
#define DMA_CHN1_WIDTH          640
#define DMA_CHN1_HEIGHT         320
#define DMA_CHN1_SRC_STRIDE     640
#define DMA_CHN1_DST_STRIDE     640


k_u8 planar[8] = {1, 1, 0, 0, 0, 0, 0, 0};
k_dma_dev_attr_t dev_attr;
k_dma_chn_attr_u chn_attr[DMA_MAX_CHN_NUMS];
k_video_frame_info df_info[DMA_MAX_CHN_NUMS];
k_u32 gdma_size[4] = {0, 0, 0, 0};
k_bool g_end = K_FALSE;

k_video_frame_info insert_pic_info[2];

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

sample_vvi_pipe_conf_t g_pipe_conf[SAMPE_VVI_PIPE_NUMS] =
{
    {
        0,
        0,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        DMA_CHN0_SRC_STRIDE / 4,
        DMA_CHN0_HEIGHT,
        PIXEL_FORMAT_ARGB_8888,
    },
    {
        0,
        1,
        1920,
        1080,
        PIXEL_FORMAT_ARGB_8888,
        DMA_CHN1_SRC_STRIDE / 4,
        DMA_CHN1_HEIGHT,
        PIXEL_FORMAT_ARGB_8888,
    },
};

static k_s32 dma_chn_attr_init(k_dma_chn_attr_u attr[8])
{
    k_gdma_chn_attr_t *gdma_attr;

    /* channel 0 */
    gdma_attr = &attr[0].gdma_attr;
    gdma_attr->buffer_num = 3;
    gdma_attr->rotation = DEGREE_90;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = DMA_CHN0_WIDTH;
    gdma_attr->height = DMA_CHN0_HEIGHT;
    gdma_attr->src_stride[0] = DMA_CHN0_SRC_STRIDE;
    gdma_attr->dst_stride[0] = DMA_CHN0_DST_STRIDE;
    gdma_attr->work_mode = DMA_BIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;

    /* channel 1 */
    gdma_attr = &attr[1].gdma_attr;
    gdma_attr->buffer_num = 3;
    gdma_attr->rotation = DEGREE_180;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = DMA_CHN1_WIDTH;
    gdma_attr->height = DMA_CHN1_HEIGHT;
    gdma_attr->src_stride[0] = DMA_CHN1_SRC_STRIDE;
    gdma_attr->dst_stride[0] = DMA_CHN1_DST_STRIDE;
    gdma_attr->work_mode = DMA_BIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;

    return K_SUCCESS;
}

static k_s32 dma_chn_data_init(k_s32 chn_num, k_dma_chn_attr_u *attr, k_video_frame_info *df_info)
{
    k_s32 i, j;
    k_s16 width, height;
    k_u8 *addr;

    if (chn_num < GDMA_MAX_CHN_NUMS)
    {
        if (chn_num == DMA_CHN0)
        {
            addr = (k_u8 *)df_info->v_frame.virt_addr[0];
            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    addr[i * width + j] = i;
                }
            }
        }
        else if (chn_num == DMA_CHN1)
        {
            addr = (k_u8 *)df_info->v_frame.virt_addr[0];
            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    addr[i * width + j] = i * 2 % 256;
                }
            }
        }
    }

    return K_SUCCESS;
}

static k_s32 dma_vb_init(k_dma_chn_attr_u attr[8], k_u8 planar[8])
{
    k_s32 ret;
    k_vb_config config;
    k_s32 i;
    k_gdma_chn_attr_t *gdma_attr;
    k_s32 size, size_v, size_h;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;
    for (i = 0; i < DMA_MAX_CHN_NUMS; i++)
    {
        if (planar[i] == 0)
            continue;

        if (i < GDMA_MAX_CHN_NUMS)
        {
            gdma_attr = &attr[i].gdma_attr;
            if (gdma_attr->rotation == DEGREE_90 ||
                    gdma_attr->rotation == DEGREE_270)
            {
                size_v = gdma_attr->src_stride[0] * gdma_attr->height;
                size_h = gdma_attr->dst_stride[0] * gdma_attr->width;
                size = (size_v > size_h) ? size_v : size_h;
            }
            else
            {
                size = gdma_attr->src_stride[0] * gdma_attr->height;
            }

            if (i == DMA_CHN0)
            {
                config.comm_pool[i].blk_cnt = (DMA_BUFF_NUM + 2);
                config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
                config.comm_pool[i].blk_size = size;
            }
            else if (i == DMA_CHN1)
            {
                config.comm_pool[i].blk_cnt = (DMA_BUFF_NUM + 2);
                config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
                config.comm_pool[i].blk_size = size;
            }
            gdma_size[i] = config.comm_pool[i].blk_size;
        }
    }
    ret = kd_mpi_vb_set_config(&config);
    printf("\n");
    printf("---------------------dma sample test---------------------\n");
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

static k_s32 dma_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_s32 sample_dma_get_blk(k_s32 size, k_video_frame_info *df_info, k_u8 chn_num)
{
    k_vb_blk_handle handle;
    k_s32 pool_id = 0;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr = NULL;

    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if (pool_id == VB_INVALID_HANDLE)
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

    virt_addr = (k_u8 *)kd_mpi_sys_mmap(phys_addr, size);
    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    df_info->pool_id = pool_id;

    if (chn_num == DMA_CHN0)
    {
        df_info->v_frame.phys_addr[0] = phys_addr;
        df_info->v_frame.virt_addr[0] = (k_u64)(intptr_t)virt_addr;
    }
    else if (chn_num == DMA_CHN1)
    {
        df_info->v_frame.phys_addr[0] = phys_addr;
        df_info->v_frame.virt_addr[0] = (k_u64)(intptr_t)virt_addr;
        df_info->v_frame.phys_addr[1] = df_info->v_frame.phys_addr[0] + size / 2;
        df_info->v_frame.virt_addr[1] = df_info->v_frame.virt_addr[0] + size / 2;
    }

    return K_SUCCESS;
}

static k_s32 sample_dma_put_blk(k_s32 size, k_u64 phys_src_addr)
{
    k_vb_blk_handle handle;
    k_s32 ret;

    handle = kd_mpi_vb_phyaddr_to_handle(phys_src_addr);
    if (handle != VB_INVALID_HANDLE)
    {
        ret =  kd_mpi_vb_release_block(handle);
    }
    else
    {
        ret = K_FAILED;
    }

    return ret;
}

static k_s32 sample_dma_prepare_data(k_dma_chn_attr_u attr[DMA_MAX_CHN_NUMS], k_u8 planar[DMA_MAX_CHN_NUMS], k_video_frame_info df_info[DMA_MAX_CHN_NUMS])
{
    k_s32 size;


    /* prepare chn0 data */
    size = gdma_size[0];
    sample_dma_get_blk(size, &df_info[DMA_CHN0], DMA_CHN0);
    dma_chn_data_init(DMA_CHN0, &attr[DMA_CHN0], &df_info[DMA_CHN0]);

    /* prepare chn1 data */
    size = gdma_size[1];
    sample_dma_get_blk(size, &df_info[DMA_CHN1], DMA_CHN1);
    dma_chn_data_init(DMA_CHN1, &attr[DMA_CHN1], &df_info[DMA_CHN1]);

    return K_SUCCESS;
}

static k_s32 sample_dma_release_data(k_dma_chn_attr_u attr[DMA_MAX_CHN_NUMS], k_u8 planar[DMA_MAX_CHN_NUMS], k_video_frame_info df_info[DMA_MAX_CHN_NUMS])
{
    k_s32 size;

    /* release chn0 data */
    size = gdma_size[0];
    sample_dma_put_blk(size, df_info[DMA_CHN0].v_frame.phys_addr[0]);

    /* release chn1 data */
    size = gdma_size[1];
    sample_dma_put_blk(size, df_info[DMA_CHN1].v_frame.phys_addr[0]);

    return K_SUCCESS;
}

static k_s32 dma_dev_attr_init(k_dma_dev_attr_t *dev_attr)
{
    dev_attr->burst_len = 0;
    dev_attr->ckg_bypass = 0xff;
    dev_attr->outstanding = 7;

    return K_SUCCESS;
}

#if 1
static k_s32 sample_vvi_prepare(sample_vvi_pipe_conf_t* pipe_conf)
{
    k_s32 i;
    k_vvi_dev_attr dev_attr;
    k_vvi_chn_attr chn_attr;

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
        memset(&chn_attr, 0, sizeof(chn_attr));
        memset(&dev_attr, 0, sizeof(dev_attr));
        dev_attr.format = pipe_conf[i].dev_format;
        dev_attr.height = pipe_conf[i].dev_height;
        dev_attr.width = pipe_conf[i].dev_width;
        chn_attr.frame_rate = SAMPLE_VVI_FRAME_RATE;
        chn_attr.format = pipe_conf[i].chn_format;
        chn_attr.height = pipe_conf[i].chn_height;
        chn_attr.width = pipe_conf[i].chn_width;
        printf("pipe[%d] dev[%d] h:%d w:%d chn[%d] h:%d w:%d \n", i,
                pipe_conf[i].dev_num, pipe_conf[i].dev_height, pipe_conf[i].dev_width,
                pipe_conf[i].chn_num, pipe_conf[i].chn_height, pipe_conf[i].chn_width);
        kd_mpi_vvi_set_dev_attr(pipe_conf[i].dev_num, &dev_attr);
        kd_mpi_vvi_set_chn_attr(pipe_conf[i].chn_num, &chn_attr);
    }

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
        kd_mpi_vvi_start_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    }

    kd_mpi_vvi_chn_insert_pic(g_pipe_conf[0].chn_num, &insert_pic_info[0]);
    kd_mpi_vvi_chn_insert_pic(g_pipe_conf[1].chn_num, &insert_pic_info[1]);

    return K_SUCCESS;
}

static void sample_vvi_stop(sample_vvi_pipe_conf_t* pipe_conf)
{
    k_s32 i;

    for(i = 0; i < SAMPE_VVI_PIPE_NUMS; i++)
        kd_mpi_vvi_stop_pipe(pipe_conf[i].dev_num, pipe_conf[i].chn_num);
    return;
}
#endif

static k_s32 sample_vvi_bind_gdma()
{
    k_s32 ret;
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn gdma_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 0;
    gdma_mpp_chn.mod_id = K_ID_DMA;
    gdma_mpp_chn.dev_id = 0;
    gdma_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &gdma_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
        return ret;
    }

#if 1
    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 1;
    gdma_mpp_chn.mod_id = K_ID_DMA;
    gdma_mpp_chn.dev_id = 0;
    gdma_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_bind(&vvi_mpp_chn, &gdma_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x:%x\n", ret);
        return ret;
    }
#endif

    return K_SUCCESS;
}

static k_s32 sample_vvi_unbind_gdma()
{
    k_s32 ret;
    k_mpp_chn vvi_mpp_chn;
    k_mpp_chn gdma_mpp_chn;

    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 0;
    gdma_mpp_chn.mod_id = K_ID_DMA;
    gdma_mpp_chn.dev_id = 0;
    gdma_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_unbind(&vvi_mpp_chn, &gdma_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
        return ret;
    }

#if 1
    vvi_mpp_chn.mod_id = K_ID_V_VI;
    vvi_mpp_chn.dev_id = 0;
    vvi_mpp_chn.chn_id = 1;
    gdma_mpp_chn.mod_id = K_ID_DMA;
    gdma_mpp_chn.dev_id = 0;
    gdma_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_unbind(&vvi_mpp_chn, &gdma_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_bind failed:0x:%x\n", ret);
        return ret;
    }
#endif

    return K_SUCCESS;
}

static k_s32 sample_vvi_insert_prepare()
{
    for (int i = 0; i < SAMPE_VVI_PIPE_NUMS; i++) {
        insert_pic_info[i].v_frame.width = g_pipe_conf[i].chn_width;
        insert_pic_info[i].v_frame.height = g_pipe_conf[i].chn_height;
        insert_pic_info[i].v_frame.pixel_format = g_pipe_conf[i].chn_format;
    }

    insert_pic_info[0].mod_id = K_ID_V_VI;
    insert_pic_info[0].pool_id = df_info[DMA_CHN0].pool_id;
    insert_pic_info[0].v_frame.phys_addr[0] = df_info[DMA_CHN0].v_frame.phys_addr[0];
    insert_pic_info[0].v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    insert_pic_info[0].v_frame.virt_addr[0] = 0;

    insert_pic_info[1].mod_id = K_ID_V_VI;
    insert_pic_info[1].pool_id = df_info[DMA_CHN1].pool_id;
    insert_pic_info[1].v_frame.phys_addr[0] = df_info[DMA_CHN1].v_frame.phys_addr[0];
    insert_pic_info[1].v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    insert_pic_info[1].v_frame.virt_addr[0] = 0;

    return K_SUCCESS;
}

int main(void)
{
    k_s32 ret;

    printf("dma sample case, press q to end the operation.\n");

    memset(chn_attr, 0, sizeof(k_dma_chn_attr_u)*DMA_MAX_CHN_NUMS);

    dma_dev_attr_init(&dev_attr);
    dma_chn_attr_init(chn_attr);

    if (dma_vb_init(chn_attr, planar))
    {
        return -1;
    }

    sample_dma_prepare_data(chn_attr, planar, df_info);

    ret = sample_vvi_bind_gdma();
    if (ret) {
        printf("sample_vvi_bind_gdma failed\n");
        return 0;
    } else {
        printf("sample_vvi_bind_gdma success\n");
    }

    sample_vvi_insert_prepare();

    ret = kd_mpi_dma_set_dev_attr(&dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("set dev attr error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS)
    {
        printf("start dev error\r\n");
        goto exit_label;
    }

#if 1
    /* DMA_CHN0 prepare */
    ret = kd_mpi_dma_set_chn_attr(DMA_CHN0, &chn_attr[DMA_CHN0]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto exit_label;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN0);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto exit_label;
    }

#endif

#if 1
    /* DMA_CHN1 prepare */
    ret = kd_mpi_dma_set_chn_attr(DMA_CHN1, &chn_attr[DMA_CHN1]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto exit_label;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN1);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto exit_label;
    }
#endif

    sample_vvi_prepare(g_pipe_conf);

    while(((char)getchar()) != 'q');

#if 1
    /* stop */
    ret = kd_mpi_dma_stop_chn(DMA_CHN0);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }
#endif

#if 1
    ret = kd_mpi_dma_stop_chn(DMA_CHN1);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }
#endif

    sample_vvi_stop(g_pipe_conf);

    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS)
    {
        printf("stop dev error\r\n");
        goto exit_label;
    }

    sample_vvi_unbind_gdma();

exit_label:
    sample_dma_release_data(chn_attr, planar, df_info);

    dma_vb_exit();
    return 0;
}