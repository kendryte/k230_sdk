#ifndef _H265_LIVEFRAMESOURCE_H
#define _H265_LIVEFRAMESOURCE_H

#include "LiveFrameSource.h"

class H265LiveFrameSource : public LiveFrameSource {
  public:
    static H265LiveFrameSource *createNew(UsageEnvironment &env, size_t queue_size);

  protected:
    H265LiveFrameSource(UsageEnvironment &env, size_t queue_size);
    virtual std::list<FramePacket> parseFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref) override;

    uint8_t *extractFrame(uint8_t *data, size_t &size, size_t &outsize);

    virtual EncodeType GetEncodeType() override { return EncodeType::H265;}
  
  private:
    std::shared_ptr<uint8_t> fVps;
    size_t vps_size{0};
    std::shared_ptr<uint8_t> fSps;
    size_t sps_size{0};
    std::shared_ptr<uint8_t> fPps;
    size_t pps_size{0};
    bool fRepeatConfig{true};
};

#endif  // _H265_LIVEFRAMESOURCE_H
