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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define ENABLE_MPI 1

#ifdef ENABLE_MPI
#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_venc_api.h"
#include "mpi_sys_api.h"
#include "k_venc_comm.h"
#include "k_audio_comm.h"
#include "k_payload_comm.h"
#include "k_ai_comm.h"
#include "k_aenc_comm.h"
#include "mpi_aenc_api.h"
#include "mpi_ai_api.h"
#include "mpi_vicap_api.h"

#else
#include "mapi_sys_api.h"
#include "mapi_ai_api.h"
#include "mapi_ao_api.h"
#include "mapi_sys_api.h"
#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "mapi_aenc_api.h"
#include "mapi_adec_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "k_aenc_comm.h"
#endif

#define ENABLE_AV_DEBUG 1

#ifdef ENABLE_AV_DEBUG
#define av_debug printf
#else
#define av_debug(ARGS...)
#endif

#ifdef ENABLE_MPI

#define VENC_MAX_IN_FRAMES 30
#define WIDTH 1280
#define HEIGHT 720
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080
#define STREAM_BUF_SIZE ((MAX_WIDTH * MAX_HEIGHT / 2 + 0xfff) & ~0xfff)
#define FRAME_BUF_SIZE ((MAX_WIDTH * MAX_HEIGHT * 2 + 0xfff) & ~0xfff)
#define INPUT_BUF_CNT 6
#define OUTPUT_BUF_CNT 15

#define PAYLOAD_H264 1
#define PAYLOAD_JPEG 2
#define PAYLOAD_H265 3

#define AUDIO_PERSEC_DIV_NUM 25
#define ENABLE_SAVE_PCM 1
#define SAVE_PCM_SECOND 15

static k_bool g_enable_audio_codec = K_TRUE;
static volatile k_bool g_aenc_test_start = K_TRUE;
static volatile k_bool g_av_test_start = K_TRUE;
static k_u32 g_sample_rate = 44100;
static k_audio_bit_width g_bit_width = KD_AUDIO_BIT_WIDTH_16;
static pthread_t g_pthread_handle;
static k_bool g_vb_init = K_FALSE;
k_aenc_chn aenc_chn = 0;

static k_u64 K_audio_stamap = 0;
static k_u32 audio_outframes = 0;

static k_s32 k_time = 1;

static k_bool quit = K_TRUE;

void fun_sig(int sig)
{
    if (sig == SIGINT)
    {
        quit = K_FALSE;
    }
}
static void _help()
{
    av_debug("Please input:\n");
    av_debug("-time: test sampe_av_sync time\n");
}

k_s32 av_sample_vb_init(k_bool enable_cache, k_u32 sample_rate)
{
    if (g_vb_init)
    {
        return K_SUCCESS;
    }
    k_s32 ret;
    k_vb_config config;
    k_s32 ch_cnt = 1;

    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;

    config.comm_pool[0].blk_cnt = 150;
    config.comm_pool[0].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config.comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[1].blk_cnt = 2;
    config.comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config.comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[2].blk_cnt = 1;
    config.comm_pool[2].blk_size = sample_rate * 2 * 4 * (SAVE_PCM_SECOND + 1); // save data to memory ,申请大点(+1s)，否则mmz_userdev_mmap会崩溃,wav文件头
    config.comm_pool[2].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config.comm_pool[3].blk_cnt = INPUT_BUF_CNT * ch_cnt;
    config.comm_pool[3].blk_size = FRAME_BUF_SIZE;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;

    config.comm_pool[4].blk_cnt = OUTPUT_BUF_CNT * ch_cnt;
    config.comm_pool[4].blk_size = STREAM_BUF_SIZE;
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;

    int blk_total_size = 0;
    for (int i = 0; i < 5; i++)
    {
        blk_total_size += config.comm_pool[i].blk_cnt * config.comm_pool[i].blk_size;
    }
    av_debug("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);

    ret = kd_mpi_vb_set_config(&config);
    if (ret)
    {
        av_debug("vb_set_config failed ret:%d\n", ret);
        return ret;
    }
    else
    {
        av_debug("vb_set_config ok\n");
    }

    ret = kd_mpi_vb_init();

    if (ret)
        av_debug("vb_init failed ret:%d\n", ret);
    else
        g_vb_init = K_TRUE;

    return ret;
}

static k_s32 sample_vb_exit(void)
{
    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        av_debug("vb_exit failed ret:%d\n", ret);
    return ret;
}

typedef struct
{
    k_u32 chn_height;
    k_u32 chn_width;
    k_pixel_format chn_format;
    k_u32 file_size;
    k_u32 input_frames;
    k_u32 output_frames;
    k_vb_blk_handle *vb_handle;
    k_u64 phys_addr[VENC_MAX_IN_FRAMES][3];
    void *virt_addr[VENC_MAX_IN_FRAMES][3];
    k_s32 pool_id;
    pthread_t input_tid;
    pthread_t output_tid;
    FILE *input_file;
    k_u32 ch_id;
    char *dump_stream;
    k_u32 dump_stream_size;
    k_bool done;
    k_pixel_format pixel_format;
    char out_filename[20];
} sample_venc_conf_t;

static sample_venc_conf_t g_venc_conf[VENC_MAX_CHN_NUMS];

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        av_debug("error ret %d, func %s line %d\n", ret, func, line);
}

static void sample_vicap_config(k_u32 ch, k_vicap_sensor_type sensor_type)
{
    sample_venc_conf_t *venc_conf;
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_vicap_chn vicap_chn = ch;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;

    venc_conf = &g_venc_conf[ch];

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

    sensor_info.sensor_type = sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    CHECK_RET(ret, __func__, __LINE__);

    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    CHECK_RET(ret, __func__, __LINE__);

    chn_attr.out_win.width = venc_conf->chn_width;
    chn_attr.out_win.height = venc_conf->chn_height;

    chn_attr.crop_win = chn_attr.out_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;

    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    chn_attr.buffer_num = INPUT_BUF_CNT;
    chn_attr.buffer_size = venc_conf->chn_width * venc_conf->chn_height * 3 / 2;

    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_vicap_init(vicap_dev);
    CHECK_RET(ret, __func__, __LINE__);
}

void sample_vicap_start(k_u32 ch)
{
    k_s32 ret;

    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
}

void sample_vicap_stop(k_u32 ch)
{
    k_s32 ret;

    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    CHECK_RET(ret, __func__, __LINE__);
}

static void *output_thread(void *arg)
{
    sample_venc_conf_t *venc_conf;
    k_venc_stream output;
    k_s32 out_cnt, out_frames;
    k_s32 ret;
    k_s32 i;
    k_u32 total_len = 0;
    venc_conf = (sample_venc_conf_t *)arg;
    k_s64 delta = 0;
    k_u64 prev_v_pts = 0LL;

    out_cnt = 0;
    out_frames = 0;
    while (g_aenc_test_start)
    {
        k_venc_chn_status status;

        ret = kd_mpi_venc_query_status(venc_conf->ch_id, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);

        ret = kd_mpi_venc_get_stream(venc_conf->ch_id, &output, -1);
        CHECK_RET(ret, __func__, __LINE__);

        out_cnt += output.pack_cnt;
        for (i = 0; i < output.pack_cnt; i++)
        {
            if (output.pack[i].type != K_VENC_HEADER)
            {
                out_frames++;

                if(prev_v_pts != 0)
                {
                    if(output.pack[i].pts == prev_v_pts)
                    {
                        av_debug("v_pts error: %ld %ld frame number %d\n", output.pack[i].pts, prev_v_pts, out_frames);
                    }
                    else
                    {
                        delta = output.pack[i].pts - prev_v_pts;
                        if(delta > 35000LL || delta < 30000LL)
                            av_debug("v_pts error: %ld %ld frame number %d\n", output.pack[i].pts, prev_v_pts, out_frames);
                    }
                }
                prev_v_pts = output.pack[i].pts;

                if (g_av_test_start && out_frames % (venc_conf->output_frames * k_time ) == 0)
                {
                    delta = (output.pack[i].pts - K_audio_stamap);
                    av_debug("v pts = %8ld a pts = %8ld av delta = %8ldms video frames = %d audio frames = %d\n", output.pack[i].pts, K_audio_stamap, delta / 1000, out_frames, audio_outframes);
                    if(delta > 33000LL)
                        av_debug("av sync error: %ld us\n", delta);
                }
            }

            total_len += output.pack[i].len;
        }

        ret = kd_mpi_venc_release_stream(venc_conf->ch_id, &output);
        CHECK_RET(ret, __func__, __LINE__);
        free(output.pack);
    }

    venc_conf->done = K_TRUE;
    av_debug("%s>done, ch %d: out_frames %d, size %d bits\n", __func__, venc_conf->ch_id, out_frames, total_len * 8);
    return arg;
}

static void sample_vi_bind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;
    k_s32 ret;

    vi_mpp_chn.mod_id = K_ID_VI;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &venc_mpp_chn);
    if (ret)
    {
        av_debug("kd_mpi_sys_bind failed:0x%x\n", ret);
    }

    return;
}

static void sample_vi_unbind_venc(k_u32 chn_id)
{
    k_mpp_chn venc_mpp_chn;
    k_mpp_chn vi_mpp_chn;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_id;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = chn_id;
    vi_mpp_chn.chn_id = chn_id;
    kd_mpi_sys_unbind(&vi_mpp_chn, &venc_mpp_chn);

    return;
}

static void *_test_ai_aenc_file_sysbind(void *arg)
{
    k_audio_stream audio_stream;
    k_aenc_chn aenc_channel = aenc_chn;
    k_u64 prev_a_pts = 0LL;
    k_s32 delta;

    while (g_aenc_test_start)
    {
        if (0 != kd_mpi_aenc_get_stream(aenc_channel, &audio_stream, 1000))
        {
            av_debug("kd_mpi_aenc_get_stream failed\n");
            continue;
        }
        else
        {
            if(prev_a_pts != 0)
            {
                if(audio_stream.time_stamp == prev_a_pts)
                {
                    av_debug("a_pts error: %ld %ld frame number %d\n", audio_stream.time_stamp, prev_a_pts, audio_outframes);
                }
                else
                {
                    delta = audio_stream.time_stamp - prev_a_pts;
                    if(delta > 42000LL || delta < 38000LL)
                        av_debug("a_pts error: %ld %ld frame number %d\n", audio_stream.time_stamp, prev_a_pts, audio_outframes);
                }
            }

            prev_a_pts = audio_stream.time_stamp;
            audio_outframes++;
            K_audio_stamap = audio_stream.time_stamp;
            kd_mpi_aenc_release_stream(aenc_channel, &audio_stream);
        }
    }
    return arg;
}
#else

#endif
int main(int argc, char const *argv[])
{
    k_vicap_sensor_type sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;

#ifdef ENABLE_MPI

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            _help();
            return 0;
        }
        else if (strcmp(argv[i], "-time") == 0)
        {
            k_time = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-sensor") == 0)
        {
            sensor_type = atoi(argv[i + 1]);
        }
        else
        {
            av_debug("Error :Invalid arguments %s\n", argv[i]);
            _help();
            return -1;
        }
    }

    k_s32 ch = 0;
    k_u32 bitrate = 1000;
    k_s32 ret;
    k_s32 k_payload = 3;

    memset(g_venc_conf, 0, sizeof(sample_venc_conf_t) * VENC_MAX_CHN_NUMS);

    av_sample_vb_init(K_TRUE, g_sample_rate);

    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;

    k_aenc_chn_attr aenc_chn_attr;
    aenc_chn_attr.type = K_PT_G711A;
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = g_sample_rate / aenc_chn_attr.buf_size;

    if (0 != kd_mpi_aenc_create_chn(aenc_chn, &aenc_chn_attr))
    {
        av_debug("kd_mpi_aenc_create_chn faild\n");
        return K_FAILED;
    }

    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = g_sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = g_bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = g_sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev, &aio_dev_attr))
    {
        av_debug("kd_mpi_ai_set_pub_attr failed\n");
        return K_FAILED;
    }

    kd_mpi_ai_enable(ai_dev);
    kd_mpi_ai_enable_chn(ai_dev, ai_chn);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_chn;
    if (0 != kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn))
    {
        av_debug("%s kd_mpi_sys_bind failed\n", __FUNCTION__);
        return -1;
    }

    g_venc_conf[ch].chn_width = WIDTH;
    g_venc_conf[ch].chn_height = HEIGHT;
    g_venc_conf[ch].pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    g_venc_conf[ch].input_frames = 0;
    g_venc_conf[ch].output_frames = 30;

    k_venc_chn_attr attr;
    switch (k_payload)
    {
    case PAYLOAD_H264:
    {
        memset(&attr, 0, sizeof(attr));
        attr.venc_attr.pic_width = g_venc_conf[ch].chn_width;
        attr.venc_attr.pic_height = g_venc_conf[ch].chn_height;
        attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
        attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

        attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
        attr.rc_attr.cbr.src_frame_rate = 30;
        attr.rc_attr.cbr.dst_frame_rate = 30;
        attr.rc_attr.cbr.bit_rate = bitrate;
        attr.venc_attr.type = K_PT_H264;
        attr.venc_attr.profile = VENC_PROFILE_H264_MAIN;
    }
    break;
    case PAYLOAD_JPEG:
    {
        k_u32 q_factor = 45;
        memset(&attr, 0, sizeof(attr));
        attr.venc_attr.pic_width = WIDTH;
        attr.venc_attr.pic_height = HEIGHT;
        attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
        attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

        attr.venc_attr.type = K_PT_JPEG;
        attr.rc_attr.rc_mode = K_VENC_RC_MODE_MJPEG_FIXQP;
        attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
        attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
        attr.rc_attr.mjpeg_fixqp.q_factor = q_factor;
    }
    break;
    case PAYLOAD_H265:
    {
        memset(&attr, 0, sizeof(attr));
        attr.venc_attr.pic_width = WIDTH;
        attr.venc_attr.pic_height = HEIGHT;
        attr.venc_attr.stream_buf_size = STREAM_BUF_SIZE;
        attr.venc_attr.stream_buf_cnt = OUTPUT_BUF_CNT;

        attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
        attr.rc_attr.cbr.src_frame_rate = 30;
        attr.rc_attr.cbr.dst_frame_rate = 30;
        attr.rc_attr.cbr.bit_rate = bitrate;
        attr.venc_attr.type = K_PT_H265;
        attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    }
    break;
    default:
        av_debug("payload type error\n");
        break;
    }

    ret = kd_mpi_venc_create_chn(ch, &attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_start_chn(ch);
    CHECK_RET(ret, __func__, __LINE__);

    sample_vicap_config(ch, sensor_type);
    sample_vi_bind_venc(ch);
    sample_vicap_start(ch);

    pthread_create(&g_venc_conf[ch].output_tid, NULL, output_thread, &g_venc_conf[ch]);
    pthread_create(&g_pthread_handle, NULL, _test_ai_aenc_file_sysbind, NULL);

    printf("press 'q' to exit application!!\n");

    while (getchar() != 'q')
    {
        usleep(50000);
    }

    g_aenc_test_start = K_FALSE;

    sample_vicap_stop(ch);
    kd_mpi_venc_stop_chn(ch);
    kd_mpi_venc_destroy_chn(ch);

    kd_mpi_ai_disable_chn(ai_dev, ai_chn);
    kd_mpi_ai_disable(ai_dev);
    kd_mpi_aenc_destroy_chn(aenc_chn);

    sample_vi_unbind_venc(ch);
    kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);

    pthread_cancel(g_venc_conf[ch].output_tid);
    pthread_join(g_venc_conf[ch].output_tid, NULL);
    pthread_cancel(g_pthread_handle);
    pthread_join(g_pthread_handle, NULL);

    ret = kd_mpi_venc_close_fd();
    CHECK_RET(ret, __func__, __LINE__);

    sample_vb_exit();

    av_debug("sample av done!\n");

    return 0;
#else
    av_debug("save MAPI lookback");
    return 0;
#endif
}
