#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/mman.h>
#include "ui_common.h"
#include "mapi_vo_api.h"
#include "intercom.h"

#define AUDIO_PERSEC_DIV_NUM 25
#define BIND_VO_LAYER 1

extern  voice_status_t g_voice_status;

static k_s32 g_mmap_fd_tmp = 0;
static std::mutex mmap_mutex_;
static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    std::unique_lock<std::mutex> lck(mmap_mutex_);
    if (g_mmap_fd_tmp == 0)
    {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);
    if (mmap_addr != (void *)(-1))
        virt_addr = (void *)((char *)mmap_addr + (phys_addr & page_mask));
    else
        printf("**** sys_mmap failed\n");

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phys_addr, void *virt_addr, k_u32 size)
{
    std::unique_lock<std::mutex> lck(mmap_mutex_);
    if (g_mmap_fd_tmp == 0)
    {
        return -1;
    }
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    void *mmap_addr = (void *)((char *)virt_addr - (phys_addr & page_mask));
    if (munmap(mmap_addr, mmap_size) < 0)
    {
        printf("**** munmap failed\n");
    }
    return 0;
}

static k_s32 _init_ao(k_audio_bit_width bit_width, k_u32 sample_rate, k_i2s_work_mode i2s_work_mode, k_handle *ao_hdl)
{
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = bit_width;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = i2s_work_mode;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = sample_rate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;

    if (K_SUCCESS != kd_mapi_ao_init(0, 0, &aio_dev_attr, ao_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

static k_s32  _init_adec(k_handle adec_hdl, k_u32 sample_rate)
{
    k_adec_chn_attr adec_chn_attr;
    memset(&adec_chn_attr, 0, sizeof(adec_chn_attr));
    adec_chn_attr.type = K_PT_G711U;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = sample_rate / adec_chn_attr.buf_size;
    adec_chn_attr.mode = K_ADEC_MODE_PACK;

    if (K_SUCCESS != kd_mapi_adec_init(adec_hdl, &adec_chn_attr))
    {
        printf("kd_mapi_adec_init faild\n");
        return -1;
    }

    return K_SUCCESS;
}

int KdMedia::Init()
{
    int ret = 0;
    // ret = kd_mapi_sys_init();
    // if (ret != K_SUCCESS) {
    //     std::cout << "kd_mapi_sys_init error." << std::endl;
    //     return ret;
    // }

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 2 + 2;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = 150;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS)
    {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }

    return ret;
}

int KdMedia::Deinit()
{
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
    return 0;
}

int KdMedia::CreateAiAEnc(IOnAEncData *on_aenc_data)
{
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_initialized) return 0;
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = audio_sample_rate_;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = audio_sample_rate_ / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    if (K_SUCCESS != kd_mapi_ai_init(0, 0, &aio_dev_attr, &ai_handle_))
    {
        std::cout << "kd_mapi_ai_init failed." << std::endl;
        return -1;
    }

    k_aenc_chn_attr aenc_chn_attr;
    memset(&aenc_chn_attr, 0, sizeof(aenc_chn_attr));
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = audio_sample_rate_ / aenc_chn_attr.buf_size;
    aenc_chn_attr.type = K_PT_G711U;
    if (K_SUCCESS != kd_mapi_aenc_init(0, &aenc_chn_attr))
    {
        std::cout << "kd_mapi_aenc_init failed." << std::endl;
        kd_mapi_ai_deinit(ai_handle_);
        return -1;
    }

    this->on_aenc_data_ = on_aenc_data;
    k_aenc_callback_s aenc_cb;
    memset(&aenc_cb, 0, sizeof(aenc_cb));
    aenc_cb.p_private_data = this;
    aenc_cb.pfn_data_cb = KdMedia::AudioEncCallback;
    kd_mapi_aenc_registercallback(0, &aenc_cb);

    ai_initialized = true;
    ai_started_ = false;
    return 0;
}

int KdMedia::DestroyAiAEnc()
{
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_)
    {
        std::cout << "KdMedia::DestroyAiAEnc called, FAILED, stop first!!!" << std::endl;
        return -1;
    }
    if (ai_initialized)
    {
        on_aenc_data_ = nullptr;
        kd_mapi_ai_deinit(ai_handle_);
        kd_mapi_aenc_deinit(0);
        ai_initialized = false;
    }
    return 0;
}

int KdMedia::StartAiAEnc()
{
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_) return 0;
    kd_mapi_ai_start(ai_handle_);
    kd_mapi_aenc_start(0);
    kd_mapi_aenc_bind_ai(ai_handle_, 0);
    ai_started_ = true;
    return 0;
}

int KdMedia::StopAiAEnc()
{
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_)
    {
        kd_mapi_aenc_unbind_ai(ai_handle_, 0);
        printf("%s>ai_handle_ %d\n", __FUNCTION__, ai_handle_);
        kd_mapi_ai_stop(ai_handle_);
        kd_mapi_aenc_stop(0);
        ai_started_ = false;
    }
    return 0;
}

k_s32 KdMedia::AudioEncCallback(k_u32 chn_num, k_audio_stream *stream_data, void *p_private_data)
{
    // printf("%s\n", __FUNCTION__);
    KdMedia *thiz = (KdMedia *)p_private_data;
    if (!thiz) return -1;
    // std::unique_lock<std::mutex> lck(thiz->ai_aenc_mutex_);
    // if (thiz->ai_started_) {
    if (thiz->on_aenc_data_)
    {
        thiz->on_aenc_data_->OnAEncData(chn_num, stream_data);
    }
    // }
    return 0;
}

int KdMedia::CreateADecAo()
{
    k_s32 ret;

    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = _init_ao(KD_AUDIO_BIT_WIDTH_16, audio_sample_rate_, K_STANDARD_MODE, &ao_handle_);
    if (ret != K_SUCCESS)
    {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    ret = _init_adec(adec_handle_, audio_sample_rate_);
    if (ret != K_SUCCESS)
    {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    memset(&audio_stream_, 0, sizeof(audio_stream_));
    return 0;
}

int KdMedia::DestroyADecAo()
{
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = kd_mapi_adec_deinit(adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("_deinit_adec error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_ao_deinit(ao_handle_);
    if (ret != K_SUCCESS)
    {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    if (audio_stream_.len)
    {
        kd_mapi_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
        _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
        memset(&audio_stream_, 0, sizeof(audio_stream_));
    }
    return 0;
}

int KdMedia::StartADecAo()
{
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret;
    ret = kd_mapi_ao_start(ao_handle_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_ao_start failed\n");
        return -1;
    }
    kd_mapi_adec_start(adec_handle_);

    ret = kd_mapi_adec_bind_ao(ao_handle_, adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_adec_bind_ao error: %x\n", ret);
        return -1;
    }
    return 0;
}

int KdMedia::StopADecAo()
{
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret = kd_mapi_adec_unbind_ao(ao_handle_, adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_adec_unbind_ao error: %x\n", ret);
        return -1;
    }
    kd_mapi_adec_stop(adec_handle_);
    kd_mapi_ao_stop(ao_handle_);
    return 0;
}

int KdMedia::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    if (audio_stream_.len != size)
    {
        if (audio_stream_.len)
        {
            kd_mapi_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
            _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
            memset(&audio_stream_, 0, sizeof(audio_stream_));
        }
        k_u32 pool_id;
        ret = kd_mapi_sys_get_vb_block(&pool_id, &audio_stream_.phys_addr, size, NULL);
        // check result, TODO
        audio_stream_.stream = _sys_mmap(audio_stream_.phys_addr, size);
        audio_stream_.len = size;
    }

    memcpy(audio_stream_.stream, data, size);
    audio_stream_.time_stamp = timestamp_ms;
    audio_stream_.seq ++;

    kd_mapi_adec_send_stream(adec_handle_, &audio_stream_);
    return 0;
}

int KdMedia::CreateVdecVBPool(const VdecInitParams &params)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = params.input_buf_num;
    pool_config.blk_size = (params.input_buf_size + 0xfff) & (~0xfff);
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    input_pool_id_ = kd_mapi_vb_create_pool(&pool_config);
    if (input_pool_id_ == VB_INVALID_POOLID)
    {
        return -1;
    }
    printf("input_pool_id = %llu\n", input_pool_id_);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = params.output_buf_num;
    pool_config.blk_size = (params.max_width * params.max_height * 2 + 0xfff) & (~0xfff);
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    output_pool_id_ = kd_mapi_vb_create_pool(&pool_config);
    if (output_pool_id_ == VB_INVALID_POOLID)
    {
        kd_mapi_vb_destory_pool(input_pool_id_);
        return -1;
    }
    printf("output_pool_id = %llu\n", output_pool_id_);
    return 0;
}

int KdMedia::DestroyVdecVBPool()
{
    k_s32 ret;

    ret = kd_mapi_vb_destory_pool(input_pool_id_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_vb_destory_pool %d failed(input), ret = %d\n", input_pool_id_, ret);
    }
    ret = kd_mapi_vb_destory_pool(output_pool_id_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_vb_destory_pool %d failed(output), ret = %d\n", output_pool_id_, ret);
    }
    return 0;
}

int KdMedia::CreateVdecVo(const VdecInitParams &params)
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (vdec_vo_created_) return 0;
    k_s32 ret;

    ret = CreateVdecVBPool(params);
    if (ret < 0)
    {
        return -1;
    }

    k_vdec_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.pic_width = params.max_width;
    attr.pic_height = params.max_height;
    attr.frame_buf_cnt = params.output_buf_num;
    attr.frame_buf_size = (params.max_width * params.max_height * 2 + 0xfff) & (~0xfff);
    attr.stream_buf_size = (params.input_buf_size + 0xfff) & (~0xfff);
    attr.pic_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    attr.frame_buf_pool_id = output_pool_id_;
    switch (params.type)
    {
    case KdMediaVideoType::TypeH264:
        attr.type = K_PT_H264;
        break;
    case KdMediaVideoType::TypeH265:
        attr.type = K_PT_H265;
        break;
    default:
        goto err_exit;
    }

    ret = kd_mapi_vdec_init(vdec_chn_id_, &attr);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::CreateVdecVo() : kd_mapi_vdec_init failed, ret = %d\n", ret);
        goto err_exit;
    }

    vdec_params_ = params;
    vdec_vo_created_ = true;
    vdec_vo_started_ = false;
    return 0;
err_exit:
    DestroyVdecVBPool();
    return -1;
}

int KdMedia::DestroyVDecVo()
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (!vdec_vo_created_) return 0;
    if (vdec_vo_started_)
    {
        printf("KdMedia::DestroyVDecVo() : stop first!!!\n");
        return -1;
    }
    kd_mapi_vdec_deinit(vdec_chn_id_);
    DestroyVdecVBPool();
    vdec_vo_created_ = false;
    vdec_vo_started_ = false;
    return 0;
}

int KdMedia::StartVDecVo()
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);

    if (vdec_vo_started_)
    {
        return 0;
    }
    k_s32 ret = kd_mapi_vdec_start(vdec_chn_id_);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::StartVDecVo() : kd_mapi_vdec_start failed, ret = %d\n", ret);
        return -1;
    }
    ret = kd_mapi_vo_enable_video_layer(K_VO_LAYER1);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::StartVDecVo() : kd_mapi_vo_enable_video_layer failed , ret = %d\n", ret);
    }
    kd_mapi_vdec_bind_vo(vdec_chn_id_, 0, BIND_VO_LAYER);

    vdec_data_feed_ = false;
    vdec_vo_started_ = true;
    return 0;
}

int KdMedia::StopVDecVo()
{
    int ret = 0;
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (!vdec_vo_started_)
    {
        return 0;
    }

    if (vdec_data_feed_)
    {
        lck.unlock();
        SendVideoData(nullptr, 0, 0);
        lck.lock();
        k_vdec_chn_status status;
        while (1)
        {
            k_s32 ret = kd_mapi_vdec_query_status(vdec_chn_id_, &status);
            if (ret != K_SUCCESS)
            {
                printf("KdMedia::StopVDecVo() : kd_mapi_vdec_query_status failed, ret = %d\n", ret);
                return -1;
            }
            if (status.end_of_stream)
            {
                printf("KdMedia::StopVDecVo() : EOS done\n");
                break;
            }
            sleep(1);
        }
    }
    kd_mapi_vdec_unbind_vo(vdec_chn_id_, 0, BIND_VO_LAYER);
    kd_mapi_vdec_stop(vdec_chn_id_);
    ret = kd_mapi_vo_disable_video_layer(K_VO_LAYER1);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_vo_disable_video_layer failed\n");
    }

    vdec_vo_started_ = false;
    return 0;
}

int KdMedia::SendVideoData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    // printf("%s\n", __FUNCTION__);
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (!vdec_vo_started_) return 0;

    k_vdec_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.len = size;
    stream.pts = timestamp_ms;
    stream.end_of_stream = (data && size) ? K_FALSE : K_TRUE;
    // printf("%s>stream.len %d, stream.pts %d, stream.end_of_stream %d\n", __FUNCTION__, stream.len, stream.pts, stream.end_of_stream);

    int count = 10;
    while (1)
    {
        k_s32 ret = kd_mapi_sys_get_vb_block_from_pool_id(input_pool_id_, &stream.phy_addr, vdec_params_.input_buf_size, NULL);
        if (ret != K_SUCCESS)
        {
            if (count < 0)
            {
                printf("KdMedia::SendVideoData() : FAILED, get vb timeout\n");
                return -1;
            }
            lck.unlock();
            usleep(30000);
            lck.lock();
            continue;
        }
        break;
    }

    void *virt_addr = _sys_mmap(stream.phy_addr, vdec_params_.input_buf_size);
    if (!virt_addr)
    {
        printf("KdMedia::SendVideoData() : _sys_mmap failed\n");
        kd_mapi_sys_release_vb_block(stream.phy_addr, vdec_params_.input_buf_size);
        return -1;
    }
    if (data && size) memcpy(virt_addr, data, size);

    // printf("call kd_mapi_vdec_send_stream\n");
    k_s32 ret = kd_mapi_vdec_send_stream(vdec_chn_id_, &stream, -1);
    usleep(10000);
    _sys_munmap(stream.phy_addr, virt_addr, vdec_params_.input_buf_size);
    kd_mapi_sys_release_vb_block(stream.phy_addr, vdec_params_.input_buf_size);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::SendVideoData() : kd_mapi_vdec_send_stream failed, ret = %d\n", ret);
        return -1;
    }
    if (data && size) vdec_data_feed_ = true;
    return 0;
}

int KdMedia::EnablePitchShift()
{
    printf("%s\n", __FUNCTION__);
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = pitch_shift_semitones_;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_handle_, &ps_param))
    {
        kd_mapi_ai_deinit(ai_handle_);
        std::cout << "kd_mapi_ai_init failed." << std::endl;
        return -1;
    }
    g_voice_status = VOICE_STATUS_CHANGED;
    return 0;
}

int KdMedia::DisablePitchShift()
{
    printf("%s\n", __FUNCTION__);
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = 0;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_handle_, &ps_param))
    {
        kd_mapi_ai_deinit(ai_handle_);
        std::cout << "kd_mapi_ai_init failed." << std::endl;
        return -1;
    }
    g_voice_status = VOICE_STATUS_ORIGIN;
    return 0;
}

int KdMedia::CreateMux()
{
    // printf("%s\n", __FUNCTION__);
    int ret = 0;
    // create mp4 instance
    k_mp4_config_s mp4_config;
    memset(&mp4_config, 0, sizeof(mp4_config));
    mp4_config.config_type = K_MP4_CONFIG_MUXER;
    strcpy(mp4_config.muxer_config.file_name, "/sharefs/test.mp4");
    mp4_config.muxer_config.fmp4_flag = 0;

    ret = kd_mp4_create(&mp4_muxer_, &mp4_config);
    if (ret < 0)
    {
        printf("mp4 muxer create failed.\n");
    }

    // create video track
    k_mp4_track_info_s video_track_info;
    memset(&video_track_info, 0, sizeof(video_track_info));
    video_track_info.track_type = K_MP4_STREAM_VIDEO;
    video_track_info.time_scale = 1000;
    video_track_info.video_info.width = vdec_params_.max_width;
    video_track_info.video_info.height = vdec_params_.max_height;
    ret = kd_mp4_create_track(mp4_muxer_, &video_track_handle_, &video_track_info);
    if (ret < 0)
    {
        printf("create video track failed.\n");
        DestroyMux();
    }

    // create audio track
    k_mp4_track_info_s audio_track_info;
    memset(&audio_track_info, 0, sizeof(audio_track_info));
    audio_track_info.track_type = K_MP4_STREAM_AUDIO;
    audio_track_info.time_scale = 1000;
    audio_track_info.audio_info.channels = 1;
    audio_track_info.audio_info.codec_id = K_MP4_CODEC_ID_G711U;
    audio_track_info.audio_info.sample_rate = audio_sample_rate_;
    audio_track_info.audio_info.bit_per_sample = 16;
    ret = kd_mp4_create_track(mp4_muxer_, &audio_track_handle_, &audio_track_info);
    if (ret < 0)
    {
        printf("create video track failed.\n");
        DestroyMux();
    }
    return ret;
}

int KdMedia::DestroyMux()
{
    // printf("%s\n", __FUNCTION__);
    int ret = 0;
    ret = kd_mp4_destroy_tracks(mp4_muxer_);
    if (ret < 0)
    {
        printf("destroy mp4 tracks failed.\n");
        return -1;
    }

    kd_mp4_destroy(mp4_muxer_);
    if (ret < 0)
    {
        printf("destroy mp4 failed.\n");
        return -1;
    }
    return 0;
}

int KdMedia::MuxVideoData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    // printf("%s\n", __FUNCTION__);
    k_mp4_frame_data_s frame_data;
    memset(&frame_data, 0, sizeof(frame_data));

    switch (vdec_params_.type)
    {
    case KdMediaVideoType::TypeH264:
        frame_data.codec_id = K_MP4_CODEC_ID_H264;
        break;
    case KdMediaVideoType::TypeH265:
        frame_data.codec_id = K_MP4_CODEC_ID_H265;
        break;
    default:
        frame_data.codec_id = K_MP4_CODEC_ID_H265;
        break;
    }
    frame_data.data = (uint8_t *)data;
    frame_data.data_length = size;
    frame_data.time_stamp = timestamp_ms;

    k_u32 ret = kd_mp4_write_frame(mp4_muxer_, video_track_handle_, &frame_data);
    if (ret < 0)
    {
        printf("mp4 write video frame failed.\n");
        return -1;
    }
    return 0;
}

int KdMedia::MuxAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    // printf("%s\n", __FUNCTION__);
    k_mp4_frame_data_s frame_data;
    memset(&frame_data, 0, sizeof(frame_data));

    frame_data.codec_id = K_MP4_CODEC_ID_G711U;
    frame_data.data = (uint8_t *)data;
    frame_data.data_length = size;
    frame_data.time_stamp = timestamp_ms;

    k_u32 ret = kd_mp4_write_frame(mp4_muxer_, audio_track_handle_, &frame_data);
    if (ret < 0)
    {
        printf("mp4 write video frame failed.\n");
        return -1;
    }
    return 0;
}