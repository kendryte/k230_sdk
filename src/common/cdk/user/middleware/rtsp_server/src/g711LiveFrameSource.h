#ifndef _G711_LIVEFRAMESOURCE_H
#define _G711_LIVEFRAMESOURCE_H

#include "LiveFrameSource.h"

class G711LiveFrameSource : public LiveFrameSource {
  public:
    static G711LiveFrameSource *createNew(UsageEnvironment &env, size_t queue_size);
 
  protected:
    G711LiveFrameSource(UsageEnvironment &env, size_t queue_size);
    virtual EncodeType GetEncodeType() override { return EncodeType::G711U;}
};

#endif  // _G711_LIVEFRAMESOURCE_H
