#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "comm_server.h"
#include "mpi_vb_api.h"

// demonstrates cowork of comm-service and mapi-service

#define ENABLE_MAPI   1
#ifdef ENABLE_MAPI
  #include "mapi_sys_api.h"
#endif

using namespace std::chrono_literals;

class MyApp : public IServerCallback {
  public:
    MyApp() {}

    // IServerCallback
    virtual void OnMessage(const UserMessage *message) override {
        UserMsgType type = static_cast<UserMsgType>(message->type);
        switch(type) {
        case UserMsgType::CLIENT_READY: {
            std::cout << "MyApp::OnMessage() -- CLIENT_READY" << std::endl;
            client_ready_.store(true);
            break;
        }
        case UserMsgType::KEY_PRESSED: {
            std::cout << "MyApp::OnMessage() -- KEY_PRESSED" << std::endl;
            break;
        }
        case UserMsgType::PLAYBACK : {
            std::cout << "MyApp::OnMessage() -- PLAYBACK" << std::endl;
            break;
        }
        case UserMsgType::EXIT : {
            std::cout << "MyApp::OnMessage() -- EXIT" << std::endl;
            client_ready_.store(false);
            break;
        }
        default: {
            std::cout << "MyApp::OnMessage() -- unknown message : " << message->type << std::endl;
            break;
        }
        }
    }
    // For local bidirection speech
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

    //
    int Init() {
        return server_.Init("peephole", this);
    }
    void DeInit() {
        server_.DeInit();
    }
    int SendEvent(const UserEventData &event) {
        return server_.SendEvent(event);
    }
    int SendVideoData(const AVEncFrameData &data) {
        return server_.SendVideoData(data);
    }
    int SendAudioData(const AVEncFrameData &data) {
        return server_.SendAudioData(data);
    }

    // for test, demonstrate how to send message to client
    void test_notify() {
        std::cout << "test send message(hello,world) to client " << std::endl;
        if (!client_ready_) return;
        uint8_t buf[128];
        UserMessage *msg = (UserMessage*)buf;
        msg->type = static_cast<int>(UserMsgType::SERVER_MSG_BASE);
        msg->len = snprintf(msg->data, sizeof(buf) - 8,"%s", "hello,world");
        msg->data[msg->len] = '\0';
        server_.SendMessage(buf, sizeof(UserMessage) + msg->len + 1);
    }

    //
    class Interrupt : public IInterruptCallback {
      public:
        explicit Interrupt(bool nonblock) : nonblock_(nonblock) {}
        virtual bool Exit() override { return nonblock_; }
      private:
        bool nonblock_{false};
    };
    // for test, demonstrate how to send event
    void test_send_event() {
        std::cout << "test send event to client " << std::endl;
        if (!client_ready_) return;
        Interrupt nonblock(true);
        for(int k = 0; k < 32; k++) {
            uint8_t *jpeg_buf = tmp_buf;
            for(int i = 0; i < 100 + k; i++) jpeg_buf[i] = i;
            UserEventData event;
            event.type = UserEventData::EventType::PIR_WAKEUP;
            event.timestamp_ms = 1000;
            event.jpeg = jpeg_buf;
            event.jpeg_size = 100 + k;
            int ret, count = 0;
            while((ret = server_.SendEvent(event, &nonblock)) > 0 && (++count < 20)) usleep(1000);
            if (ret < 0) {
                std::cout << "test send event to client, failed" << std::endl;
                break;
            }
            usleep(1000);
        }
        std::cout << "test send event to client, done" << std::endl;
    }
    // for test, demonstrate how to send video bitstream data
    void test_send_venc_data() {
        std::cout << "test send video bitstream to client " << std::endl;
        if (!client_ready_) return;
        uint8_t *bits_buf = tmp_buf;
        for(int i = 0; i < 100; i++) bits_buf[i] = i ;
        AVEncFrameData data;
        data.data = bits_buf;
        data.size = 100;
        data.keyframe = false;
        data.timestamp_ms = 1001;
        data.type = AVEncFrameData::Type::H265;
        data.sequence = v_sequence_ ++;
        data.flags = 0;
        int ret = server_.SendVideoData(data);
        if (ret < 0) {
            std::cout << "test send venc data to client, failed" << std::endl;
            return;
        }
        std::cout << "test send venc data to client -- done" << std::endl;
    }
    // for test, demonstrate how to send audio bitstream data
    void test_send_aenc_data() {
        std::cout << "test send audio bitstream to client " << std::endl;
        if (!client_ready_) return;
        uint8_t *bits_buf = tmp_buf;
        for(int i = 0; i < 100; i++) bits_buf[i] = i ;
        AVEncFrameData data;
        data.data = bits_buf;
        data.size = 100;
        data.keyframe = false;
        data.timestamp_ms = 1002;
        data.type = AVEncFrameData::Type::PCMU;
        data.sequence = a_sequence_ ++;
        data.flags = 0;
        int ret = server_.SendAudioData(data);
        if (ret < 0) {
            std::cout << "test send aenc data to client, failed" << std::endl;
            return;
        }
        std::cout << "test send aenc data to client -- done" << std::endl;
    }

  private:
    UserCommServer server_;
    std::atomic<bool> client_ready_{false};
    uint8_t tmp_buf[1024];
    uint32_t v_sequence_{0};
    uint32_t a_sequence_{0};
};

/* vb is required for transfering audio/video data as shared-memory,
*  private vb-pools are used in UserCommServer
*/
int main(int argc, char *argv[]) {
#ifndef ENABLE_MAPI
    k_vb_config config;
    memset(&config, 0, sizeof(config));
    config.max_pool_cnt = 64;  // some private vb-pools will be created as well
    // setup common-pool for audio & video, TODO
    int audio_sample_rate_ = 8000;
    int AUDIO_PERSEC_DIV_NUM = 25;
    config.comm_pool[0].blk_cnt = 150;
    config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_cnt = 2;
    config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    config.comm_pool[1].mode = VB_REMAP_MODE_NONE;
    if (kd_mpi_vb_set_config(&config) != K_SUCCESS) {
        std::cout << "kd_mpi_vb_set_config failed." << std::endl;
        return -1;
    }

    if (kd_mpi_vb_init() != K_SUCCESS) {
        std::cout << "kd_mpi_vb_init failed." << std::endl;
        return -1;
    }
#else
    k_s32 ret;
    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error:%d\n", ret);
        return -1;
    }

    k_mapi_media_attr_t media_attr;
    memset(&media_attr, 0, sizeof(media_attr));
    media_attr.media_config.vb_config.max_pool_cnt = 64;
    // setup common-pool for audio & video, TODO
    int audio_sample_rate_ = 8000;
    int AUDIO_PERSEC_DIV_NUM = 25;
    media_attr.media_config.vb_config.comm_pool[0].blk_cnt = 150;
    media_attr.media_config.vb_config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    media_attr.media_config.vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    media_attr.media_config.vb_config.comm_pool[1].blk_cnt = 2;
    media_attr.media_config.vb_config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2;
    media_attr.media_config.vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mapi_media_init error." << std::endl;
        kd_mapi_sys_deinit();
        return ret;
    }
#endif

    MyApp *app = new MyApp();
    if (!app || app->Init() < 0) {
        std::cout << "MyApp Init failed." << std::endl;
        return -1;
    }

    while (1) {
        int c = getchar();
        if(c == 'q') break;
        if(c == 't') app->test_notify();
        if(c == 'e') app->test_send_event();
        if(c == 'v') app->test_send_venc_data();
        if(c == 'a') app->test_send_aenc_data();
        std::this_thread::sleep_for(100ms);
    }

    std::cout << " 'q' received, exit ..." << std::endl;
    app->DeInit();
    delete app;

#ifndef ENABLE_MAPI
    if (kd_mpi_vb_exit() != K_SUCCESS) {
        std::cout << "kd_mpi_vb_exit failed." << std::endl;
    }
#else
    kd_mapi_media_deinit();
    ret = kd_mapi_sys_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit error:%d\n", ret);
    }
#endif
    return 0;
}
