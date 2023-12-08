#ifndef _MY_MUXER_H
#define _MY_MUXER_H

#include <unistd.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <vector>
#include <string>
#include "comm/include/comm_msg.h"

// TODO
class IMp4MuxerCallback {
  public:
    ~IMp4MuxerCallback() {}
    virtual void GetMp4Filename(std::string &filename) = 0;
    virtual bool IsMp4SpaceAvailable() = 0;
    virtual void OnMp4MuxerError(int err_code) = 0;
};

class Mp4Muxer;
class MyMp4Muxer {
  public:
    MyMp4Muxer() {}    
    int Init(IMp4MuxerCallback *cb);
    void DeInit(bool force = false);
    int Write(const AVEncFrameData &data);    

  private:
    void Process();

  private:  // emphasize the following members are private
    
    MyMp4Muxer(const MyMp4Muxer &);
    const MyMp4Muxer &operator=(const MyMp4Muxer &);
  
  private:
    friend class Mp4Muxer;
    std::mutex mutex_;
    bool initialized_{false};
    IMp4MuxerCallback *callback_{nullptr};
    std::thread thread_;
    std::atomic<bool> exit_flag_{false};
    std::atomic<bool> force_exit_{false};
    struct AVEncData {
        std::shared_ptr<uint8_t> data{nullptr};
        size_t size{0};
        int64_t timestamp_ms{0}; // gettimeofday_ms
        AVEncFrameData::Type type{AVEncFrameData::Type::INVALID};
        bool keyframe{false};
        uint32_t sequence{0};
        uint32_t flags{0};
        friend bool operator <(const AVEncData &f1, const AVEncData &f2) {
            return f1.timestamp_ms > f2.timestamp_ms;
        }
    };
    using DataType = AVEncFrameData::Type;
    std::mutex q_mutex_;
    std::priority_queue<AVEncData> queue_;
    bool start_process_{false};

    int64_t video_ts_base_{-1};
    int64_t audio_ts_base_{-1};
};

class IJpegMuxerCallback {
  public:
    ~IJpegMuxerCallback() {}
    virtual void GetJpegFilename(UserEventData::EventType type, int64_t timestamp_ms, std::string &filename) = 0;
    virtual bool IsJpegSpaceAvailable() = 0;
    virtual void OnJpegMuxerError(int err_code) = 0;
};

class MyJpegMuxer {
  public:
    MyJpegMuxer() {}
    int Init(IJpegMuxerCallback *cb);
    void DeInit(bool force = false);
    int Write(const UserEventData &event);

  private:
    void Process();

  private:  // emphasize the following members are private    
    MyJpegMuxer(const MyJpegMuxer &);
    const MyJpegMuxer &operator=(const MyJpegMuxer &);

  private:
    std::mutex mutex_;
    bool initialized_{false};
    IJpegMuxerCallback *callback_;
    std::thread thread_;
    std::atomic<bool> exit_flag_{false};
    std::atomic<bool> force_exit_{false};
    struct EventData {
        UserEventData::EventType type {UserEventData::EventType::INVALID};
        int64_t timestamp_ms{0};  // gettimeofday_ms
        std::shared_ptr<uint8_t> jpeg{nullptr};
        size_t jpeg_size{0};
    };    
    std::mutex q_mutex_;
    std::queue<EventData> queue_;
};

extern volatile int storage_ready_;

#endif // _MY_MUXER_H
