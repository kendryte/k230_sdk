#ifndef _KD_RTSP_CLIENT_H
#define _KD_RTSP_CLIENT_H

#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include "liveMedia.hh"
#include "noncopyable.h"
#include "g711LiveFrameSource.h"

class IOnAudioData {
  public:
    virtual ~IOnAudioData() {}
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) = 0;
};

class IRtspClientEvent {
  public:
    virtual ~IRtspClientEvent() {}
    virtual void OnRtspClientEvent(int event) = 0; // event 0: shutdown
};

class KdRtspClient : public Noncopyable {
  public:
    KdRtspClient() {}
    ~KdRtspClient() { DeInit(); }

    int Init(IOnAudioData *on_audio_data, IRtspClientEvent *on_event = nullptr);
    void DeInit();
    int Open(const char *url);
    void Close();

    int SendAudioData(const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    G711LiveFrameSource *g711_source_{nullptr}; // source, to send data to rtsp-server
    std::thread thread_;
    volatile char watchVariable_{0};
    IOnAudioData *on_audio_data_{nullptr};
    IRtspClientEvent *on_event_{nullptr};
};

#endif // _KD_RTSP_CLIENT_H