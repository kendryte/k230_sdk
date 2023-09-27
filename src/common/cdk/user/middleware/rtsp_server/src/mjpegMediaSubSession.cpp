#include "mjpegMediaSubSession.h"
#include "FramedSource.hh"
#include "mjpegLiveFrameSource.h"
#include "JPEGVideoRTPSink.hh"

MjpegMediaSubsession *MjpegMediaSubsession::createNew(UsageEnvironment &env, JpegStreamReplicator *replicator) {
    return new MjpegMediaSubsession(env, replicator);
}

MjpegMediaSubsession::MjpegMediaSubsession(UsageEnvironment &env, JpegStreamReplicator *replicator)
    : OnDemandServerMediaSubsession(env, False/*reuseFirstSource*/), fReplicator(replicator)
    { }

MjpegMediaSubsession::~MjpegMediaSubsession() {
}

FramedSource* MjpegMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) {
    if (fReplicator) {
        // std::cout << "MjpegMediaSubsession::createNewStreamSource replicator called" << std::endl;
        estBitrate = 9000;
        MjpegLiveVideoSource *frameSource = fReplicator->createStreamReplica();
        return frameSource;
    }
    return nullptr;
}

RTPSink* MjpegMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource){
    // std::cout << "MjpegMediaSubsession::createNewRTPSink called"<< std::endl;
    OutPacketBuffer::maxSize = 2 * 1024 * 1024;
    return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}