#include <iostream>
#include <atomic>
#include <chrono>
#include "rtsp_server.h"
#include "media.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

class MyRtspServer : public IOnBackChannel, public IOnAEncData, public Noncopyable {
  public:
    // IOnBackChannel
    virtual void OnBackChannelData(std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) override {
        if (started_) {
            //  TODO， need to queue data to handle jitter and control accumulation
            //   gather data to get complete frame(40ms).
            if (backchannel_data_size == 0) {
                timestamp_backchanel = timestamp;
            }
            for (size_t i = 0; i < size ;i++) {
                g711_buffer_backchannel[backchannel_data_size++] = data[i];
                if (backchannel_data_size == 320) {
                    media_.SendData(g711_buffer_backchannel, backchannel_data_size, timestamp_backchanel);
                    backchannel_data_size = 0;
                    timestamp_backchanel = timestamp;
                }
            }
        }
    }

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) override {
        if (started_) {
            rtsp_server_.SendAudioData(stream_url_, (const uint8_t*)stream_data->stream, stream_data->len, stream_data->time_stamp);
        }
    }

    int Init(const std::string &stream_url = "BackChannelTest", Port port = 8554) {
        if (rtsp_server_.Init(port, this) < 0) {
            return -1;
        }
        // enable audio-track and backchannel-track
        SessionAttr session_attr;
        session_attr.with_audio = true;
        session_attr.with_audio_backchannel = true;
        if (rtsp_server_.CreateSession(stream_url, session_attr) < 0) {
            return -1;
        }
        stream_url_ = stream_url;

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
        rtsp_server_.DeInit();
        return 0;
    }

    int Start() {
        if(started_) return 0;
        media_.StartADecAo();
        rtsp_server_.Start();
        media_.StartAiAEnc();
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        rtsp_server_.Stop();
        started_ = false;
        media_.StopADecAo();
        media_.StopAiAEnc();
        return 0;
    }

  private:
    KdRtspServer rtsp_server_;
    KdMedia media_;
    std::string stream_url_;
    std::atomic<bool> started_{false};
    uint8_t g711_buffer_backchannel[320];
    size_t backchannel_data_size = 0;
    uint64_t timestamp_backchanel;
};


int main() {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    MyRtspServer *server = new MyRtspServer();
    if (!server || server->Init() < 0) {
        std::cout << "KdRtspServer Init failed." << std::endl;
        return -1;
    }
    server->Start();

    while (!g_exit_flag) {
        std::this_thread::sleep_for(100ms);
    }

    server->Stop();
    server->DeInit();
    delete server;
    return 0;
}
