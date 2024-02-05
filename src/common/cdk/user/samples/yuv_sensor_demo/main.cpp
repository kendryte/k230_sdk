#include <iostream>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include "rtsp_server.h"
#include "media.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "mapi_vo_api.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

#define MAX_SESSION_NUM 3
static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void Usage() {
    std::cout << "Usage: ./special_vehicle [-v] [-s <sensor_type>] [-t <codec_type>] [-w <width>] [-h <height>] [-b <bitrate_kbps>] [-a <semitones>]" << std::endl;
    std::cout << "-n: video channel count, default 1" << std::endl;
    std::cout << "-o: video output, default 0" << std::endl;
    std::cout << "-s: the sensor type, default 30 :" << std::endl;
    std::cout << "       see camera sensor doc." << std::endl;
    std::cout << "-h: the video encoder height, default 720" << std::endl;
    exit(-1);
}

int parse_config(int argc, char *argv[], KdMediaInputConfig &config) {
    int result;

    opterr = 0;
    config.video_valid = true;

    for (int i = 1; i < argc; i+=2)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            Usage();
            break;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            config.vo = (k_connector_type)atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            config.session_num = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            config.sensor_type = (k_vicap_sensor_type)atoi(argv[i + 1]);
        }
        else
        {
            Usage();
            break;
        }
    }

    return 0;
}

class MyRtspServer : public IOnVEncData {
  public:
    MyRtspServer() {}

    // IOnVEncData
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, uint64_t timestamp) override {
        if (started_) {
            rtsp_server_.SendVideoData(stream_url_[chn_id], (const uint8_t*)data, size, timestamp);
        }
    }

    int Init(const KdMediaInputConfig &config, const std::string &stream_url = "BackChannelTest", int port = 8554) {
        config_ = config;
        if (rtsp_server_.Init(port, NULL) < 0) {
            return -1;
        }
        // enable audio-track and backchannel-track
        SessionAttr session_attr;
        session_attr.with_audio = false;
        session_attr.with_audio_backchannel = false;
        session_attr.with_video = config.video_valid;
        if (config.video_valid) {
            if (config.video_type == KdMediaVideoType::kVideoTypeH264) session_attr.video_type = VideoType::kVideoTypeH264;
            else if (config.video_type == KdMediaVideoType::kVideoTypeH265) session_attr.video_type = VideoType::kVideoTypeH265;
            else {
                std::cout << "video codec type not supported yet" << std::endl;
                return -1;
            }
        }

        for (int i =0;i < config.session_num;i ++){

            if (rtsp_server_.CreateSession(stream_url + std::to_string(i), session_attr) < 0)  return -1;
            stream_url_[i] = stream_url + std::to_string(i);

        }

        if (media_.Init(config) < 0) return -1;

        // venc init
        for (int i =0;i < config.session_num;i ++){

            media_.InitVenc(i,this);
        }
        printf("vo type %d\n", config.vo);
        media_.InitVO(config.vo); //LT9611_MIPI_4LAN_1920X1080_60FPS  HX8377_V2_MIPI_4LAN_1080X1920_30FPS

        printf("config_.session_num is %d \n", config_.session_num);
        // vicap init
        if(config_.session_num == 1)
            media_.InitVcap(VICAP_DEV_ID_0, XS9950_MIPI_CSI0_1280X720_30FPS_YUV422);
        else if(config_.session_num == 2)
        {
            media_.InitVcap(VICAP_DEV_ID_0, XS9950_MIPI_CSI0_1280X720_30FPS_YUV422);
            media_.InitVcap(VICAP_DEV_ID_1, XS9950_MIPI_CSI1_1280X720_30FPS_YUV422);
        }
        else if(config_.session_num == 3)
        {
            media_.InitVcap(VICAP_DEV_ID_0, XS9950_MIPI_CSI0_1280X720_30FPS_YUV422);
            media_.InitVcap(VICAP_DEV_ID_1, XS9950_MIPI_CSI1_1280X720_30FPS_YUV422);
            media_.InitVcap(VICAP_DEV_ID_2, XS9950_MIPI_CSI2_1280X720_30FPS_YUV422);
        }

        return 0;
    }
    int DeInit() {
        Stop();
        for (int i =0;i < config_.session_num;i ++){
            media_.DeinitVenc(i);
        }
        media_.DeinitVO();
        media_.Deinit();
        rtsp_server_.DeInit();
        return 0;
    }

    int Start() {
        if(started_) return 0;

        rtsp_server_.Start();

        for (int i =0;i < config_.session_num;i ++){
            media_.StartVenc(i);
            media_.StartVcap((k_vicap_dev)i);
        }

        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        rtsp_server_.Stop();
        started_ = false;

        for (int i =0;i < config_.session_num;i ++){
            media_.StopVcap((k_vicap_dev)i);
        }
        usleep(1000*1000);

        for (int i =0;i < config_.session_num;i ++){
            media_.StopVenc((k_vicap_dev)i);
        }
        return 0;
    }

  private:
    KdMediaInputConfig config_;
    KdRtspServer rtsp_server_;
    KdMedia media_;
    std::string stream_url_[MAX_SESSION_NUM];
    std::atomic<bool> started_{false};
};

int main(int argc, char *argv[]) {
    std::cout << "./rtsp_server -H to show usage" << std::endl;
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    KdMediaInputConfig config;
    parse_config(argc, argv, config);
    printf("===session number:%d\n",config.session_num);

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
