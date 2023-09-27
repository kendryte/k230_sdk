/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MY_APP_IMPL_H_
#define MY_APP_IMPL_H_

#include <iostream>
#include <atomic>
#include "media.h"
#include "rtsp_client.h"

class PeepholeApp : 
      // bi-direction speech with video 
      public IOnBackChannel, public IOnAudioData, public IOnVideoData, public IOnAEncData, public IRtspClientEvent
{
  public:
    static PeepholeApp *GetInstance() {
        static PeepholeApp instance;
        return &instance;
    }
    
    int Init(int flag = 0);
    void DeInit(int flag = 0);

    int Start(const char *url);
    int Stop();
    int Suspend();
    int Resume();

    int EnablePitchShift();
    int DisablePitchShift();

  private:
    // IOnBackChannel
    virtual void OnBackChannelStart() override;
    // IOnAEncData, data to be sent to the server
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream *stream_data) override;
    // IOnAudioData, received from the server
    virtual void OnAudioStart() override;
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) override;
    // IOnVideoData, received from the server
    virtual void OnVideoType(IOnVideoData::VideoType type,  uint8_t *extra_data, size_t extra_data_size) override;
    virtual void OnVideoData(const uint8_t *data, size_t size, uint64_t timestamp, bool keyframe) override;
    // IRtspClientEvent
    virtual void OnRtspClientEvent(int event) override;

  private:  // emphasize the following members are private
    PeepholeApp() {}
    PeepholeApp(const PeepholeApp &);
    const PeepholeApp &operator=(const PeepholeApp &);

  private:
    KdRtspClient rtsp_client_;
    KdMedia media_;
    std::atomic<bool> started_{false};
    std::atomic<bool> inited_{false};
    uint8_t g711_buffer_[320] = {0};
    size_t audio_data_size = 0;
    uint64_t audio_timestamp;
    uint64_t tmp_timestamp_;
    uint64_t local_audio_timestamp[2] = {0};
    IOnVideoData::VideoType video_type_{IOnVideoData::VideoTypeInvalid};
    std::atomic<bool> rtsp_disconnected_{true};
    std::string rtsp_url_;

    std::mutex mutex_;
    bool first_video_frame_{true};
    bool get_idr_frame_{false};
};

#endif //MY_APP_IMPL_H_