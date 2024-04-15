#ifndef _MJPEG_LIVEFRAMESOURCE_H
#define _MJPEG_LIVEFRAMESOURCE_H

#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include "JPEGVideoSource.hh"
#include "JpegFrameParser.hh"

class MjpegLiveVideoSource : public JPEGVideoSource {
  public:
    static MjpegLiveVideoSource *createNew(UsageEnvironment &env, size_t queue_size = 8);
    JpegFrameParser &Parser() { return fParser;}
    void pushData(const uint8_t *data, size_t data_size, uint64_t timestamp);

  protected:
    MjpegLiveVideoSource(UsageEnvironment &env, size_t queue_size = 8);
    virtual ~MjpegLiveVideoSource();

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

    int getFrame();
    void processFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref);
    void queueFramePacket(FramePacket &packet);

  protected:
    std::list<FramePacket> fFramePacketQueue;
    std::list<RawData> fRawDataQueue;
    size_t fQueueSize;
    std::thread fThread;
    std::mutex fMutex;
    std::mutex fMutexRaw;
    std::atomic<bool> fNeedReadFrame{true};

  protected:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
    static void deliverFrame0(void *clientData);
    void deliverFrame();

    virtual u_int8_t type() { return fParser.type(); }
    virtual u_int8_t qFactor() { return fParser.qFactor(); }
    virtual u_int8_t width() { return fParser.width(); }
    virtual u_int8_t height() { return fParser.height(); }
    virtual u_int8_t const *quantizationTables(u_int8_t &precision, u_int16_t &length) {
        precision = fParser.precision();
        return fParser.quantizationTables(length);
    }
    virtual u_int16_t restartInterval() { return fParser.restartInterval(); }

    JpegFrameParser fParser;
    EventTriggerId fEventTriggerId = 0;
};

#endif // _MJPEG_LIVEFRAMESOURCE_H