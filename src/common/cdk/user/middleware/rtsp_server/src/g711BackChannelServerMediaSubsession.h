#ifndef __g711_BackChannelServerMediaSubsession__
#define __g711_BackChannelServerMediaSubsession__

// #include "FileServerMediaSubsession.hh"
#include "BackChannelServerMediaSubsession.h"
#include "MultiFramedRTPSource.hh"
#include <string>

class G711BackChannelServerMediaSubsession: public BackChannel::OnDemandServerMediaSubsession{
  public:
    static G711BackChannelServerMediaSubsession*
      createNew(UsageEnvironment& env, IOnData *onData,
                                      Boolean reuseFirstSource = False);

  protected:
    G711BackChannelServerMediaSubsession(UsageEnvironment& env,
                                      IOnData *onData,
                                      Boolean reuseFirstSource = False);
    // called only by createNew();
    virtual ~G711BackChannelServerMediaSubsession();

  protected: // redefined virtual functions
    virtual DataSink* createNewStreamDestination(unsigned clientSessionId,
                                    unsigned& estBitrate);
    // "estBitrate" is the stream's estimated bitrate, in kbps
    virtual RTPSource* createNewRTPSource(Groupsock* rtpGroupsock,
                                          unsigned char rtpPayloadTypeIfDynamic,
                                          DataSink* outputSink);
  protected:
    IOnData *fOnData;
};

class G711RTPSource: public MultiFramedRTPSource {
  public:
    static G711RTPSource*
    createNew(UsageEnvironment& env, Groupsock* RTPgs,
	      unsigned char rtpPayloadFormat,
	      unsigned rtpTimestampFrequency);

  protected:
    virtual ~G711RTPSource();

  private:
    G711RTPSource(UsageEnvironment& env, Groupsock* RTPgs,
		     unsigned char rtpPayloadFormat,
		     unsigned rtpTimestampFrequency);
      // called only by createNew()

  private:
     virtual char const* MIMEtype() const;
};

#endif
