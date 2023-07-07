#ifndef _MJPEG_MEDIASUBSESSION_H
#define _MJPEG_MEDIASUBSESSION_H

#include "liveMedia.hh"
#include "OnDemandServerMediaSubsession.hh"
#include "FramedSource.hh"
#include "mjpegStreamReplicator.h"

class MjpegMediaSubsession: public OnDemandServerMediaSubsession {
  public:
    static MjpegMediaSubsession *createNew(UsageEnvironment &env, JpegStreamReplicator *replicator);

  protected:
    MjpegMediaSubsession(UsageEnvironment& env, JpegStreamReplicator *replicator);
    ~MjpegMediaSubsession();

  protected:
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

  protected:
    JpegStreamReplicator *fReplicator{nullptr};
};

#endif // _MJPEG_MEDIASUBSESSION_H