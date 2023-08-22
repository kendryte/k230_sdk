#ifndef _KD_MEDIA_H
#define _KD_MEDIA_H

#include <unistd.h>
#include <iostream>
#include <memory>
#include <mutex>
#include "mapi_sys_api.h"
#include "mapi_ai_api.h"
#include "mapi_aenc_api.h"
#include "mapi_ao_api.h"
#include "mapi_adec_api.h"
#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

enum class KdMediaVideoType {
    kVideoTypeH264,
    kVideoTypeH265,
    kVideoTypeMjpeg,
    kVideoTypeButt
};

struct KdMediaInputConfig {
    bool video_valid = false;
    k_vicap_sensor_type sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    int sensor_num = 1;
    KdMediaVideoType video_type = KdMediaVideoType::kVideoTypeH265;
    int venc_width = 1280;
    int venc_height = 720;
    int bitrate_kbps = 2000;
    int pitch_shift_semitones = 0; // [-12, 12]
};

class IOnAEncData {
  public:
    virtual ~IOnAEncData() {}
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) = 0;
};

class IOnVEncData {
  public:
    virtual ~IOnVEncData() {}
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, uint64_t timestamp) = 0;
};


// TODO,
//   At the moment, KdMedia is for bidirection-speech only.
//
class KdMedia {
  public:
    KdMedia() {}
    int Init(const KdMediaInputConfig &config);
    int Deinit();

    int CreateAiAEnc(IOnAEncData *on_aenc_data);
    int DestroyAiAEnc();
    int StartAiAEnc();
    int StopAiAEnc();
    static k_s32 AudioEncCallback(k_u32 chn_num, k_audio_stream* stream_data, void* p_private_data);

    int CreateADecAo();
    int DestroyADecAo();
    int StartADecAo();
    int StopADecAo();
    int SendData(const uint8_t *data, size_t size, uint64_t timestamp_ms);

    int CreateVcapVEnc(IOnVEncData *on_venc_data);
    int DestroyVcapVEnc();
    int StartVcapVEnc();
    int StopVcapVEnc();
    static k_s32 VideoEncCallback(k_u32 chn_num, kd_venc_data_s* stream_data, k_u8* p_private_data);

  private:  // emphasize the following members are private
    KdMedia( const KdMedia& );
    const KdMedia& operator=( const KdMedia& );

  private:
    k_u32 audio_sample_rate_{8000}; // for G711
    k_handle ai_handle_{0};
    bool ai_initialized {false};
    bool ai_started_ {false};
    k_handle aenc_handle_{0};
    k_handle ao_handle_{0};
    bool ao_initialized {false};
    k_handle adec_handle_{0};
    IOnAEncData *on_aenc_data_ {nullptr};
    k_audio_stream audio_stream_;
    std::mutex ai_aenc_mutex_;
    std::mutex adec_ao_mutex_;
    //
    KdMediaInputConfig config_;
    k_vicap_dev_set_info vcap_dev_info_;
    IOnVEncData *on_venc_data_ {nullptr};
};

#endif // _KD_MEDIA_H
