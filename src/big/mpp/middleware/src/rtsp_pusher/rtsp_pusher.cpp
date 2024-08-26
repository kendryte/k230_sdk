#include <functional>
#include <future>
#include <iostream>
#include <fstream>
#include <memory>
#include <list>
#include <atomic>
#include <thread>
#include <mutex>
#include "rtsp_pusher.h"
#include "RtspPusherImpl.h"
#include "media.h"

#define MAX_LIVE_FRAME_CNT   25
#define MAX_LIVE_FRAME_SIZE  1000000
struct LiveFramePacket
{
    char *sData;
    int  dataLen;
    uint64_t timestamp;
    bool  key_frame;
};

typedef std::list<LiveFramePacket*>   LIVE_FRAME_PACKET_LIST;

class KdRtspPusher::Impl {
  public:
    Impl() {}
    ~Impl() { }

    int  Init(const RtspPusherInitParam &param);
    void DeInit();
    int  Open();
    void Close();

    int PushVideoData(const uint8_t *data, size_t size, bool key_frame,uint64_t timestamp);

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;
    void _init_frame_free_queue();
    void _deinit_frame_free_queue();
    LiveFramePacket* _get_frame_from_free_queue();
    int  _put_frame_to_free_queue(LiveFramePacket* frame_packet);
    static void* push_data_thread(void* pParam);
    int  _do_push_frame_data();

  private:
    RTSPPusherImpl   rtsp_pusher_;
    LIVE_FRAME_PACKET_LIST fLiveFrameFreeQueue_;
    LIVE_FRAME_PACKET_LIST fLiveFrameQueue_;
    std::mutex   fMutexFrame_;
    std::mutex   fMutexFreeFrame_;
    pthread_t    hpushFrameThread_;
    bool         bStartPushFrame_ = false;
    RtspPusherInitParam PusherInitParam_;

};

int KdRtspPusher::Impl::Init(const RtspPusherInitParam &param) {
  printf("rtsp_pusher url:%s\n",param.sRtspUrl);
  PusherInitParam_ = param;
  _init_frame_free_queue();
  rtsp_pusher_.init(param.sRtspUrl,param.video_width,param.video_height);
  return 0;
}

void KdRtspPusher::Impl::DeInit() {
    _deinit_frame_free_queue();
    return ;
}

int KdRtspPusher::Impl::Open() {

  bStartPushFrame_ = true;
  pthread_create(&hpushFrameThread_, NULL, push_data_thread, this);

  return rtsp_pusher_.open();
}

void KdRtspPusher::Impl::Close() {
  if (!bStartPushFrame_)
  {
      return;
  }
  bStartPushFrame_ = false;
  pthread_join(hpushFrameThread_,nullptr);
  rtsp_pusher_.close();
}

int KdRtspPusher::Impl::PushVideoData(const uint8_t *data, size_t size, bool key_frame,uint64_t timestamp) {

  std::unique_lock<std::mutex> lck(fMutexFrame_);
  if (fLiveFrameQueue_.size() >= MAX_LIVE_FRAME_CNT)
  {
      for (LIVE_FRAME_PACKET_LIST::iterator itr = fLiveFrameQueue_.begin();itr != fLiveFrameQueue_.end();itr ++)
      {
          fLiveFrameFreeQueue_.push_back(*itr);
      }
      fLiveFrameQueue_.clear();
      printf("%s fLiveFrameQueue_ is full\n",PusherInitParam_.sRtspUrl);
      return 0;
  }
  if (size > MAX_LIVE_FRAME_SIZE)
  {
      printf("%s  push_venc_data size:%d(max size:%d)\n",PusherInitParam_.sRtspUrl,size,MAX_LIVE_FRAME_SIZE);
      return -1;
  }

  LiveFramePacket *livePacket = _get_frame_from_free_queue();
  if (livePacket == nullptr)
  {
      printf("%s _get_frame_from_free_queue failed:fLiveFrameQueue_ size:%d\n",PusherInitParam_.sRtspUrl,fLiveFrameQueue_.size());
      for (LIVE_FRAME_PACKET_LIST::iterator itr = fLiveFrameQueue_.begin();itr != fLiveFrameQueue_.end();itr ++)
      {
          fLiveFrameFreeQueue_.push_back(*itr);
      }
      fLiveFrameQueue_.clear();
      printf("%s fLiveFrameQueue_ is full2\n",PusherInitParam_.sRtspUrl);
      return -1;
  }

  livePacket->timestamp = timestamp;
  livePacket->dataLen = size;
  livePacket->key_frame = key_frame;
  memcpy(livePacket->sData,data,size);
  fLiveFrameQueue_.push_back(livePacket);
  return 0;
}

void KdRtspPusher::Impl::_init_frame_free_queue()
{
    std::unique_lock<std::mutex> lck(fMutexFreeFrame_);
    for (int i =0;i < MAX_LIVE_FRAME_CNT;i ++)
    {
        LiveFramePacket *frame_packet = new LiveFramePacket();
        frame_packet->dataLen = 0;
        frame_packet->timestamp = 0;
        frame_packet->sData = new char[MAX_LIVE_FRAME_SIZE];
        if (frame_packet->sData == nullptr)
        {
            printf("new frame packet failed\n");
            continue;
        }
        fLiveFrameFreeQueue_.push_back(frame_packet);
    }
}

void KdRtspPusher::Impl::_deinit_frame_free_queue()
{
    std::unique_lock<std::mutex> lck(fMutexFreeFrame_);
    for (LIVE_FRAME_PACKET_LIST::iterator itr = fLiveFrameQueue_.begin();itr != fLiveFrameQueue_.end();itr ++)
    {
        fLiveFrameFreeQueue_.push_back(*itr);
    }
    fLiveFrameQueue_.clear();

    for (LIVE_FRAME_PACKET_LIST::iterator itr = fLiveFrameFreeQueue_.begin();itr != fLiveFrameFreeQueue_.end();itr ++)
    {
        delete[] (*itr)->sData;
        (*itr)->sData = nullptr;
    }
    fLiveFrameFreeQueue_.clear();
}

LiveFramePacket* KdRtspPusher::Impl::_get_frame_from_free_queue()
{
    std::unique_lock<std::mutex> lck(fMutexFreeFrame_);
    if (fLiveFrameFreeQueue_.size() > 0)
    {
        LiveFramePacket* frame_packet = fLiveFrameFreeQueue_.front();
        fLiveFrameFreeQueue_.pop_front();
        return frame_packet;
    }

    return nullptr;
}

int  KdRtspPusher::Impl::_put_frame_to_free_queue(LiveFramePacket* frame_packet)
{
    std::unique_lock<std::mutex> lck(fMutexFreeFrame_);
    if (fLiveFrameFreeQueue_.size() >= MAX_LIVE_FRAME_CNT)
    {
        return -1;
    }
    fLiveFrameFreeQueue_.push_back(frame_packet);
    return 0;
}

int  KdRtspPusher::Impl::_do_push_frame_data()
{
    bool key_frame = false;
    int ncount = 0;
    while(bStartPushFrame_)
    {
        std::unique_lock<std::mutex> lck(fMutexFrame_);
        if (fLiveFrameQueue_.size() > 0)
        {
            if (++ ncount  % 100 == 0)
            {
                printf("[%d]%s fLiveFrameQueue_ size:%d,free framequeue size:%d\n",ncount,PusherInitParam_.sRtspUrl, fLiveFrameQueue_.size(),fLiveFrameFreeQueue_.size());
            }

            LiveFramePacket* live_packet = fLiveFrameQueue_.front();
            fLiveFrameQueue_.pop_front();
            lck.unlock();

            key_frame = live_packet->key_frame;
            rtsp_pusher_.pushVideo(live_packet->sData,live_packet->dataLen,key_frame,live_packet->timestamp);
            _put_frame_to_free_queue(live_packet);

        }
        else
        {
            lck.unlock();
            usleep(1000*5);
        }

    }
    return 0;
}

void* KdRtspPusher::Impl::push_data_thread(void* pParam)
{
    KdRtspPusher::Impl* pthis = (KdRtspPusher::Impl*)pParam;
    pthis->_do_push_frame_data();
    return nullptr;
}



KdRtspPusher::KdRtspPusher() : impl_(std::make_unique<Impl>()) {}
KdRtspPusher::~KdRtspPusher() {}

int KdRtspPusher::Init(const RtspPusherInitParam &param) {
  return impl_->Init(param);
}

void KdRtspPusher::DeInit() {
  return impl_->DeInit();
}

int KdRtspPusher::Open() {
  return impl_->Open();
}

void KdRtspPusher::Close() {
  impl_->Close();
}

int KdRtspPusher::PushVideoData(const uint8_t *data, size_t size, bool key_frame,uint64_t timestamp) {
  return impl_->PushVideoData(data, size,key_frame, timestamp);
}

