#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>
#include <iostream>
#include <fstream>
#include "msg_venc.h"
#include "msg_sys.h"
#include "msg_client_dispatch.h"
#include "read_venc_data.h"

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

    std::fstream stream_file;
    stream_file.open("./doorbell.g711u", std::ios::in | std::ios::binary);
    if (!stream_file.is_open()) {
        std::cout << "music file cannot open." << std::endl;
        return -1;
    }

    stream_file.seekg(0, std::ios::end);
    music_data_size_ = stream_file.tellg();
    stream_file.seekg(0, std::ios::beg);
    music_data_ = new k_u8[music_data_size_];
    stream_file.read((char *)music_data_, music_data_size_);
    stream_file.close();

    config_ = config;
    return 0;
}

int KdMedia::Deinit() {
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
    delete[] music_data_;
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

    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = config_.pitch_shift_semitones;
    if (K_SUCCESS != kd_mapi_ai_set_pitch_shift_attr(ai_handle_, &ps_param))  {
        kd_mapi_ai_deinit(ai_handle_);
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
        kd_mapi_ai_stop(ai_handle_);
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
    kd_mapi_aenc_start(0);
    kd_mapi_ai_start(ai_handle_);
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
    std::unique_lock<std::mutex> lck(thiz->ai_aenc_mutex_);
    if (thiz->ai_started_) {
        if (thiz->on_aenc_data_) {
            thiz->on_aenc_data_->OnAEncData(chn_num, stream_data);
        }
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
        ReleaseVbBuffer(audio_stream_.phys_addr, audio_stream_.len);
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
            ReleaseVbBuffer(audio_stream_.phys_addr, audio_stream_.len);
            _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
            memset(&audio_stream_, 0, sizeof(audio_stream_));
        }
        k_u32 pool_id;
        GetVbbuffer(&pool_id, &audio_stream_.phys_addr, size);

        audio_stream_.stream = _sys_mmap(audio_stream_.phys_addr,size);
        audio_stream_.len = size;
    }

    memcpy(audio_stream_.stream, data, size);
    audio_stream_.time_stamp = timestamp_ms;
    audio_stream_.seq ++;

    kd_mapi_adec_send_stream(adec_handle_,&audio_stream_);
    return 0;
}

int KdMedia::GetVbbuffer(k_u32 *pool_id, k_u64 *phys_addr, k_u64 blk_size) {
    msg_vb_info_t vb_info;
    memset(&vb_info, 0, sizeof(vb_info));
    vb_info.blk_size = blk_size;
    int ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_GET_VB,
            &vb_info, sizeof(vb_info), NULL);
    *phys_addr = vb_info.phys_addr;
    *pool_id = vb_info.pool_id;

    return 0;
}

int KdMedia::ReleaseVbBuffer(k_u64 phys_addr, k_u64 blk_size) {
    msg_vb_info_t vb_info;
    memset(&vb_info, 0, sizeof(vb_info));
    vb_info.blk_size = blk_size;
    vb_info.phys_addr = phys_addr;
    int ret = mapi_send_sync(MODFD(K_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_RELEASE_VB,
        &vb_info, sizeof(vb_info), NULL);
}

int KdMedia::PlayCallMusic(int duration_s) {
    int stream_len = audio_sample_rate_ * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;
    int cur_data_index = 0;
    uint64_t pts = 0;
    std::chrono::time_point<std::chrono::steady_clock> starttime_;
    std::chrono::time_point<std::chrono::steady_clock> endtime_;
    starttime_ = std::chrono::steady_clock::now();
    while (1) {
        if (cur_data_index >= music_data_size_) {
            cur_data_index = 0;
        }
        SendData((const uint8_t*)(music_data_ + cur_data_index), stream_len, pts);
        pts += 40;
        cur_data_index += stream_len;
        endtime_ = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_duration = endtime_ - starttime_;
        if (time_duration.count() >= duration_s)
            break;
    }
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

    uint32_t venc_chn_id = 0;
    msg_venc_fifo_t venc_fifo;
    memset(&venc_fifo, 0, sizeof(venc_fifo));

    venc_fifo.fifo_chn = venc_chn_id;
    k_s32 ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_INIT_DATAFIFO,
                        &venc_fifo, sizeof(venc_fifo), NULL);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("mapi_send_sync failed\n");
    }

    k_u64 datafifo_phyAddr;
    datafifo_phyAddr = venc_fifo.phyAddr;
    printf("create_datafifo datafifo_phyAddr %lx \n", datafifo_phyAddr);

    read_venc_data_init(venc_chn_id, datafifo_phyAddr);

    kd_venc_callback_s venc_callback;
    memset(&venc_callback, 0, sizeof(venc_callback));
    venc_callback.p_private_data = (k_u8*)this;
    venc_callback.pfn_data_cb = KdMedia::VideoEncCallback;
    kd_mapi_venc_registercallback(venc_chn_id, &venc_callback);

    return 0;
}

int KdMedia::DestroyVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    read_venc_data_deinit(chn_id);

    k_s32 ret;
    msg_venc_fifo_t venc_fifo;

    memset(&venc_fifo, 0, sizeof(venc_fifo));
    venc_fifo.fifo_chn = chn_id;

    ret = mapi_send_sync(MODFD(K_MAPI_MOD_VENC, 0, 0), MSG_CMD_MEDIA_VENC_DELETE_DATAFIFO,
                         &venc_fifo, sizeof(venc_fifo), NULL);

    return 0;
}

int KdMedia::StartVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    kd_mapi_venc_start(chn_id, -1);
    return 0;
}

int KdMedia::StopVcapVEnc() {
    if (!config_.video_valid) {
        return 0;
    }
    uint32_t chn_id = 0;
    kd_mapi_venc_stop(chn_id);
    return 0;
}
