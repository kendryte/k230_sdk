#ifndef _STREAMING_PLAYER_H
#define _STREAMING_PLAYER_H

#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "LiveServerMediaSession.h"
#include "h264LiveFrameSource.h"
#include "h265LiveFrameSource.h"
#include "g711LiveFrameSource.h"
#include "mjpegLiveFrameSource.h"
#include "mjpegMediaSubSession.h"
#include "mjpegStreamReplicator.h"
#include "mapi_sys_api.h"
#include "mapi_vvi_api.h"
#include "mapi_venc_api.h"
#include "mapi_ai_api.h"
#include "mapi_aenc_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

enum VideoType {
    kVideoTypeH264,
    kVideoTypeH265,
    kVideoTypeMjpeg,
    kVideoTypeButt
};

enum AudioType {
    kAudioTypeG711a,
    kAudioTypeButt
};

struct SessionAttr {
    uint32_t session_idx;
    std::string session_name;
    VideoType video_type;
    uint32_t video_width;
    uint32_t video_height;
    k_vicap_sensor_type sensor_type;
    k_i2s_in_mono_channel  auido_mono_channel_type;
};

#define MAX_SESSION_NUM 3

class StreamingPlayer {
public:

    StreamingPlayer(const k_vicap_sensor_type &sensor_type, int video_width, int video_height, int session_num = 1) :
        sensor_type_(sensor_type), video_width_(video_width), video_height_(video_height), session_num_(session_num) {
        if (session_num_ < 1 || session_num_ > MAX_SESSION_NUM) exit(-1);
        scheduler_ = BasicTaskScheduler::createNew();
        env_ = BasicUsageEnvironment::createNew(*scheduler_);
        UserAuthenticationDatabase* authDB = nullptr;
        unsigned reclamationSeconds = 10;
        rtspServer_ = RTSPServer::createNew(*env_, 8554, authDB, reclamationSeconds);
        if (!rtspServer_) {
            *env_ << "create rtsp server failed." << env_->getResultMsg() << "\n";
            exit(-1);
        }

        int ret = StreamingPlayerInit();
        if (ret != K_SUCCESS) {
            std::cout << "streaming player init failed." << std::endl;
            Medium::close(rtspServer_);
            exit(-1);
        }

        audio_created_.store(false);
    }

    void DeInit() {
        StreamingPlayerDeinit();
        Medium::close(rtspServer_);
        env_->reclaim();
        delete scheduler_;
    }

    int CreateSession(const SessionAttr &session_attr);
    int DestroySession(int session_idx);

    void Start();

    void Stop();

private:
    int CreateAudioEncode(const SessionAttr &session_attr);
    int CreateVideoEncode(const SessionAttr &session_attr);
    int StreamingPlayerInit();
    int StreamingPlayerDeinit();
    int createSubSession(ServerMediaSession *sms, const SessionAttr &session_attr);
    void announceStream(ServerMediaSession* sms, char const* streamName);

    int video_width_;
    int video_height_;
    int session_num_{1};
    k_u32 audio_sample_rate_{8000};
    k_mapi_media_attr_t media_attr_;
    k_vicap_dev_set_info dev_attr_info_;
    k_vicap_sensor_type sensor_type_;
    std::atomic<bool> audio_created_{false};

    volatile char watchVariable_{0};
    std::thread server_loop_;

    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    RTSPServer *rtspServer_{nullptr};
};

#endif // _STREAMING_PLAYER_H