#ifndef _H264_LIVEFRAMESOURCE_H
#define _H264_LIVEFRAMESOURCE_H

#include "LiveFrameSource.h"

class H264LiveFrameSource : public LiveFrameSource {
  public:
    static H264LiveFrameSource *createNew(UsageEnvironment &env, size_t queue_size);

  protected:
    H264LiveFrameSource(UsageEnvironment &env, size_t queue_size);
    virtual std::list<FramePacket> parseFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref) override;

    uint8_t *extractFrame(uint8_t *data, size_t &size, size_t &outsize);

    virtual EncodeType GetEncodeType() override { return EncodeType::H264;}
  
  private:
    std::shared_ptr<uint8_t> fSps;
    size_t sps_size{0};
    std::shared_ptr<uint8_t> fPps;
    size_t pps_size{0};
    bool fRepeatConfig{true};
};

#endif  // _H264_LIVEFRAMESOURCE_H
