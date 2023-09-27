#include <iostream>
#include <atomic>
#include <chrono>
#include <signal.h>
#include <thread>
#include "comm_client.h"

// demonstrates cowork of comm-service and mapi-service
#define ENABLE_MAPI  1
#ifdef ENABLE_MAPI
  #include "media.h"
#endif

using namespace std::chrono_literals;
std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
}

class MyApp : public IClientCallback
#ifdef ENABLE_MAPI
             , public IOnAEncData
#endif
{
  public:
    MyApp() {}

    // IClientCallback
    virtual void OnMessage(const UserMessage *message) override {
        UserMsgType type = static_cast<UserMsgType>(message->type);
        switch(type) {
        case UserMsgType::SERVER_MSG_BASE: {
            std::cout << "MyApp::OnMessage() -- SERVER_MSG_BASE" << std::endl;
            std::cout << message->data << std::endl; // "hello,world"
            break;
        }
        default: {
            std::cout << "MyApp::OnMessage() -- unknown message : " << message->type << std::endl;
            break;
        }
        }
    }
    virtual void OnEvent(const UserEventData &event) override {
        switch(event.type) {
        case UserEventData::EventType::PIR_WAKEUP:{
            std::cout << "MyApp::OnEvent() -- PIR_WAKEUP" << std::endl;
            std::cout << "MyApp::OnEvent() -- addr = " << (long)event.jpeg << ", size = " << event.jpeg_size << std::endl;
            for(int i = 0; i < event.jpeg_size; i++) {
                if (event.jpeg[i] != (i & 0xff)) {
                    std::cout << "OnEvent::PIR_WAKEUP, check data failed" << std::endl;
                }
            }
            std::cout << "OnEvent::PIR_WAKEUP, check data done" << std::endl;
            break;
        }
        case UserEventData::EventType::KEY_WAKEUP:{
            std::cout << "MyApp::OnEvent() -- KEY_WAKEUP" << std::endl;
            break;
        }
        case UserEventData::EventType::STAY_ALARM:{
            std::cout << "MyApp::OnEvent() -- KEY_WAKEUP" << std::endl;
            break;
        }
        default: {
            std::cout << "MyApp::OnEvent() -- unknown message : " << static_cast<int>(event.type) << std::endl;
            break;
        }
        }
    }
    virtual void OnVEncFrameData(const AVEncFrameData &data) override {
        std::cout << "MyApp::OnVEncFrameData() -- called" << std::endl;
        std::cout << "MyApp::OnVEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
        for(int i = 0; i < data.size; i++) {
            if (data.data[i] != (i & 0xff)) {
                std::cout << "OnVEncFrameData, check data failed" << std::endl;
            }
        }
        std::cout << "OnVEncFrameData, check data done" << std::endl;
    }
    virtual void OnAEncFrameData(const AVEncFrameData &data) override {
        std::cout << "MyApp::OnAEncFrameData() -- called" << std::endl;
        std::cout << "MyApp::OnAEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
        for(int i = 0; i < data.size; i++) {
            if (data.data[i] != (i & 0xff)) {
                std::cout << "OnAEncFrameData, check data failed" << std::endl;
            }
        }
        std::cout << "MyApp, check data done" << std::endl;
    }
#ifdef ENABLE_MAPI
    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) override {
        // std::cout << "OnAEncData -- size  = " << stream_data->len << ", timestamp = " << stream_data->time_stamp << std::endl;
        // loopback, just for test
        if(!started_) return;
        media_.SendAudioData((const uint8_t*)stream_data->stream, stream_data->len, stream_data->time_stamp);
    }
#endif
    //
    int Init() {
        // it seems other services must be started before mapi-service, it's weired...
        if (client_.Init("peephole", this) < 0) return -1;
        if (client_.Start() < 0) return -1;
#ifdef ENABLE_MAPI
        if (media_.Init() < 0) return -1;
#endif
        return 0;
    }

#ifdef ENABLE_MAPI
    int Start() {
        if(started_) return 0;
        if (media_.CreateADecAo() < 0) return -1;
        media_.StartADecAo();
        if (media_.CreateAiAEnc(this) < 0) return -1;
        media_.StartAiAEnc();
        started_ = true;
        return 0;
    }
    int Stop() {
        if (!started_) return 0;
        started_ = false;
        media_.StopAiAEnc();
        media_.DestroyAiAEnc();
        media_.StopADecAo();
        media_.DestroyADecAo();
        return 0;
    }
#endif

    int DeInit() {
        client_.Stop();
        client_.DeInit();
#ifdef ENABLE_MAPI
        Stop();
        media_.Deinit();
#endif
        return 0;
    }
    int EnterPlaybackMode() {
        return client_.EnterPlaybackMode();
    }
    int KeyPressed() {
        return client_.KeyPressed();
    }

    // for test, demonstrate how to send audio bitstream data
    class Interrupt : public IInterruptCallback {
      public:
        explicit Interrupt(bool nonblock) : nonblock_(nonblock) {}
        virtual bool Exit() override { return nonblock_; }
      private:
        bool nonblock_{false};
    };
    void test_send_aenc_data() {
        std::cout << "test send audio bitstream to client " << std::endl;
        uint8_t *bits_buf = tmp_buf;
        for(int i = 0; i < 320; i++) bits_buf[i] = (i &0xff);
        AVEncFrameData data;
        data.data = bits_buf;
        data.size = 320;
        data.keyframe = false;
        data.timestamp_ms = a_sequence_ * 40;
        data.type = AVEncFrameData::Type::PCMU;
        data.sequence = a_sequence_ ++;
        data.flags = 0;

        Interrupt nonblock(true);
        Interrupt block(false);
        int ret;
        ret = client_.SendAudioData(data, &nonblock);
        if (ret < 0) {
            std::cout << "test send aenc data to client -- nonblock, failed" << std::endl;
        }
        ret = client_.SendAudioData(data, &block);
        if (ret < 0) {
            std::cout << "test send aenc data to client -- block, failed" << std::endl;
        }
        std::cout << "test send aenc data to client -- done" << std::endl;
    }
  private:
    UserCommClient client_;
#ifdef ENABLE_MAPI
    KdMedia media_;
#endif
    std::atomic<bool> started_{false};
    uint8_t tmp_buf[1024];
    uint32_t a_sequence_{0};
};

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    MyApp *app = new MyApp();
    if (!app || app->Init() < 0) {
        std::cout << "MyApp Init failed." << std::endl;
        return -1;
    }
    
    while (!g_exit_flag) {
        int c = getchar();
        switch(c) {
        case 'q' : {
#ifdef ENABLE_MAPI
            app->Stop();
#endif
            g_exit_flag.store(true);
            break;
        }
        case 't' : {
            std::cout << "MyApp-- send keypressed & playback  message to server" << std::endl;
            app->KeyPressed();
            app->EnterPlaybackMode();
            break;
        }
        case 'a' : {
            std::cout << "MyApp-- send aenc data to server" << std::endl;
            app->test_send_aenc_data();
            break;
        }
#ifdef ENABLE_MAPI
        case 'S' : {
            std::cout << "MyApp-- start audio loopback" << std::endl;
            app->Start();
            break;
        }
        case 'P' : {
            std::cout << "MyApp-- stop audio loopback" << std::endl;
            app->Stop();
        }
#endif
        default: break;
        }
        std::this_thread::sleep_for(100ms);
    }

    app->DeInit();
    delete app;
    return 0;
}
