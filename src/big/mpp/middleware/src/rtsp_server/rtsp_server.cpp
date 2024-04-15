#include <functional>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <atomic>
#include <thread>
#include <map>

#include "rtsp_server.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "LiveServerMediaSession.h"
#include "h264LiveFrameSource.h"
#include "h265LiveFrameSource.h"
#include "g711LiveFrameSource.h"
#include "mjpegLiveFrameSource.h"
#include "mjpegMediaSubSession.h"
#include "mjpegStreamReplicator.h"
#include "g711BackChannelServerMediaSubsession.h"

class OnBackChannel: public IOnData {
  public:
    explicit OnBackChannel(const std::string &name, IOnBackChannel *back_channel): name_(name), back_channel_(back_channel) {}
    virtual void OnData(const unsigned char *data, unsigned dataSize, struct timeval presentationTime);
  private:
    std::string name_;
    IOnBackChannel *back_channel_{nullptr};
};

struct SessionInfo {
    // video,  FIXME
    LiveFrameSource *h26x_source = nullptr;
    StreamReplicator *h26x_replicator = nullptr;
    MjpegLiveVideoSource *jpeg_source = nullptr;
    JpegStreamReplicator *jpeg_replicator = nullptr;
    // audio
    LiveFrameSource *g711_source = nullptr;
    StreamReplicator *g711_replicator = nullptr;
    // backchannel
    std::shared_ptr<IOnData> back_channel = nullptr;
};

static std::mutex session_info_map_mutex_;
static std::map<std::string, SessionInfo> session_info_map_;

void OnBackChannel::OnData(unsigned char const* data, unsigned size, struct timeval presentationTime) {
    if (back_channel_) {
        uint64_t ms = presentationTime.tv_sec * 1000 + presentationTime.tv_usec/1000;
        // std::cout << "OnBackChannel::OnData : size = " << size << ", timestamp = " << ms << std::endl;
        back_channel_->OnBackChannelData(name_, data, size, ms);
    }
}

class KdRtspServer::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(Port port = 8554, IOnBackChannel *back_channel = nullptr);
    void DeInit();

    int CreateSession(const std::string &session_name, const SessionAttr &session_attr);
    int DestroySession(const std::string &session_name);
    void Start();
    void Stop();

    int SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);
    int SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp);

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    void announceStream(ServerMediaSession* sms, char const* streamName);

  private:
    TaskScheduler *scheduler_{nullptr};
    UsageEnvironment* env_{nullptr};
    RTSPServer *rtspServer_{nullptr};
    volatile char watchVariable_{0};
    std::thread server_loop_;
    IOnBackChannel *back_channel_{nullptr};
};

int KdRtspServer::Impl::Init(Port port, IOnBackChannel *back_channel) {
    scheduler_ = BasicTaskScheduler::createNew();
    env_ = BasicUsageEnvironment::createNew(*scheduler_);
    UserAuthenticationDatabase* authDB = nullptr;
    unsigned reclamationSeconds = 10;
    rtspServer_ = RTSPServer::createNew(*env_, port, authDB, reclamationSeconds);
    if (!rtspServer_) {
        *env_ << "create rtsp server failed." << env_->getResultMsg() << "\n";
        return -1;
    }
    back_channel_ = back_channel;
    return 0;
}

void KdRtspServer::Impl::DeInit() {
    if (rtspServer_) {
      Medium::close(rtspServer_);
      env_->reclaim();
      delete scheduler_;
      env_ = nullptr;
      scheduler_ = nullptr;
      rtspServer_ = nullptr;
    }
}

int KdRtspServer::Impl::CreateSession(const std::string &session_name, const SessionAttr &session_attr) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        *env_ << "stream session has already been created\n";
        return 0;
    }
    lck.unlock();
    if (!session_attr.with_video && !session_attr.with_audio && !session_attr.with_audio_backchannel)  {
        *env_ << "no subSessions\n";
        return -1;
    }

    char const* descriptionString = "Session streamed by \"KdRTSPServer\"";
    ServerMediaSession *sms = nullptr;

    // create live-sources and replicators
    SessionInfo info;
    if(session_attr.with_video) {
        if (session_attr.video_type == VideoType::kVideoTypeH264) {
            info.h26x_source = H264LiveFrameSource::createNew(*env_, 8);
            info.h26x_replicator = StreamReplicator::createNew(*env_, info.h26x_source, false);
            if (!info.h26x_replicator) goto err_exit;
        } else if (session_attr.video_type == VideoType::kVideoTypeH265) {
            info.h26x_source = H265LiveFrameSource::createNew(*env_, 8);
            info.h26x_replicator = StreamReplicator::createNew(*env_, info.h26x_source, false);
            if (!info.h26x_replicator) goto err_exit;
        } else if (session_attr.video_type == VideoType::kVideoTypeMjpeg) {
            info.jpeg_source = MjpegLiveVideoSource::createNew(*env_, 8);
            info.jpeg_replicator = JpegStreamReplicator::createNew(*env_, info.jpeg_source, false);
            if (!info.jpeg_replicator) goto err_exit;
        } else {
            *env_ << "video type not supported yet\n";
            goto err_exit;
        }
        std::cout << "with_video" << std::endl;
    }

    if (session_attr.with_audio) {
        std::cout << "with_audio" << std::endl;
        info.g711_source = G711LiveFrameSource::createNew(*env_, 8);
        info.g711_replicator = StreamReplicator::createNew(*env_, info.g711_source, false);
    }

    if (session_attr.with_audio_backchannel) {
        std::cout << "with_audio_backchannel" << std::endl;
        info.back_channel = std::make_shared<OnBackChannel>(session_name, back_channel_);
    }

    // create SMS and subsessions
    sms = ServerMediaSession::createNew(*env_, session_name.c_str(), session_name.c_str(), descriptionString);
    if (info.h26x_replicator) {
        LiveServerMediaSession *h26xliveSubSession = LiveServerMediaSession::createNew(*env_, info.h26x_replicator);
        sms->addSubsession(h26xliveSubSession);
    }

    if (info.jpeg_replicator) {
        MjpegMediaSubsession *jpegliveSubSession = MjpegMediaSubsession::createNew(*env_, info.jpeg_replicator);
        sms->addSubsession(jpegliveSubSession);
    }

    if (info.g711_replicator) {
        LiveServerMediaSession *g711liveSubSession = LiveServerMediaSession::createNew(*env_, info.g711_replicator);
        std::cout << "g711liveSubSession" << std::endl;
        sms->addSubsession(g711liveSubSession);
    }

    if (info.back_channel) {
        G711BackChannelServerMediaSubsession *backChannel = G711BackChannelServerMediaSubsession::createNew(*env_, info.back_channel.get());
        std::cout << "G711BackChannelServerMediaSubsession" << std::endl;
        sms->addSubsession(backChannel);
    }

    rtspServer_->addServerMediaSession(sms);
    announceStream(sms, session_name.c_str());

    lck.lock();
    session_info_map_[session_name] = info;
    return 0;

err_exit:
    if (info.h26x_replicator) Medium::close(info.h26x_replicator);
    if (info.jpeg_replicator) Medium::close(info.jpeg_replicator);
    if (info.g711_replicator) Medium::close(info.g711_replicator);
    if (info.back_channel) info.back_channel.reset();
    return -1;
}


int KdRtspServer::Impl::DestroySession(const std::string &session_name) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    auto iter = session_info_map_.find(session_name);
    if (iter != session_info_map_.end()) {
        if (iter->second.h26x_replicator) Medium::close(iter->second.h26x_replicator);
        if (iter->second.jpeg_replicator) Medium::close(iter->second.jpeg_replicator);
        if (iter->second.g711_replicator) Medium::close(iter->second.g711_replicator);
        if (iter->second.back_channel) iter->second.back_channel.reset();
    }
    session_info_map_.erase(iter);
    return 0;
}

void KdRtspServer::Impl::Start() {
    if (rtspServer_->setUpTunnelingOverHTTP(80) || rtspServer_->setUpTunnelingOverHTTP(8000) || rtspServer_->setUpTunnelingOverHTTP(8080)) {
        *env_ << "\n(We use port " << rtspServer_->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
    } else {
        *env_ << "\n(RTSP-over-HTTP tunneling is not available.)\n";
    }

    watchVariable_ = 0;
    server_loop_ = std::thread([this]() {
        env_->taskScheduler().doEventLoop(&watchVariable_);
    });
}

void KdRtspServer::Impl::Stop() {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    for(auto &it: session_info_map_) {
        if (it.second.h26x_replicator) Medium::close(it.second.h26x_replicator);
        if (it.second.jpeg_replicator) Medium::close(it.second.jpeg_replicator);
        if (it.second.g711_replicator) Medium::close(it.second.g711_replicator);
        if (it.second.back_channel) it.second.back_channel.reset();
    }
    session_info_map_.clear();
    lck.unlock();

    if(server_loop_.joinable()) {
      watchVariable_ = 1;
      server_loop_.join();
    }
}


void KdRtspServer::Impl::announceStream(ServerMediaSession* sms, char const* streamName) {
    char* url = rtspServer_->rtspURL(sms);
    UsageEnvironment& env = rtspServer_->envir();
    env << "\n\"" << streamName << "\" stream " << "\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

int KdRtspServer::Impl::SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        auto &info = session_info_map_[session_name];
        if (info.h26x_source) {
            // TODO， send when there is at least one client
            info.h26x_source->pushData(data, size, timestamp);
        }
        if (info.jpeg_source) {
            // TODO， send when there is at least one client
            info.jpeg_source->pushData(data, size, timestamp);
        }
        return 0;
    }
    return -1;
}

int KdRtspServer::Impl::SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    std::unique_lock<std::mutex> lck(session_info_map_mutex_);
    if (session_info_map_.count(session_name)) {
        auto &info = session_info_map_[session_name];
        if (info.g711_source) {
            // TODO， send when there is at least one client
            info.g711_source->pushData(data, size, timestamp);
        }
        return 0;
    }
    return -1;
}


KdRtspServer::KdRtspServer() : impl_(std::make_unique<Impl>()) {}
KdRtspServer::~KdRtspServer() {}

int KdRtspServer::Init(int port, IOnBackChannel *back_channel) {
    return impl_->Init((Port)port, back_channel);
}

void KdRtspServer::DeInit() {
    impl_->DeInit();
}

int KdRtspServer::CreateSession(const std::string &session_name, const SessionAttr &session_attr) {
    return impl_->CreateSession(session_name, session_attr);
}

int KdRtspServer::DestroySession(const std::string &session_name) {
    return impl_->DestroySession(session_name);
}

void KdRtspServer::Start() {
    impl_->Start();
}

void KdRtspServer::Stop() {
    impl_->Stop();
}

int KdRtspServer::SendVideoData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    return impl_->SendVideoData(session_name, data, size, timestamp);
}

int KdRtspServer::SendAudioData(const std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    return impl_->SendAudioData(session_name, data, size, timestamp);
}
