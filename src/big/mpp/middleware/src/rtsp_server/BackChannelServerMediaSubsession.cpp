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
// Implementation

#include "BackChannelServerMediaSubsession.h"
#include <GroupsockHelper.hh>

DataSink::DataSink(UsageEnvironment& env, IOnData *onData)
  : MediaSink(env), fOnData(onData),fSamePresentationTimeCounter(0) {
  fBufferSize = 20000;
  fBuffer = new unsigned char[fBufferSize];
  fPrevPresentationTime.tv_sec = ~0; fPrevPresentationTime.tv_usec = 0;
}

DataSink::~DataSink() {
  delete[] fBuffer;
}

DataSink* DataSink::createNew(UsageEnvironment& env, IOnData *onData) {
  return new DataSink(env, onData);
}

Boolean DataSink::continuePlaying() {
  if (fSource == NULL) return False;

  fSource->getNextFrame(fBuffer, fBufferSize,
			afterGettingFrame, this,
			onSourceClosure, this);

  return True;
}

void DataSink::afterGettingFrame(void* clientData, unsigned frameSize,
				 unsigned numTruncatedBytes,
				 struct timeval presentationTime,
				 unsigned /*durationInMicroseconds*/) {
  DataSink* sink = (DataSink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

void DataSink::addData(unsigned char const* data, unsigned dataSize,
		       struct timeval presentationTime) {
  if (fOnData) {
    fOnData->OnData(data, dataSize, presentationTime);
  }
}

void DataSink::afterGettingFrame(unsigned frameSize,
				 unsigned numTruncatedBytes,
				 struct timeval presentationTime) {
  if (numTruncatedBytes > 0) {
    envir() << "DataSink::afterGettingFrame(): The input frame data was too large for our buffer size ("
	    << fBufferSize << ").  "
            << numTruncatedBytes << " bytes of trailing data was dropped!  Correct this by increasing the \"bufferSize\" parameter in the \"createNew()\" call to at least "
            << fBufferSize + numTruncatedBytes << "\n";
  }
  addData(fBuffer, frameSize, presentationTime);

  // Then try getting the next frame:
  continuePlaying();
}

namespace BackChannel {

OnDemandServerMediaSubsession
::OnDemandServerMediaSubsession(UsageEnvironment& env,
				Boolean reuseFirstSource,
				portNumBits initialPortNum,
				Boolean multiplexRTCPWithRTP)
  : ServerMediaSubsession(env),
    fSDPLines(NULL), fMIKEYStateMessage(NULL), fMIKEYStateMessageSize(0),
    fReuseFirstSource(reuseFirstSource),
    fMultiplexRTCPWithRTP(multiplexRTCPWithRTP), fLastStreamToken(NULL),
    fAppHandlerTask(NULL), fAppHandlerClientData(NULL) {
  fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
  if (fMultiplexRTCPWithRTP) {
    fInitialPortNum = initialPortNum;
  } else {
    // Make sure RTP ports are even-numbered:
    fInitialPortNum = (initialPortNum+1)&~1;
  }
  gethostname(fCNAME, sizeof fCNAME);
  fCNAME[sizeof fCNAME-1] = '\0'; // just in case
}

OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession() {
  delete[] fMIKEYStateMessage;
  delete[] fSDPLines;

  // Clean out the destinations hash table:
  while (1) {
    Destinations* destinations
      = (Destinations*)(fDestinationsHashTable->RemoveNext());
    if (destinations == NULL) break;
    delete destinations;
  }
  delete fDestinationsHashTable;
}

char const*
OnDemandServerMediaSubsession::sdpLines(int addressFamily) {
  if (fSDPLines == NULL) {
    unsigned estBitrate;
    // The file sink should bind to specific type
    // We can know the type once we invoke the createNewStreamDestination() implemented by descendant
    DataSink* sink = createNewStreamDestination(0, estBitrate);
    if (sink == NULL) {
      return NULL; // file not found
    }

    Groupsock* dummyGroupsock = createGroupsock(nullAddress(addressFamily), 0);
    unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic

    RTPSource* dummyRTPSource = createNewRTPSource(dummyGroupsock, rtpPayloadType, sink);
    setSDPLinesFromMediaSink(dummyRTPSource, sink, estBitrate);
    Medium::close(sink);
    Medium::close(dummyRTPSource);
  }
  return fSDPLines;
}

void OnDemandServerMediaSubsession
::getStreamParameters(unsigned clientSessionId,
		      struct sockaddr_storage const& clientAddress,
		      Port const& clientRTPPort,
		      Port const& clientRTCPPort,
		      int tcpSocketNum,
		      unsigned char rtpChannelId,
		      unsigned char rtcpChannelId,
		      TLSState* tlsState,
		      struct sockaddr_storage& destinationAddress,
		      u_int8_t& /*destinationTTL*/,
		      Boolean& isMulticast,
		      Port& serverRTPPort,
		      Port& serverRTCPPort,
		      void*& streamToken) {
  if (addressIsNull(destinationAddress)) {
    // normal case - use the client address as the destination address:
    destinationAddress = clientAddress;
  }
  isMulticast = False;

  if (fLastStreamToken != NULL && fReuseFirstSource) {
    // Special case: Rather than creating a new 'StreamState',
    // we reuse the one that we've already created:
    serverRTPPort = ((StreamState*)fLastStreamToken)->serverRTPPort();
    serverRTCPPort = ((StreamState*)fLastStreamToken)->serverRTCPPort();
    ++((StreamState*)fLastStreamToken)->referenceCount();
    streamToken = fLastStreamToken;
  } else {
    // Normal case: Create a new media source:
    unsigned streamBitrate = 0;
    DataSink *dataSink = createNewStreamDestination(clientSessionId, streamBitrate);

    // Create 'groupsock' and 'sink' objects for the destination,
    // using previously unused server port numbers:
    RTPSource* rtpSource = NULL;
    BasicUDPSource* udpSource = NULL;
    Groupsock* rtpGroupsock = NULL;
    Groupsock* rtcpGroupsock = NULL;

    if (clientRTPPort.num() != 0 || tcpSocketNum >= 0) { // Normal case: Create destinations
      portNumBits serverPortNum;
      if (clientRTCPPort.num() == 0) {
	// We're streaming raw UDP (not RTP). Create a single groupsock:
	NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
	for (serverPortNum = fInitialPortNum; ; ++serverPortNum) {
	  serverRTPPort = serverPortNum;
	  rtpGroupsock = createGroupsock(nullAddress(destinationAddress.ss_family), serverRTPPort);
	  if (rtpGroupsock->socketNum() >= 0) break; // success
	}

	udpSource = BasicUDPSource::createNew(envir(), rtpGroupsock);
    } else {
	// Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
	// groupsocks (RTP and RTCP), with adjacent port numbers (RTP port number even).
	// (If we're multiplexing RTCP and RTP over the same port number, it can be odd or even.)
	NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
	for (portNumBits serverPortNum = fInitialPortNum; ; ++serverPortNum) {
	  serverRTPPort = serverPortNum;
	  rtpGroupsock = createGroupsock(nullAddress(destinationAddress.ss_family), serverRTPPort);
	  if (rtpGroupsock->socketNum() < 0) {
	    delete rtpGroupsock;
	    continue; // try again
	  }

	  if (fMultiplexRTCPWithRTP) {
	    // Use the RTP 'groupsock' object for RTCP as well:
	    serverRTCPPort = serverRTPPort;
	    rtcpGroupsock = rtpGroupsock;
	  } else {
	    // Create a separate 'groupsock' object (with the next (odd) port number) for RTCP:
	    serverRTCPPort = ++serverPortNum;
	    rtcpGroupsock = createGroupsock(nullAddress(destinationAddress.ss_family), serverRTCPPort);
	    if (rtcpGroupsock->socketNum() < 0) {
	      delete rtpGroupsock;
	      delete rtcpGroupsock;
	      continue; // try again
	    }
	  }

	  break; // success
	}

	unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic

	rtpSource = createNewRTPSource(rtpGroupsock, rtpPayloadType, dataSink);
      }

      // Turn off the destinations for each groupsock.  They'll get set later
      // (unless TCP is used instead):
      if (rtpGroupsock != NULL) rtpGroupsock->removeAllDestinations();
      if (rtcpGroupsock != NULL) rtcpGroupsock->removeAllDestinations();

      if (rtpGroupsock != NULL) {
	// Try to use a big send buffer for RTP -  at least 0.1 second of
	// specified bandwidth and at least 50 KB
	unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
	if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
	increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
      }
    }

    // Set up the state of the stream.  The stream will get started later:
    streamToken = fLastStreamToken
      = new StreamState(*this, serverRTPPort, serverRTCPPort,
			streamBitrate, rtpGroupsock, rtcpGroupsock, rtpSource, udpSource, dataSink);
  }

  // Record these destinations as being for this client session id:
  Destinations* destinations;
  if (tcpSocketNum < 0) { // UDP
    destinations = new Destinations(destinationAddress, clientRTPPort, clientRTCPPort);
  } else { // TCP
    destinations = new Destinations(tcpSocketNum, rtpChannelId, rtcpChannelId, tlsState);
  }
  fDestinationsHashTable->Add((char const*)clientSessionId, destinations);
}

void OnDemandServerMediaSubsession::startStream(unsigned clientSessionId,
						void* streamToken,
						TaskFunc* rtcpRRHandler,
						void* rtcpRRHandlerClientData,
						unsigned short& rtpSeqNum,
						unsigned& rtpTimestamp,
						ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
						void* serverRequestAlternativeByteHandlerClientData) {
  StreamState* streamState = (StreamState*)streamToken;
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (streamState != NULL) {
    streamState->startPlaying(destinations, clientSessionId,
			      rtcpRRHandler, rtcpRRHandlerClientData,
			      serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
    RTPSource* rtpSource = streamState->rtpSource(); // alias
    if (rtpSource != NULL) {
      rtpSeqNum = rtpSource->curPacketRTPSeqNum();
      rtpTimestamp = rtpSource->curPacketRTPTimestamp(); // how to get RTPTimestamp if live555 not modified???
    }
  }
}

void OnDemandServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/,
						void* streamToken) {
  // Pausing isn't allowed if multiple clients are receiving data from
  // the same source:
  if (fReuseFirstSource) return;

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL) streamState->pause();
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
					       void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown

  return;
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
					       void* streamToken, char*& absStart, char*& absEnd) {
  // Seeking isn't allowed
  return;
}

void OnDemandServerMediaSubsession::nullSeekStream(unsigned /*clientSessionId*/, void* streamToken,
						   double streamEndTime, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown
  return;
}

void OnDemandServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/,
						   void* streamToken, float scale) {
  return;
}

float OnDemandServerMediaSubsession::getCurrentNPT(void* streamToken) {
  return 0.0;
}

DataSink* OnDemandServerMediaSubsession::getStreamSink(void* streamToken) {
  if (streamToken == NULL) return NULL;
  printf("%s %s line=%d \n",__FILE__, __FUNCTION__ , __LINE__);
  StreamState* streamState = (StreamState*)streamToken;
  return streamState->mediaSink();
}


void OnDemandServerMediaSubsession::deleteStream(unsigned clientSessionId,
						 void*& streamToken) {
  StreamState* streamState = (StreamState*)streamToken;

  // Look up (and remove) the destinations for this client session:
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (destinations != NULL) {
    fDestinationsHashTable->Remove((char const*)clientSessionId);

    // Stop streaming to these destinations:
    if (streamState != NULL) streamState->endPlaying(destinations, clientSessionId);
  }

  // Delete the "StreamState" structure if it's no longer being used:
  if (streamState != NULL) {
    if (streamState->referenceCount() > 0) --streamState->referenceCount();
    if (streamState->referenceCount() == 0) {
      delete streamState;
      streamToken = NULL;
    }
  }

  // Finally, delete the destinations themselves:
  delete destinations;
}

char const* OnDemandServerMediaSubsession
::getAuxSDPLineForBackChannel(DataSink* mediaSink, RTPSource* rtpSource) {
  return NULL;
}

void OnDemandServerMediaSubsession::closeStreamSink(MediaSink *outputSink) {
  Medium::close(outputSink);
}

Groupsock* OnDemandServerMediaSubsession
::createGroupsock(struct sockaddr_storage const& addr, Port port) {
  // Default implementation; may be redefined by subclasses:
  return new Groupsock(envir(), addr, port, 255);
}

RTCPInstance* OnDemandServerMediaSubsession
::createRTCP(Groupsock* RTCPgs, unsigned totSessionBW, /* in kbps */
	     unsigned char const* cname, RTPSink* sink) {
  // Default implementation; may be redefined by subclasses:
  return RTCPInstance::createNew(envir(), RTCPgs, totSessionBW, cname, sink, NULL/*we're a server*/);
}

void OnDemandServerMediaSubsession
::setRTCPAppPacketHandler(RTCPAppHandlerFunc* handler, void* clientData) {
  fAppHandlerTask = handler;
  fAppHandlerClientData = clientData;
}

void OnDemandServerMediaSubsession
::sendRTCPAppPacket(u_int8_t subtype, char const* name,
		    u_int8_t* appDependentData, unsigned appDependentDataSize) {
  StreamState* streamState = (StreamState*)fLastStreamToken;
  if (streamState != NULL) {
    streamState->sendRTCPAppPacket(subtype, name, appDependentData, appDependentDataSize);
  }
}

char* OnDemandServerMediaSubsession::getRtpMapLine(RTPSource* rtpSource) const {

  // assume numChannels = 1;
  int vNumChannels=1;
  if (rtpSource->rtpPayloadFormat() >= 96) { // the payload format type is dynamic
    char* encodingParamsPart;
    if (vNumChannels != 1) {
      encodingParamsPart = new char[1 + 20 /* max int len */];
      sprintf(encodingParamsPart, "/%d", vNumChannels);
    } else {
      encodingParamsPart = strDup("");
    }
    char const* const rtpmapFmt = "a=rtpmap:%d %s/%d%s\r\n";
    unsigned rtpmapFmtSize = strlen(rtpmapFmt)
      + 3 /* max char len */ + strlen("MPEG4-GENERIC")
      + 20 /* max int len */ + strlen(encodingParamsPart);
    char* rtpmapLine = new char[rtpmapFmtSize];
    sprintf(rtpmapLine, rtpmapFmt,
	    rtpSource->rtpPayloadFormat(), "MPEG4-GENERIC",
	    rtpSource->timestampFrequency(), encodingParamsPart);
    delete[] encodingParamsPart;

    return rtpmapLine;
  } else {
    // The payload format is staic, so there's no "a=rtpmap:" line:
    return strDup("a=rtpmap:0 PCMU/8000\r\n"); // FIXME, hardcode at the moment
  }
}

void OnDemandServerMediaSubsession
::setSDPLinesFromMediaSink(RTPSource* rtpSource, DataSink* mediaSink, unsigned estBitrate) {
  if (rtpSource == NULL) {
    printf("%s line:%d\n", __FUNCTION__, __LINE__);
    return;
  }
  char const* mediaType = "audio";
  unsigned char rtpPayloadType = rtpSource->rtpPayloadFormat();
  struct sockaddr_storage const& addressForSDP = rtpSource->RTPgs()->groupAddress();
  portNumBits portNumForSDP = ntohs(rtpSource->RTPgs()->port().num());

  AddressString ipAddressStr(addressForSDP);
  char* rtpmapLine = getRtpMapLine(rtpSource);
  char const* rangeLine = rangeSDPLine();
  // TODO: fix me
  //char const* auxSDPLine = "a=fmtp:96 streamtype=5;\r\n";
  char const* auxSDPLine = getAuxSDPLineForBackChannel(mediaSink, rtpSource);
  if (auxSDPLine == NULL) auxSDPLine = "";

  char const* const sdpFmt =
    "m=%s %u RTP/%sAVP %d\r\n"
    "c=IN %s %s\r\n"
    "b=AS:%u\r\n"
    "%s"
    "%s"
    "%s"
    "a=sendonly\r\n"
    "a=control:%s\r\n";
  unsigned sdpFmtSize = strlen(sdpFmt)
    + strlen(mediaType) + 5 /* max short len */ + 1 + 3 /* max char len */
    + 3/*IP4 or IP6*/ + strlen(ipAddressStr.val())
    + 20 /* max int len */
    + strlen(rtpmapLine)
    + strlen(rangeLine)
    + strlen(auxSDPLine)
    + strlen(trackId());
  char* sdpLines = new char[sdpFmtSize];
  sprintf(sdpLines, sdpFmt,
	  mediaType, // m= <media>
	  portNumForSDP, // m= <port>
          fParentSession->streamingUsesSRTP ? "S" : "",
	  rtpPayloadType, // m= <fmt list>
	  addressForSDP.ss_family == AF_INET ? "IP4" : "IP6",
          ipAddressStr.val(), // c= address
	  estBitrate, // b=AS:<bandwidth>
	  rtpmapLine, // a=rtpmap:... (if present)
	  rangeLine, // a=range:... (if present)
	  auxSDPLine, // optional extra SDP line
	  trackId()); // a=control:<track-id>
  // std::cout << "SDPLines:\r\n" << sdpLines << std::endl;
  delete[] (char*)rangeLine; delete[] rtpmapLine;

  delete[] fSDPLines; fSDPLines = strDup(sdpLines);
  delete[] sdpLines;
}


DataSink* OnDemandServerMediaSubsession::createNewStreamDestination(unsigned clientSessionId,
                                                                       unsigned& estBitrate)
{ return NULL;}


// "estBitrate" is the stream's estimated bitrate, in kbps
RTPSource* OnDemandServerMediaSubsession::createNewRTPSource(Groupsock* rtpGroupsock,
                                                                             unsigned char rtpPayloadTypeIfDynamic,
                                                                             DataSink* outputSink)
{ return NULL;}



////////// StreamState implementation //////////

static void afterPlayingStreamState(void* clientData) {
  StreamState* streamState = (StreamState*)clientData;
  if (streamState->streamDuration() == 0.0) {
    // When the input stream ends, tear it down.  This will cause a RTCP "BYE"
    // to be sent to each client, teling it that the stream has ended.
    // (Because the stream didn't have a known duration, there was no other
    //  way for clients to know when the stream ended.)
    streamState->reclaim();
  }
  // Otherwise, keep the stream alive, in case a client wants to
  // subsequently re-play the stream starting from somewhere other than the end.
  // (This can be done only on streams that have a known duration.)
}

StreamState::StreamState(OnDemandServerMediaSubsession& master,
                         Port const& serverRTPPort, Port const& serverRTCPPort,
			 unsigned totalBW, Groupsock* rtpGS, Groupsock* rtcpGS,
          RTPSource* rtpSource, BasicUDPSource* udpSource, DataSink* dataSink)
  : fMaster(master), fAreCurrentlyPlaying(False), fReferenceCount(1),
    fServerRTPPort(serverRTPPort), fServerRTCPPort(serverRTCPPort),
    fStreamDuration(master.duration()),
    fTotalBW(totalBW), fRTCPInstance(NULL) /* created later */,
    fStartNPT(0.0), fRTPgs(rtpGS), fRTCPgs(rtcpGS),
    fRTPSource(rtpSource), fUDPSource(udpSource), fDataSink(dataSink){
}

StreamState::~StreamState() {
  reclaim();
}

void StreamState
::startPlaying(Destinations* dests, unsigned clientSessionId,
	       TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
	       ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
	       void* serverRequestAlternativeByteHandlerClientData) {
  if (dests == NULL) return;

  // printf("%s %s %d fIsBackChannel=%d\n", __FILE__, __FUNCTION__, __LINE__, fMaster.fIsBackChannel);
  if (fRTCPInstance == NULL && fRTPSource != NULL) {
    // Create (and start) a 'RTCP Instance' for this RTP Source:
    fRTCPInstance = RTCPInstance::createNew(fRTPSource->envir(), fRTCPgs, fTotalBW, (unsigned char*)fMaster.fCNAME,
            NULL/*RTPSink*/, fRTPSource/*We are a server with backchannel*/);
         // Note: This starts RTCP running automatically
  }

  if (dests->isTCP) {
    // Change RTP and RTCP to use the TCP socket instead of UDP:
    if (fRTPSource!= NULL) {
      fRTPSource->setStreamSocket(dests->tcpSocketNum, dests->rtpChannelId, dests->tlsState);
      RTPInterface::setServerRequestAlternativeByteHandler(fRTPSource->envir(), dests->tcpSocketNum,
                  serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
                  // So that we continue to handle RTSP commands from the client
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->addStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId, dests->tlsState);

      struct sockaddr_storage tcpSocketNumAsAddress; // hack
      tcpSocketNumAsAddress.ss_family = AF_INET;
      ((sockaddr_in&)tcpSocketNumAsAddress).sin_addr.s_addr = dests->tcpSocketNum;
      fRTCPInstance->setSpecificRRHandler(tcpSocketNumAsAddress, dests->rtcpChannelId,
					  rtcpRRHandler, rtcpRRHandlerClientData);
    }
 } else {
    // Tell the RTP and RTCP 'groupsocks' about this destination
    // (in case they don't already have it):
    if (fRTPgs != NULL) fRTPgs->addDestination(dests->addr, dests->rtpPort, clientSessionId);
    if (fRTCPgs != NULL && !(fRTCPgs == fRTPgs && dests->rtcpPort.num() == dests->rtpPort.num())) {
      fRTCPgs->addDestination(dests->addr, dests->rtcpPort, clientSessionId);
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->setSpecificRRHandler(dests->addr, dests->rtcpPort,
					  rtcpRRHandler, rtcpRRHandlerClientData);
    }
  }

  if (fRTCPInstance != NULL) {
    // Hack: Send an initial RTCP "SR" packet, before the initial RTP packet, so that receivers will (likely) be able to
    // get RTCP-synchronized presentation times immediately:
    fRTCPInstance->sendReport();
  }

  if (!fAreCurrentlyPlaying && fDataSink != NULL) {
    if (fRTPSource != NULL) {
      fDataSink->startPlaying(*fRTPSource, afterPlayingStreamState, this);
      fAreCurrentlyPlaying = True;
    } else if (fUDPSource != NULL) {
      fDataSink->startPlaying(*fUDPSource, afterPlayingStreamState, this);
      fAreCurrentlyPlaying = True;
    }
  }
     // printf("%s %s %d fIsBackChannel=%d\n", __FILE__, __FUNCTION__, __LINE__, fMaster.fIsBackChannel);
}

void StreamState::pause() {
  if (fRTPSource != NULL) fDataSink->stopPlaying();
  if (fUDPSource != NULL) fDataSink->stopPlaying();

  fAreCurrentlyPlaying = False;
}

void StreamState::endPlaying(Destinations* dests, unsigned clientSessionId) {
#if 0
  // The following code is temporarily disabled, because it erroneously sends RTCP "BYE"s to all clients if multiple
  // clients are streaming from the same data source (i.e., if "reuseFirstSource" is True), and we don't want that to happen
  // if we're being called as a result of a single one of these clients having sent a "TEARDOWN" (rather than the whole stream
  // having been closed, for all clients).
  // This will be fixed for real later.
  if (fRTCPInstance != NULL) {
    // Hack: Explicitly send a RTCP "BYE" packet now, because the code below will prevent that from happening later,
    // when "fRTCPInstance" gets deleted:
    fRTCPInstance->sendBYE();
  }
#endif

  if (dests->isTCP) {
    if (fRTPSource != NULL) {
      fRTPSource->removeStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->removeStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);

      struct sockaddr_storage tcpSocketNumAsAddress; // hack
      tcpSocketNumAsAddress.ss_family = AF_INET;
      ((sockaddr_in&)tcpSocketNumAsAddress).sin_addr.s_addr = dests->tcpSocketNum;
      fRTCPInstance->unsetSpecificRRHandler(tcpSocketNumAsAddress, dests->rtcpChannelId);
    }
  } else {
    // Tell the RTP and RTCP 'groupsocks' to stop using these destinations:
    if (fRTPgs != NULL) fRTPgs->removeDestination(clientSessionId);
    if (fRTCPgs != NULL && fRTCPgs != fRTPgs) fRTCPgs->removeDestination(clientSessionId);
    if (fRTCPInstance != NULL) {
      fRTCPInstance->unsetSpecificRRHandler(dests->addr, dests->rtcpPort);
    }
  }
}

void StreamState::sendRTCPAppPacket(u_int8_t subtype, char const* name,
				    u_int8_t* appDependentData, unsigned appDependentDataSize) {
  if (fRTCPInstance != NULL) {
    fRTCPInstance->sendAppPacket(subtype, name, appDependentData, appDependentDataSize);
  }
}

void StreamState::reclaim() {
  // Delete allocated media objects
  Medium::close(fRTCPInstance) /* will send a RTCP BYE */; fRTCPInstance = NULL;
  if (fMaster.fLastStreamToken == this) fMaster.fLastStreamToken = NULL;

  if(fDataSink)
  {
    fMaster.closeStreamSink(fDataSink);
    fDataSink = NULL;
  }

  if(fRTPSource)
  {
    Medium::close(fRTPSource);
    fRTPSource = NULL;
  }

  if(fUDPSource)
  {
    Medium::close(fUDPSource);
    fUDPSource = NULL;
  }

  delete fRTPgs;
  if (fRTCPgs != fRTPgs) delete fRTCPgs;
  fRTPgs = NULL; fRTCPgs = NULL;
}

} // namespace BackChannel
