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

#include "sample_dpu_vicap.h"

/* dma global variable */
k_dma_dev_attr_t dma_dev_attr;
k_dma_chn_attr_u chn_attr[DMA_MAX_CHN_NUMS];

static k_s32 dma_dev_attr_init(k_dma_dev_attr_t *dev_attr)
{
    dev_attr->burst_len = 0;
    dev_attr->ckg_bypass = 0xff;
    dev_attr->outstanding = 7;

    return K_SUCCESS;
}

static k_s32 dma_chn_attr_init(k_dma_chn_attr_u attr[8], k_bool gen_calibration)
{
    k_gdma_chn_attr_t *gdma_attr;

    if(gen_calibration)
    {
        gdma_attr = &attr[DMA_CHN1].gdma_attr;
        gdma_attr->buffer_num = 1;
        if(gdma_attr->rotation == DEGREE_0)
            gdma_attr->rotation = DEGREE_270;
        gdma_attr->x_mirror = K_FALSE;
        gdma_attr->y_mirror = K_FALSE;
        gdma_attr->width = DMA_CHN0_WIDTH;
        gdma_attr->height = DMA_CHN0_HEIGHT;
        gdma_attr->src_stride[0] = DMA_CHN0_SRC_STRIDE;
        gdma_attr->dst_stride[0] = DMA_CHN0_DST_STRIDE;
        gdma_attr->src_stride[1] = DMA_CHN0_SRC_STRIDE;
        gdma_attr->dst_stride[1] = DMA_CHN0_DST_STRIDE;
        gdma_attr->work_mode = DMA_UNBIND;
        gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YVU_SEMIPLANAR_420_8BIT;
    }
    else
    {
        gdma_attr = &attr[DMA_CHN1].gdma_attr;
        gdma_attr->buffer_num = 1;
        if(gdma_attr->rotation == DEGREE_0)
            gdma_attr->rotation = DEGREE_90;
        gdma_attr->x_mirror = K_FALSE;
        gdma_attr->y_mirror = K_FALSE;
        gdma_attr->width = DMA_CHN0_WIDTH;
        gdma_attr->height = DMA_CHN0_HEIGHT;
        gdma_attr->src_stride[0] = DMA_CHN0_SRC_STRIDE;
        gdma_attr->dst_stride[0] = DMA_CHN0_DST_STRIDE;
        gdma_attr->work_mode = DMA_UNBIND;
        gdma_attr->pixel_format = DMA_PIXEL_FORMAT_YUV_400_8BIT;
    }

    gdma_attr = &attr[DMA_CHN0].gdma_attr;
    gdma_attr->buffer_num = 1;
    if(gdma_attr->rotation == DEGREE_0)
        gdma_attr->rotation = DEGREE_90;
    gdma_attr->x_mirror = K_FALSE;
    gdma_attr->y_mirror = K_FALSE;
    gdma_attr->width = DMA_CHN0_WIDTH;
    gdma_attr->height = DMA_CHN0_HEIGHT;
    gdma_attr->src_stride[0] = DMA_CHN0_SRC_STRIDE;
    gdma_attr->dst_stride[0] = DMA_CHN0_DST_STRIDE;
    gdma_attr->src_stride[1] = DMA_CHN0_SRC_STRIDE;
    gdma_attr->dst_stride[1] = DMA_CHN0_DST_STRIDE;
    gdma_attr->src_stride[2] = DMA_CHN0_SRC_STRIDE;
    gdma_attr->dst_stride[2] = DMA_CHN0_DST_STRIDE;
    gdma_attr->work_mode = DMA_UNBIND;
    gdma_attr->pixel_format = DMA_PIXEL_FORMAT_BGR_888_PLANAR;

    printf("gdma rotation: ch 0 = %d, ch 1 = %d\n", attr[DMA_CHN0].gdma_attr.rotation, attr[DMA_CHN1].gdma_attr.rotation);

    return K_SUCCESS;
}

int sample_dv_dma_init(k_gdma_rotation_e *dma_rotation, k_bool gen_calibration)
{
    k_s32 ret;

    /************************************************************
     * This part is used to initialize the parameters of the dma.
     ***********************************************************/
    memset(chn_attr, 0, sizeof(k_dma_chn_attr_u)*DMA_MAX_CHN_NUMS);
    chn_attr[0].gdma_attr.rotation = dma_rotation[0];
    chn_attr[1].gdma_attr.rotation = dma_rotation[1];

    dma_dev_attr_init(&dma_dev_attr);
    dma_chn_attr_init(chn_attr, gen_calibration);

    /************************************************************
     * This part is the demo that actually starts to use DMA
     ***********************************************************/
    ret = kd_mpi_dma_set_dev_attr(&dma_dev_attr);
    if (ret != K_SUCCESS) {
        printf("set dev attr error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS) {
        printf("start dev error\r\n");
        goto err_return;
    }

    ret = kd_mpi_dma_set_chn_attr(DMA_CHN0, &chn_attr[DMA_CHN0]);
    if (ret != K_SUCCESS) {
        printf("set chn attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN0);
    if (ret != K_SUCCESS) {
        printf("start chn error\r\n");
        goto err_dma_dev;
    }

    ret = kd_mpi_dma_set_chn_attr(DMA_CHN1, &chn_attr[DMA_CHN1]);
    if (ret != K_SUCCESS) {
        printf("set chn attr error\r\n");
        goto err_dma_dev;
    }
    ret = kd_mpi_dma_start_chn(DMA_CHN1);
    if (ret != K_SUCCESS) {
        printf("start chn error\r\n");
        goto err_dma_dev;
    }

    return K_SUCCESS;

    /************************************************************
     * This part is used to stop the DMA
     ***********************************************************/
    ret = kd_mpi_dma_stop_chn(DMA_CHN0);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN1);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

err_dma_dev:
    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS) {
        printf("stop dev error\r\n");
    }

err_return:
    return K_FAILED;
}

int sample_dv_dma_delete()
{
    k_s32 ret;

    /************************************************************
     * This part is used to stop the DMA
     ***********************************************************/
    ret = kd_mpi_dma_stop_chn(DMA_CHN0);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_chn(DMA_CHN1);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

    ret = kd_mpi_dma_stop_dev();
    if (ret != K_SUCCESS) {
        printf("stop dev error\r\n");
    }

    return K_SUCCESS;
}