#include "g711BackChannelServerMediaSubsession.h"

G711BackChannelServerMediaSubsession*
G711BackChannelServerMediaSubsession::createNew(UsageEnvironment& env,
                                      IOnData *onData,
                                      Boolean reuseFirstSource) {
    return new G711BackChannelServerMediaSubsession(env, onData, reuseFirstSource);
}

G711BackChannelServerMediaSubsession::G711BackChannelServerMediaSubsession(UsageEnvironment& env,
                                                IOnData *onData,
                                                Boolean reuseFirstSource)
: BackChannel::OnDemandServerMediaSubsession(env, reuseFirstSource), fOnData(onData) {
}


G711BackChannelServerMediaSubsession::~G711BackChannelServerMediaSubsession() {
}


DataSink* G711BackChannelServerMediaSubsession::createNewStreamDestination(unsigned clientSessionId,
                                                unsigned& estBitrate) {
  estBitrate = 64; // kbps, estimate
  return  DataSink::createNew(envir(), fOnData);
}

#include <iostream>
// "estBitrate" is the stream's estimated bitrate, in kbps
RTPSource* G711BackChannelServerMediaSubsession::createNewRTPSource(Groupsock* rtpGroupsock,
                                                unsigned char rtpPayloadTypeIfDynamic,
                                                DataSink* outputSink)
{
  rtpPayloadTypeIfDynamic = 0; // PCMU
  unsigned fSamplingFrequency = 8000;
  RTPSource* fReadSource = G711RTPSource::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, fSamplingFrequency);
  return fReadSource;
}

G711RTPSource* G711RTPSource::createNew(UsageEnvironment& env,
            Groupsock* RTPgs,
            unsigned char rtpPayloadFormat,
            unsigned rtpTimestampFrequency) {
  return new G711RTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency);
}

G711RTPSource::G711RTPSource(UsageEnvironment& env,
           Groupsock* rtpGS,
           unsigned char rtpPayloadFormat,
           unsigned rtpTimestampFrequency)
  : MultiFramedRTPSource(env, rtpGS, rtpPayloadFormat, rtpTimestampFrequency) {
}

G711RTPSource::~G711RTPSource() {
}

char const* G711RTPSource::MIMEtype() const {
  return "audio";
}
