#include <iostream>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include <sys/time.h>
#include "rtsp_pusher.h"
#include <string>
#include "rtsp_pusher.h"
#include "media.h"

using namespace std::chrono_literals;
static std::string g_rtsp_url;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void Usage() {
    std::cout << "Usage: ./sample_rtsppusher.elf [-s <sensor_type>] [-w <width>] [-h <height>] [-b <bitrate_kbps>] [-o <rtspurl>]" << std::endl;
    std::cout << "-s: the sensor type, default 7 :" << std::endl;
    std::cout << "       see camera sensor doc." << std::endl;
    std::cout << "-w: the video encoder width, default 1280" << std::endl;
    std::cout << "-h: the video encoder height, default 720" << std::endl;
    std::cout << "-b: the video encoder bitrate(kbps), default 2000" << std::endl;
    exit(-1);
}

int parse_config(int argc, char *argv[], KdMediaInputConfig &config) {
    int result;
    opterr = 0;
    while ((result = getopt(argc, argv, "Hs:w:h:b:o:")) != -1) {
        switch(result) {
        case 'H' : {
            Usage(); break;
        }
        case 's' : {
            int n = atoi(optarg);
            if (n < 0 || n > 27) Usage();
            config.sensor_type = (k_vicap_sensor_type)n;
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
        case 'o':{
            g_rtsp_url = optarg;
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

static long long get_timestamp_ticket() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long time_ticket = (long long)(tv.tv_sec) * 1000000 + tv.tv_usec; // 微妙
    return time_ticket;
}

class MyRtspServer : public IOnAEncData, public IOnVEncData {
  public:
    MyRtspServer() {}

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_u8*pdata,size_t size,k_u64 time_stamp) override {
        // printf("===========OnAEncData chn_id:%d,size:%d\n",chn_id,size);
        // return ;
        if (started_) {
            //rtsp_server_.SendAudioData(stream_url_, (const uint8_t*)pdata, size, time_stamp);
        }
    }

    // IOnVEncData
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, k_venc_pack_type type,uint64_t timestamp) override {
        if (started_) {
            if (0 == timestamp_video_)
            {
                timestamp_video_ = get_timestamp_ticket();
            }
            if (type == K_VENC_I_FRAME)
            {
                char nalu_type = ((char*)data)[4];
                if ((nalu_type & 0x1f) == 5)
                {
                    type = K_VENC_I_FRAME;
                }
                else
                {
                    type = K_VENC_P_FRAME;
                }
            }
            rtsp_pusher_.PushVideoData((uint8_t*)data,size,type == K_VENC_I_FRAME,get_timestamp_ticket() - timestamp_video_);
        }
    }

    int Init(const KdMediaInputConfig &config) {

        if (media_.Init(config) < 0) return -1;
        if (media_.CreateAiAEnc(this) < 0) return -1;
        if (media_.CreateADecAo() < 0) return -1;
        if (config.video_valid && media_.CreateVcapVEnc(this) < 0) return -1;

        RtspPusherInitParam pusher_param;
        strcpy(pusher_param.sRtspUrl,g_rtsp_url.c_str());
        pusher_param.video_width = config.venc_width;
        pusher_param.video_height = config.venc_height;

        rtsp_pusher_.Init(pusher_param);
        return 0;
    }
    int DeInit() {
        Stop();
        media_.DestroyVcapVEnc();
        media_.DestroyADecAo();
        media_.DestroyAiAEnc();
        media_.Deinit();

        rtsp_pusher_.DeInit();

        return 0;
    }

    int Start() {
        if(started_) return 0;
        media_.StartADecAo();
        rtsp_pusher_.Open();
        media_.StartAiAEnc();
        media_.StartVcapVEnc();
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        rtsp_pusher_.Close();
        started_ = false;
        media_.StopVcapVEnc();
        media_.StopADecAo();
        media_.StopAiAEnc();
        return 0;
    }

  private:
    //KdRtspServer rtsp_server_;
    KdRtspPusher   rtsp_pusher_;
    KdMedia media_;
    std::string stream_url_;
    std::atomic<bool> started_{false};
    uint8_t g711_buffer_backchannel[320];
    size_t backchannel_data_size = 0;
    uint64_t timestamp_backchanel;
    uint64_t timestamp_video_ = 0;
};

int main(int argc, char *argv[]) {
    std::cout << "./rtsp_server -H to show usage" << std::endl;
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    KdMediaInputConfig config;
    config.video_type = KdMediaVideoType::kVideoTypeH264;
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
