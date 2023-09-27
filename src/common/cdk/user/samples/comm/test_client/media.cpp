#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>

#define AUDIO_PERSEC_DIV_NUM 25

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

int KdMedia::Init() {
    int ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_sys_init error." << std::endl;
        return ret;
    }

    // vbconfig is performed by the app on rt-smart core,
    //    but we have to call kd_mapi_media_init() in order to use kd_mapi_vb_create_pool etc.
#if 1
    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 64;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }
#endif
    return ret;
}

int KdMedia::Deinit() {
    // NOTE:  vbconfig is not performed by us, do not call media_deinit
    // kd_mapi_media_deinit();
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

int KdMedia::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms) {
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
