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

#ifndef __SAMPLE_DPU_VO_H
#define __SAMPLE_DPU_VO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_dma_comm.h"
#include "k_dpu_comm.h"
#include "k_sys_comm.h"
#include "k_vo_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_dma_api.h"
#include "mpi_dpu_api.h"
#include "mpi_sys_api.h"
#include "mpi_vo_api.h"

#define SAMPLE_VDD_CHN_NUM  1
#define SAMPLE_VDD_CHN      0
#define VDD_INPUT_BUF_CNT   6
#define DPU_FRAME_COUNT     3

#define DMA_CHN0                0
#define DMA_CHN0_WIDTH          1280
#define DMA_CHN0_HEIGHT         720
#define DMA_CHN0_SRC_STRIDE     1280
#define DMA_CHN0_DST_STRIDE     720

typedef struct
{
    k_u32 img_width;
    k_u32 img_height;
} sample_dv_cfg_t;

static inline void VDD_CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

/* vo */
#define PRIVATE_POLL_SZE                        (1920 * 1080 * 3 / 2) + (4096 * 2)
#define PRIVATE_POLL_NUM                        (4)

typedef struct
{
    k_u64 osd_phy_addr;
    void *osd_virt_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
} dv_osd_info;


/* vi */
void sample_dv_vicap_config(k_u32 ch, k_s32 sensor_index);
void sample_dv_vicap_start(k_u32 ch);
void sample_dv_vicap_stop(k_u32 ch);
int sample_dv_dma_init();
int sample_dv_dma_delete();
int sample_dv_dpu_init();
int sample_dv_dpu_delete();
int sample_dv_vo_init(k_bool mirror);

#endif