#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "comm_server.h"
#include "mpi_vb_api.h"
#include "util.h"

// demonstrates cowork of comm-service and mapi-service

using namespace std::chrono_literals;

class MyCommServer : public IServerCallback {
  public:
    MyCommServer() : receive_audio_queue_(30) {}

    // IServerCallback
    virtual void OnMessage(const UserMessage *message) override {
        UserMsgType type = static_cast<UserMsgType>(message->type);
        switch(type) {
        case UserMsgType::CLIENT_READY: {
            std::cout << "OnMessage() -- CLIENT_READY" << std::endl;
            client_ready_.store(true);
            exit_msg_.store(false);
            break;
        }
        case UserMsgType::KEY_PRESSED: {
            std::cout << "OnMessage() -- KEY_PRESSED" << std::endl;
            key_pressed_.store(true);
            break;
        }
        case UserMsgType::PLAYBACK : {
            std::cout << "OnMessage() -- PLAYBACK" << std::endl;
            palyback_.store(true);
            break;
        }
        case UserMsgType::EXIT : {
            std::cout << "OnMessage() -- EXIT" << std::endl;
            exit_msg_.store(true);
            client_ready_.store(false);
            break;
        }
        default: {
            std::cout << "OnMessage() -- unknown message : " << message->type << std::endl;
            break;
        }
        }
    }
    // For backchannel audio
    virtual void OnAEncFrameData(const AVEncFrameData &data) override {
        // std::cout << "OnAEncFrameData() -- data.size = " << data.size << std::endl;
        AVEncFrameData aframe;
        aframe.flags = data.flags;
        aframe.keyframe = data.keyframe;
        aframe.sequence = data.sequence;
        aframe.size = data.size;
        aframe.timestamp_ms = data.timestamp_ms;
        aframe.type = data.type;
        aframe.data = new uint8_t[data.size];
        memcpy(aframe.data, data.data, data.size);

        receive_audio_queue_.put(aframe);
    }

    //
    int Init() {
        return server_.Init("peephole", this);
    }
    void DeInit() {
        server_.DeInit();
    }
    int SendEvent(const UserEventData &event, IInterruptCallback *cb) {
        return server_.SendEvent(event, cb);
    }
    int SendVideoData(const AVEncFrameData &data, IInterruptCallback *cb) {
        return server_.SendVideoData(data, cb);
    }
    int SendAudioData(const AVEncFrameData &data, IInterruptCallback * cb) {
        return server_.SendAudioData(data, cb);
    }

    int GetAudioFrame(AVEncFrameData &aframe_data) {
        if (receive_audio_queue_.empty()) {
            return false;
        }
        receive_audio_queue_.take(aframe_data);

        return true;
    }

    bool ClientReady() {
        if (!client_ready_) {
            return false;
        }

        client_ready_.store(false);
        return true;
    }

    bool GetExitMsg() {
        if (!exit_msg_) {
            return false;
        }

        return true;
    }

    bool GetKeyPressed() {
        if (!key_pressed_) {
            return false;
        }

        key_pressed_.store(false);
        return true;
    }

    bool GetPlayback() {
        if (!palyback_) {
            return false;
        }

        return true;
    }

  private:
    UserCommServer server_;
    BufThreadQueue<AVEncFrameData> receive_audio_queue_;
    std::atomic<bool> client_ready_{false};
    std::atomic<bool> exit_msg_ {false};
    std::atomic<bool> key_pressed_ {false};
    std::atomic<bool> palyback_ {false};
};
