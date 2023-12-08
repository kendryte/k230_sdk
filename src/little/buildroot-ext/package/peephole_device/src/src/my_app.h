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

#ifndef MY_APP_H_
#define MY_APP_H_
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <thread>
#include "comm/include/comm_rpc_server.h"
#include "comm/include/comm_client.h"
#include "rtsp_server.h"
#include "my_muxer.h"

static uint64_t perf_get_smodecycles(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdcycle %0" : "=r"(cnt)
    );
    return cnt;
}

typedef enum {
  PIR_MODE = 1,
  DOORBELL_MODE,
  MODE_BUTT,
} CurrentMode;

/*
   remote bidirection-speech with video:
   1. audio-input(innner_codec) --> OnAEncFrameData() --> rtsp_server.SendAudioData
   2. OnBackChannelData() --> media.SendAudioData --> audio-output(extern_codec)
   3. video-capture --> OnVEncFrameData() --> rtsp_server.SendVideoData
*/

class MyApp : public IRpcServerCallback, public IClientCallback, public IOnBackChannel, public IJpegMuxerCallback, public IMp4MuxerCallback {
  public:
    static MyApp *GetInstance() {
        static MyApp instance;
        return &instance;
    }

    int Init(unsigned short port = 9000);
    void DeInit();
    void SetRecordDir(const std::string &mp4_dir, const std::string &jpeg_dir) {
        mp4_dir_ = mp4_dir;
        jpeg_dir_ = jpeg_dir;
    }
    void GetRecordDir(std::string &mp4_dir, std::string &jpeg_dir) {
        mp4_dir = mp4_dir_;
        jpeg_dir = jpeg_dir_;
    }
    void EnableRecord(bool enable) {
        enable_mp4_record_.store(enable);
        if (!enable) {
            cmd_stop_record();
        } else {
            cmd_start_record();
        }
    }

    int GetCurrentMode() {
      return current_mode_;
    }

    void SetPowerOff() { request_power_off_.store(true); }
    bool RpcPowerOff() const { return request_power_off_.load(); }

    // comm with the app on rt-smart core
    int EnterPlaybackMode(bool sync = true);
    bool PlaybackEntered() const { return playback_ack_.load();}

    int SendAudioData(const AVEncFrameData &data, IInterruptCallback *cb) {
        return client_.SendAudioData(data, cb);
    }

  private:
    int KeyPressed() { return client_.KeyPressed(); }
    int RpcSendEvent(const RpcEventData &event);

  private:
    // IRpcClientCallback, comm with the app on peephole-phone
    virtual void OnNewClient(int64_t conn_id, const std::string &remote_ip) override;
    virtual void OnDisconnect(int64_t conn_id, const std::string &reason) override;
    virtual void OnGetInfo(RpcServerInfo &info) override;
    virtual void OnRequest(const RpcRequest &req) override;

    // IClientCallback, comm with the app on rt-smart core
    virtual void OnMessage(const UserMessage *message) override;
    virtual void OnEvent(const UserEventData &event) override;
    virtual void OnVEncFrameData(const AVEncFrameData &data) override;
    virtual void OnAEncFrameData(const AVEncFrameData &data) override;

    // IOnBackChannel
    virtual void OnBackChannelData(std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) override;

    // IJpegMuxerCallback
    virtual void GetJpegFilename(UserEventData::EventType type, int64_t timestamp_ms, std::string &filename) override;
    virtual bool IsJpegSpaceAvailable() override;
    virtual void OnJpegMuxerError(int err_code) override;

    // IMp4MuxerCallback
    virtual void GetMp4Filename(std::string &filename) override;
    virtual bool IsMp4SpaceAvailable() override;
    virtual void OnMp4MuxerError(int err_code) override;

  private:
    int CreateRtspServer();
    int DestroyRtspServer();
    int StartRtspServer();
    int StopRtspServer();
    void key_event_handler();
    void cmd_start_record() {
        AVEncFrameData data;
        data.flags = 0x1001;
        mp4_muxer_.Write(data);
    }
    void cmd_stop_record() {
        AVEncFrameData data;
        data.flags = 0x1000;
        data.timestamp_ms = __INT64_MAX__;
        mp4_muxer_.Write(data);
    }

  private:  // emphasize the following members are private
    MyApp() {}
    MyApp(const MyApp &);
    const MyApp &operator=(const MyApp &);

  private:
    RpcServer rpc_server_;
    std::atomic<bool> request_power_off_{false};
    //
    UserCommClient client_;
    std::atomic<bool> playback_ack_{false};
    std::atomic<int> current_mode_ {0};
    //
    KdRtspServer rtsp_server_;
    std::string stream_url_ = "peephole_stream";
    unsigned short stream_port_{8554};
    std::mutex rtsp_mutex_;
    std::atomic<bool> rtsp_started_{false};
    uint8_t g711_buffer_backchannel[320];
    size_t backchannel_data_size = 0;
    uint64_t timestamp_backchanel;
    std::atomic<bool> video_data_realtime_flag_{false};
    uint64_t previous_video_timestamp_ms_{0};
    std::atomic<bool> audio_data_realtime_flag_{false};
    uint64_t previous_audio_timestamp_ms_{0};

    // key-press detection
    std::thread key_detect_thread_;
    std::atomic<bool> key_detect_thread_exit_flag_;

    // for record
    std::string mp4_dir_{"/sharefs/app/peephole_device"};
    std::string jpeg_dir_{"/sharefs/app/peephole_device"};
    MyMp4Muxer mp4_muxer_;
    MyJpegMuxer jpeg_muxer_;
    std::atomic<bool> enable_mp4_record_{false};
    std::atomic<bool> enable_jpeg_recod_ {true};
    struct BackchannelData{
        std::shared_ptr<uint8_t> data{nullptr};
        size_t size{0};
    };
    std::mutex aq_mutex_;
    std::queue<BackchannelData> audio_queue_;
    std::thread sd_thd_;
};

#endif // MY_APP_H_
