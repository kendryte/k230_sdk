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
#include "vo_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_vo_api.h"
#include "mapi_sys_api.h"
#include "k_video_comm.h"

#include "k_vo_comm.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

int sample_vo_creat_layer(k_vo_layer chn_id, layer_info *info)
{
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2)))
    {
        printf("input layer num failed \n");
        return -1 ;
    }

    // check scaler

    // set offset
    attr.display_rect = info->offset;
    // set act
    attr.img_size = info->act_size;
    // sget size
    info->size = info->act_size.height * info->act_size.width * 3 / 2;
    //set pixel format
    attr.pixel_format = info->format;
    if (info->format != PIXEL_FORMAT_YVU_PLANAR_420)
    {
        printf("input pix format failed \n");
        return -1;
    }
    // set stride
    attr.stride = (info->act_size.width / 8 - 1) + ((info->act_size.height - 1) << 16);
    // set function
    attr.func = info->func;
    // set scaler attr
    attr.scaler_attr = info->attr;

    // set video layer atrr
    kd_mapi_vo_set_video_layer_attr(chn_id, &attr);
    // enable layer
    kd_mapi_vo_enable_video_layer(chn_id);

    return 0;
}

k_u32 vo_creat_osd_test(k_vo_osd osd, osd_info *info)
{
    k_vo_video_osd_attr attr;

    // set attr
    attr.global_alptha = info->global_alptha;

    if (info->format == PIXEL_FORMAT_ABGR_8888 || info->format == PIXEL_FORMAT_ARGB_8888)
    {
        info->size = info->act_size.width * info->act_size.height * 4;
        info->stride = info->act_size.width * 4 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_565 || info->format == PIXEL_FORMAT_BGR_565)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_888 || info->format == PIXEL_FORMAT_BGR_888)
    {
        info->size = info->act_size.width * info->act_size.height * 3;
        info->stride = info->act_size.width * 3 / 8;
    }
    else if (info->format == PIXEL_FORMAT_ARGB_4444 || info->format == PIXEL_FORMAT_ABGR_4444)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_ARGB_1555 || info->format == PIXEL_FORMAT_ABGR_1555)
    {
        info->size = info->act_size.width * info->act_size.height * 2;
        info->stride = info->act_size.width * 2 / 8;
    }
    else
    {
        printf("set osd pixel format failed  \n");
    }

    attr.stride = info->stride;
    attr.pixel_format = info->format;
    attr.display_rect = info->offset;
    attr.img_size = info->act_size;
    kd_mapi_vo_set_video_osd_attr(osd, &attr);

    // 打开osd层
    kd_mapi_vo_osd_enable(osd);

    return 0;
}

static k_s32 sample_connector_init(void)
{
    k_s32 ret = 0;
    char dev_name[64] = {0};
    k_s32 connector_fd;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
    k_connector_info connector_info;

    memset(&connector_info, 0, sizeof(k_connector_info));
    connector_info.connector_name = (char *)dev_name;

    //connector get sensor info
    ret = kd_mapi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }
    // printf("connector_info name is %s \n", connector_info.connector_name);

    connector_fd = kd_mapi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    printf("connector_fd  is %d ret is %d \n", connector_fd, ret);

    // set connect power
    kd_mapi_connector_power_set(connector_fd, K_TRUE);
    // // connector init
    kd_mapi_connector_init(connector_fd, &connector_info);

    return 0;
}


void vo_layer_init(k_u32 width,k_u32 height)
{
    k_video_frame_info vf_info;
    layer_info info;

    void *pic_vaddr = NULL;
    k_vo_layer chn_id = (k_vo_layer)ENABLE_VO_LAYER;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&info, 0, sizeof(info));

    printf("start sample_connector_init \n");
    sample_connector_init();

    // config lyaer
    if (width > 1080)
    {
        info.act_size.width = height;
        info.act_size.height = width;
        info.func = K_ROTATION_90;
    }
    else
    {
        info.act_size.width = width;
        info.act_size.height = height;
        info.func = 0;
    }

    info.format = PIXEL_FORMAT_YVU_PLANAR_420;

    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    sample_vo_creat_layer(chn_id, &info);
}

void vo_layer_deinit()
{
    kd_mapi_vo_disable_video_layer((k_vo_layer)ENABLE_VO_LAYER);
}
