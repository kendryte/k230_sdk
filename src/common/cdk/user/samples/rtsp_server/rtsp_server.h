#ifndef _KD_RTSP_SERVER_H
#define _KD_RTSP_SERVER_H

#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include <map>
#include "liveMedia.hh"
#include "noncopyable.h"

enum VideoType {
    kVideoTypeH264,
    kVideoTypeH265,
    kVideoTypeMjpeg,
    kVideoTypeButt
};

struct SessionAttr {
    bool with_video {false};
    bool with_audio {false};   // G711U
    bool with_audio_backchannel{false}; // G711U
    VideoType video_type; // valid when with_video is true
};

class IOnBackChannel {
  public:
    virtual ~IOnBackChannel() {}
    virtual void OnBackChannelData(std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) = 0;
};

class KdRtspServer : public Noncopyable {
  public:
    KdRtspServer() {}
    ~KdRtspServer() { DeInit(); }

    int Init(Port port = 8554, IOnBackChannel *back_channel = nullptr);
    void DeInit();

    int CreateSession(const std::string &session_name, const SessionAttr &session_attr);
    int DestroySession(const std::string &session_name);
    void Start();
    void Stop();

    int SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);
    int SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    void announceStream(ServerMediaSession* sms, char const* streamName);

  private:
    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    RTSPServer *rtspServer_{nullptr};
    volatile char watchVariable_{0};
    std::thread server_loop_;
    IOnBackChannel *back_channel_{nullptr};
};

#endif // _RTSP_SERVER_H