#ifndef _LIVEFRAMESOURCE_H
#define _LIVEFRAMESOURCE_H

#include "FramedSource.hh"
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

enum class EncodeType {
   INVALID = 0,
   H264 = 1,
   H265 = 2,
   G711U = 100,
   BOTTOM
};

template <typename T>
std::shared_ptr<T> make_shared_array(size_t size) {
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

class LiveFrameSource : public FramedSource {
  public:
    static LiveFrameSource *createNew(UsageEnvironment &env, size_t queue_size);
    void pushData(const uint8_t *data, size_t data_size, uint64_t timestamp);
    std::string getAuxLine() { return fAuxLine; };
    virtual EncodeType GetEncodeType() { return EncodeType::INVALID;}

  public:
    struct FramePacket {
      FramePacket() = default;
      FramePacket(std::shared_ptr<uint8_t> buffer, size_t offset, size_t size, struct timeval timestamp) :
        buffer_(buffer), offset_(offset), size_(size), timestamp_(timestamp) {}
      std::shared_ptr<uint8_t> buffer_{nullptr};
      size_t offset_{0};
      size_t size_{0};
      struct timeval timestamp_{0};
    };

    struct RawData {
        std::shared_ptr<uint8_t> buffer_{nullptr};
        size_t size_{0};
        uint64_t timestamp_{0};
    };

  protected:
    LiveFrameSource(UsageEnvironment &env, size_t queue_size);
    virtual ~LiveFrameSource();

    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    virtual unsigned maxFrameSize() const {
      return 128 * 1024; // set a reasonable value ..TODO
    }
    static void deliverFrame0(void *clientData);
    void deliverFrame();

    int getFrame();
    void processFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref);
    virtual std::list<FramePacket> parseFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref);
    void queueFramePacket(FramePacket &packet);

  protected:
    std::list<FramePacket> fFramePacketQueue;
    std::list<RawData> fRawDataQueue;
    EventTriggerId fEventTriggerId = 0;
    size_t fQueueSize;

    std::thread fThread;
    std::mutex fMutex;
    std::mutex fMutexRaw;
    std::atomic<bool> fNeedReadFrame{true};
    std::string fAuxLine;
};

#endif  // CLIENT_LIVEFRAMESOURCE_H
