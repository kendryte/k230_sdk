/**
 * @file vo_test_case.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-09-01
 *
 * @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#ifndef __VO_H__
#define __VO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/ioctl.h"
#include <stdatomic.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

#include "k_vicap_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "k_video_comm.h"


#include <iostream>
#include <codecvt>
#include <locale>
#include <string>


using namespace std;

#define VICAP_VO_TESET                         1   

#define PRIVATE_POLL_SZE                        (1920 * 1080 * 3 / 2) + (4096 * 2)
#define PRIVATE_POLL_NUM                        (4)

#define ISP_CHN1_WIDTH  (320)
#define ISP_CHN1_HEIGHT (240)

#define ISP_CHN0_WIDTH  (320)
#define ISP_CHN0_HEIGHT (240)

#define ISP_INPUT_WIDTH (640)
#define ISP_INPUT_HEIGHT (480)
#define ISP_CROP_W_OFFSET (0)
#define ISP_CROP_H_OFFSET (0)

#define VICAP_MAX_FRAME_COUNT     (30)

#define VICAP_ALIGN_1K 0x400
#define VICAP_ALIGN_UP(addr, size)	(((addr)+((size)-1U))&(~((size)-1U)))


#define	GPIO_DM_OUTPUT           _IOW('G', 0, int)
#define	GPIO_DM_INPUT            _IOW('G', 1, int)
#define	GPIO_DM_INPUT_PULL_UP    _IOW('G', 2, int)
#define	GPIO_DM_INPUT_PULL_DOWN  _IOW('G', 3, int)
#define	GPIO_WRITE_LOW           _IOW('G', 4, int)
#define	GPIO_WRITE_HIGH          _IOW('G', 5, int)

#define	GPIO_PE_RISING           _IOW('G', 7, int)
#define	GPIO_PE_FALLING          _IOW('G', 8, int)
#define	GPIO_PE_BOTH             _IOW('G', 9, int)
#define	GPIO_PE_HIGH             _IOW('G', 10, int)
#define	GPIO_PE_LOW              _IOW('G', 11, int)

#define GPIO_READ_VALUE       	_IOW('G', 12, int)

#define KEY_PIN_NUM1    46                  //gpio46
#define KEY_PIN_NUM2    27                  //gpio27

#define LED_PIN_NUM1    42                  //GPIO42
#define LED_PIN_NUM2    42                  //  ???

typedef struct kd_pin_mode
{
    unsigned short pin;     /* pin number, from 0 to 63 */
    unsigned short mode;    /* pin level status, 0 low level, 1 high level */
} pin_mode_t;


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
} osd_info;

typedef struct
{
    k_u64 layer_phy_addr;
    k_pixel_format format;
    k_vo_point offset;
    k_vo_size act_size;
    k_u32 size;
    k_u32 stride;
    k_u8 global_alptha;
    //only layer0、layer1
    k_u32 func;
    // only layer0
    k_vo_scaler_attr attr;

} layer_info;

typedef struct {
    osd_info osd;
    k_u32 g_pool_id;
}osd_plane;

typedef struct {
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    osd_plane plane;
    k_vo_osd chn;
    k_u32 exit_flag;
    pthread_mutex_t mutex_key;
    k_u32 key_val;

    pthread_mutex_t stitch;
    k_u32 stitch_val;
    
}cidianbi_attr;

extern cidianbi_attr cidianbi;

k_s32 sample_connector_init(k_connector_type type);
k_u32 sample_vo_creat_osd(k_vo_osd osd, osd_info *info);
k_vb_blk_handle sample_vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr);
int sample_vo_creat_private_poll(void);
void sample_vo_filling_color(void *pic_vaddr, string in, string out, bool is_en);
k_s32 sample_vo_layer_config(void);

int sample_vivcap_init(void);
int sample_vivcap_deinit(void);

int sample_gpio_init(void);

#endif