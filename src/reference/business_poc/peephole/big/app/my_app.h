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

#ifndef _MY_APP_
#define _MY_APP_

#include <atomic>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "person_detect.h"
#include "media.h"
#include "util.h"
#include "my_comm_server.h"

enum EventType {
    INVALID,
    PIR_WAKEUP,
    KEY_WAKEUP,
    STAY_ALARM,
    KEY_PRESSED,
    BOTTOM
};

enum TrigerMode {
    PIR_MODE,
    DOORBELL_MODE,
    PLAYBACK_MODE,
    BOTTOM_MODE
};

struct DoorBellEventData {
    EventType event_type;
    k_u64 timestamp_ms;
    k_u64 phyaddr;
    k_u32 len;
};

struct VEncFrameData {
    uint8_t *data{nullptr};
    size_t size = 0;
    int64_t timestamp_ms;  // gettimeofday_ms
    enum class Type : int { INVALID, H264, H265, BOTTOM} type;
    bool keyframe;
};

struct AEncFrameData {
    uint8_t *data{nullptr};
    size_t size = 0;
    int64_t timestamp_ms;
};

class MyApp {
  public:
    MyApp(int max_queue_size)
       : pir_event_queue_(max_queue_size),
         venc_stream_queue_(1000),
         aenc_stream_queue_(800)
        {}

    int Init();
    int DeInit(bool send_msg = false);
    TrigerMode EnterPirMode();
    TrigerMode EnterDoorBellMode();
    int EnterPlaybackMode();

    void ReceiveMessageThread();
    void StartSendEventThread();
    TrigerMode GetCurrentMode();

  private:
    void StartPirSnapThread();
    void StartPersonDetectThread();
    void StartPlayAudioThread();
    void StartVencSnapThread();
    void StartVencThread();
    void StartAencThread();

    void StartVencSendThread();
    void StartAencSendThread();

  private:
    Media *media_ {nullptr};
    MyCommServer *comm_server_ {nullptr};
    std::atomic<bool> client_ready_ {false};
    std::atomic<bool> exit_msg_ {false};
    std::atomic<bool> key_pressed_ {false};
    std::atomic<bool> playback_ {false};
    TrigerMode mode_switch_;

    BufThreadQueue<UserEventData> pir_event_queue_;
    BufThreadQueue<AVEncFrameData> venc_stream_queue_;
    BufThreadQueue<AVEncFrameData> aenc_stream_queue_;

    std::condition_variable play_music_cv_;
    std::mutex play_music_mutex_;
    std::atomic<bool> paly_music_ {false};

    k_u32 detect_width_ {720};
    k_u32 detect_height_ {1280};
    TrigerMode current_mode_ {PIR_MODE};
    personDetect *person_detect_ {nullptr};
    std::thread venc_thread_;
    std::thread person_detect_thread_;
    std::thread receive_message_thread_;
    std::thread play_audio_thread_;
    std::thread play_intercom_audio_thread_;
    std::thread doorbell_snap_thread_;
    std::thread doorbell_venc_thread_;
    std::thread doorbell_aenc_thread_;
    std::thread send_event_thread_;

    std::atomic<bool> venc_senc_exit_ {false};
    std::thread venc_send_thread_;

    std::atomic<bool> aenc_senc_exit_ {false};
    std::thread aenc_send_thread_;

    std::atomic<bool> venc_exit_flag_ {false};
    std::atomic<bool> ai_exit_flag_ {false};
    std::atomic<bool> doorbell_venc_exit_flag_ {false};
    std::atomic<bool> doorbell_aenc_exit_flag_ {false};
    std::atomic<bool> doorbell_palyaudio_exit_flag_ {false};
    std::atomic<bool> recevie_msg_exit_flag_ {false};
    std::atomic<bool> send_event_exit_flag_ {false};
    std::atomic<bool> vicap_started_ {false};
    std::atomic<bool> start_audio_real_stream_ {false};

    std::vector<UserEventData> doorbell_event_vec_;
    std::mutex doorbell_event_mutex_;
    std::vector<AVEncFrameData> doorbell_frame_vec_;
    k_u8 venc_header_[1024];
    k_u8 venc_header_size_;
    std::atomic<bool> get_idr_ {false};

    std::vector<AVEncFrameData> doorbell_audio_vec_;
    std::atomic<bool> send_video_event_data_{false};
    std::atomic<bool> send_record_audio_data_{false};
    std::atomic<bool> first_snap_ {true};
    std::atomic<bool> check_stay_ {false};
    std::atomic<bool> start_detect_ {false};
    std::atomic<bool> doorbell_first_snap_ {true};
    std::string kmodel_path_;
    std::atomic<bool> sys_init_ {false};
};

#endif // _MY_APP_