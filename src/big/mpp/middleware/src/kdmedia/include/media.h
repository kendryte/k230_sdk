#ifndef _KD_MEDIA_H
#define _KD_MEDIA_H
#include <stddef.h>
#include <stdint.h>
#include <memory>
#include "k_vicap_comm.h"
#include "k_venc_comm.h"
#include "k_connector_comm.h"

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
    int bitrate_kbps = 4000;

    int audio_samplerate = 8000;
    int audio_channel_cnt = 1;
    int pitch_shift_semitones = 0; // [-12, 12]
};

struct VdecInitParams {
    KdMediaVideoType type = KdMediaVideoType::kVideoTypeH265;
    size_t input_buf_size = 1920 * 1088;
    int input_buf_num = 4;
    int max_width = 1920;
    int max_height = 1088;
    int output_buf_num = 6;
};

class IOnAEncData {
  public:
    virtual ~IOnAEncData() {}
    virtual void OnAEncData(k_u32 chn_id, k_u8*pdata,size_t size,k_u64 time_stamp) = 0;
};

class IOnVEncData {
  public:
    virtual ~IOnVEncData() {}
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, k_venc_pack_type type,uint64_t timestamp) = 0;
};


// TODO,
//   At the moment, KdMedia is for bidirection-speech only.
//
class KdMedia {
  public:
    KdMedia();
    ~KdMedia();
    int Init(const KdMediaInputConfig &config);
    int Init();
    int Deinit();

    //for audio capture and encoder
    int CreateAiAEnc(IOnAEncData *on_aenc_data);
    int DestroyAiAEnc();
    int StartAiAEnc();
    int StopAiAEnc();

    //for audio decoder and audio output
    int CreateADecAo();
    int DestroyADecAo();
    int StartADecAo();
    int StopADecAo();
    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms);

    // for video capture and encoder
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

    static void set_vo_connector_type(k_connector_type &connector_type);

  private:
    KdMedia(const KdMedia &) = delete;
    KdMedia& operator=(const KdMedia &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _KD_MEDIA_H
