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
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_dma_api.h"
#include "mpi_sys_api.h"

#define BLOCK_TIME              100
#define DMA_BUFF_NUM            3

#define DMA_CHN0                0
#define DMA_CHN0_WIDTH          1920
#define DMA_CHN0_HEIGHT         1080
#define DMA_CHN0_SRC_STRIDE     1920
#define DMA_CHN0_DST_STRIDE     1280
// #define DMA_CHN0_WIDTH          128
// #define DMA_CHN0_HEIGHT         64
// #define DMA_CHN0_SRC_STRIDE     128
// #define DMA_CHN0_DST_STRIDE     128
// #define DMA_CHN0_WIDTH          640
// #define DMA_CHN0_HEIGHT         320
// #define DMA_CHN0_SRC_STRIDE     640
// #define DMA_CHN0_DST_STRIDE     320

#define DMA_CHN1                1
#define DMA_CHN1_WIDTH          1280
#define DMA_CHN1_HEIGHT         720
#define DMA_CHN1_SRC_STRIDE     1280
#define DMA_CHN1_DST_STRIDE     1280
// #define DMA_CHN1_WIDTH          128
// #define DMA_CHN1_HEIGHT         64
// #define DMA_CHN1_SRC_STRIDE     128
// #define DMA_CHN1_DST_STRIDE     128
// #define DMA_CHN1_WIDTH          640
// #define DMA_CHN1_HEIGHT         320
// #define DMA_CHN1_SRC_STRIDE     640
// #define DMA_CHN1_DST_STRIDE     640

#define DMA_CHN2                2
#define DMA_CHN2_WIDTH          1280
#define DMA_CHN2_HEIGHT         720
#define DMA_CHN2_SRC_STRIDE     (DMA_CHN2_WIDTH * 2)
#define DMA_CHN2_DST_STRIDE     (DMA_CHN2_WIDTH * 2)
// #define DMA_CHN2_WIDTH          128
// #define DMA_CHN2_HEIGHT         64
// #define DMA_CHN2_SRC_STRIDE     (DMA_CHN2_WIDTH * 2)
// #define DMA_CHN2_DST_STRIDE     (DMA_CHN2_WIDTH * 2)
// #define DMA_CHN2_WIDTH          640
// #define DMA_CHN2_HEIGHT         320
// #define DMA_CHN2_SRC_STRIDE     (DMA_CHN2_WIDTH * 2)
// #define DMA_CHN2_DST_STRIDE     (DMA_CHN2_WIDTH * 2)


#define DMA_CHN4                4
#define DMA_CHN4_LINE_SIZE      0x1000

#define DMA_CHN5                5
#define DMA_CHN5_LINE_SIZE      0x40
#define DMA_CHN5_LINE_SPACE     0xc0
#define DMA_CHN5_LINE_NUM       0x05

k_u8 planar[8] = {1, 1, 1, 0, 1, 1, 0, 0};
k_dma_dev_attr_t dev_attr;
k_dma_chn_attr_u chn_attr[DMA_MAX_CHN_NUMS];
k_video_frame_info df_info[DMA_MAX_CHN_NUMS];
k_video_frame_info df_info_dst[DMA_MAX_CHN_NUMS];
k_u32 gdma_size[4] = {0, 0, 0, 0};
k_bool g_end = K_FALSE;

static pthread_t tid1;

static void *getchar_entry(void *parameter)
{
    while (1)
    {
        if((char)(getchar()) == 'q')
        {
            g_end = K_TRUE;
            break;
        }
        usleep(50000);
    }

    return NULL;
}

static k_s32 dma_chn_attr_init(k_dma_chn_attr_u attr[8])
{
    // k_u32 i;
    k_gdma_chn_attr_t *gdma_attr;
    k_sdma_chn_attr_t *sdma_attr;

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
    gdma_attr->work_mode = DMA_UNBIND;
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
    gdma_attr->src_stride[1] = DMA_CHN1_SRC_STRIDE;
    gdma_attr->dst_stride[1] = DMA_CHN1_DST_STRIDE;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_8BIT;

    /* channel 2 */
    gdma_attr = &attr[2].gdma_attr;
    gdma_attr->buffer_num = 3;
    gdma_attr->rotation = DEGREE_0;
    gdma_attr->x_mirror = K_TRUE;
    gdma_attr->y_mirror = K_TRUE;
    gdma_attr->width = DMA_CHN2_WIDTH;
    gdma_attr->height = DMA_CHN2_HEIGHT;
    gdma_attr->src_stride[0] = DMA_CHN2_SRC_STRIDE;
    gdma_attr->dst_stride[0] = DMA_CHN2_DST_STRIDE;
    gdma_attr->src_stride[1] = DMA_CHN2_SRC_STRIDE / 2;
    gdma_attr->dst_stride[1] = DMA_CHN2_DST_STRIDE / 2;
    gdma_attr->src_stride[2] = DMA_CHN2_SRC_STRIDE / 2;
    gdma_attr->dst_stride[2] = DMA_CHN2_DST_STRIDE / 2;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_PLANAR_420_10BIT;

    /* channel 4 */
    sdma_attr = &attr[4].sdma_attr;
    sdma_attr->buffer_num = 3;
    sdma_attr->line_size = DMA_CHN4_LINE_SIZE;
    sdma_attr->data_mode = DIMENSION1;
    sdma_attr->work_mode = DMA_UNBIND;

    /* channel 5 */
    sdma_attr = &attr[5].sdma_attr;
    sdma_attr->buffer_num = 3;
    sdma_attr->line_size = DMA_CHN5_LINE_SIZE;
    sdma_attr->line_space = DMA_CHN5_LINE_SPACE;
    sdma_attr->line_num = DMA_CHN5_LINE_NUM;
    sdma_attr->data_mode = DIMENSION2;
    sdma_attr->work_mode = DMA_UNBIND;

    return K_SUCCESS;
}

static k_s32 dma_chn_data_init(k_s32 chn_num, k_dma_chn_attr_u *attr, k_video_frame_info *df_info)
{
    k_s32 i, j;
    k_s32 line_size, line_space, line_num;
    k_s16 width, height;
    k_u8 *addr;
    k_u16 *addr_16bit;

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
            /* Y */
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

            /* UV */
            addr = (k_u8 *)df_info->v_frame.virt_addr[1];
            for (i = 0; i < height / 2; i++)
            {
                for (j = 0; j < width; j++)
                {
                    addr[i * width + j] = j % 256;
                }
            }
            printf("\n");
        }
        else if (chn_num == DMA_CHN2)
        {
            addr_16bit = (k_u16 *)df_info->v_frame.virt_addr[0];
            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    addr_16bit[i * width + j] = j % 256;
                }
            }

            addr_16bit = (k_u16 *)df_info->v_frame.virt_addr[1];
            for (i = 0; i < height / 2; i++)
            {
                for (j = 0; j < width / 2; j++)
                {
                    if (j % 2 == 0)
                    {
                        addr_16bit[i * width / 2 + j] = 1;
                    }
                    else
                    {
                        addr_16bit[i * width / 2 + j] = 2;
                    }
                }
            }

            addr_16bit = (k_u16 *)df_info->v_frame.virt_addr[2];
            for (i = 0; i < height / 2; i++)
            {
                for (j = 0; j < width / 2; j++)
                {
                    if (j % 2 == 0)
                    {
                        addr_16bit[i * width / 2 + j] = 2;
                    }
                    else
                    {
                        addr_16bit[i * width / 2 + j] = 1;
                    }
                }
            }
        }
    }
    else
    {
        addr = (k_u8 *)df_info->v_frame.virt_addr[0];
        line_size = attr->sdma_attr.line_size;
        line_space = attr->sdma_attr.line_space;
        line_num = attr->sdma_attr.line_num;
        if (attr->sdma_attr.data_mode == DIMENSION1)
        {
            for (i = 0; i < line_size; i++)
            {
                addr[i] = i % 256;
            }
        }
        else
        {
            for (i = 0; i < (line_size + line_space) * line_num; i++)
            {
                addr[i] = i % 256;
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
                config.comm_pool[i].blk_cnt = (DMA_BUFF_NUM + 1);
                config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
                config.comm_pool[i].blk_size = size;
            }
            else if (i == DMA_CHN1)
            {
                config.comm_pool[i].blk_cnt = (DMA_BUFF_NUM + 1);
                config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
                config.comm_pool[i].blk_size = size / 2 * 3;
            }
            if (i == DMA_CHN2)
            {
                config.comm_pool[i].blk_cnt = (DMA_BUFF_NUM + 1);
                config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
                config.comm_pool[i].blk_size = size / 2 * 3;
            }
            gdma_size[i] = config.comm_pool[i].blk_size;
        }
        else
        {
            config.comm_pool[i].blk_cnt = DMA_BUFF_NUM + 1;
            config.comm_pool[i].mode = VB_REMAP_MODE_NOCACHE;
            if (attr[i].sdma_attr.data_mode == DIMENSION1)
            {
                config.comm_pool[i].blk_size = attr[i].sdma_attr.line_size;
            }
            else
            {
                config.comm_pool[i].blk_size = (attr[i].sdma_attr.line_size + attr[i].sdma_attr.line_space) *
                                               attr[i].sdma_attr.line_num;
            }
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
        df_info->v_frame.phys_addr[1] = df_info->v_frame.phys_addr[0] + size * 2 / 3;
        df_info->v_frame.virt_addr[1] = df_info->v_frame.virt_addr[0] + size * 2 / 3;
    }
    else if (chn_num == DMA_CHN2)
    {
        df_info->v_frame.phys_addr[0] = phys_addr;
        df_info->v_frame.virt_addr[0] = (k_u64)(intptr_t)virt_addr;
        df_info->v_frame.phys_addr[1] = df_info->v_frame.phys_addr[0] + size * 2 / 3;
        df_info->v_frame.virt_addr[1] = df_info->v_frame.virt_addr[0] + size * 2 / 3;
        df_info->v_frame.phys_addr[2] = df_info->v_frame.phys_addr[1] + size / 3 / 2;
        df_info->v_frame.virt_addr[2] = df_info->v_frame.virt_addr[1] + size / 3 / 2;
    }
    else if (chn_num == DMA_CHN4)
    {
        df_info->v_frame.phys_addr[0] = phys_addr;
        df_info->v_frame.virt_addr[0] = (k_u64)(intptr_t)virt_addr;
    }
    else if (chn_num == DMA_CHN5)
    {
        df_info->v_frame.phys_addr[0] = phys_addr;
        df_info->v_frame.virt_addr[0] = (k_u64)(intptr_t)virt_addr;
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

    /* prepare chn2 data */
    size = gdma_size[2];
    sample_dma_get_blk(size, &df_info[DMA_CHN2], DMA_CHN2);
    dma_chn_data_init(DMA_CHN2, &attr[DMA_CHN2], &df_info[DMA_CHN2]);

    /* prepare chn4 data*/
    size = attr[4].sdma_attr.line_size;
    sample_dma_get_blk(size, &df_info[DMA_CHN4], DMA_CHN4);
    dma_chn_data_init(DMA_CHN4, &attr[DMA_CHN4], &df_info[DMA_CHN4]);

    /* prepare chn5 data*/
    size = (attr[5].sdma_attr.line_size + attr[5].sdma_attr.line_space) * attr[5].sdma_attr.line_num;
    sample_dma_get_blk(size, &df_info[DMA_CHN5], DMA_CHN5);
    dma_chn_data_init(DMA_CHN5, &attr[DMA_CHN5], &df_info[DMA_CHN5]);

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

    /* release chn1 data */
    size = gdma_size[2];
    sample_dma_put_blk(size, df_info[DMA_CHN2].v_frame.phys_addr[0]);

    /* release chn4 data */
    size = attr[4].sdma_attr.line_size;
    sample_dma_put_blk(size, df_info[DMA_CHN4].v_frame.phys_addr[0]);

    /* release chn5 data */
    size = (attr[5].sdma_attr.line_size + attr[5].sdma_attr.line_space) * attr[5].sdma_attr.line_num;
    sample_dma_put_blk(size, df_info[DMA_CHN5].v_frame.phys_addr[0]);

    return K_SUCCESS;
}

static k_s32 dma_dev_attr_init(k_dma_dev_attr_t *dev_attr)
{
    dev_attr->burst_len = 0;
    dev_attr->ckg_bypass = 0xff;
    dev_attr->outstanding = 7;

    return K_SUCCESS;
}

static k_s32 dma_get_dev_attr()
{
    k_s32 ret;
    k_dma_dev_attr_t dev_attr;

    ret = kd_mpi_dma_get_dev_attr(&dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("get dev attr error\r\n");
        return K_FAILED;
    }
    printf("dev_attr: burst_len: %d, ckg_bypass: %d, outstanding: %d\r\n",
           dev_attr.burst_len, dev_attr.ckg_bypass, dev_attr.outstanding);

    return K_SUCCESS;
}

static k_s32 dma_get_chn_attr(k_s32 chn_num)
{
    k_s32 ret;
    k_dma_chn_attr_u chn_attr;

    if (chn_num >= DMA_MAX_CHN_NUMS)
        return K_FAILED;

    if (chn_num < GDMA_MAX_CHN_NUMS)
    {
        ret = kd_mpi_dma_get_chn_attr(chn_num, &chn_attr);
        if (ret != K_SUCCESS)
        {
            printf("get chn attr error\r\n");
            return K_FAILED;
        }
        // printf("chn_attr: rotation:%d, x_mirror:%d, y_mirror:%d\r\n",
        //     chn_attr.gdma_attr.rotation, chn_attr.gdma_attr.x_mirror, chn_attr.gdma_attr.y_mirror);
        // printf("chn_attr: width:%d, height:%d, work_mode:%d, pixel_format:%d\r\n",
        //     chn_attr.gdma_attr.width, chn_attr.gdma_attr.height, chn_attr.gdma_attr.work_mode, chn_attr.gdma_attr.pixel_format);
        // printf("chn_attr: src_stride[0]:%d, src_stride[0]:%d, src_stride[0]:%d\r\n",
        //     chn_attr.gdma_attr.src_stride[0], chn_attr.gdma_attr.src_stride[1], chn_attr.gdma_attr.src_stride[2]);
        // printf("chn_attr: dst_stride[0]:%d, dst_stride[0]:%d, dst_stride[0]:%d\r\n",
        //     chn_attr.gdma_attr.dst_stride[0], chn_attr.gdma_attr.dst_stride[1], chn_attr.gdma_attr.dst_stride[2]);
    }
    else
    {
        ret = kd_mpi_dma_get_chn_attr(chn_num, &chn_attr);
        if (ret != K_SUCCESS)
        {
            printf("get chn attr error\r\n");
            return K_FAILED;
        }
        // printf("chn_attr: line_size:%d, line_num:%d, line_space:%d\r\n",
        //     chn_attr.sdma_attr.line_size, chn_attr.sdma_attr.line_num, chn_attr.sdma_attr.line_space);
        // printf("chn_attr: data_mode:%d, work_mode:%d\r\n",
        //     chn_attr.sdma_attr.data_mode, chn_attr.sdma_attr.work_mode);
    }

    return K_SUCCESS;
}

static k_s32 check_result(k_s32 chn_num, k_dma_chn_attr_u *attr, k_video_frame_info *df_info_src, k_video_frame_info *df_info_dst)
{
    k_s32 i, j;
    k_s32 line_size;
    k_s32 line_num;
    k_s16 width, height, src_stride, dst_stride;
    k_u8 *src_addr;
    k_u8 *dst_addr;
    k_u16 *src_16;
    k_u16 *dst_16;
    k_u32 src_index, dst_index;

    if (chn_num < GDMA_MAX_CHN_NUMS)
    {
        if (chn_num == DMA_CHN0)
        {
            src_addr = (k_u8 *)df_info_src->v_frame.virt_addr[0];
            dst_addr = (k_u8 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[0], gdma_size[0]);
            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            src_stride = attr->gdma_attr.src_stride[0];
            dst_stride = attr->gdma_attr.dst_stride[0];
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    src_index = i * src_stride + j;
                    dst_index = j * dst_stride + height - i - 1;
                    if (src_addr[src_index] != dst_addr[dst_index])
                    {
                        printf("src_addr[%4d]:%2x, dst_adr[%4d]:%2x\r\n",
                               src_index, src_addr[src_index], dst_index, dst_addr[dst_index]);
                        return K_FAILED;
                    }
                }
            }
            printf("**************DMA_CHN%d success**************\n", DMA_CHN0);
        }
        else if (chn_num == DMA_CHN1)
        {
            src_addr = (k_u8 *)df_info_src->v_frame.virt_addr[0];
            dst_addr = (k_u8 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[0], gdma_size[1]);
            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            src_stride = attr->gdma_attr.src_stride[0];
            dst_stride = attr->gdma_attr.dst_stride[0];
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    src_index = i * src_stride + j;
                    dst_index = dst_stride * (height - i - 1) + width - j - 1;
                    if (src_addr[src_index] != dst_addr[dst_index])
                    {
                        printf("src_addr[%4d]:%2x, dst_adr[%4d]:%2x\r\n",
                               src_index, src_addr[src_index], dst_index, dst_addr[dst_index]);
                        return K_FAILED;
                    }
                }
            }

            src_16 = (k_u16 *)df_info_src->v_frame.virt_addr[1];
            dst_16 = (k_u16 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[1], gdma_size[1] / 2);

            src_stride = attr->gdma_attr.src_stride[1];
            dst_stride = attr->gdma_attr.dst_stride[1];
            for (i = 0; i < height / 2; i++)
            {
                for (j = 0; j < width / 2; j++)
                {
                    src_index = i * src_stride / 2 + j;
                    dst_index = dst_stride / 2 * (height / 2 - i - 1) + width / 2 - j - 1;
                    if (src_16[src_index] != dst_16[dst_index])
                    {
                        printf("src_addr[%4d]:%04x, dst_adr[%4d]:%04x\r\n",
                               src_index, src_16[src_index], dst_index, dst_16[dst_index]);
                        return K_FAILED;
                    }
                }
            }
            printf("**************DMA_CHN%d success**************\n", DMA_CHN1);
        }
        else if (chn_num == DMA_CHN2)
        {
            src_16 = (k_u16 *)df_info_src->v_frame.virt_addr[0];
            dst_16 = (k_u16 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[0], gdma_size[2] / 3 * 2);

            width = attr->gdma_attr.width;
            height = attr->gdma_attr.height;
            src_stride = attr->gdma_attr.src_stride[0] / 2;
            dst_stride = attr->gdma_attr.dst_stride[0] / 2;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    src_index = i * src_stride + j;
                    dst_index = dst_stride * (height - i - 1) + width - j - 1;
                    if (src_16[src_index] != dst_16[dst_index])
                    {
                        printf("src_addr[%2d]:%2x, dst_addr[%4d]:%2x\r\n",
                               src_index, src_16[src_index], dst_index, dst_16[dst_index]);
                        return K_FAILED;
                    }
                }
            }

            src_16 = (k_u16 *)df_info_src->v_frame.virt_addr[1];
            dst_16 = (k_u16 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[1], gdma_size[2] / 3 / 2);
            width = attr->gdma_attr.width / 2;
            height = attr->gdma_attr.height / 2;
            src_stride = attr->gdma_attr.src_stride[1] / 2;
            dst_stride = attr->gdma_attr.dst_stride[1] / 2;
            for (i = 0; i < height; i ++)
            {
                for (j = 0; j < width; j++)
                {
                    src_index = i * src_stride + j;
                    dst_index = dst_stride * (height - i - 1) + width - j - 1;
                    if (src_16[src_index] != dst_16[dst_index])
                    {
                        printf("src_addr[%2d]:%2x, dst_addr[%4d]:%2x\r\n",
                               src_index, src_16[src_index], dst_index, dst_16[dst_index]);
                        return K_FAILED;
                    }
                }
            }

            src_16 = (k_u16 *)df_info_src->v_frame.virt_addr[2];
            dst_16 = (k_u16 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[2], gdma_size[2] / 3 / 2);
            width = attr->gdma_attr.width / 2;
            height = attr->gdma_attr.height / 2;
            src_stride = attr->gdma_attr.src_stride[1] / 2;
            dst_stride = attr->gdma_attr.dst_stride[1] / 2;
            for (i = 0; i < height; i ++)
            {
                for (j = 0; j < width; j++)
                {
                    src_index = i * src_stride + j;
                    dst_index = dst_stride * (height - i - 1) + width - j - 1;
                    if (src_16[src_index] != dst_16[dst_index])
                    {
                        printf("src_addr[%2d]:%2x, dst_addr[%4d]:%2x\r\n",
                               src_index, src_16[src_index], dst_index, dst_16[dst_index]);
                        return K_FAILED;
                    }
                }
            }
            printf("**************DMA_CHN%d success**************\n", DMA_CHN2);
        }
    }
    else
    {
        if (chn_num == DMA_CHN4)
        {
            line_size = attr->sdma_attr.line_size;
            src_addr = (k_u8 *)df_info_src->v_frame.virt_addr[0];
            dst_addr = (k_u8 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[0], line_size);
            for (i = 0; i < line_size; i++)
            {
                if (src_addr[i] != dst_addr[i])
                {
                    printf("src_addr[%4d]:%2x, dst_addr[%4d]:%2x\r\n",
                           i, src_addr[i], i, dst_addr[i]);
                    return K_FAILED;
                }
            }
            printf("**************DMA_CHN%d success**************\n", DMA_CHN4);
        }
        else if (chn_num == DMA_CHN5)
        {
            line_num = 0;
            line_size = (attr->sdma_attr.line_size + attr->sdma_attr.line_space) * attr->sdma_attr.line_num;
            src_addr = (k_u8 *)df_info_src->v_frame.virt_addr[0];
            dst_addr = (k_u8 *)kd_mpi_sys_mmap(df_info_dst->v_frame.phys_addr[0], line_size);
            for (i = 0; i < (DMA_CHN5_LINE_SIZE + DMA_CHN5_LINE_SPACE) * DMA_CHN5_LINE_NUM; i++)
            {
                if (i % (DMA_CHN5_LINE_SIZE + DMA_CHN5_LINE_SPACE) >= DMA_CHN5_LINE_SIZE)
                {
                    continue;
                }
                line_num = i / (DMA_CHN5_LINE_SIZE + DMA_CHN5_LINE_SPACE);
                if (src_addr[i] != dst_addr[i - line_num * DMA_CHN5_LINE_SPACE])
                {
                    printf("src_addr[%4d]:%2x, dst_addr[%4d]:%2x\r\n",
                           i, src_addr[i], i, dst_addr[i]);
                    return K_FAILED;
                }
            }
            printf("**************DMA_CHN%d success**************\n", DMA_CHN5);
        }
    }

    return K_SUCCESS;
}

int main(void)
{
    k_s32 ret;
    k_u32 loop = 0;
    k_s32 check_ret = K_SUCCESS;

    printf("dma sample case, press q to end the operation.\n");

    ret = pthread_create(&tid1, NULL, getchar_entry, NULL);
    if (ret)
        printf("create pthread failed\n");

    memset(chn_attr, 0, sizeof(k_dma_chn_attr_u)*DMA_MAX_CHN_NUMS);

    dma_dev_attr_init(&dev_attr);
    dma_chn_attr_init(chn_attr);

    if (dma_vb_init(chn_attr, planar))
    {
        return -1;
    }

    sample_dma_prepare_data(chn_attr, planar, df_info);

    ret = kd_mpi_dma_set_dev_attr(&dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("set dev attr error\r\n");
        goto exit_label;
    }

    ret = dma_get_dev_attr();
    if (ret != K_SUCCESS)
        goto exit_label;

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS)
    {
        printf("start dev error\r\n");
        goto exit_label;
    }

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

    ret = dma_get_chn_attr(DMA_CHN0);
    if (ret != K_SUCCESS)
        goto exit_label;

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

    ret = dma_get_chn_attr(DMA_CHN1);
    if (ret != K_SUCCESS)
        goto exit_label;

    /* DMA_CHN1 prepare */
    ret = kd_mpi_dma_set_chn_attr(DMA_CHN2, &chn_attr[DMA_CHN2]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto exit_label;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN2);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto exit_label;
    }

    ret = dma_get_chn_attr(DMA_CHN2);
    if (ret != K_SUCCESS)
        goto exit_label;

    /* DMA_CHN4 prepare*/
    ret = kd_mpi_dma_set_chn_attr(DMA_CHN4, &chn_attr[DMA_CHN4]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto exit_label;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN4);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto exit_label;
    }

    ret = dma_get_chn_attr(DMA_CHN4);
    if (ret != K_SUCCESS)
        goto exit_label;

    /* DMA_CHN5 prepare*/
    ret = kd_mpi_dma_set_chn_attr(DMA_CHN5, &chn_attr[DMA_CHN5]);
    if (ret != K_SUCCESS)
    {
        printf("set chn attr error\r\n");
        goto exit_label;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN5);
    if (ret != K_SUCCESS)
    {
        printf("start chn error\r\n");
        goto exit_label;
    }

    ret = dma_get_chn_attr(DMA_CHN5);
    if (ret != K_SUCCESS)
        goto exit_label;

    while (1)
    {
        /* DMA_CHN0 send */
        ret = kd_mpi_dma_send_frame(DMA_CHN0, &df_info[DMA_CHN0], BLOCK_TIME);
        if (ret != K_SUCCESS)
        {
            printf("send frame error\r\n");
            goto exit_label;
        }

        ret = kd_mpi_dma_get_frame(DMA_CHN0, &df_info_dst[DMA_CHN0], -1);
        if (ret != K_SUCCESS)
        {
            printf("get frame error\r\n");
            goto exit_label;
        }

        check_ret += check_result(DMA_CHN0, &chn_attr[DMA_CHN0], &df_info[DMA_CHN0], &df_info_dst[DMA_CHN0]);
        kd_mpi_dma_release_frame(DMA_CHN0,  &df_info_dst[DMA_CHN0]);

        /* DMA_CHN1 send */
        ret = kd_mpi_dma_send_frame(DMA_CHN1, &df_info[DMA_CHN1], BLOCK_TIME);
        if (ret != K_SUCCESS)
        {
            printf("send frame error\r\n");
            goto exit_label;
        }

        ret = kd_mpi_dma_get_frame(DMA_CHN1, &df_info_dst[DMA_CHN1], -1);
        if (ret != K_SUCCESS)
        {
            printf("get frame error\r\n");
            goto exit_label;
        }

        check_ret += check_result(DMA_CHN1, &chn_attr[DMA_CHN1], &df_info[DMA_CHN1], &df_info_dst[DMA_CHN1]);
        kd_mpi_dma_release_frame(DMA_CHN1,  &df_info_dst[DMA_CHN1]);

        /* DMA_CHN2 send */
        ret = kd_mpi_dma_send_frame(DMA_CHN2, &df_info[DMA_CHN2], BLOCK_TIME);
        if (ret != K_SUCCESS)
        {
            printf("send frame error\r\n");
            goto exit_label;
        }

        ret = kd_mpi_dma_get_frame(DMA_CHN2, &df_info_dst[DMA_CHN2], -1);
        if (ret != K_SUCCESS)
        {
            printf("get frame error\r\n");
            goto exit_label;
        }

        check_ret += check_result(DMA_CHN2, &chn_attr[DMA_CHN2], &df_info[DMA_CHN2], &df_info_dst[DMA_CHN2]);
        kd_mpi_dma_release_frame(DMA_CHN2,  &df_info_dst[DMA_CHN2]);

        /* DMA_CHN4 send */
        ret = kd_mpi_dma_send_frame(DMA_CHN4, &df_info[DMA_CHN4], BLOCK_TIME);
        if (ret != K_SUCCESS)
        {
            printf("send frame error\r\n");
            goto exit_label;
        }

        ret = kd_mpi_dma_get_frame(DMA_CHN4, &df_info_dst[DMA_CHN4], -1);
        if (ret != K_SUCCESS)
        {
            printf("get frame error\r\n");
            goto exit_label;
        }

        check_ret += check_result(DMA_CHN4, &chn_attr[DMA_CHN4], &df_info[DMA_CHN4], &df_info_dst[DMA_CHN4]);
        kd_mpi_dma_release_frame(DMA_CHN4,  &df_info_dst[DMA_CHN4]);

        /* DMA_CHN5 send */
        ret = kd_mpi_dma_send_frame(DMA_CHN5, &df_info[DMA_CHN5], BLOCK_TIME);
        if (ret != K_SUCCESS)
        {
            printf("send frame error\r\n");
            goto exit_label;
        }

        ret = kd_mpi_dma_get_frame(DMA_CHN5, &df_info_dst[DMA_CHN5], -1);
        if (ret != K_SUCCESS)
        {
            printf("get frame error\r\n");
            goto exit_label;
        }

        check_ret += check_result(DMA_CHN5, &chn_attr[DMA_CHN5], &df_info[DMA_CHN5], &df_info_dst[DMA_CHN5]);
        kd_mpi_dma_release_frame(DMA_CHN5,  &df_info_dst[DMA_CHN5]);

        loop++;
        printf("loop:%05d\n", loop);
        if (check_ret != 0)
        {
            printf("error times:%d\n", -check_ret);
        }

        if (g_end == K_TRUE)
        {
            break;
        }

        sleep(1);
    }
    pthread_detach(tid1);

    /* stop */
    ret = kd_mpi_dma_stop_chn(DMA_CHN0);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN1);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN2);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN4);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN5);
    if (ret != K_SUCCESS)
    {
        printf("stop chn error\r\n");
        goto exit_label;
    }

    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS)
    {
        printf("stop dev error\r\n");
        goto exit_label;
    }

exit_label:
    sample_dma_release_data(chn_attr, planar, df_info);

    dma_vb_exit();
    return 0;
}