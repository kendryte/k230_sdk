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
// An class that can be used to create (possibly multiple) 'replicas' of an incoming stream.
// C++ header

#ifndef _MJPEG_STREAM_REPLICATOR_H
#define _MJPEG_STREAM_REPLICATOR_H

#include "mjpegLiveFrameSource.h"

class JpegStreamReplica; // forward

class JpegStreamReplicator: public Medium {
public:
  static JpegStreamReplicator* createNew(UsageEnvironment& env, MjpegLiveVideoSource *inputSource, Boolean deleteWhenLastReplicaDies = True);
    // If "deleteWhenLastReplicaDies" is True (the default), then the "JpegStreamReplicator" object is deleted when (and only when)
    //   all replicas have been deleted.  (In this case, you must *not* call "Medium::close()" on the "JpegStreamReplicator" object,
    //   unless you never created any replicas from it to begin with.)
    // If "deleteWhenLastReplicaDies" is False, then the "JpegStreamReplicator" object remains in existence, even when all replicas
    //   have been deleted.  (This allows you to create new replicas later, if you wish.)  In this case, you delete the
    //   "JpegStreamReplicator" object by calling "Medium::close()" on it - but you must do so only when "numReplicas()" returns 0.

  MjpegLiveVideoSource* createStreamReplica();

  unsigned numReplicas() const { return fNumReplicas; }

  MjpegLiveVideoSource* inputSource() const { return fInputSource; }

  // Call before destruction if you want to prevent the destructor from closing the input source
  void detachInputSource() { fInputSource = NULL; }

protected:
  JpegStreamReplicator(UsageEnvironment& env, MjpegLiveVideoSource *inputSource, Boolean deleteWhenLastReplicaDies);
    // called only by "createNew()"
  virtual ~JpegStreamReplicator();

private:
  // Routines called by replicas to implement frame delivery, and the stopping/restarting/deletion of replicas:
  friend class JpegStreamReplica;
  void getNextFrame(JpegStreamReplica* replica);
  void deactivateStreamReplica(JpegStreamReplica* replica);
  void removeStreamReplica(JpegStreamReplica* replica);

private:
  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
             struct timeval presentationTime, unsigned durationInMicroseconds);

  static void onSourceClosure(void* clientData);
  void onSourceClosure();

  void deliverReceivedFrame();

private:
  MjpegLiveVideoSource* fInputSource{nullptr};
  Boolean fDeleteWhenLastReplicaDies, fInputSourceHasClosed; 
  unsigned fNumReplicas, fNumActiveReplicas, fNumDeliveriesMadeSoFar;
  int fFrameIndex; // 0 or 1; used to figure out if a replica is requesting the current frame, or the next frame

  JpegStreamReplica* fPrimaryReplica; // the first replica that requests each frame.  We use its buffer when copying to the others.
  JpegStreamReplica* fReplicasAwaitingCurrentFrame; // other than the 'primary' replica
  JpegStreamReplica* fReplicasAwaitingNextFrame; // replicas that have already received the current frame, and have asked for the next
};
#endif
