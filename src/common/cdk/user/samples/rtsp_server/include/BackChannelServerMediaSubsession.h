/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2023 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand.
// C++ header

#ifndef _BACKCHANNEL_SERVER_MEDIA_SUBSESSION_HH
#define _BACKCHANNEL_SERVER_MEDIA_SUBSESSION_HH

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif
#ifndef _RTCP_HH
#include "RTCP.hh"
#endif

#ifndef _RTP_SOURCE_HH
#include "RTPSource.hh"
#endif
#ifndef _BASIC_UDP_SOURCE_HH
#include "BasicUDPSource.hh"
#endif

class IOnData {
  public:
    virtual ~IOnData(){}
    virtual void OnData(unsigned char const* data, unsigned dataSize, struct timeval presentationTime) = 0;
};

class DataSink: public MediaSink {
public:
  static DataSink* createNew(UsageEnvironment& env, IOnData *onData);

  virtual void addData(unsigned char const* data, unsigned dataSize,
		       struct timeval presentationTime);
  // (Available in case a client wants to add extra data to the output file)

protected:
  DataSink(UsageEnvironment& env, IOnData *onData);
      // called only by createNew()
  virtual ~DataSink();

protected: // redefined virtual functions:
  virtual Boolean continuePlaying();

protected:
  static void afterGettingFrame(void* clientData, unsigned frameSize,
				unsigned numTruncatedBytes,
				struct timeval presentationTime,
				unsigned durationInMicroseconds);
  virtual void afterGettingFrame(unsigned frameSize,
				 unsigned numTruncatedBytes,
				 struct timeval presentationTime);

  IOnData *fOnData;
  unsigned char* fBuffer;
  unsigned fBufferSize;
  struct timeval fPrevPresentationTime;
  unsigned fSamePresentationTimeCounter;
};

namespace BackChannel {

class OnDemandServerMediaSubsession: public ServerMediaSubsession {
 
protected: // we're a virtual base class
  OnDemandServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource,
				portNumBits initialPortNum = 6970,
				Boolean multiplexRTCPWithRTP = False);
  virtual ~OnDemandServerMediaSubsession();

protected: // redefined virtual functions
  virtual char const* sdpLines(int addressFamily);
  virtual void getStreamParameters(unsigned clientSessionId,
				   struct sockaddr_storage const& clientAddress,
                                   Port const& clientRTPPort,
                                   Port const& clientRTCPPort,
				   int tcpSocketNum,
                                   unsigned char rtpChannelId,
                                   unsigned char rtcpChannelId,
				   TLSState* tlsState,
                                   struct sockaddr_storage& destinationAddress,
				   u_int8_t& destinationTTL,
                                   Boolean& isMulticast,
                                   Port& serverRTPPort,
                                   Port& serverRTCPPort,
                                   void*& streamToken);
  virtual void startStream(unsigned clientSessionId, void* streamToken,
			   TaskFunc* rtcpRRHandler,
			   void* rtcpRRHandlerClientData,
			   unsigned short& rtpSeqNum,
                           unsigned& rtpTimestamp,
			   ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                           void* serverRequestAlternativeByteHandlerClientData);
  virtual void pauseStream(unsigned clientSessionId, void* streamToken);
  virtual void seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes);
  virtual void seekStream(unsigned clientSessionId, void* streamToken, char*& absStart, char*& absEnd);
  virtual void nullSeekStream(unsigned clientSessionId, void* streamToken,
			      double streamEndTime, u_int64_t& numBytes);
  virtual void setStreamScale(unsigned clientSessionId, void* streamToken, float scale);
  virtual float getCurrentNPT(void* streamToken);

  virtual FramedSource* getStreamSource(void* streamToken) { return NULL;};
  virtual void getRTPSinkandRTCP(void* streamToken,
				 RTPSink const*& rtpSink, RTCPInstance const*& rtcp) {return;}
  virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

  // added for audio back channel
  virtual DataSink* getStreamSink(void* streamToken);

protected: // new virtual functions, possibly redefined by subclasses
  // added for audio back channel
  virtual char const* getAuxSDPLineForBackChannel(DataSink* mediaSink,
                                      RTPSource* rtpSource);

  // added for audio back channel
  virtual void closeStreamSink(MediaSink *outputSink);

protected: // new virtual functions, defined by all subclasses
  // use RTPSource to read data, and use DataSink to receive data
  virtual DataSink* createNewStreamDestination(unsigned clientSessionId,
					      unsigned& estBitrate);
      // "estBitrate" is the stream's estimated bitrate, in kbps
  virtual RTPSource* createNewRTPSource(Groupsock* rtpGroupsock,
				    unsigned char rtpPayloadTypeIfDynamic,
				    DataSink* outputSink);


protected: // new virtual functions, may be redefined by a subclass:
  virtual Groupsock* createGroupsock(struct sockaddr_storage const& addr, Port port);
  virtual RTCPInstance* createRTCP(Groupsock* RTCPgs, unsigned totSessionBW, /* in kbps */
				   unsigned char const* cname, RTPSink* sink);

public:
  void multiplexRTCPWithRTP() { fMultiplexRTCPWithRTP = True; }
    // An alternative to passing the "multiplexRTCPWithRTP" parameter as True in the constructor

  void setRTCPAppPacketHandler(RTCPAppHandlerFunc* handler, void* clientData);
    // Sets a handler to be called if a RTCP "APP" packet arrives from any future client.
    // (Any current clients are not affected; any "APP" packets from them will continue to be
    // handled by whatever handler existed when the client sent its first RTSP "PLAY" command.)
    // (Call with (NULL, NULL) to remove an existing handler - for future clients only)

  void sendRTCPAppPacket(u_int8_t subtype, char const* name,
			 u_int8_t* appDependentData, unsigned appDependentDataSize);
    // Sends a custom RTCP "APP" packet to the most recent client (if "reuseFirstSource" was False),
    // or to all current clients (if "reuseFirstSource" was True).
    // The parameters correspond to their
    // respective fields as described in the RTP/RTCP definition (RFC 3550).
    // Note that only the low-order 5 bits of "subtype" are used, and only the first 4 bytes
    // of "name" are used.  (If "name" has fewer than 4 bytes, or is NULL,
    // then the remaining bytes are '\0'.)

protected:
  void setSDPLinesFromMediaSink(RTPSource* rtpSource, DataSink* mediaSink,
			      unsigned estBitrate);
  char* getRtpMapLine(RTPSource* rtpSource) const;

  // used to implement "sdpLines()"

protected:
  char* fSDPLines;
  u_int8_t* fMIKEYStateMessage; // used if we're streaming SRTP
  unsigned fMIKEYStateMessageSize; // ditto
  HashTable* fDestinationsHashTable; // indexed by client session id

private:
  Boolean fReuseFirstSource;
  portNumBits fInitialPortNum;
  Boolean fMultiplexRTCPWithRTP;
  void* fLastStreamToken;
  char fCNAME[100]; // for RTCP
  RTCPAppHandlerFunc* fAppHandlerTask;
  void* fAppHandlerClientData;

  friend class StreamState;
};

// A class that represents the state of an ongoing stream.  This is used only internally, in the implementation of
// "OnDemandServerMediaSubsession", but we expose the definition here, in case subclasses of "OnDemandServerMediaSubsession"
// want to access it.

class Destinations {
public:
  Destinations(struct sockaddr_storage const& destAddr,
               Port const& rtpDestPort,
               Port const& rtcpDestPort)
    : isTCP(False), addr(destAddr), rtpPort(rtpDestPort), rtcpPort(rtcpDestPort) {
  }
  Destinations(int tcpSockNum, unsigned char rtpChanId, unsigned char rtcpChanId,
	       TLSState* tlsSt)
    : isTCP(True), rtpPort(0) /*dummy*/, rtcpPort(0) /*dummy*/,
      tcpSocketNum(tcpSockNum), rtpChannelId(rtpChanId), rtcpChannelId(rtcpChanId),
      tlsState(tlsSt) {
  }

public:
  Boolean isTCP;
  struct sockaddr_storage addr;
  Port rtpPort;
  Port rtcpPort;
  int tcpSocketNum;
  unsigned char rtpChannelId, rtcpChannelId;
  TLSState* tlsState;
};

class StreamState {
public:
  StreamState(OnDemandServerMediaSubsession& master,
              Port const& serverRTPPort, Port const& serverRTCPPort,
	            unsigned totalBW, Groupsock* rtpGS, Groupsock* rtcpGS,
              RTPSource* rtpSource, BasicUDPSource* udpSource, DataSink* fFileSink);
  virtual ~StreamState();

  void startPlaying(Destinations* destinations, unsigned clientSessionId,
		    TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
		    ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                    void* serverRequestAlternativeByteHandlerClientData);
  void pause();
  void sendRTCPAppPacket(u_int8_t subtype, char const* name,
			 u_int8_t* appDependentData, unsigned appDependentDataSize);
  void endPlaying(Destinations* destinations, unsigned clientSessionId);
  void reclaim();

  unsigned& referenceCount() { return fReferenceCount; }

  Port const& serverRTPPort() const { return fServerRTPPort; }
  Port const& serverRTCPPort() const { return fServerRTCPPort; }

  RTCPInstance* rtcpInstance() const { return fRTCPInstance; }

  RTPSource* rtpSource() const { return fRTPSource; }
  DataSink* mediaSink() const {return fDataSink;}

  float streamDuration() const { return fStreamDuration; }
  float& startNPT() { return fStartNPT; }

private:
  OnDemandServerMediaSubsession& fMaster;
  Boolean fAreCurrentlyPlaying;
  unsigned fReferenceCount;

  Port fServerRTPPort, fServerRTCPPort;

  float fStreamDuration;
  unsigned fTotalBW;
  RTCPInstance* fRTCPInstance;

  float fStartNPT; // initial 'normal play time'; reset after each seek

  Groupsock* fRTPgs;
  Groupsock* fRTCPgs;

  RTPSource*        fRTPSource;
  BasicUDPSource*   fUDPSource;
  DataSink*         fDataSink;
};

} // namespace BackChannel

#endif
