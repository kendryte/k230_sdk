#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>

#define AUDIO_PERSEC_DIV_NUM 25
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

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
    if (g_mmap_fd_tmp == 0) {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);
    if (mmap_addr != (void*)(-1))
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));
    else
        printf("**** sys_mmap failed\n");

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phys_addr, void *virt_addr, k_u32 size)
{
    std::unique_lock<std::mutex> lck(mmap_mutex_);
    if (g_mmap_fd_tmp == 0) {
        return -1;
    }
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    void* mmap_addr = (void*)((char*)virt_addr - (phys_addr & page_mask));
    if( munmap(mmap_addr, mmap_size) < 0) {
        printf("**** munmap failed\n");
    }
    return 0;
}

int KdMedia::Init(const KdMediaInputConfig &config) {
    int ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_sys_init error." << std::endl;
        return ret;
    }

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = config.video_valid? 5 : 2;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = 150;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    if (config.video_valid) {
        k_u64 pic_size = config.venc_width * config.venc_height * 2;
        k_u64 stream_size = config.venc_width * config.venc_height / 2;
        media_attr.media_config.vb_config.comm_pool[2].blk_cnt = 6;
        media_attr.media_config.vb_config.comm_pool[2].blk_size = ((pic_size + 0xfff) & ~0xfff);
        media_attr.media_config.vb_config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
        media_attr.media_config.vb_config.comm_pool[3].blk_cnt = 30;
        media_attr.media_config.vb_config.comm_pool[3].blk_size = ((stream_size + 0xfff) & ~0xfff);
        media_attr.media_config.vb_config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;

        memset(&vcap_dev_info_, 0, sizeof(vcap_dev_info_));
        if (config.sensor_type <= OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE || config.sensor_type >= SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR)
            vcap_dev_info_.dw_en = K_FALSE;
        else
            vcap_dev_info_.dw_en = K_TRUE;

        if (vcap_dev_info_.dw_en) {
            k_vicap_sensor_info sensor_info;
            memset(&sensor_info, 0, sizeof(sensor_info));
            sensor_info.sensor_type = config.sensor_type;
            int ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
            if (ret != K_SUCCESS) {
                std::cout << "KdMedia::Init() kd_mapi_vicap_get_sensor_info failed, ret = " << ret << std::endl;
                kd_mapi_sys_deinit();
                return ret;
            }
            media_attr.media_config.vb_config.comm_pool[4].blk_cnt = 6;
            media_attr.media_config.vb_config.comm_pool[4].blk_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x1000);
            media_attr.media_config.vb_config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
        }
        // memset(&media_attr_.media_config.vb_supp.supplement_config, 0, sizeof(media_attr_.media_config.vb_supp.supplement_config));
        // media_attr_.media_config.vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;
    }

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }

    if (config.video_valid) {
        vcap_dev_info_.pipe_ctrl.data = 0xFFFFFFFF;
        vcap_dev_info_.sensor_type = config.sensor_type;
        vcap_dev_info_.vicap_dev = VICAP_DEV_ID_0;
        ret = kd_mapi_vicap_set_dev_attr(vcap_dev_info_);
        if (ret != K_SUCCESS) {
            std::cout << "KdMedia::Init() kd_mapi_vicap_set_dev_attr failed, ret = " << ret << std::endl;
            kd_mapi_media_deinit();
            kd_mapi_sys_deinit();
            return ret;
        }
    }
    config_ = config;
    return ret;
}

int KdMedia::Deinit() {
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
    return 0;
}

int KdMedia::CreateAiAEnc(IOnAEncData *on_aenc_data) {
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
    if (K_SUCCESS != kd_mapi_ai_init(0, 0, &aio_dev_attr, &ai_handle_)) {
        std::cout << "kd_mapi_ai_init failed." << std::endl;
        return -1;
    }

    // for debug pitch-shift
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = config_.pitch_shift_semitones;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_handle_, &ps_param))  {
        kd_mapi_ai_deinit(ai_handle_);
        std::cout << "kd_mapi_ai_init kd_mapi_ai_set_pitch_shift_attr." << std::endl;
        return -1;
    }

    k_aenc_chn_attr aenc_chn_attr;
    memset(&aenc_chn_attr, 0, sizeof(aenc_chn_attr));
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = audio_sample_rate_ / aenc_chn_attr.buf_size;
    aenc_chn_attr.type = K_PT_G711U;
    if (K_SUCCESS != kd_mapi_aenc_init(0, &aenc_chn_attr)) {
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

int KdMedia::DestroyAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_) {
        std::cout << "KdMedia::DestroyAiAEnc called, FAILED, stop first!!!" << std::endl;
        return -1;
    }
    if (ai_initialized) {
        on_aenc_data_ = nullptr;
        kd_mapi_ai_deinit(ai_handle_);
        kd_mapi_aenc_deinit(0);
        ai_initialized = false;
    }
    return 0;
}

int KdMedia::StartAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if(ai_started_) return 0;
    kd_mapi_ai_start(ai_handle_);
    kd_mapi_aenc_start(0);
    kd_mapi_aenc_bind_ai(ai_handle_, 0);
    ai_started_ = true;
    return 0;
}

int KdMedia::StopAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_) {
        kd_mapi_aenc_unbind_ai(ai_handle_, 0);
        kd_mapi_ai_stop(ai_handle_);
        kd_mapi_aenc_stop(0);
        ai_started_ = false;
    }
    return 0;
}

k_s32 KdMedia::AudioEncCallback(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data) {
    KdMedia *thiz = (KdMedia*)p_private_data;
    if(!thiz) return -1;
    if (thiz->on_aenc_data_) {
        thiz->on_aenc_data_->OnAEncData(chn_num, stream_data);
    }
    return 0;
}

int KdMedia::CreateADecAo() {
    k_s32 ret;

    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = audio_sample_rate_;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = audio_sample_rate_ / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    ret = kd_mapi_ao_init(0, 0, &aio_dev_attr, &ao_handle_);
    if(ret != K_SUCCESS) {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    k_adec_chn_attr adec_chn_attr;
    memset(&adec_chn_attr, 0, sizeof(adec_chn_attr));
    adec_chn_attr.type = K_PT_G711U;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = audio_sample_rate_ / adec_chn_attr.buf_size;
    adec_chn_attr.mode = K_ADEC_MODE_PACK;
    ret = kd_mapi_adec_init(adec_handle_, &adec_chn_attr);
    if(ret != K_SUCCESS) {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    memset(&audio_stream_, 0, sizeof(audio_stream_));
    return 0;
}

int KdMedia::DestroyADecAo() {
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = kd_mapi_adec_deinit(adec_handle_);
    if(ret != K_SUCCESS) {
        printf("_deinit_adec error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_ao_deinit(ao_handle_);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    if (audio_stream_.len) {
        kd_mapi_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
        _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
        memset(&audio_stream_, 0, sizeof(audio_stream_));
    }
    return 0;
}

int KdMedia::StartADecAo() {
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret;
    ret = kd_mapi_ao_start(ao_handle_);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_ao_start failed\n");
        return -1;
    }
    kd_mapi_adec_start(adec_handle_);

    ret = kd_mapi_adec_bind_ao(ao_handle_, adec_handle_);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_adec_bind_ao error: %x\n", ret);
        return -1;
    }
    return 0;
}

int KdMedia::StopADecAo() {
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret = kd_mapi_adec_unbind_ao(ao_handle_, adec_handle_);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_unbind_ao error: %x\n", ret);
        return -1;
    }
    kd_mapi_adec_stop(adec_handle_);
    kd_mapi_ao_stop(ao_handle_);
    return 0;
}

int KdMedia::SendData(const uint8_t *data, size_t size, uint64_t timestamp_ms) {
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    if (audio_stream_.len != size) {
        if (audio_stream_.len) {
            kd_mapi_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
            _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
            memset(&audio_stream_, 0, sizeof(audio_stream_));
        }
        k_u32 pool_id;
        ret = kd_mapi_sys_get_vb_block(&pool_id, &audio_stream_.phys_addr, size, NULL);
        // check result, TODO
        audio_stream_.stream = _sys_mmap(audio_stream_.phys_addr,size);
        audio_stream_.len = size;
    }

    memcpy(audio_stream_.stream, data, size);
    audio_stream_.time_stamp = timestamp_ms;
    audio_stream_.seq ++;

    kd_mapi_adec_send_stream(adec_handle_,&audio_stream_);
    return 0;
}

k_s32 KdMedia::VideoEncCallback(k_u32 chn_num, kd_venc_data_s* stream_data, k_u8* p_private_data) {
    KdMedia *thiz = (KdMedia*)p_private_data;
    if (thiz && thiz->on_venc_data_) {
        int cut = stream_data->status.cur_packs;
        for (int i = 0; i < cut; i++) {
            thiz->on_venc_data_->OnVEncData(chn_num, stream_data->astPack[i].vir_addr, stream_data->astPack[i].len, stream_data->astPack[i].pts);
        }
    }
    return 0;
}

int KdMedia::CreateVcapVEnc(IOnVEncData *on_venc_data) {
    if (!config_.video_valid) {
        return 0;
    }
    on_venc_data_ = on_venc_data;

    k_venc_chn_attr chn_attr;
    memset(&chn_attr, 0, sizeof(chn_attr));
    k_u64 stream_size = config_.venc_width * config_.venc_height / 2;
    chn_attr.venc_attr.pic_width = config_.venc_width;
    chn_attr.venc_attr.pic_height = config_.venc_height;
    chn_attr.venc_attr.stream_buf_size = ((stream_size + 0xfff) & ~0xfff);
    chn_attr.venc_attr.stream_buf_cnt = 30;
    chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
    chn_attr.rc_attr.cbr.src_frame_rate = 30;
    chn_attr.rc_attr.cbr.dst_frame_rate = 30;
    if (config_.sensor_type == OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR || 
        config_.sensor_type == OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE) {
            chn_attr.rc_attr.cbr.src_frame_rate = 15;
            chn_attr.rc_attr.cbr.dst_frame_rate = 15;
    } else if (config_.sensor_type == OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE) {
            chn_attr.rc_attr.cbr.src_frame_rate = 60;
            chn_attr.rc_attr.cbr.dst_frame_rate = 60;
    }
    chn_attr.rc_attr.cbr.bit_rate = config_.bitrate_kbps;
    if (config_.video_type ==  KdMediaVideoType::kVideoTypeH264) {
        chn_attr.venc_attr.type = K_PT_H264;
        chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
    } else if (config_.video_type ==  KdMediaVideoType::kVideoTypeH265) {
        chn_attr.venc_attr.type = K_PT_H265;
        chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    } else if (config_.video_type ==  KdMediaVideoType::kVideoTypeMjpeg) {
        chn_attr.venc_attr.type = K_PT_JPEG;
        chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_MJPEG_FIXQP;
        chn_attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.q_factor = 45;
    }

    uint32_t venc_chn_id = 0;
    int ret = kd_mapi_venc_init(venc_chn_id, &chn_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_venc_init error." << std::endl;
        return -1;
    }

    if (config_.video_type != KdMediaVideoType::kVideoTypeMjpeg) {
        ret = kd_mapi_venc_enable_idr(venc_chn_id, K_TRUE);
        if (ret != K_SUCCESS) {
            std::cout << "kd_mapi_venc_enable_idr error." << std::endl;
            return -1;
        }
    }

    kd_venc_callback_s venc_callback;
    memset(&venc_callback, 0, sizeof(venc_callback));
    venc_callback.p_private_data = (k_u8*)this;
    venc_callback.pfn_data_cb = KdMedia::VideoEncCallback;
    kd_mapi_venc_registercallback(venc_chn_id, &venc_callback);

    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(sensor_info));
    sensor_info.sensor_type = config_.sensor_type;
    ret = kd_mapi_vicap_get_sensor_info(&sensor_info);
    if (ret != K_SUCCESS) {
        printf("kd_mapi_vicap_get_sensor_info failed, %x.\n", ret);
        return -1;
    }

    k_vicap_chn_set_info vi_chn_attr_info;
    memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));

    vi_chn_attr_info.crop_en = K_FALSE;
    vi_chn_attr_info.scale_en = K_FALSE;
    vi_chn_attr_info.chn_en = K_TRUE;
    vi_chn_attr_info.crop_h_start = 0;
    vi_chn_attr_info.crop_v_start = 0;
    vi_chn_attr_info.out_width = config_.venc_width;
    vi_chn_attr_info.out_height = config_.venc_height;
    vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    vi_chn_attr_info.vicap_dev = VICAP_DEV_ID_0;
    vi_chn_attr_info.vicap_chn = (k_vicap_chn)venc_chn_id;
    if (!vcap_dev_info_.dw_en)
        vi_chn_attr_info.buf_size = VI_ALIGN_UP(config_.venc_width * config_.venc_height * 3 / 2, 0x400);
    else
        vi_chn_attr_info.buf_size = VI_ALIGN_UP(config_.venc_width * config_.venc_height * 3 / 2, 0x400);
    ret = kd_mapi_vicap_set_chn_attr(vi_chn_attr_info);
    if (ret != K_SUCCESS) {
        printf("vicap chn %d set attr failed, %x.\n", venc_chn_id, ret);
        return -1;
    }

    return 0;
}

int KdMedia::DestroyVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    kd_mapi_venc_deinit(chn_id);
    return 0;
}

int KdMedia::StartVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    kd_mapi_venc_start(chn_id, -1);
    kd_mapi_venc_bind_vi(0, chn_id, chn_id);
    kd_mapi_vicap_start(VICAP_DEV_ID_0);
    return 0;
}

int KdMedia::StopVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    // unbind all
    kd_mapi_venc_unbind_vi(VICAP_DEV_ID_0, chn_id, chn_id);
    // stop vcap
    kd_mapi_vicap_stop(VICAP_DEV_ID_0);
    // stop all encoders
    kd_mapi_venc_stop(chn_id);
    return 0;
}
