#ifndef _KD_MEDIA_H
#define _KD_MEDIA_H

#include <unistd.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include "mapi_sys_api.h"
#include "mapi_ai_api.h"
#include "mapi_aenc_api.h"
#include "mapi_ao_api.h"
#include "mapi_adec_api.h"

class IOnAEncData {
  public:
    virtual ~IOnAEncData() {}
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) = 0;
};

enum class KdMediaVideoType {
    TypeInvalid,
    TypeH264,
    TypeH265
};

struct VdecInitParams {
    KdMediaVideoType type = KdMediaVideoType::TypeH265;
    size_t input_buf_size = 1920 * 1088;
    int input_buf_num = 4;
    int max_width = 1920;
    int max_height = 1088;
    int output_buf_num = 6;
};

// TODO,
//   At the moment, KdMedia is for bidirection-speech only.
//
class KdMedia {
  public:
    KdMedia() {}
    int Init();
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
    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp_ms);

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
};

#endif // _KD_MEDIA_H
