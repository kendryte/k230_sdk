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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>

#include "sample_venc.h"
#include "kstream.h"
#include "log.h"
#include "sample_yuv.h"

#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "frame_cache.h"

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#define   ENCWIDTH    1280
#define   ENCHEIGHT   720

#define   D1_WIDTH    640
#define   D1_HEIGHT   480

#define MAX_CHN_NUM 3
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

#define   VB_YUV_PIC(w,h)   (((w)*(h)*2 + 0xfff) & ~0xfff)//(w*h*3/2+4095)/4096
#define   VB_PIC_STREAM(w,h)   (((w)*(h)/2 + 0xfff) & ~0xfff)//(w*h/2+4095)/4096
#define   PIX_FMT   PIXEL_FORMAT_YUV_SEMIPLANAR_420

extern unsigned int g_sensor;

static FILE * fp = NULL;
static k_mapi_media_attr_t media_attr = {0};
int fd = -1;

static k_u8 __started = 0;
encoder_property __encoder_property;

typedef struct {
    k_vicap_sensor_type sensor_type;
    k_payload_type type;
    k_u32 chn_num;
} SampleCtx;

typedef struct {
    k_u32 buffsize_with_align;
    k_u32 buffsize;
    k_char *suffix;
} buf_size_calc;

SampleCtx sample_context = {
    .sensor_type = OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR,
    .type = K_PT_H264,
    .chn_num = 0
};

extern k_s32 start_get_yuv(k_u32 dev_num, k_u32 chn_num);
extern k_s32 stop_get_yuv(void);
static frame_node_t *fnode = NULL;
static int count = 0;
k_s32 get_venc_stream(k_u32 chn_num, kd_venc_data_s* p_vstream_data, k_u8 *p_private_data)
{
    uvc_cache_t *uvc_cache = uvc_cache_get();

    k_u32 data_len = 0;
    k_u32 copy_size = 0;

    int cut = p_vstream_data->status.cur_packs;
    int i = 0;
    k_char * pdata  = NULL;

    if (uvc_cache)
    {
        if (!fnode)
        {
            get_node_from_queue(uvc_cache->free_queue, &fnode);
            if (!fnode)
            {
                return K_SUCCESS;
            }
            fnode->used = 0;
        }
    }

    for(i = 0;i < cut; i++)
    {
        pdata =  p_vstream_data->astPack[i].vir_addr;
        data_len = p_vstream_data->astPack[i].len;
        if(data_len > fnode->length + fnode->used)
        {
            return K_SUCCESS;
        }

        // printf("recv count:%d node_idx:%d  phys_addr:0x%lx len:%d   used:%d cut:%d pdata:[0x%lx] length:%d data_len:%d\n",
        //     count, fnode->index, p_vstream_data->astPack[i].phys_addr, p_vstream_data->astPack[i].len, fnode->used, cut, pdata, fnode->length, data_len);

        if (data_len > 0)
        {
            // for(int j=0; j<data_len; j++)
            // {
            //     *(fnode->mem + fnode->used + j) = *(pdata++);
            // }
            memcpy(fnode->mem + fnode->used, pdata, data_len);
            fnode->used += data_len;
        }
    }

    if(sample_context.type == K_PT_JPEG)
        count += 5;
    else
        count ++;

    if(count % 5 == 0)
    {
        put_node_to_queue(uvc_cache->ok_queue, fnode);
        fnode = NULL;
    }
}

static k_vicap_sensor_type get_sensor_type(k_u16 sensor)
{
    switch (sensor)
    {
    case 0:
        return OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    case 1:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
    case 2:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    case 3:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    case 4:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 5:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 6:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR;
    case 7:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    case 8:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    default:
        printf("unsupport sensor type %d, use default IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR\n", sensor);
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    }
    return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
}

static k_s32 calc_buffer_size(k_pixel_format pix_fmt, k_u32 width, k_u32 height, buf_size_calc * size_calc)
{
    switch (pix_fmt)
    {
    case PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "yuv420sp";
        break;
    case PIXEL_FORMAT_RGB_888:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888p";
        break;
    case PIXEL_FORMAT_BGR_888_PLANAR:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3;
        size_calc->suffix = "rgb888";
        break;
    case PIXEL_FORMAT_RGB_BAYER_10BPP:
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 2;
        size_calc->suffix = "raw10";
        break;
    default:
        printf("unsupported pixel format %d, use dafault YUV_SEMIPLANAR_420 size\n", pix_fmt);
        size_calc->buffsize_with_align = VICAP_ALIGN_UP((width * height * 3 / 2), VICAP_ALIGN_1K);
        size_calc->buffsize = width * height * 3 / 2;
        size_calc->suffix = "unkown";
    }
    return K_SUCCESS;
}

static k_s32 sample_vicap_vb_init(k_bool dw_en, k_u32 dev_num, k_u32 chn_num, buf_size_calc in_size, buf_size_calc out_size, buf_size_calc dw_size)
{
    printf("sample_vicap_vb_init start\n");
    k_s32 ret = 0;
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));

    media_attr.media_config.vb_config.max_pool_cnt = 64;
    int k = 0;
    for(int i = 0; i <= dev_num; i++) {
        for(int j = 0; j <= chn_num; j++) {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = out_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
        if(dw_en)
        {
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = dw_size.buffsize_with_align;
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
            k++;
        }
    }

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
    }
    printf("sample_vicap_vb_init end\n");
    return 0;
}

static k_s32 sample_venc_init(void)
{
    printf("%s\n", __func__);
    k_s32 ret = 0;

    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }

    return 0;
}

static k_s32 sample_venc_deinit(void)
{
    printf("%s\n", __func__);
    k_s32 ret = 0;

    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }

    return 0;
}

static k_s32 sample_venc_startup(void)
{
    printf("%s\n", __func__);
    k_s32 ret = 0;
    k_s32 i = 0;
    __started = 1;

    sample_context.sensor_type = get_sensor_type(g_sensor);
    sample_context.chn_num = 1;
    switch (__encoder_property.format)
    {
    case V4L2_PIX_FMT_H264:
        sample_context.type = K_PT_H264;
        break;
    case V4L2_PIX_FMT_MJPEG:
        sample_context.type = K_PT_JPEG;
        break;
    case V4L2_PIX_FMT_HEVC:
        sample_context.type = K_PT_H265;
        break;
    case V4L2_PIX_FMT_NV12:
        sample_context.type = K_PT_BUTT;
        break;
    default:
        printf("unsupport venc format \n");
        return -1;
    }
    // vicap init
    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(sensor_info));
    sensor_info.sensor_type = sample_context.sensor_type;
    ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_get_sensor_info failed, %x.\n", ret);
        goto sys_deinit;
    }

    k_vicap_dev_set_info dev_attr_info;
    memset(&dev_attr_info, 0, sizeof(dev_attr_info));
    dev_attr_info.dw_en = K_FALSE;
    dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr_info.sensor_type = sample_context.sensor_type;
    dev_attr_info.vicap_dev = VICAP_DEV_ID_0;
    ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_set_dev_attr failed, %x.\n", ret);
        goto sys_deinit;
    }

    if (__encoder_property.format != V4L2_PIX_FMT_YUYV && __encoder_property.format != V4L2_PIX_FMT_NV12)
    {
        k_u32 pic_width[MAX_CHN_NUM] = {1280, 960, 640 };
        k_u32 pic_height[MAX_CHN_NUM] = {720, 640, 480};
        k_mapi_media_attr_t media_attr;
        memset(&media_attr, 0, sizeof(media_attr));
        if(dev_attr_info.dw_en)
        {
            media_attr.media_config.vb_config.max_pool_cnt = sample_context.chn_num * 2 + 1;
        }
        else
        {
            media_attr.media_config.vb_config.max_pool_cnt = sample_context.chn_num * 2;
        }
        k_u32 k = 0;
        for (int i = 0; i < sample_context.chn_num; i++) {
            k_u32 pic_size = pic_width[i] * pic_height[i] * 3 / 2;
            k_u32 stream_size = pic_width[i] * pic_height[i] * 3 / 4;
            media_attr.media_config.vb_config.comm_pool[i * 2].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[i * 2].blk_size = ((pic_size + 0xfff) & ~0xfff);
            media_attr.media_config.vb_config.comm_pool[i * 2].mode = VB_REMAP_MODE_NOCACHE;
            media_attr.media_config.vb_config.comm_pool[i * 2 + 1].blk_cnt = 30;
            media_attr.media_config.vb_config.comm_pool[i * 2 + 1].blk_size = ((stream_size + 0xfff) & ~0xfff);
            media_attr.media_config.vb_config.comm_pool[i * 2 + 1].mode = VB_REMAP_MODE_NOCACHE;
            k += 2;
        }

        // dw vb
        if(dev_attr_info.dw_en)
        {
            k_u32 dw_vb_buf_size = sensor_info.width * sensor_info.height * 3 / 2;
            media_attr.media_config.vb_config.comm_pool[k].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[k].blk_size = ((dw_vb_buf_size + 0xfff) & ~0xfff);
            media_attr.media_config.vb_config.comm_pool[k].mode = VB_REMAP_MODE_NOCACHE;
        }
        memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
        media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

        ret = kd_mapi_media_init(&media_attr);
        if (ret != K_SUCCESS) {
            printf("kd_mapi_media_init failed, %x.\n", ret);
            goto sys_deinit;
        }

        k_venc_chn_attr venc_chn_attr;
        memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
        venc_chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
        venc_chn_attr.rc_attr.cbr.src_frame_rate = 30;
        venc_chn_attr.rc_attr.cbr.dst_frame_rate = 30;
        venc_chn_attr.rc_attr.cbr.bit_rate = 4000;
        venc_chn_attr.venc_attr.type = sample_context.type;
        if (sample_context.type == K_PT_H264) {
            venc_chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
        } else if (sample_context.type == K_PT_H265) {
            venc_chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
        } else if (sample_context.type == K_PT_JPEG) {
            venc_chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_FIXQP;
            venc_chn_attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
            venc_chn_attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
            venc_chn_attr.rc_attr.mjpeg_fixqp.q_factor = 45;
        }

        for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
            k_u32 stream_size = pic_width[venc_idx] * pic_height[venc_idx] * 3 / 4;
            venc_chn_attr.venc_attr.pic_width = pic_width[venc_idx];
            venc_chn_attr.venc_attr.pic_height = pic_height[venc_idx];
            venc_chn_attr.venc_attr.stream_buf_cnt = 30;
            venc_chn_attr.venc_attr.stream_buf_size = ((stream_size + 0xfff) & ~0xfff);
            ret = kd_mapi_venc_init(venc_idx, &venc_chn_attr);
            if (ret != K_SUCCESS) {
                printf("init venc %d failed, %x.\n", venc_idx, ret);
                for (int create_idx = 0; create_idx < venc_idx; create_idx++) {
                    ret = kd_mapi_venc_deinit(create_idx);
                    if (ret != K_SUCCESS) {
                        printf("deinit venc %d failed, %x.\n", create_idx, ret);
                    }
                }
                goto sys_deinit;
            }
        }

        for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
            int user_data = venc_idx;
            kd_venc_callback_s venc_cb;
            venc_cb.pfn_data_cb = get_venc_stream;
            venc_cb.p_private_data = (k_u8 *)&user_data;
            kd_mapi_venc_registercallback(venc_idx, &venc_cb);
        }

        for (int vichn_idx = 0; vichn_idx < sample_context.chn_num; vichn_idx++) {
            k_vicap_chn_set_info vi_chn_attr_info;
            memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));

            vi_chn_attr_info.crop_en = K_FALSE;
            vi_chn_attr_info.scale_en = K_FALSE;
            vi_chn_attr_info.chn_en = K_TRUE;
            vi_chn_attr_info.crop_h_start = 0;
            vi_chn_attr_info.crop_v_start = 0;
            vi_chn_attr_info.out_width = pic_width[vichn_idx];
            vi_chn_attr_info.out_height = pic_height[vichn_idx];
            vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
            vi_chn_attr_info.vicap_dev = VICAP_DEV_ID_0;
            vi_chn_attr_info.vicap_chn = (k_vicap_chn)vichn_idx;
            vi_chn_attr_info.alignment = 12;
            if (!dev_attr_info.dw_en)
                vi_chn_attr_info.buf_size = VI_ALIGN_UP(pic_width[vichn_idx] * pic_height[vichn_idx] * 3 / 2, 0x400);
            else
                vi_chn_attr_info.buf_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x400);
            ret = kd_mapi_vicap_set_chn_attr(vi_chn_attr_info);
            if (ret != K_SUCCESS) {
                printf("vicap chn %d set attr failed, %x.\n", vichn_idx, ret);
                goto venc_deinit;
            }
        }

        // chn start
        for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
            if(sample_context.type == K_PT_H264)
            {
                kd_mapi_venc_enable_idr(venc_idx, 1);
            }
            ret = kd_mapi_venc_start(venc_idx, -1);
            if (ret != K_SUCCESS) {
                printf("venc chn %d start failed, %x.\n", venc_idx, ret);
                for (int create_idx = 0; create_idx < venc_idx; create_idx++) {
                    ret = kd_mapi_venc_stop(create_idx);
                    if (ret != K_SUCCESS) {
                        printf("venc chn %d stop failed, %x.\n", create_idx, ret);
                    }
                }
                goto venc_deinit;
            }
        }

        k_s32 src_dev = VICAP_DEV_ID_0;
        for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
            k_s32 src_chn = venc_idx;
            ret = kd_mapi_venc_bind_vi(src_dev, src_chn, venc_idx);
            if (ret != K_SUCCESS) {
                printf("venc chn %d bind vi failed, %x.\n", venc_idx, ret);
                for (int idx = 0; idx < venc_idx; idx++) {
                    ret = kd_mapi_venc_unbind_vi(src_dev, idx, idx);
                    if (ret != K_SUCCESS) {
                        printf("venc idx %d unbind vi failed, %x.\n", idx, ret);
                    }
                }
                goto venc_stop;
            }
        }

        ret = kd_mapi_vicap_start(VICAP_DEV_ID_0);
        if (ret != K_SUCCESS) {
            printf("kd_mapi_vicap_start failed, %x.\n", ret);
            goto vicap_stop;
        }
    }
    else
    {
        k_u32 out_width = 640;
        k_u32 out_height = 360;
        k_vicap_dev arg_dev = VICAP_DEV_ID_0;
        k_vicap_chn arg_chn = VICAP_CHN_ID_0;
        k_pixel_format arg_pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

        // get resolution
        k_u32 in_width = sensor_info.width;
        k_u32 in_height = sensor_info.height;

        // calc buffer size
        buf_size_calc size_calc_in;
        buf_size_calc size_calc_out;
        buf_size_calc size_calc_dw;
        calc_buffer_size(arg_pixel_format, in_width, in_height, &size_calc_in);
        calc_buffer_size(arg_pixel_format, out_width, out_height, &size_calc_out);
        calc_buffer_size(PIXEL_FORMAT_YUV_SEMIPLANAR_420, in_width, in_height, &size_calc_dw);
        // init vb
        ret = sample_vicap_vb_init(dev_attr_info.dw_en, (k_u32)arg_dev, (k_u32)arg_chn, size_calc_in, size_calc_out, size_calc_dw);
        if(ret)
        {
            ret |= kd_mapi_media_deinit();
        }

        // set chn attr
        for(k_u32 i = 0; i <= (k_u32)arg_chn; i++)
        {
            k_vicap_chn_set_info chn_attr_info;
            memset(&chn_attr_info, 0, sizeof(k_vicap_chn_set_info));
            chn_attr_info.crop_en = K_FALSE;
            chn_attr_info.scale_en = K_FALSE;
            chn_attr_info.chn_en = K_TRUE;
            chn_attr_info.crop_h_start = 0;
            chn_attr_info.crop_v_start = 0;
            chn_attr_info.out_height = out_height;
            chn_attr_info.out_width = out_width;
            chn_attr_info.pixel_format = arg_pixel_format;
            chn_attr_info.vicap_dev = arg_dev;
            chn_attr_info.vicap_chn = (k_vicap_chn)i;

            if(arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_10BPP && arg_pixel_format != PIXEL_FORMAT_RGB_BAYER_12BPP)
            {
                // dewarp can enable in bayer pixel format
                if(dev_attr_info.dw_en && i == 0)
                {
                    printf("dw buf\n");
                    chn_attr_info.buf_size = size_calc_dw.buffsize_with_align;
                }
                else
                {
                    printf("dw disable buf\n");
                    chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
                }
            }
            else
            {
                printf("dw is force disabled, becase of pixformat BAYER\n");
                dev_attr_info.dw_en = 0;
                chn_attr_info.buf_size = size_calc_out.buffsize_with_align;
            }
            ret = kd_mapi_vicap_set_chn_attr(chn_attr_info);
        }

        if(!ret)
        {
            printf("kd_mapi_vicap_set_attr ok\n");
        }
        ret = kd_mapi_vicap_start(arg_dev);
        if(!ret)
        {
            printf("vicap start...\n");
        }

        ret = start_get_yuv(VICAP_DEV_ID_0, VICAP_CHN_ID_0);
        if (K_SUCCESS != ret)
        {
            printf("Start get YUV failed!\n");
            goto sys_deinit;
        }
    }

    return K_SUCCESS;

vicap_stop:
    ret = kd_mapi_vicap_stop(VICAP_DEV_ID_0);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_stop failed, %x.\n", ret);
        return -1;
    }

venc_stop:
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        ret = kd_mapi_venc_stop(venc_idx);
        if (ret != K_SUCCESS) {
            printf("venc chn %d stop failed, %x.\n", venc_idx, ret);
            return -1;
        }
    }

venc_deinit:
    for (int venc_idx = 0; venc_idx < sample_context.chn_num; venc_idx++) {
        ret = kd_mapi_venc_deinit(venc_idx);
        if (ret != K_SUCCESS) {
            printf("venc %d deinit failed, %x.\n", venc_idx, ret);
            return -1;
        }
    }

media_deinit:
    ret = kd_mapi_media_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_media_deinit failed, %x.\n", ret);
        return -1;
    }
sys_deinit:
    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit failed, %x.\n", ret);
        return -1;
    }
    printf("__SAMPLE_VENC_NORMALP_CLASSIC error Exit!!!!\n");
    return ret;
}

static k_s32 sample_venc_shutdown(void)
{
    printf("%s\n", __func__);
    if ((!__started))
    {
        return 0;
    }
    if (__encoder_property.format != V4L2_PIX_FMT_YUYV && __encoder_property.format != V4L2_PIX_FMT_NV12)
    {
        usleep(50000);

        kd_mapi_venc_unbind_vi(VICAP_DEV_ID_0, 0, 0);
        kd_mapi_vicap_stop(VICAP_DEV_ID_0);
        kd_mapi_venc_stop(0);
        kd_mapi_venc_deinit(0);
    }
    else
    {
        stop_get_yuv();
        kd_mapi_vicap_stop(VICAP_DEV_ID_0);

    }
    kd_mapi_media_deinit();

    fnode = NULL;
    count = 0;

    __started = 0;

    return K_SUCCESS;
}

static k_s32 sample_venc_set_property(encoder_property *p)
{
    __encoder_property = *p;
    return 0;
}

static struct stream_control_ops venc_sc_ops = {
    .init = sample_venc_init,
    .startup = sample_venc_startup,
    .shutdown = sample_venc_shutdown,
    .set_property = sample_venc_set_property,
    .deinit = sample_venc_deinit,
};

void sample_venc_config(void)
{
    kstream_register_mpi_ops(&venc_sc_ops);
}
