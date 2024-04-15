#include <iostream>
#include <atomic>
#include <chrono>
#include <signal.h>
#include <thread>
#include "rtsp_client.h"
#include "media.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
}

class MyRtspClient : public IOnBackChannel, public IOnAudioData, public IOnVideoData, public IOnAEncData, public IRtspClientEvent {
  public:
    MyRtspClient() {}

    // IOnBackChannel
    virtual void OnBackChannelStart() override {
        std::cout << "OnBackChannelStart called" << std::endl;
        media_.StartAiAEnc();
    }
    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_u8*pdata,size_t size,k_u64 time_stamp) override {
        // std::cout << "OnAEncData -- size  = " << stream_data->len << ", timestamp = " << stream_data->time_stamp << std::endl;
        if (started_) {
            rtsp_client_.SendAudioData((const uint8_t*)pdata, size, time_stamp);
        }
    }
    // IOnAudioData, received from the server
    virtual void OnAudioStart() override {
        std::cout << "OnAudioStart called" << std::endl;
        media_.StartADecAo();
    }
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) override {
        if (started_) {
            // std::cout << "OnAudioData -- size  = " << size << ", timestamp = " << timestamp << std::endl;

            // TODO， to handle jitter and control accumulation
            //   gather data to get complete frame(40ms).
            if (audio_data_size == 0) {
                audio_timestamp = timestamp;
            }
            for (size_t i = 0; i < size ;i++) {
                g711_buffer_[audio_data_size++] = data[i];
                if (audio_data_size == 320) {
                    media_.SendAudioData(g711_buffer_, audio_data_size, audio_timestamp);
                    audio_data_size = 0;
                    audio_timestamp = timestamp;
                }
            }
        }
    }
    // IOnVideoData, received from the server
    virtual void OnVideoType(IOnVideoData::VideoType type,  uint8_t *extra_data, size_t extra_data_size) {
        video_type_ = type;
        VdecInitParams params;
        if (type == IOnVideoData::VideoTypeH264) {
            std::cout << "OnVideoType() called, type H264" << std::endl;
            params.type = KdMediaVideoType::kVideoTypeH264;
        }
        if (type == IOnVideoData::VideoTypeH265) {
            std::cout << "OnVideoType() called, type H265" << std::endl;
            params.type = KdMediaVideoType::kVideoTypeH265;
        }
        media_.CreateVdecVo(params);
        media_.StartVDecVo();
        first_video_frame_ = true;
    }
    virtual void OnVideoData(const uint8_t *data, size_t size, uint64_t timestamp, bool keyframe) override {
        // TODO, to handle jitter and control accumulation
        if (started_) {
            if (first_video_frame_ && !keyframe) return;
            first_video_frame_ = false;
            media_.SendVideoData(data, size, timestamp);
        }
    }
    // IRtspClientEvent
    virtual void OnRtspClientEvent(int event) override {
        // At the moment, there is only one event : shutdown
        printf("OnRtspClientEvent -- shutdown\n");
        g_exit_flag.store(true);
    }

    int Init() {
        RtspClientInitParam param;
        param.on_video_data = this;
        param.on_audio_data = this;
        param.on_backchannel = this;
        param.on_event = this;
        if (rtsp_client_.Init(param) < 0) return -1;
        if (media_.Init() < 0) return -1;
        if (media_.CreateAiAEnc(this) < 0) return -1;
        if (media_.CreateADecAo() < 0) return -1;
        return 0;
    }
    int DeInit() {
        Stop();
        media_.DestroyAiAEnc();
        media_.DestroyADecAo();
        media_.DestroyVDecVo();
        media_.Deinit();
        rtsp_client_.DeInit();
        return 0;
    }

    int Start(const char *url) {
        if(started_) return 0;
        rtsp_client_.Open(url);
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        started_ = false;
        rtsp_client_.Close();
        media_.StopAiAEnc();
        media_.StopADecAo();
        media_.StopVDecVo();
        return 0;
    }

  private:
    KdRtspClient rtsp_client_;
    KdMedia media_;
    std::atomic<bool> started_{false};
    uint8_t g711_buffer_[320];
    size_t audio_data_size = 0;
    uint64_t audio_timestamp;
    IOnVideoData::VideoType video_type_{IOnVideoData::VideoTypeInvalid};
    bool first_video_frame_{true};
};

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    if (argc < 2) {
        printf("Usage: ./backchannel_client <rtsp_url> <out_type>\n");
        printf("        out_type: vo type, see vo doc, default 0.\n");
        return 0;
    }
    std::string url = argv[1];
    if (argc > 2) {
        k_connector_type connector_type = (k_connector_type)atoi(argv[2]);
        KdMedia::set_vo_connector_type(connector_type);
    }

    MyRtspClient *client = new MyRtspClient();
    if (!client || client->Init() < 0) {
        std::cout << "KdRtspClient Init failed." << std::endl;
        return -1;
    }
    client->Start(url.c_str());

    while (!g_exit_flag) {
        std::this_thread::sleep_for(100ms);
    }

    client->Stop();
    client->DeInit();
    delete client;
    return 0;
}
