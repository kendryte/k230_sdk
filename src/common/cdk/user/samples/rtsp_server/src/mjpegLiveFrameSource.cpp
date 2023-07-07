#include <iostream>
#include <MediaSink.hh>
#include "mjpegLiveFrameSource.h"

MjpegLiveVideoSource *MjpegLiveVideoSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new MjpegLiveVideoSource(env, queue_size);
}

MjpegLiveVideoSource::MjpegLiveVideoSource(UsageEnvironment &env, size_t queue_size) :
    JPEGVideoSource(env), fQueueSize(queue_size) {
    OutPacketBuffer::maxSize = 512000;
    fEventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    fThread = std::thread([this](){
        while(this->fNeedReadFrame) {
            this->getFrame();
            usleep(1000 * 10); // FIXME
        }
    });
}

MjpegLiveVideoSource::~MjpegLiveVideoSource() {
    fNeedReadFrame.store(false);
    if(fThread.joinable()) {
        fThread.join();
    }
    while (!fFramePacketQueue.empty()) {
        fFramePacketQueue.pop_front();
    }
    while (!fRawDataQueue.empty()) {
        fRawDataQueue.pop_front();
    }
    if(fEventTriggerId) {
        envir().taskScheduler().deleteEventTrigger(fEventTriggerId);
        fEventTriggerId = 0;
    }
}

template <typename T>
std::shared_ptr<T> make_shared_array(size_t size) {
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}
void MjpegLiveVideoSource::pushData(const uint8_t *data, size_t data_size, uint64_t timestamp) {
    std::shared_ptr<uint8_t> buf = make_shared_array<uint8_t>(data_size);
    memcpy(buf.get(), data, data_size); 

    RawData raw_data;
    raw_data.buffer_ = buf;
    raw_data.size_ = data_size;
    raw_data.timestamp_ = timestamp;
    std::unique_lock<std::mutex> lck(fMutexRaw);
    fRawDataQueue.push_back(raw_data);
}

int MjpegLiveVideoSource::getFrame() {
    RawData raw_data;
    std::unique_lock<std::mutex> lck(fMutexRaw);
    if (!fRawDataQueue.empty()) {
        raw_data = fRawDataQueue.front();
        fRawDataQueue.pop_front();
    }
    lck.unlock();

    int frameSize = 0;
    if (raw_data.buffer_ && raw_data.size_) {
        // use system-time when getFrame() called as PresentationTime
        struct timeval ref;
        gettimeofday(&ref, NULL);
        frameSize = raw_data.size_;
        processFrame(raw_data.buffer_, frameSize, ref);
    }
    return frameSize;
}

void MjpegLiveVideoSource::processFrame(std::shared_ptr<uint8_t> data, size_t size, const struct timeval &ref) {
    if (fParser.parse(const_cast<uint8_t*>(data.get()), size) == 0) {
        unsigned int len = 0;
        const uint8_t *frame_bits = fParser.scandata(len);
        FramePacket packet(data, frame_bits - data.get(), len, ref);
        queueFramePacket(packet);
    }
}

void MjpegLiveVideoSource::queueFramePacket(MjpegLiveVideoSource::FramePacket &packet) {
    std::unique_lock<std::mutex> lck(fMutex);
    while (fFramePacketQueue.size() >= fQueueSize) {
        fFramePacketQueue.pop_front();
    }
    fFramePacketQueue.push_back(packet);
    lck.unlock();
    // post an event to ask to deliver the frame
    envir().taskScheduler().triggerEvent(fEventTriggerId, this);
}

void MjpegLiveVideoSource::doGetNextFrame() {
    deliverFrame();
}

void MjpegLiveVideoSource::doStopGettingFrames() {
    FramedSource::doStopGettingFrames();
}

void MjpegLiveVideoSource::deliverFrame0(void *clientData) {
    ((MjpegLiveVideoSource*)clientData)->deliverFrame();
}

void MjpegLiveVideoSource::deliverFrame() {
    if (isCurrentlyAwaitingData()) {
        fDurationInMicroseconds = 0;
        fFrameSize = 0;

        FramePacket packet;
        std::unique_lock<std::mutex> lck(fMutex);
        if (!fFramePacketQueue.empty()) {
            packet = fFramePacketQueue.front();
            fFramePacketQueue.pop_front();
        }
        lck.unlock();

        if(packet.size_) {
            if (packet.size_ > fMaxSize) {
                fFrameSize = fMaxSize;
                fNumTruncatedBytes = packet.size_ - fMaxSize;
                std::cout << "MjpegLiveVideoSource::deliverFrame() -- truncate bytes " << fNumTruncatedBytes << std::endl;
            } else {
                fFrameSize = packet.size_;
            }

            fPresentationTime = packet.timestamp_;
            memcpy(fTo, packet.buffer_.get() + packet.offset_, fFrameSize);
        }

        if (fFrameSize > 0) {
            FramedSource::afterGetting(this);
            // envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
        }
    }
}
