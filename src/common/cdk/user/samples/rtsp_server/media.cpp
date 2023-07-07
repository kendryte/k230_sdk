#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>

#define AUDIO_PERSEC_DIV_NUM 25

static k_s32 g_mmap_fd_tmp = 0;
static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    // k_u32 mmap_size = (((size) + (page_size) - 1) & ~((page_size) - 1));
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    if (g_mmap_fd_tmp == 0)
    {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);
    if (mmap_addr != (void*)(-1))
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phys_addr, void *virt_addr, k_u32 size)
{
    if (g_mmap_fd_tmp == 0)
    {
        return -1;
    }
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    void* mmap_addr = (void*)((char*)virt_addr - (phys_addr & page_mask));
    munmap(mmap_addr, mmap_size);
    return 0;
}

static k_s32 _init_ao(k_audio_bit_width bit_width, k_u32 sample_rate, k_i2s_work_mode i2s_work_mode,k_handle* ao_hdl) 
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

static k_s32  _init_adec(k_handle adec_hdl,k_u32 sample_rate)
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

    if (K_SUCCESS != kd_mapi_adec_start(adec_hdl))
    {
        printf("kd_mapi_adec_start faild\n");
        return -1;
    }

    return K_SUCCESS;
}

static k_s32 _deinit_adec(k_handle adec_hdl)
{
    if (K_SUCCESS !=kd_mapi_adec_stop(adec_hdl))
    {
        printf("kd_mapi_adec_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_adec_deinit(adec_hdl))
    {
        printf("kd_mapi_adec_deinit failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

int KdMedia::Init() {
    int ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_sys_init error." << std::endl;
        return ret;
    }

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = 150;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }

    return ret;
}

int KdMedia::Deinit() {
    kd_mapi_media_deinit();
    kd_mapi_sys_deinit();
    return 0;
}

int KdMedia::CreateAiAEnc(IOnAEncData *on_aenc_data) {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
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
    ai_initialized = true;

    if (K_SUCCESS != kd_mapi_ai_start(ai_handle_)) {
        std::cout << "kd_mapi_ai_start failed." << std::endl;
        kd_mapi_ai_deinit(ai_handle_);
        ai_initialized = false;
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
        ai_initialized = false;
        return -1;
    }

    this->on_aenc_data_ = on_aenc_data;
    k_aenc_callback_s aenc_cb;
    memset(&aenc_cb, 0, sizeof(aenc_cb));
    aenc_cb.p_private_data = this;
    aenc_cb.pfn_data_cb = KdMedia::AudioEncCallback;
    kd_mapi_aenc_registercallback(0, &aenc_cb);
    return 0;
}

int KdMedia::DestroyAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_initialized) {
        kd_mapi_ai_deinit(ai_handle_);
        kd_mapi_aenc_deinit(0);
        ai_initialized = false;
    }
    return 0;
}

int KdMedia::StartAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_initialized) {
        std::cout << "KdMedia::StartAiAEnc" << std::endl;
        kd_mapi_aenc_start(0);
        kd_mapi_aenc_bind_ai(ai_handle_, 0);
        ai_initialized = false;
    }
    return 0;
}

int KdMedia::StopAiAEnc() {
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_initialized) {
        kd_mapi_aenc_unbind_ai(ai_handle_, 0);
        kd_mapi_ai_stop(ai_handle_);
        kd_mapi_aenc_stop(0);
        ai_initialized = false;
    }
    return 0;
}

k_s32 KdMedia::AudioEncCallback(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data) {
    KdMedia *thiz = (KdMedia*)p_private_data;
    if (thiz && thiz->on_aenc_data_) {
        thiz->on_aenc_data_->OnAEncData(chn_num, stream_data);
    }
    return 0;
}

int KdMedia::CreateADecAo() {
    k_s32 ret;

    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = _init_ao(KD_AUDIO_BIT_WIDTH_16,audio_sample_rate_,K_STANDARD_MODE,&ao_handle_);
    if(ret != K_SUCCESS) {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    ret = _init_adec(adec_handle_,audio_sample_rate_);
    if(ret != K_SUCCESS) {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    ret = kd_mapi_adec_bind_ao(ao_handle_, adec_handle_);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_bind_ao error: %x\n", ret);
        return -1;
    }
    memset(&audio_stream_, 0, sizeof(audio_stream_));
    return 0;
}

int KdMedia::DestroyADecAo() {
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = kd_mapi_adec_unbind_ao(ao_handle_, adec_handle_);
    if(ret != K_SUCCESS) {
        printf("kd_mapi_adec_unbind_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_ao(ao_handle_);
    if(ret != K_SUCCESS) {
        printf("_deinit_ao error: %x\n", ret);
        return -1;
    }

    ret = _deinit_adec(adec_handle_);
    if(ret != K_SUCCESS) {
        printf("_deinit_adec error: %x\n", ret);
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
    return 0;
}

int KdMedia::StopADecAo() {
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
    audio_stream_.seq ++;
    kd_mapi_adec_send_stream(adec_handle_,&audio_stream_);
    return 0;
}
