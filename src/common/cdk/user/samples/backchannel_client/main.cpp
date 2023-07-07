#include <iostream>
#include <atomic>
#include <chrono>
#include "rtsp_client.h"
#include "media.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
}

class MyRtspClient : public IOnAudioData, public IOnAEncData, public IRtspClientEvent, public Noncopyable {
  public:
    // IOnAudioData, received from the server
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) override {
        if (started_) {
            // TODO， to handle jitter and control accumulation
            //   gather data to get complete frame(40ms).
            if (audio_data_size == 0) {
                audio_timestamp = timestamp;
            }
            for (size_t i = 0; i < size ;i++) {
                g711_buffer_[audio_data_size++] = data[i];
                if (audio_data_size == 320) {
                    media_.SendData(g711_buffer_, audio_data_size, audio_timestamp);
                    audio_data_size = 0;
                    audio_timestamp = timestamp;
                }
            }
        }
    }

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) override {
        if (started_) {
            rtsp_client_.SendAudioData((const uint8_t*)stream_data->stream, stream_data->len, stream_data->time_stamp);
        }
    }

    // IRtspClientEvent
    virtual void OnRtspClientEvent(int event) override {
        // At the moment, there is only one event : shutdown
        printf("OnRtspClientEvent -- shutdown\n");
        g_exit_flag.store(true);
    }

    int Init() {
        if (rtsp_client_.Init(this, this) < 0) {
            return -1;
        }
        if (media_.Init() < 0) {
            return -1;
        }
        if (media_.CreateAiAEnc(this) < 0) {
            return -1;
        }
        if (media_.CreateADecAo() < 0) {
            return -1;
        }
        return 0;
    }
    int DeInit() {
        Stop();
        media_.Deinit();
        rtsp_client_.DeInit();
        return 0;
    }

    int Start(const char *url) {
        if(started_) return 0;
        media_.StartADecAo();
        rtsp_client_.Open(url);
        media_.StartAiAEnc();
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        rtsp_client_.Close();
        started_ = false;
        media_.StopAiAEnc();
        media_.StopADecAo();
        return 0;
    }

  private:
    KdRtspClient rtsp_client_;
    KdMedia media_;
    std::atomic<bool> started_{false};
    uint8_t g711_buffer_[320];
    size_t audio_data_size = 0;
    uint64_t audio_timestamp;
};

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    if (argc < 2) {
        printf("Usage: ./backchannel_client <rtsp_url> \n");
        return 0;
    }
    std::string url = argv[1];

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
