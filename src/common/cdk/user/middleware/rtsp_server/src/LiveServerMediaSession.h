#ifndef _LIVFRAMESERVERMEDIASUBSESSION_H
#define _LIVFRAMESERVERMEDIASUBSESSION_H

#include "liveMedia.hh"
#include "OnDemandServerMediaSubsession.hh"
#include "LiveFrameSource.h"

class LiveServerMediaSession : public OnDemandServerMediaSubsession {
  public:
    static LiveServerMediaSession *createNew(UsageEnvironment &env, StreamReplicator *replicator);

  protected:
    LiveServerMediaSession(UsageEnvironment &env, StreamReplicator *replicator);
    virtual ~LiveServerMediaSession();

  protected:
    virtual char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource);
    virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
    virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
  
  protected:
    StreamReplicator *fReplicator{nullptr};
};

#endif // _LIVFRAMESERVERMEDIASUBSESSION_H