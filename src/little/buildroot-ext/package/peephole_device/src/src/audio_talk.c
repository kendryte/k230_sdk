#include "audio_talk.h"
#include <stdio.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>

#include "mapi_sys_api.h"
#include "mapi_ao_api.h"
#include "mapi_ai_api.h"

#define AUDIO_PERSEC_DIV_NUM 25
static k_bool g_vb_init = K_FALSE;
static k_bool g_enable_audio_codec = K_TRUE;

static k_s32 _mapi_sample_vb_init(k_bool enable_cache,k_u32 sample_rate)
{
    if (g_vb_init)
    {
        printf("%s already init\n",__FUNCTION__);
        return K_SUCCESS;
    }

    k_s32 ret;
    #if 0
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error: %x\n", ret);
        return K_FAILED;
    }
    #endif

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));
    k_vb_config* config = &media_attr.media_config.vb_config;

    memset(config, 0, sizeof(*config));
    config->max_pool_cnt = 64;

    config->comm_pool[0].blk_cnt = 150;
    config->comm_pool[0].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config->comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

    config->comm_pool[1].blk_cnt = 2;
    config->comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config->comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;


    int blk_total_size = 0;
    for (int i = 0; i < 2; i++)
    {
        blk_total_size += config->comm_pool[i].blk_cnt * config->comm_pool[i].blk_size;
    }
    printf("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);

    ret = kd_mapi_media_init(&media_attr);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_init error: %x\n", ret);
        return K_FAILED;
    }

    g_vb_init = K_TRUE;

    return ret;
}

static k_s32 _mapi_sample_vb_deinit()
{
    if (!g_vb_init)
   {
        printf("%s not init\n",__FUNCTION__);
        return K_FAILED;
    }
    k_s32 ret;
    ret = kd_mapi_media_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_media_deinit error: %x\n", ret);
        return -1;
    }
    kd_mapi_media_init_workaround(K_TRUE);

    #if 0
    ret = kd_mapi_sys_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit error: %x\n", ret);
        return -1;
    }
    #endif

    g_vb_init = K_FALSE;

    return ret;
}

static k_s32 _deinit_ai(k_handle ai_hdl)
{
    if (K_SUCCESS !=kd_mapi_ai_stop(ai_hdl))
    {
        printf("kd_mapi_ai_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_deinit(ai_hdl))
    {
        printf("kd_mapi_ai_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32 _init_ao(k_audio_bit_width bit_width, k_u32 sample_rate,k_u32 channels, k_i2s_work_mode i2s_work_mode,k_handle* ao_hdl)
{
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;

    if (!g_enable_audio_codec)
    {
        printf("force the i2s_mode to right justified(tm8821)\n");
        aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE; // tm8821 为i2s 右对齐
    }

    if (K_SUCCESS != kd_mapi_ao_init(0,0, &aio_dev_attr,ao_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_start(*ao_hdl))
    {
        printf("kd_mapi_ao_start failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32 _deinit_ao(k_handle ao_hdl)
{
    if (K_SUCCESS !=kd_mapi_ao_stop(ao_hdl))
    {
        printf("kd_mapi_ao_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_deinit(ao_hdl))
    {
        printf("kd_mapi_ao_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}


static k_handle ai_i2s_hdl = 0;
static k_handle ao_i2s_hdl = 0;
static k_handle ai_pdm_hdl = 0;

static k_s32 _enable_audio_talk(k_u32 sample_rate,k_u32 channels,k_bool enable_audio_codec)
{
    k_s32 ret;

    ret = _mapi_sample_vb_init(K_TRUE, sample_rate);
    if (ret != K_SUCCESS)
    {
        printf("audio_sample_vb_init failed\n");
        return -1;
    }

    //pdm in 1 channel enable
    k_aio_dev_attr ai_pdm_dev_attr;
    ai_pdm_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_PDM;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.sample_rate = sample_rate;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.chn_cnt = 4; // max pdm channel
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.snd_mode = (1 == channels) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.pdm_oversample = KD_AUDIO_PDM_INPUT_OVERSAMPLE_64;
    ai_pdm_dev_attr.kd_audio_attr.pdm_attr.point_num_per_frame = sample_rate / ai_pdm_dev_attr.kd_audio_attr.pdm_attr.frame_num;
    if (K_SUCCESS != kd_mapi_ai_init(1,0, &ai_pdm_dev_attr,&ai_pdm_hdl))
    {
        printf("kd_mapi_ai_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_start(ai_pdm_hdl))
    {
        printf("pdm kd_mapi_ai_start failed\n");
        return K_FAILED;
    }

    //i2s in 1 channel enable

    k_aio_dev_attr ai_i2s_dev_attr;
    ai_i2s_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / ai_i2s_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ai_i2s_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;
    if (K_SUCCESS != kd_mapi_ai_init(0,0, &ai_i2s_dev_attr,&ai_i2s_hdl))
    {
        printf("kd_mapi_ai_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ai_start(ai_i2s_hdl))
    {
        printf("i2s kd_mapi_ai_start failed\n");
        return K_FAILED;
    }

    //i2s out 2 channel enable
    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==channels)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = g_enable_audio_codec ? K_AIO_I2STYPE_INNERCODEC : K_AIO_I2STYPE_EXTERN;


    if (K_SUCCESS != kd_mapi_ao_init(0,2/*all channel*/, &ao_dev_attr,&ao_i2s_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_start(ao_i2s_hdl))
    {
        printf("kd_mapi_ao_start failed\n");
        return K_FAILED;
    }

    //sysbind
    //ai_0(i2s) bind ao_1(i2s)
    ret = kd_mapi_ai_bind_ao(ai_i2s_hdl,(((0) << 16) | ((1) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("i2s->i2s kd_mapi_ai_bind_ao error: %x\n", ret);
        return -1;
    }

    //ai_1(pdm) bind ao_0(i2s)
    ret = kd_mapi_ai_bind_ao(ai_pdm_hdl,(((0) << 16) | ((0) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("pdm->i2s kd_mapi_ai_bind_ao error: %x\n", ret);
        return -1;
    }

    //modify ao audio volume
    kd_mapi_ao_set_volume(ao_i2s_hdl,6);
    //modify ai audio volume
    kd_mapi_ai_set_volume(ai_i2s_hdl,-3);

    return 0;
}

static k_s32 _disable_audio_talk()
{
    k_s32 ret;

    //sys unbind
    //ai_0(i2s) unbind ao_1(i2s)
    ret = kd_mapi_ai_unbind_ao(ai_i2s_hdl,(((0) << 16) | ((1) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("i2s->i2s kd_mapi_ai_unbind_ao error: %x\n", ret);
        return -1;
    }

    //ai_1(pdm) unbind ao_0(i2s)
    ret = kd_mapi_ai_unbind_ao(ai_pdm_hdl,(((0) << 16) | ((0) & 0xFFFF)));
    if(ret != K_SUCCESS) {
        printf("pdm->i2s kd_mapi_ai_unbind_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ao(ao_i2s_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_i2s_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ai(ai_pdm_hdl);
    if(ret != K_SUCCESS) {
        printf("_deinit_ai error: %x\n", ret);
        return -1;
    }

    //reset audio codec
    kd_mapi_acodec_reset();

    ret = _mapi_sample_vb_deinit();
    if(ret != K_SUCCESS) {
        printf("_mapi_sample_vb_deinit error: %x\n", ret);
        return -1;
    }

    return 0;
}


static int pitch_shift_semitones_ = 12; // [-12, 12]
static int _enable_pitch_shift()
{
    printf("%s\n", __FUNCTION__);
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = pitch_shift_semitones_;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_i2s_hdl, &ps_param))
    {
        kd_mapi_ai_deinit(ai_i2s_hdl);
        printf("kd_mapi_ai_init failed.\n");
        return -1;
    }

    return 0;
}

static int _disable_pitch_shift()
{
    printf("%s\n", __FUNCTION__);
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = 0;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_i2s_hdl, &ps_param))
    {
        kd_mapi_ai_deinit(ai_i2s_hdl);
        printf("kd_mapi_ai_init failed.\n");
        return -1;
    }

    return 0;
}

k_s32  audio_talk(k_bool start)
{
    if (start)
    {
        _enable_audio_talk(8000,1,K_TRUE);
    }
    else
    {
        _disable_audio_talk();
    }
    return K_SUCCESS;
}

k_s32  audio_pitch_shift(k_bool enable)
{
    if (enable)
    {
        _enable_pitch_shift();
    }
    else
    {
        _disable_pitch_shift();
    }

    return 0;
}
