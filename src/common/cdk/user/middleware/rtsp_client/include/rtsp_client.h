#ifndef _KD_RTSP_CLIENT_H
#define _KD_RTSP_CLIENT_H

#include <unistd.h>
#include <memory>

class IOnAudioData {
  public:
    virtual ~IOnAudioData() {}
    virtual void OnAudioStart() = 0;
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) = 0;
};

class IOnBackChannel {
  public:
    virtual ~IOnBackChannel() {}
    virtual void OnBackChannelStart() = 0;
};

class IOnVideoData {
  public:
    enum VideoType {VideoTypeInvalid, VideoTypeH264, VideoTypeH265};
    virtual ~IOnVideoData() {}
    virtual void OnVideoType(VideoType type, uint8_t *extra_data, size_t extra_data_size) = 0;
    virtual void OnVideoData(const uint8_t *data, size_t size, uint64_t timestamp, bool keyframe) = 0;
};

class IRtspClientEvent {
  public:
    virtual ~IRtspClientEvent() {}
    virtual void OnRtspClientEvent(int event) = 0; // event 0: shutdown
};

struct RtspClientInitParam {
    IOnVideoData *on_video_data{nullptr};
    IOnAudioData *on_audio_data{nullptr};
    IOnBackChannel *on_backchannel{nullptr};
    IRtspClientEvent *on_event{nullptr};
};

class KdRtspClient {
  public:
    KdRtspClient();
    ~KdRtspClient();

    int Init(const RtspClientInitParam &param);
    void DeInit();
    int Open(const char *url);
    void Close();

    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    KdRtspClient(const KdRtspClient &) = delete;
    KdRtspClient& operator=(const KdRtspClient &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _KD_RTSP_CLIENT_H