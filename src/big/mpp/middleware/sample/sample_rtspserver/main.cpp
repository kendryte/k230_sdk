#include <iostream>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include "rtsp_server.h"
#include "media.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void Usage() {
    std::cout << "Usage: ./rtsp_sever [-v] [-s <sensor_type>] [-t <codec_type>] [-w <width>] [-h <height>] [-b <bitrate_kbps>] [-a <semitones>]" << std::endl;
    std::cout << "-v: enable video session" << std::endl;
    std::cout << "-s: the sensor type, default 7 :" << std::endl;
    std::cout << "       see camera sensor doc." << std::endl;
    std::cout << "-t: the video encoder type: h264/h265, default h265" << std::endl;
    std::cout << "-w: the video encoder width, default 1280" << std::endl;
    std::cout << "-h: the video encoder height, default 720" << std::endl;
    std::cout << "-b: the video encoder bitrate(kbps), default 2000" << std::endl;
    std::cout << "-a: pitch shift semitones [-12,12], default 0" << std::endl;
    exit(-1);
}

int parse_config(int argc, char *argv[], KdMediaInputConfig &config) {
    int result;
    opterr = 0;
    while ((result = getopt(argc, argv, "Hvs:n:t:w:h:b:a:")) != -1) {
        switch(result) {
        case 'H' : {
            Usage(); break;
        }
        case 'v' : {
            config.video_valid = true;
            break;
        }
        case 's' : {
            int n = atoi(optarg);
            if (n < 0 || n > 27) Usage();
            config.sensor_type = (k_vicap_sensor_type)n;
            config.video_valid = true;
            break;
        }
        case 't': {
            std::string s = optarg;
            if (s == "h264") config.video_type = KdMediaVideoType::kVideoTypeH264;
            else if (s == "h265") config.video_type = KdMediaVideoType::kVideoTypeH264;
            else Usage();
            config.video_valid = true;
            break;
        }
        case 'w': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.venc_width = n;
            config.video_valid = true;
            break;
        }
        case 'h': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.venc_height = n;
            config.video_valid = true;
            break;
        }
        case 'b': {
            int n = atoi(optarg);
            if (n < 0) Usage();
            config.bitrate_kbps = n;
            config.video_valid = true;
            break;
        }
        case 'a': {
            int n = atoi(optarg);
            if (n < -12 || n > 12) Usage();
            config.pitch_shift_semitones = n;
            break;
        }
        default: Usage(); break;
        }
    }
    if (config.video_valid) {
        // validate the parameters... TODO
        std::cout << "Validate the input config, not implemented yet, TODO." << std::endl;
    }
    return 0;
}

class MyRtspServer : public IOnBackChannel, public IOnAEncData, public IOnVEncData {
  public:
    MyRtspServer() {}

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
                    media_.SendAudioData(g711_buffer_backchannel, backchannel_data_size, timestamp_backchanel);
                    backchannel_data_size = 0;
                    timestamp_backchanel = timestamp;
                }
            }
        }
    }

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_u8*pdata,size_t size,k_u64 time_stamp) override {
        if (started_) {
            rtsp_server_.SendAudioData(stream_url_, (const uint8_t*)pdata, size, time_stamp);
        }
    }

    // IOnVEncData
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, k_venc_pack_type type,uint64_t timestamp) override {
        if (started_) {
            rtsp_server_.SendVideoData(stream_url_, (const uint8_t*)data, size, timestamp);
        }
    }

    int Init(const KdMediaInputConfig &config, const std::string &stream_url = "BackChannelTest", int port = 8554) {
        if (rtsp_server_.Init(port, this) < 0) {
            return -1;
        }
        // enable audio-track and backchannel-track
        SessionAttr session_attr;
        session_attr.with_audio = true;
        session_attr.with_audio_backchannel = true;
        session_attr.with_video = config.video_valid;
        if (config.video_valid) {
            if (config.video_type == KdMediaVideoType::kVideoTypeH264) session_attr.video_type = VideoType::kVideoTypeH264;
            else if (config.video_type == KdMediaVideoType::kVideoTypeH265) session_attr.video_type = VideoType::kVideoTypeH265;
            else {
                std::cout << "video codec type not supported yet" << std::endl;
                return -1;
            }
        }
        if (rtsp_server_.CreateSession(stream_url, session_attr) < 0)  return -1;
        stream_url_ = stream_url;

        if (media_.Init(config) < 0) return -1;
        if (media_.CreateAiAEnc(this) < 0) return -1;
        if (media_.CreateADecAo() < 0) return -1;
        if (config.video_valid && media_.CreateVcapVEnc(this) < 0) return -1;
        return 0;
    }
    int DeInit() {
        Stop();
        media_.DestroyVcapVEnc();
        media_.DestroyADecAo();
        media_.DestroyAiAEnc();
        media_.Deinit();
        rtsp_server_.DeInit();
        return 0;
    }

    int Start() {
        if(started_) return 0;
        media_.StartADecAo();
        rtsp_server_.Start();
        media_.StartAiAEnc();
        media_.StartVcapVEnc();
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        rtsp_server_.Stop();
        started_ = false;
        media_.StopVcapVEnc();
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

int main(int argc, char *argv[]) {
    std::cout << "./rtsp_server -H to show usage" << std::endl;
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    KdMediaInputConfig config;
    parse_config(argc, argv, config);

    MyRtspServer *server = new MyRtspServer();
    if (!server || server->Init(config) < 0) {
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
