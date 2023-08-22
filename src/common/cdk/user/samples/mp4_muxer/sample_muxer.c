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
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include "mapi_sys_api.h"
#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"
#include "mapi_ai_api.h"
#include "mapi_aenc_api.h"
#include "mp4_format.h"

#define AUDIO_PERSEC_DIV_NUM 25
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

static k_u8 g_exit = 0;
static int stream_count = 0;
static int get_idr = 0;
static uint8_t save_idr[1280 * 720 * 3 / 2];
static uint32_t save_size = 0;
static uint64_t first_time_stamp = 0;
KD_HANDLE mp4_muxer = NULL;
KD_HANDLE video_track_handle = NULL;
KD_HANDLE audio_track_handle = NULL;
static k_handle ai_handle = 0;

typedef struct {
    k_vicap_sensor_type sensor_type;
    k_u32 sensor_width;
    k_u32 sensor_height;
    k_payload_type type;
    k_u32 video_pic_width;
    k_u32 video_pic_height;
    k_u32 audio_sample_rate;
} SampleCtx;

static void sig_handler(int sig) {
    g_exit = 1;
}

static k_s32 get_venc_stream(k_u32 chn_num, kd_venc_data_s* p_vstream_data, k_u8 *p_private_data)
{
    int cut = p_vstream_data->status.cur_packs;
    int i = 0;
    k_mp4_frame_data_s frame_data;
    
    for(i = 0; i < cut; i++) {
        if (stream_count % 30 == 0)
            printf("recv count:%d, chn:%d, len:%d\n", stream_count, chn_num, p_vstream_data->astPack[i].len);

        memset(&frame_data, 0, sizeof(frame_data));
        frame_data.codec_id = K_MP4_CODEC_ID_H265;
        frame_data.data =  p_vstream_data->astPack[i].vir_addr;
        frame_data.data_length = p_vstream_data->astPack[i].len;
        frame_data.time_stamp = p_vstream_data->astPack[i].pts;

        if (!get_idr) {
            memcpy(save_idr + save_size, frame_data.data, frame_data.data_length);
            save_size += frame_data.data_length;
            first_time_stamp = p_vstream_data->astPack[i].pts;
        }
    }

    stream_count++;
    if (stream_count == 2) {
        get_idr = 1;
        memset(&frame_data, 0, sizeof(frame_data));
        frame_data.codec_id = K_MP4_CODEC_ID_H265;
        frame_data.data =  save_idr;
        frame_data.data_length = save_size;
        frame_data.time_stamp = first_time_stamp;
    }

    if (get_idr) {
        k_u32 ret = kd_mp4_write_frame(mp4_muxer, video_track_handle, &frame_data);
        if (ret < 0) {
            printf("mp4 write video frame failed.\n");
            return -1;
        }
    }
}

static k_s32 get_aenc_stream(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data) {
    k_mp4_frame_data_s frame_data;
    memset(&frame_data, 0, sizeof(frame_data));
    frame_data.codec_id = K_MP4_CODEC_ID_G711U;
    frame_data.data = (uint8_t *)stream_data->stream;
    frame_data.data_length = stream_data->len;
    frame_data.time_stamp = stream_data->time_stamp;

    k_u32 ret = kd_mp4_write_frame(mp4_muxer, audio_track_handle, &frame_data);
    if (ret < 0) {
        printf("mp4 write audio frame failed.\n");
        return -1;
    }
}

static k_s32 vb_pool_init(SampleCtx *psample_ctx) {
    k_u32 pic_width = psample_ctx->video_pic_width;
    k_u32 pic_height = psample_ctx->video_pic_height;
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 5; // video + audio + vicap

    // video pool parameter
    k_u32 pic_size = pic_width * pic_height * 3 / 2;
    k_u32 stream_size = pic_width * pic_height * 3 / 4;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = 6;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = ((pic_size + 0xfff) & ~0xfff);
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = 30;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = ((stream_size + 0xfff) & ~0xfff);
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    // audio pool parameter
    k_u32 sample_rate = psample_ctx->audio_sample_rate;
    media_attr.media_config.vb_config.comm_pool[2].blk_cnt = 150;
    media_attr.media_config.vb_config.comm_pool[2].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    media_attr.media_config.vb_config.comm_pool[2].mode = VB_REMAP_MODE_CACHED;
    media_attr.media_config.vb_config.comm_pool[3].blk_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[3].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    media_attr.media_config.vb_config.comm_pool[3].mode = VB_REMAP_MODE_CACHED;

    // vicap pool parameter
    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(sensor_info));
    sensor_info.sensor_type = psample_ctx->sensor_type;
    k_u32 ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_get_sensor_info failed, %x.\n", ret);
        return -1;
    }
    psample_ctx->sensor_width = sensor_info.width;
    psample_ctx->sensor_height = sensor_info.height;

    media_attr.media_config.vb_config.comm_pool[4].blk_cnt = 6;
    media_attr.media_config.vb_config.comm_pool[4].blk_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x400);
    media_attr.media_config.vb_config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;

    memset(&media_attr.media_config.vb_supp.supplement_config, 0, sizeof(media_attr.media_config.vb_supp.supplement_config));
    media_attr.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_media_init failed, %x.\n", ret);
        return -1;
    }

    return 0;
}

static k_s32 venc_init(SampleCtx *psample_ctx) {
    // venc init
    k_venc_chn_attr venc_chn_attr;
    memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
    venc_chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
    venc_chn_attr.rc_attr.cbr.src_frame_rate = 30;
    venc_chn_attr.rc_attr.cbr.dst_frame_rate = 30;
    venc_chn_attr.rc_attr.cbr.bit_rate = 4000;
    venc_chn_attr.venc_attr.type = psample_ctx->type;
    if (psample_ctx->type == K_PT_H264) {
        venc_chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
    } else if (psample_ctx->type == K_PT_H265) {
        venc_chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    }
    venc_chn_attr.venc_attr.pic_width = psample_ctx->video_pic_width;
    venc_chn_attr.venc_attr.pic_height = psample_ctx->video_pic_height;
    venc_chn_attr.venc_attr.stream_buf_cnt = 30;
    k_u32 stream_size = psample_ctx->video_pic_width * psample_ctx->video_pic_height * 3 / 4;
    venc_chn_attr.venc_attr.stream_buf_size = ((stream_size + 0xfff) & ~0xfff);
    k_s32 ret = kd_mapi_venc_init(0, &venc_chn_attr);
    if (ret != K_SUCCESS) {
        printf("init venc failed, %x.\n", ret);
        return -1;
    }

    kd_venc_callback_s venc_cb;
    venc_cb.pfn_data_cb = get_venc_stream;
    venc_cb.p_private_data = NULL;
    ret = kd_mapi_venc_registercallback(0, &venc_cb);
    if (ret < 0) {
        printf("kd_mapi_venc_registercallback failed.\n");
        goto venc_deinit;
    }

    return 0;

venc_deinit:
    kd_mapi_venc_deinit(0);

    return -1;
}

static k_s32 vicap_init(SampleCtx *psample_ctx) {
    k_vicap_dev_set_info dev_attr_info;
    memset(&dev_attr_info, 0, sizeof(dev_attr_info));
    dev_attr_info.dw_en = K_TRUE;
    dev_attr_info.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr_info.sensor_type = psample_ctx->sensor_type;
    dev_attr_info.vicap_dev = VICAP_DEV_ID_0;
    k_s32 ret = kd_mapi_vicap_set_dev_attr(dev_attr_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_set_dev_attr failed, %x.\n", ret);
        return -1;
    }

    k_vicap_chn_set_info vi_chn_attr_info;
    memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));
    vi_chn_attr_info.crop_en = K_FALSE;
    vi_chn_attr_info.scale_en = K_FALSE;
    vi_chn_attr_info.chn_en = K_TRUE;
    vi_chn_attr_info.crop_h_start = 0;
    vi_chn_attr_info.crop_v_start = 0;
    vi_chn_attr_info.out_width = psample_ctx->video_pic_width;
    vi_chn_attr_info.out_height = psample_ctx->video_pic_height;
    vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    vi_chn_attr_info.vicap_dev = VICAP_DEV_ID_0;
    vi_chn_attr_info.vicap_chn = 0;
    vi_chn_attr_info.buf_size = VI_ALIGN_UP(psample_ctx->sensor_width * psample_ctx->sensor_height * 3 / 2, 0x400);
    ret = kd_mapi_vicap_set_chn_attr(vi_chn_attr_info);
    if (ret != K_SUCCESS) {
        printf("vicap chn set attr failed, %x.\n", ret);
        return -1;
    }

    return 0;
}

static k_s32 aenc_init(SampleCtx *psample_ctx) {
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = psample_ctx->audio_sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = psample_ctx->audio_sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    if (K_SUCCESS != kd_mapi_ai_init(0, 0, &aio_dev_attr, &ai_handle)) {
        printf("kd_mapi_ai_init failed.\n");
        return -1;
    }
    if (K_SUCCESS != kd_mapi_ai_start(ai_handle)) {
        printf("kd_mapi_ai_start failed.\n");
        goto ai_deinit;
    }

    k_aenc_chn_attr aenc_chn_attr;
    memset(&aenc_chn_attr, 0, sizeof(aenc_chn_attr));
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = psample_ctx->audio_sample_rate / aenc_chn_attr.buf_size;
    aenc_chn_attr.type = K_PT_G711U;
     
    if (K_SUCCESS != kd_mapi_aenc_init(0, &aenc_chn_attr)) {
        printf("kd_mapi_aenc_init failed.\n");
        goto ai_stop;
    }

    k_aenc_callback_s aenc_cb;
    memset(&aenc_cb, 0, sizeof(aenc_cb));
    aenc_cb.p_private_data = NULL;
    aenc_cb.pfn_data_cb = get_aenc_stream;
    kd_mapi_aenc_registercallback(0, &aenc_cb);

    return 0;

ai_stop:
    kd_mapi_ai_stop(ai_handle);

ai_deinit:
    kd_mapi_ai_deinit(ai_handle);
    
    return -1;
}

static k_s32 start_run(SampleCtx *psample_ctx) {
    k_s32 ret = kd_mapi_venc_start(0, -1);
    if (ret != K_SUCCESS) {
        printf("venc chn start failed, %x.\n", ret);
        return -1;
    }

    ret = kd_mapi_venc_bind_vi(VICAP_DEV_ID_0, 0, 0);
    if (ret != K_SUCCESS) {
        printf("venc chn bind vi failed, %x.\n", ret);
        goto venc_stop;
    }

    ret = kd_mapi_aenc_start(0);
    if (ret < 0) {
        printf("kd_mapi_aenc_start failed.\n");
        goto venc_unbind;
    }
    
    ret = kd_mapi_aenc_bind_ai(ai_handle, 0);
    if (ret < 0) {
        printf("kd_mapi_aenc_bind_ai failed.\n");
        goto aenc_stop;
    }

    ret = kd_mapi_vicap_start(VICAP_DEV_ID_0);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_start failed, %x.\n", ret);
        goto aenc_unbind;
    }

    return 0;

aenc_unbind:
    ret = kd_mapi_aenc_unbind_ai(ai_handle, 0);

aenc_stop:
    kd_mapi_aenc_stop(0);

venc_unbind:
    kd_mapi_venc_unbind_vi(VICAP_DEV_ID_0, 0, 0);

venc_stop:
    kd_mapi_venc_stop(0);

    return ret;
}

static void vb_pool_deinit() {
    kd_mapi_media_deinit();
    
    return;
}

static void venc_deinit() {
    kd_venc_callback_s venc_cb;
    venc_cb.p_private_data = NULL;
    venc_cb.pfn_data_cb = get_venc_stream;
    kd_mapi_venc_unregistercallback(0, &venc_cb);

    kd_mapi_venc_deinit(0);

    return;
}

static void aenc_deinit() {
    kd_mapi_aenc_deinit(0);
    kd_mapi_ai_stop(ai_handle);
    kd_mapi_ai_deinit(ai_handle);
}

static void stop_run() {
    kd_mapi_aenc_unbind_ai(ai_handle, 0);
    kd_mapi_aenc_stop(0);

    kd_mapi_venc_unbind_vi(VICAP_DEV_ID_0, 0, 0);
    kd_mapi_venc_stop(0);
    kd_mapi_vicap_stop(VICAP_DEV_ID_0);
}

int main(int argc, char *argv[]) {

    signal(SIGINT, sig_handler);

    SampleCtx sample_context = {
        .sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR,
        .sensor_width = 1920,
        .sensor_height = 1080,
        .type = K_PT_H265,
        .video_pic_width = 1280,
        .video_pic_height = 720,
        .audio_sample_rate = 44100
    };

    printf("mp4 muxer...\n");

    k_u32 ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_init failed, %x.\n", ret);
        return -1;
    }

    ret = vb_pool_init(&sample_context);
    if (ret < 0) {
        printf("vb_pool_init failed.\n");
        goto sys_deinit;
    }

    ret = venc_init(&sample_context);
    if (ret < 0) {
        printf("venc_init failed.\n");
        goto vbpool_deinit;
    }

    ret = vicap_init(&sample_context);
    if (ret < 0) {
        printf("vicap_init failed.\n");
        goto venc_deinit;
    }

    ret = aenc_init(&sample_context);
    if (ret < 0) {
        printf("aenc_init failed.\n");
        goto venc_deinit;
    }

    // create mp4 instance
    k_mp4_config_s mp4_config;
    memset(&mp4_config, 0, sizeof(mp4_config));
    mp4_config.config_type = K_MP4_CONFIG_MUXER;
    strcpy(mp4_config.muxer_config.file_name, "/tmp/test.mp4");
    mp4_config.muxer_config.fmp4_flag = 0;

    ret = kd_mp4_create(&mp4_muxer, &mp4_config);
    if (ret < 0) {
        printf("mp4 muxer create failed.\n");
        goto aenc_deinit;
    }

    // create video track
    k_mp4_track_info_s video_track_info;
    memset(&video_track_info, 0, sizeof(video_track_info));
    video_track_info.track_type = K_MP4_STREAM_VIDEO;
    video_track_info.time_scale = 1000;
    video_track_info.video_info.width = 1280;
    video_track_info.video_info.height = 720;
    ret = kd_mp4_create_track(mp4_muxer, &video_track_handle, &video_track_info);
    if (ret < 0) {
        printf("create video track failed.\n");
        goto destroy_mp4;
    }

    // create audio track
    k_mp4_track_info_s audio_track_info;
    memset(&audio_track_info, 0, sizeof(audio_track_info));
    audio_track_info.track_type = K_MP4_STREAM_AUDIO;
    audio_track_info.time_scale = 1000;
    audio_track_info.audio_info.channels = 2;
    audio_track_info.audio_info.codec_id = K_MP4_CODEC_ID_G711U;
    audio_track_info.audio_info.sample_rate = 44100;
    audio_track_info.audio_info.bit_per_sample = 16;
    ret = kd_mp4_create_track(mp4_muxer, &audio_track_handle, &audio_track_info);
    if (ret < 0) {
        printf("create video track failed.\n");
        goto destroy_mp4;
    }

    ret = start_run(&sample_context);
    if (ret < 0) {
        printf("start_run failed.\n");
        goto destroy_track;
    }

    // waiting thread
    while (!g_exit) {
        usleep(50000);
    }

    stop_run();

destroy_track:
    ret = kd_mp4_destroy_tracks(mp4_muxer);
    if (ret < 0) {
        printf("destroy mp4 tracks failed.\n");
        return -1;
    }

destroy_mp4:
    ret = kd_mp4_destroy(mp4_muxer);
    if (ret < 0) {
        printf("destroy mp4 failed.\n");
        return -1;
    }

aenc_deinit:
    aenc_deinit();

venc_deinit:
    venc_deinit();

vbpool_deinit:
    vb_pool_deinit();

sys_deinit:
    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit failed, %x.\n", ret);
        return -1;
    }

    printf("mp4 muxer end...\n");
    return 0;
}
