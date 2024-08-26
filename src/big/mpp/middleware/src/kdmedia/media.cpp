#include "media.h"
#include <fcntl.h>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include "mpi_sys_api.h"
#include "mpi_ai_api.h"
#include "mpi_aenc_api.h"
#include "mpi_ao_api.h"
#include "mpi_adec_api.h"
#include "mpi_vvi_api.h"
#include "mpi_venc_api.h"
#include "mpi_vicap_api.h"
#include "k_vicap_comm.h"
#include "k_vb_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vdec_api.h"
#include "vo_cfg.h"

#define AUDIO_PERSEC_DIV_NUM 25
#define BIND_VO_LAYER 1
#define VI_ALIGN_UP(addr, size) (((addr) + ((size)-1U)) & (~((size)-1U)))

static k_s32 g_mmap_fd_tmp = 0;
static std::mutex mmap_mutex_;

typedef struct
{
    pthread_t output_tid;
    k_u32 venc_chn;
    k_bool is_start;
    k_bool is_end;
    k_s32 frame_cnt;
    k_u32 pic_width;
    k_u32 pic_height;

} venc_output_pthread;

static venc_output_pthread venc_output_arr[VENC_MAX_CHN_NUMS];

class KdMedia::Impl
{
public:
    Impl() {}
    ~Impl() {}
    int Init(const KdMediaInputConfig &config);
    int Init();
    int Deinit();

    int CreateAiAEnc(IOnAEncData *on_aenc_data);
    int DestroyAiAEnc();
    int StartAiAEnc();
    int StopAiAEnc();

    int CreateADecAo();
    int DestroyADecAo();
    int StartADecAo();
    int StopADecAo();
    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms);

    int CreateVcapVEnc(IOnVEncData *on_venc_data);
    int DestroyVcapVEnc();
    int StartVcapVEnc();
    int StopVcapVEnc();

    // for video decoder and render
    int CreateVdecVo(const VdecInitParams &params);
    int DestroyVDecVo();
    int StartVDecVo();
    int StopVDecVo();
    int SendVideoData(const uint8_t *data, size_t size, uint64_t timestamp_ms);

private: // emphasize the following members are private
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

private:
    static void *aenc_chn_get_stream_threads(void *arg);
    k_s32 kd_sample_aenc_start(k_handle aenc_hdl);
    k_s32 kd_sample_aenc_stop(k_handle aenc_hdl);
    k_s32 kd_sample_get_venc_stream(k_u32 venc_chn);
    k_s32 kd_sample_ao_start(k_u32 dev, k_u32 chn);
    k_s32 kd_sample_ao_stop(k_u32 dev, k_u32 chn);

    static void *venc_stream_threads(void *arg);
    k_s32 kd_sample_venc_init(k_u32 chn_num, k_venc_chn_attr *pst_venc_attr);

private:
    int CreateVdecVBPool(const VdecInitParams &params);
    int DestroyVdecVBPool();

private:
    k_u32 audio_sample_rate_{8000}; // for G711
    k_handle ai_handle_{0};
    bool ai_initialized{false};
    bool ai_started_{false};

    k_u32 venc_chn_id_{0};
    pthread_t venc_tid_;
    bool start_get_video_stream_{false};

    k_vicap_dev vi_dev_id_{VICAP_DEV_ID_0};
    k_vicap_chn vi_chn_id_{VICAP_CHN_ID_0};

    k_u32 ai_dev_{0};
    k_u32 ai_chn_{0};
    k_handle aenc_handle_{0};
    k_audio_stream audio_stream_;
    pthread_t get_audio_stream_tid_{0};
    bool start_get_audio_stream_{false};

    k_u32 ao_dev_{0};
    k_u32 ao_chn_{0};
    bool ao_initialized{false};
    k_handle adec_handle_{0};
    IOnAEncData *on_aenc_data_{nullptr};
    std::mutex ai_aenc_mutex_;
    std::mutex adec_ao_mutex_;
    //
    KdMediaInputConfig config_;
    k_vicap_dev_set_info vcap_dev_info_;
    IOnVEncData *on_venc_data_{nullptr};

    //
    VdecInitParams vdec_params_;
    k_u32 input_pool_id_{VB_INVALID_POOLID};
    k_u32 output_pool_id_{VB_INVALID_POOLID};
    int vdec_chn_id_{0};
    std::mutex vdec_vo_mutex_;
    bool vdec_vo_created_{false};
    bool vdec_vo_started_{false};
    bool vdec_data_feed_{false};
    std::atomic<bool> vo_cfg_done_{false};
};

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

static k_vicap_sensor_info sensor_info[VICAP_DEV_ID_MAX];
static k_s32 kd_sample_vicap_set_dev_attr(k_vicap_dev_set_info dev_info)
{
    k_s32 ret = 0;

    /* dev attr */
    if (dev_info.vicap_dev >= VICAP_DEV_ID_MAX || dev_info.vicap_dev < VICAP_DEV_ID_0)
    {
        printf("kd_mpi_vicap_set_dev_attr failed, dev_num %d out of range\n", dev_info.vicap_dev);
        return K_FAILED;
    }

    if (dev_info.sensor_type > SENSOR_TYPE_MAX || dev_info.sensor_type < OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR)
    {
        printf("kd_mpi_vicap_set_dev_attr failed, sensor_type %d out of range\n", dev_info.sensor_type);
        return K_FAILED;
    }

    k_vicap_dev_attr dev_attr;

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&sensor_info[dev_info.vicap_dev], 0, sizeof(k_vicap_sensor_info));

    sensor_info[dev_info.vicap_dev].sensor_type = dev_info.sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info[dev_info.vicap_dev].sensor_type, &sensor_info[dev_info.vicap_dev]);
    if (ret)
    {
        printf("kd_mpi_vicap_get_sensor_info failed:0x%x\n", ret);
        return K_FAILED;
    }
    dev_attr.dw_enable = dev_info.dw_en;

    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = sensor_info[dev_info.vicap_dev].width;
    dev_attr.acq_win.height = sensor_info[dev_info.vicap_dev].height;
    // vicap work mode process
    if ((dev_info.mode == VICAP_WORK_OFFLINE_MODE) || (dev_info.mode == VICAP_WORK_LOAD_IMAGE_MODE) || (dev_info.mode == VICAP_WORK_ONLY_MCM_MODE))
    {
        dev_attr.mode = dev_info.mode;
        dev_attr.buffer_num = dev_info.buffer_num;
        dev_attr.buffer_size = dev_info.buffer_size;
    }

    dev_attr.pipe_ctrl.data = dev_info.pipe_ctrl.data;
    // af need disable
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info[dev_info.vicap_dev], sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_set_dev_attr(dev_info.vicap_dev, dev_attr);
    if (ret)
    {
        printf("kd_mpi_vicap_set_dev_attr failed:0x%x\n", ret);
        return K_FAILED;
    }
    return K_SUCCESS;
}

int KdMedia::Impl::Init(const KdMediaInputConfig &config)
{
    k_s32 ret = 0;
    k_vb_config vb_config;
    memset(&vb_config, 0, sizeof(vb_config));
    vb_config.max_pool_cnt = config.video_valid ? 5 : 2;
    vb_config.comm_pool[0].blk_cnt = 150;
    vb_config.comm_pool[0].blk_size = config.audio_samplerate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[1].blk_cnt = 2;
    vb_config.comm_pool[1].blk_size = config.audio_samplerate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    if (config.video_valid)
    {
        k_u64 pic_size = config.venc_width * config.venc_height * 2;
        k_u64 stream_size = config.venc_width * config.venc_height / 2;
        vb_config.comm_pool[2].blk_cnt = 6;
        vb_config.comm_pool[2].blk_size = ((pic_size + 0xfff) & ~0xfff);
        vb_config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
        vb_config.comm_pool[3].blk_cnt = 30;
        vb_config.comm_pool[3].blk_size = ((stream_size + 0xfff) & ~0xfff);
        vb_config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;

        memset(&vcap_dev_info_, 0, sizeof(vcap_dev_info_));
        if (config.sensor_type <= OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE || config.sensor_type >= SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR)
            vcap_dev_info_.dw_en = K_FALSE;
        else
            vcap_dev_info_.dw_en = K_TRUE;

        if (vcap_dev_info_.dw_en)
        {
            k_vicap_sensor_info sensor_info;
            memset(&sensor_info, 0, sizeof(sensor_info));
            sensor_info.sensor_type = config.sensor_type;
            int ret = kd_mpi_vicap_get_sensor_info(config.sensor_type, &sensor_info);
            if (ret != K_SUCCESS)
            {
                std::cout << "KdMedia::Init() kd_mpi_vicap_get_sensor_info failed, ret = " << ret << std::endl;
                return ret;
            }
            vb_config.comm_pool[4].blk_cnt = 6;
            vb_config.comm_pool[4].blk_size = VI_ALIGN_UP(sensor_info.width * sensor_info.height * 3 / 2, 0x1000);
            vb_config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
        }
    }

    ret = kd_mpi_vb_set_config(&vb_config);
    if (ret)
    {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }
    else
    {
        printf("vb_set_config ok\n");
    }

    ret = kd_mpi_vb_init();

    if (config.video_valid)
    {
        vcap_dev_info_.pipe_ctrl.data = 0xFFFFFFFF;
        vcap_dev_info_.sensor_type = config.sensor_type;
        vcap_dev_info_.vicap_dev = vi_dev_id_;
        ret = kd_sample_vicap_set_dev_attr(vcap_dev_info_);
        if (ret != K_SUCCESS)
        {
            std::cout << "KdMedia::Init() kd_mpi_vicap_set_dev_attr failed, ret = " << ret << std::endl;
            ret = kd_mpi_vb_exit();
            return ret;
        }
    }
    config_ = config;
    return ret;
}

int KdMedia::Impl::Init()
{
    k_s32 ret = 0;
    k_vb_config vb_config;
    memset(&vb_config, 0, sizeof(vb_config));
    vb_config.max_pool_cnt = 2 + 2;
    vb_config.comm_pool[0].blk_cnt = 150;
    vb_config.comm_pool[0].blk_size = 48000 * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[1].blk_cnt = 2;
    vb_config.comm_pool[1].blk_size = 48000 * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    ret = kd_mpi_vb_set_config(&vb_config);
    if (ret)
    {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }
    else
    {
        printf("vb_set_config ok\n");
    }

    ret = kd_mpi_vb_init();

    return ret;
}

int KdMedia::Impl::Deinit()
{
    kd_mpi_vb_exit();
    return 0;
}

int KdMedia::Impl::CreateAiAEnc(IOnAEncData *on_aenc_data)
{
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_initialized)
        return 0;
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = config_.audio_samplerate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (config_.audio_channel_cnt == 1) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = config_.audio_samplerate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    if (K_SUCCESS != kd_mpi_ai_set_pub_attr(ai_dev_, &aio_dev_attr))
    {
        std::cout << "kd_mpi_ai_init failed." << std::endl;
        return -1;
    }

    // for debug pitch-shift
    k_ai_chn_pitch_shift_param ps_param;
    memset(&ps_param, 0, sizeof(ps_param));
    ps_param.semitones = config_.pitch_shift_semitones;
    if (K_SUCCESS != kd_mpi_ai_set_pitch_shift_attr(ai_dev_, ai_chn_, &ps_param))
    {
        std::cout << "kd_mpi_ai_init kd_mpi_ai_set_pitch_shift_attr." << std::endl;
        return -1;
    }

    k_aenc_chn_attr aenc_chn_attr;
    memset(&aenc_chn_attr, 0, sizeof(aenc_chn_attr));
    aenc_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    aenc_chn_attr.point_num_per_frame = config_.audio_samplerate / aenc_chn_attr.buf_size;
    aenc_chn_attr.type = K_PT_G711U;
    if (K_SUCCESS != kd_mpi_aenc_create_chn(aenc_handle_, &aenc_chn_attr))
    {
        std::cout << "kd_mpi_aenc_init failed." << std::endl;
        // kd_mpi_ai_deinit(ai_handle_);
        return -1;
    }

    this->on_aenc_data_ = on_aenc_data;
    ai_initialized = true;
    ai_started_ = false;
    return 0;
}

int KdMedia::Impl::DestroyAiAEnc()
{
    k_s32 ret;
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_)
    {
        std::cout << "KdMedia::DestroyAiAEnc called, FAILED, stop first!!!" << std::endl;
        return -1;
    }
    if (ai_initialized)
    {
        on_aenc_data_ = nullptr;
        ret = kd_mpi_aenc_destroy_chn(aenc_handle_);
        if (ret != K_SUCCESS)
        {
            printf("kd_mpi_aenc_destroy_chn failed:0x%x\n", ret);
        }
        return K_FAILED;
        ai_initialized = false;
    }
    return 0;
}

static k_s32 kd_sample_aenc_bind_ai(k_handle ai_dev, k_handle ai_chn, k_handle aenc_hdl)
{
    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_hdl;

    return kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn);
}

static k_s32 kd_sample_aenc_unbind_ai(k_handle ai_dev, k_handle ai_chn, k_handle aenc_hdl)
{

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;

    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_hdl;

    return kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);
}

void *KdMedia::Impl::aenc_chn_get_stream_threads(void *arg)
{
    KdMedia::Impl *pthis = (KdMedia::Impl *)arg;
    k_handle aenc_hdl = pthis->aenc_handle_;
    k_audio_stream audio_stream;
    k_u8 *pData;

    while (pthis->start_get_audio_stream_)
    {
        if (0 != kd_mpi_aenc_get_stream(aenc_hdl, &audio_stream, 500))
        {
            // printf("kd_mpi_aenc_get_stream failed\n");
            continue;
        }
        else
        {
            if (pthis->on_aenc_data_)
            {
                pData = (k_u8 *)kd_mpi_sys_mmap(audio_stream.phys_addr, audio_stream.len);
                pthis->on_aenc_data_->OnAEncData(aenc_hdl, pData, audio_stream.len, audio_stream.time_stamp);
                kd_mpi_sys_munmap(pData, audio_stream.len);
            }
        }
        kd_mpi_aenc_release_stream(aenc_hdl, &audio_stream);
    }

    return NULL;
}

k_s32 KdMedia::Impl::kd_sample_aenc_start(k_handle aenc_hdl)
{
    if (!start_get_audio_stream_)
    {
        start_get_audio_stream_ = K_TRUE;
    }
    else
    {
        printf("aenc handle:%d already start\n", aenc_hdl);
        return K_FAILED;
    }

    if (get_audio_stream_tid_ != 0)
    {
        start_get_audio_stream_ = K_FALSE;
        pthread_join(get_audio_stream_tid_, NULL);
        get_audio_stream_tid_ = 0;
    }
    pthread_create(&get_audio_stream_tid_, NULL, aenc_chn_get_stream_threads, this);

    return K_SUCCESS;
}

int KdMedia::Impl::StartAiAEnc()
{
    k_s32 ret;
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_)
        return 0;
    ret = kd_mpi_ai_enable(ai_dev_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_ai_enable failed:0x%x\n", ret);
        return K_FAILED;
    }

    ret = kd_mpi_ai_enable_chn(ai_dev_, ai_chn_);
    kd_sample_aenc_bind_ai(ai_dev_, ai_chn_, aenc_handle_);

    kd_sample_aenc_start(aenc_handle_);
    ai_started_ = true;
    return 0;
}

k_s32 KdMedia::Impl::kd_sample_aenc_stop(k_handle aenc_hdl)
{
    if (start_get_audio_stream_)
    {
        start_get_audio_stream_ = K_FALSE;
    }
    else
    {
        printf("aenc handle:%d already stop\n", aenc_hdl);
    }

    if (get_audio_stream_tid_ != 0)
    {
        start_get_audio_stream_ = K_FALSE;
        pthread_join(get_audio_stream_tid_, NULL);
        get_audio_stream_tid_ = 0;
    }

    return K_SUCCESS;
}

int KdMedia::Impl::StopAiAEnc()
{
    k_s32 ret = 0;
    std::unique_lock<std::mutex> lck(ai_aenc_mutex_);
    if (ai_started_)
    {
        kd_sample_aenc_unbind_ai(ai_dev_, ai_chn_, aenc_handle_);
        ret = kd_mpi_ai_disable_chn(ai_dev_, ai_chn_);
        ret = kd_mpi_ai_disable(ai_dev_);

        kd_sample_aenc_stop(aenc_handle_);
        ai_started_ = false;
    }
    return ret;
}

int KdMedia::Impl::CreateADecAo()
{
    k_s32 ret;

    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = config_.audio_samplerate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = config_.audio_samplerate / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    ret = kd_mpi_ao_set_pub_attr(ao_dev_, &aio_dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("_init_ao error: %x\n", ret);
        return -1;
    }

    k_adec_chn_attr adec_chn_attr;
    memset(&adec_chn_attr, 0, sizeof(adec_chn_attr));
    adec_chn_attr.type = K_PT_G711U;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = config_.audio_samplerate / adec_chn_attr.buf_size;
    adec_chn_attr.mode = K_ADEC_MODE_PACK;
    ret = kd_mpi_adec_create_chn(adec_handle_, &adec_chn_attr);
    if (ret != K_SUCCESS)
    {
        printf("_init_adec error: %x\n", ret);
        return -1;
    }

    memset(&audio_stream_, 0, sizeof(audio_stream_));
    return 0;
}

static k_s32 kd_sample_sys_release_vb_block(k_u64 phys_addr, k_u64 blk_size)
{
    k_s32 ret;
    k_vb_blk_handle handle;

    handle = kd_mpi_vb_phyaddr_to_handle(phys_addr);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("kd_mpi_vb_phyaddr_to_handle failed\n");
        return K_FAILED;
    }

    ret = kd_mpi_vb_release_block(handle);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_vb_release_block failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

int KdMedia::Impl::DestroyADecAo()
{
    k_s32 ret;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    ret = kd_mpi_adec_destroy_chn(adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("_deinit_adec error: %x\n", ret);
        return -1;
    }

    if (audio_stream_.len)
    {
        kd_sample_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
        _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
        memset(&audio_stream_, 0, sizeof(audio_stream_));
    }
    return 0;
}

k_s32 KdMedia::Impl::kd_sample_ao_start(k_u32 dev, k_u32 chn)
{
    k_s32 ret;

    if (dev != 0)
    {
        printf("dev value not supported\n");
        return K_FAILED;
    }

    if (chn < 0 || chn > 2)
    {
        printf("chn value not supported\n");
        return K_FAILED;
    }

    ret = kd_mpi_ao_enable(dev);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_ao_enable failed:0x%x\n", ret);
        return K_FAILED;
    }

    if (2 == chn)
    {
        for (int i = 0; i < 2; i++)
        {
            ret = kd_mpi_ao_enable_chn(dev, i);
            if (ret != K_SUCCESS)
            {
                printf("kd_mpi_ao_enable_chn(%d) failed:0x%x\n", i, ret);
                return K_FAILED;
            }
        }
    }
    else
    {
        ret = kd_mpi_ao_enable_chn(dev, chn);
        if (ret != K_SUCCESS)
        {
            printf("kd_mpi_ao_enable_chn failed:0x%x\n", ret);
            return K_FAILED;
        }
    }

    return ret;
}

static k_s32 kd_sample_adec_bind_ao(k_u32 ao_dev, k_u32 ao_chn, k_handle adec_hdl)
{
    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_hdl;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_chn;

    return kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);
}

static k_s32 kd_sample_adec_unbind_ao(k_u32 ao_dev, k_u32 ao_chn, k_handle adec_hdl)
{
    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;

    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_hdl;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_chn;

    return kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);
}

int KdMedia::Impl::StartADecAo()
{
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret;
    ret = kd_sample_ao_start(ao_dev_, ao_chn_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_ao_start failed\n");
        return -1;
    }

    ret = kd_sample_adec_bind_ao(ao_dev_, ao_chn_, adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_adec_bind_ao error: %x\n", ret);
        return -1;
    }
    return 0;
}

k_s32 KdMedia::Impl::kd_sample_ao_stop(k_u32 dev, k_u32 chn)
{
    k_s32 ret;

    if (dev != 0)
    {
        printf("dev value not supported\n");
        return K_FAILED;
    }

    if (chn < 0 || chn > 2)
    {
        printf("chn value not supported\n");
        return K_FAILED;
    }

    if (2 == chn)
    {
        for (int i = 0; i < 2; i++)
        {
            ret = kd_mpi_ao_disable_chn(dev, i);
            if (ret != K_SUCCESS)
            {
                printf("kd_mpi_ao_disable_chn(%d) failed:0x%x\n", i, ret);
                return K_FAILED;
            }
        }
    }
    else
    {
        ret = kd_mpi_ao_disable_chn(dev, chn);
        if (ret != K_SUCCESS)
        {
            printf("kd_mpi_ao_disable_chn failed:0x%x\n", ret);
            return K_FAILED;
        }
    }

    ret = kd_mpi_ao_disable(dev);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_ai_disable failed:0x%x\n", ret);
        return K_FAILED;
    }

    return ret;
}

int KdMedia::Impl::StopADecAo()
{
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    k_s32 ret = kd_sample_adec_unbind_ao(ao_dev_, ao_chn_, adec_handle_);
    if (ret != K_SUCCESS)
    {
        printf("kd_sample_adec_unbind_ao error: %x\n", ret);
        return -1;
    }

    kd_sample_ao_stop(ao_dev_, ao_chn_);
    return 0;
}

static k_s32 kd_sample_sys_get_vb_block(k_u32 *pool_id, k_u64 *phys_addr, k_u64 blk_size, const char *mmz_name)
{
    k_vb_blk_handle handle;
    k_u64 get_phys_addr;
    k_u32 get_pool_id;

    handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, blk_size, mmz_name);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("kd_mpi_vb_get_block get failed\n");
        return K_FAILED;
    }

    get_phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (get_phys_addr == 0)
    {
        printf("kd_mpi_vb_handle_to_phyaddr failed\n");
        return K_FAILED;
    }

    get_pool_id = kd_mpi_vb_handle_to_pool_id(handle);
    if (get_pool_id == VB_INVALID_POOLID)
    {
        printf("kd_mpi_vb_handle_to_pool_id failed\n");
        return K_FAILED;
    }
#if 0
    get_virt_addr = kd_mpi_sys_mmap(get_phys_addr, blk_size);
#endif
    *phys_addr = get_phys_addr;
    *pool_id = get_pool_id;

    return K_SUCCESS;
}

int KdMedia::Impl::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    k_s32 ret = 0;
    std::unique_lock<std::mutex> lck(adec_ao_mutex_);
    if (audio_stream_.len != size)
    {
        if (audio_stream_.len)
        {
            kd_sample_sys_release_vb_block(audio_stream_.phys_addr, audio_stream_.len);
            _sys_munmap(audio_stream_.phys_addr, audio_stream_.stream, audio_stream_.len);
            memset(&audio_stream_, 0, sizeof(audio_stream_));
        }
        k_u32 pool_id;
        ret = kd_sample_sys_get_vb_block(&pool_id, &audio_stream_.phys_addr, size, NULL);
        // check result, TODO
        audio_stream_.stream = _sys_mmap(audio_stream_.phys_addr, size);
        audio_stream_.len = size;
    }

    memcpy(audio_stream_.stream, data, size);
    audio_stream_.time_stamp = timestamp_ms;
    audio_stream_.seq++;

    kd_mpi_adec_send_stream(adec_handle_, &audio_stream_, K_TRUE);
    return ret;
}

k_s32 KdMedia::Impl::kd_sample_venc_init(k_u32 chn_num, k_venc_chn_attr *pst_venc_attr)
{
    printf("kd_mpi_venc_init start %d\n", chn_num);
    memset(&venc_output_arr[chn_num], 0, sizeof(venc_output_pthread));

    k_s32 ret;
    venc_output_arr[chn_num].pic_width = pst_venc_attr->venc_attr.pic_width;
    venc_output_arr[chn_num].pic_height = pst_venc_attr->venc_attr.pic_height;
    printf("venc[%d] %d*%d size:%d cnt:%d srcfps:%d dstfps:%d rate:%d rc_mode:%d type:%d profile:%d\n", chn_num,
           pst_venc_attr->venc_attr.pic_width,
           pst_venc_attr->venc_attr.pic_height,
           pst_venc_attr->venc_attr.stream_buf_size,
           pst_venc_attr->venc_attr.stream_buf_cnt,
           pst_venc_attr->rc_attr.cbr.src_frame_rate,
           pst_venc_attr->rc_attr.cbr.dst_frame_rate,
           pst_venc_attr->rc_attr.cbr.bit_rate,
           (int)pst_venc_attr->rc_attr.rc_mode,
           (int)pst_venc_attr->venc_attr.type,
           (int)pst_venc_attr->venc_attr.profile);

    ret = kd_mpi_venc_create_chn(chn_num, pst_venc_attr);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_venc_create_chn failed:0x%x\n", ret);
        return K_FAILED;
    }

    printf("kd_mpi_venc_init end \n");
    return K_SUCCESS;
}

int KdMedia::Impl::CreateVdecVBPool(const VdecInitParams &params)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = params.input_buf_num;
    pool_config.blk_size = params.input_buf_size;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    input_pool_id_ = kd_mpi_vb_create_pool(&pool_config);
    if (input_pool_id_ == VB_INVALID_POOLID)
    {
        return -1;
    }
    printf("input_pool_id = %llu\n", input_pool_id_);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = params.output_buf_num;
    pool_config.blk_size = params.max_width * params.max_height * 2;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    output_pool_id_ = kd_mpi_vb_create_pool(&pool_config);
    if (output_pool_id_ == VB_INVALID_POOLID)
    {
        kd_mpi_vb_destory_pool(input_pool_id_);
        return -1;
    }
    printf("output_pool_id = %llu\n", output_pool_id_);
    return 0;
}

int KdMedia::Impl::DestroyVdecVBPool()
{
    k_s32 ret;
    ret = kd_mpi_vb_destory_pool(input_pool_id_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_vb_destory_pool %d failed(input), ret = %d\n", input_pool_id_, ret);
    }
    ret = kd_mpi_vb_destory_pool(output_pool_id_);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_vb_destory_pool %d failed(output), ret = %d\n", output_pool_id_, ret);
    }
    return 0;
}

static k_s32 kd_sample_vicap_set_chn_attr(k_vicap_chn_set_info chn_info)
{
    k_s32 ret = 0;
    if (chn_info.vicap_dev >= VICAP_DEV_ID_MAX || chn_info.vicap_dev < VICAP_DEV_ID_0)
    {
        printf("kd_mpi_vicap_set_dev_attr failed, dev_num %d out of range\n", chn_info.vicap_dev);
        return K_FAILED;
    }

    if (chn_info.vicap_chn >= VICAP_CHN_ID_MAX || chn_info.vicap_chn < VICAP_CHN_ID_0)
    {
        printf("kd_mpi_vicap_set_attr failed, chn_num %d out of range\n", chn_info.vicap_chn);
        return K_FAILED;
    }

    k_vicap_chn_attr chn_attr;
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    /* chn attr */
    if (!chn_info.out_height && chn_info.out_width)
    {
        printf("kd_mpi_vicap_set_attr, failed, out_width: %d, out_height: %d error\n", chn_info.out_width, chn_info.out_height);
        return K_FAILED;
    }

    if (chn_info.pixel_format > PIXEL_FORMAT_BUTT || chn_info.pixel_format < PIXEL_FORMAT_RGB_444)
    {
        printf("kd_mpi_vicap_set_attr, failed, pixel_formatr: %d out of range\n", chn_info.pixel_format);
        return K_FAILED;
    }

    if (chn_info.pixel_format == PIXEL_FORMAT_RGB_BAYER_10BPP || chn_info.pixel_format == PIXEL_FORMAT_RGB_BAYER_12BPP)
    {
        chn_attr.out_win.h_start = 0;
        chn_attr.out_win.v_start = 0;
        chn_attr.out_win.width = sensor_info[chn_info.vicap_dev].width;
        chn_attr.out_win.height = sensor_info[chn_info.vicap_dev].height;
    }
    else
    {
        chn_attr.out_win.width = chn_info.out_width;
        chn_attr.out_win.height = chn_info.out_height;
    }

    if (chn_info.crop_en)
    {
        chn_attr.crop_win.h_start = chn_info.crop_h_start;
        chn_attr.crop_win.v_start = chn_info.crop_v_start;
        chn_attr.crop_win.width = chn_info.out_width;
        chn_attr.crop_win.height = chn_info.out_height;
    }
    else
    {
        chn_attr.crop_win.h_start = 0;
        chn_attr.crop_win.v_start = 0;
        chn_attr.crop_win.width = sensor_info[chn_info.vicap_dev].width;
        chn_attr.crop_win.height = sensor_info[chn_info.vicap_dev].height;
    }

    chn_attr.scale_win.h_start = 0;
    chn_attr.scale_win.v_start = 0;
    chn_attr.scale_win.width = sensor_info[chn_info.vicap_dev].width;
    chn_attr.scale_win.height = sensor_info[chn_info.vicap_dev].height;

    chn_attr.crop_enable = chn_info.crop_en;
    chn_attr.scale_enable = chn_info.scale_en;
    chn_attr.chn_enable = chn_info.chn_en;
    chn_attr.pix_format = chn_info.pixel_format;
    chn_attr.buffer_num = chn_info.buffer_num;
    chn_attr.buffer_size = chn_info.buf_size;
    chn_attr.alignment = chn_info.alignment;
    chn_attr.fps = chn_info.fps;
    ret = kd_mpi_vicap_set_chn_attr(chn_info.vicap_dev, chn_info.vicap_chn, chn_attr);
    if (ret)
    {
        printf("kd_mpi_vicap_set_chn_attr failed:0x%x\n", ret);
        return K_FAILED;
    }
    return K_SUCCESS;
}

int KdMedia::Impl::CreateVcapVEnc(IOnVEncData *on_venc_data)
{
    if (!config_.video_valid)
    {
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
        config_.sensor_type == OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE)
    {
        chn_attr.rc_attr.cbr.src_frame_rate = 15;
        chn_attr.rc_attr.cbr.dst_frame_rate = 15;
    }
    else if (config_.sensor_type == OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE)
    {
        chn_attr.rc_attr.cbr.src_frame_rate = 60;
        chn_attr.rc_attr.cbr.dst_frame_rate = 60;
    }
    chn_attr.rc_attr.cbr.bit_rate = config_.bitrate_kbps;
    if (config_.video_type == KdMediaVideoType::kVideoTypeH264)
    {
        chn_attr.venc_attr.type = K_PT_H264;
        chn_attr.venc_attr.profile = VENC_PROFILE_H264_HIGH;
    }
    else if (config_.video_type == KdMediaVideoType::kVideoTypeH265)
    {
        chn_attr.venc_attr.type = K_PT_H265;
        chn_attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    }
    else if (config_.video_type == KdMediaVideoType::kVideoTypeMjpeg)
    {
        chn_attr.venc_attr.type = K_PT_JPEG;
        chn_attr.rc_attr.rc_mode = K_VENC_RC_MODE_MJPEG_FIXQP;
        chn_attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
        chn_attr.rc_attr.mjpeg_fixqp.q_factor = 45;
    }

    int ret = kd_sample_venc_init(venc_chn_id_, &chn_attr);
    if (ret != K_SUCCESS)
    {
        std::cout << "kd_sample_venc_init error." << std::endl;
        return -1;
    }

    if (config_.video_type != KdMediaVideoType::kVideoTypeMjpeg)
    {
        ret = kd_mpi_venc_enable_idr(venc_chn_id_, K_TRUE);
        if (ret != K_SUCCESS)
        {
            std::cout << "kd_mpi_venc_enable_idr error." << std::endl;
            return -1;
        }
    }

    k_vicap_sensor_info sensor_info;
    memset(&sensor_info, 0, sizeof(sensor_info));
    sensor_info.sensor_type = config_.sensor_type;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    if (ret != K_SUCCESS)
    {
        printf("kd_mpi_vicap_get_sensor_info failed, %x.\n", ret);
        return -1;
    }

    k_vicap_chn_set_info vi_chn_attr_info;
    memset(&vi_chn_attr_info, 0, sizeof(vi_chn_attr_info));

    vi_chn_attr_info.crop_en = K_FALSE;
    vi_chn_attr_info.scale_en = K_FALSE;
    vi_chn_attr_info.chn_en = K_TRUE;
    vi_chn_attr_info.crop_h_start = 0;
    vi_chn_attr_info.crop_v_start = 0;
    vi_chn_attr_info.out_width = VI_ALIGN_UP(config_.venc_width, 16);
    vi_chn_attr_info.out_height = config_.venc_height;
    vi_chn_attr_info.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    vi_chn_attr_info.vicap_dev = vi_dev_id_;
    vi_chn_attr_info.buffer_num = 6;
    vi_chn_attr_info.alignment = 12;
    vi_chn_attr_info.vicap_chn = (k_vicap_chn)venc_chn_id_;
    if (!vcap_dev_info_.dw_en)
        vi_chn_attr_info.buf_size = VI_ALIGN_UP(VI_ALIGN_UP(config_.venc_width, 16) * config_.venc_height * 3 / 2, 0x100);
    else
        vi_chn_attr_info.buf_size = VI_ALIGN_UP(VI_ALIGN_UP(config_.venc_width, 16) * config_.venc_height * 3 / 2, 0x400);
    ret = kd_sample_vicap_set_chn_attr(vi_chn_attr_info);
    if (ret != K_SUCCESS)
    {
        printf("vicap chn %d set attr failed, %x.\n", venc_chn_id_, ret);
        return -1;
    }

    return 0;
}

int KdMedia::Impl::DestroyVcapVEnc()
{
    if (!config_.video_valid)
    {
        return 0;
    }
    kd_mpi_venc_destroy_chn(venc_chn_id_);
    return 0;
}

static k_s32 kd_sample_venc_bind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        printf("kd_mpi_venc_bind_vi chn_num:%d error\n", chn_num);
        return -1;
    }

    k_mpp_chn vi_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = src_dev;
    vi_mpp_chn.chn_id = src_chn;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_num;
    k_s32 ret = kd_mpi_sys_bind(&vi_mpp_chn, &venc_mpp_chn);

    return ret;
}

static k_s32 kd_sample_vicap_start(k_vicap_dev vicap_dev)
{
    k_s32 ret = 0;
    if (vicap_dev > VICAP_DEV_ID_MAX || vicap_dev < VICAP_DEV_ID_0)
    {
        printf("kd_mpi_vicap_start failed, dev_num %d out of range\n", vicap_dev);
        return K_FAILED;
    }
    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret)
    {
        printf("kd_mpi_vicap_init, vicap dev(%d) init failed.\n", vicap_dev);
        kd_mpi_vicap_deinit(vicap_dev);
        return K_FAILED;
    }

    ret = kd_mpi_vicap_start_stream(vicap_dev);
    if (ret)
    {
        printf("kd_mpi_vicap_start_stream, vicap dev(%d) start stream failed.\n", vicap_dev);
        kd_mpi_vicap_deinit(vicap_dev);
        kd_mpi_vicap_stop_stream(vicap_dev);
        return K_FAILED;
    }
    return K_SUCCESS;
}

k_s32 KdMedia::Impl::kd_sample_get_venc_stream(k_u32 venc_chn)
{
    k_s32 ret = 0;
    k_venc_stream output;
    k_venc_chn_status status;
    k_u8 *pData;
    while (start_get_video_stream_)
    {
        ret = kd_mpi_venc_query_status(venc_chn, &status);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        output.pack = (k_venc_pack *)malloc(sizeof(k_venc_pack) * output.pack_cnt);

        ret = kd_mpi_venc_get_stream(venc_chn, &output, -1);

        for (int i = 0; i < output.pack_cnt; i++)
        {
            pData = (k_u8 *)kd_mpi_sys_mmap(output.pack[i].phys_addr, output.pack[i].len);

            if (on_venc_data_ != nullptr)
            {
                on_venc_data_->OnVEncData(venc_chn, pData, output.pack[i].len, output.pack[i].type, output.pack[i].pts);
            }
            // printf("=====vend data size:%d\n",output.pack[i].len);

            kd_mpi_sys_munmap(pData, output.pack[i].len);
        }

        ret = kd_mpi_venc_release_stream(venc_chn, &output);

        free(output.pack);
    }
    return ret;
}

void *KdMedia::Impl::venc_stream_threads(void *arg)
{
    KdMedia::Impl *pthis = (KdMedia::Impl *)arg;
    pthis->kd_sample_get_venc_stream(pthis->venc_chn_id_);
    return NULL;
}

int KdMedia::Impl::StartVcapVEnc()
{
    if (!config_.video_valid)
    {
        return 0;
    }

    kd_mpi_venc_start_chn(venc_chn_id_);
    if (!start_get_video_stream_)
    {
        start_get_video_stream_ = true;
        pthread_create(&venc_tid_, NULL, venc_stream_threads, this);
    }
    kd_sample_venc_bind_vi(vi_dev_id_, vi_chn_id_, venc_chn_id_);
    kd_sample_vicap_start(vi_dev_id_);
    return 0;
}

static k_s32 kd_sample_venc_unbind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = src_dev;
    vi_mpp_chn.chn_id = src_chn;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_num;
    k_s32 ret = kd_mpi_sys_unbind(&vi_mpp_chn, &venc_mpp_chn);

    return ret;
}

static k_s32 kd_sample_vicap_stop(k_vicap_dev vicap_dev)
{
    k_s32 ret = 0;
    if (vicap_dev > VICAP_DEV_ID_MAX || vicap_dev < VICAP_DEV_ID_0)
    {
        printf("kd_mpi_vicap_stop failed, dev_num %d out of range\n", vicap_dev);
        return K_FAILED;
    }
    ret = kd_mpi_vicap_stop_stream(vicap_dev);
    if (ret)
    {
        printf("kd_mpi_vicap_stop_stream failed\n");
    }
    ret = kd_mpi_vicap_deinit(vicap_dev);
    if (ret)
    {
        printf("kd_mpi_vicap_deinit failed\n");
        return K_FAILED;
    }
    return K_SUCCESS;
}

int KdMedia::Impl::StopVcapVEnc()
{
    if (!config_.video_valid)
    {
        return 0;
    }

    if (start_get_video_stream_)
    {
        start_get_video_stream_ = false;
        pthread_join(venc_tid_, NULL);
    }
    // unbind all
    kd_sample_venc_unbind_vi(vi_dev_id_, vi_chn_id_, venc_chn_id_);
    // stop vcap
    kd_sample_vicap_stop(vi_dev_id_);
    // stop all encoders
    kd_mpi_venc_stop_chn(vi_chn_id_);
    return 0;
}

int KdMedia::Impl::CreateVdecVo(const VdecInitParams &params)
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (vdec_vo_created_)
        return 0;
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
    attr.frame_buf_size = params.max_width * params.max_height * 2;
    attr.stream_buf_size = params.input_buf_size;
    attr.frame_buf_pool_id = output_pool_id_;
    switch (params.type)
    {
    case KdMediaVideoType::kVideoTypeH264:
        attr.type = K_PT_H264;
        break;
    case KdMediaVideoType::kVideoTypeH265:
        attr.type = K_PT_H265;
        break;
    default:
        goto err_exit;
    }

    ret = kd_mpi_vdec_create_chn(vdec_chn_id_, &attr);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::CreateVdecVo() : kd_mpi_vdec_init failed, ret = %d\n", ret);
        goto err_exit;
    }

    // FIXME
    vo_init();
    vo_enable();

    vdec_params_ = params;
    vdec_vo_created_ = true;
    vdec_vo_started_ = false;
    return 0;
err_exit:
    DestroyVdecVBPool();
    return -1;
}

int KdMedia::Impl::DestroyVDecVo()
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (!vdec_vo_created_)
        return 0;
    if (vdec_vo_started_)
    {
        printf("KdMedia::DestroyVDecVo() : stop first!!!\n");
        return -1;
    }
    kd_mpi_vdec_destroy_chn(vdec_chn_id_);
    vo_layer_deinit();
    vo_deinit();
    DestroyVdecVBPool();

    vdec_vo_created_ = false;
    vdec_vo_started_ = false;
    return 0;
}

static k_s32 kd_sample_vdec_bind_vo(k_u32 chn_num, k_u32 vo_dev, k_u32 vo_chn)
{
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = vo_dev;
    vvo_mpp_chn.chn_id = vo_chn;
    ret = kd_mpi_sys_bind(&vdec_mpp_chn, &vvo_mpp_chn);
    if (ret)
    {
        printf("kd_mpi_sys_bind failed:0x%x\n", ret);
    }
    return ret;
}

static k_s32 kd_sample_vdec_unbind_vo(k_u32 chn_num, k_u32 vo_dev, k_u32 vo_chn)
{
    k_mpp_chn vdec_mpp_chn;
    k_mpp_chn vvo_mpp_chn;
    k_s32 ret;

    vdec_mpp_chn.mod_id = K_ID_VDEC;
    vdec_mpp_chn.dev_id = 0;
    vdec_mpp_chn.chn_id = chn_num;
    vvo_mpp_chn.mod_id = K_ID_VO;
    vvo_mpp_chn.dev_id = vo_dev;
    vvo_mpp_chn.chn_id = vo_chn;

    ret = kd_mpi_sys_unbind(&vdec_mpp_chn, &vvo_mpp_chn);
    return ret;
}

int KdMedia::Impl::StartVDecVo()
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (vdec_vo_started_)
    {
        return 0;
    }
    k_s32 ret = kd_mpi_vdec_start_chn(vdec_chn_id_);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::StartVDecVo() : kd_mpi_vdec_start_chn failed, ret = %d\n", ret);
        return -1;
    }
    kd_sample_vdec_bind_vo(vdec_chn_id_, 0, BIND_VO_LAYER);
    vdec_data_feed_ = false;
    vdec_vo_started_ = true;
    return 0;
}

int KdMedia::Impl::StopVDecVo()
{
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
            k_s32 ret = kd_mpi_vdec_query_status(vdec_chn_id_, &status);
            if (ret != K_SUCCESS)
            {
                printf("KdMedia::StopVDecVo() : kd_mpi_vdec_query_status failed, ret = %d\n", ret);
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
    kd_sample_vdec_unbind_vo(vdec_chn_id_, 0, BIND_VO_LAYER);
    kd_mpi_vdec_stop_chn(vdec_chn_id_);
    vdec_vo_started_ = false;
    return 0;
}

static k_s32 kd_sample_sys_get_vb_block_from_pool_id(k_u32 pool_id, k_u64 *phys_addr, k_u64 blk_size, const char *mmz_name)
{
    k_vb_blk_handle handle;
    k_u64 get_phys_addr;
    k_u32 get_pool_id;

    handle = kd_mpi_vb_get_block(pool_id, blk_size, mmz_name);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("kd_mpi_vb_get_block get failed\n");
        return K_FAILED;
    }

    get_phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (get_phys_addr == 0)
    {
        printf("kd_mpi_vb_handle_to_phyaddr failed\n");
        return K_FAILED;
    }

    *phys_addr = get_phys_addr;

    return K_SUCCESS;
}

int KdMedia::Impl::SendVideoData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    std::unique_lock<std::mutex> lck(vdec_vo_mutex_);
    if (!vdec_vo_started_)
        return 0;

    k_vdec_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.len = size;
    stream.pts = timestamp_ms;
    stream.end_of_stream = (data && size) ? K_FALSE : K_TRUE;

    int count = 10;
    while (1)
    {
        k_s32 ret = kd_sample_sys_get_vb_block_from_pool_id(input_pool_id_, &stream.phy_addr, vdec_params_.input_buf_size, NULL);
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
        kd_sample_sys_release_vb_block(stream.phy_addr, vdec_params_.input_buf_size);
        return -1;
    }
    if (data && size)
        memcpy(virt_addr, data, size);

    k_s32 ret = kd_mpi_vdec_send_stream(vdec_chn_id_, &stream, -1);
    _sys_munmap(stream.phy_addr, virt_addr, vdec_params_.input_buf_size);
    kd_sample_sys_release_vb_block(stream.phy_addr, vdec_params_.input_buf_size);
    if (ret != K_SUCCESS)
    {
        printf("KdMedia::SendVideoData() : kd_mpi_vdec_send_stream failed, ret = %d\n", ret);
        return -1;
    }
    if (data && size)
        vdec_data_feed_ = true;

    // init vo layer
    if (!vo_cfg_done_)
    {
        k_vdec_chn_status status;
        k_s32 ret = kd_mpi_vdec_query_status(vdec_chn_id_, &status);
        if (ret == K_SUCCESS && status.width != 0 && status.height != 0)
        {
            vo_layer_init(status.width, status.height);
            vo_cfg_done_.store(true);
        }
    }

    return 0;
}

KdMedia::KdMedia() : impl_(std::make_unique<Impl>()) {}
KdMedia::~KdMedia() {}

int KdMedia::Init(const KdMediaInputConfig &config)
{
    return impl_->Init(config);
}
int KdMedia::Init()
{
    return impl_->Init();
}
int KdMedia::Deinit()
{
    return impl_->Deinit();
}

int KdMedia::CreateAiAEnc(IOnAEncData *on_aenc_data)
{
    return impl_->CreateAiAEnc(on_aenc_data);
}
int KdMedia::DestroyAiAEnc()
{
    return impl_->DestroyAiAEnc();
}
int KdMedia::StartAiAEnc()
{
    return impl_->StartAiAEnc();
}
int KdMedia::StopAiAEnc()
{
    return impl_->StopAiAEnc();
}

int KdMedia::CreateADecAo()
{
    return impl_->CreateADecAo();
}
int KdMedia::DestroyADecAo()
{
    return impl_->DestroyADecAo();
}
int KdMedia::StartADecAo()
{
    return impl_->StartADecAo();
}
int KdMedia::StopADecAo()
{
    return impl_->StopADecAo();
}
int KdMedia::SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    return impl_->SendAudioData(data, size, timestamp_ms);
}

int KdMedia::CreateVcapVEnc(IOnVEncData *on_venc_data)
{
    return impl_->CreateVcapVEnc(on_venc_data);
}
int KdMedia::DestroyVcapVEnc()
{
    return impl_->DestroyVcapVEnc();
}
int KdMedia::StartVcapVEnc()
{
    return impl_->StartVcapVEnc();
}
int KdMedia::StopVcapVEnc()
{
    return impl_->StopVcapVEnc();
}

// for video decoder and render
int KdMedia::CreateVdecVo(const VdecInitParams &params)
{
    return impl_->CreateVdecVo(params);
}
int KdMedia::DestroyVDecVo()
{
    return impl_->DestroyVDecVo();
}
int KdMedia::StartVDecVo()
{
    return impl_->StartVDecVo();
}
int KdMedia::StopVDecVo()
{
    return impl_->StopVDecVo();
}
int KdMedia::SendVideoData(const uint8_t *data, size_t size, uint64_t timestamp_ms)
{
    return impl_->SendVideoData(data, size, timestamp_ms);
}

void KdMedia::set_vo_connector_type(k_connector_type &connector_type)
{
    set_connector_type(connector_type);
}