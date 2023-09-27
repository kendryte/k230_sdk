#include <iostream>
#include <atomic>
#include "comm_rpc_client.h"

class MyApp : public IRpcClientCallback {
  public:
    MyApp() {}

    int Init(const std::string &ipaddr, unsigned short port) {
        return rpc_client_.Init(this, ipaddr, port);
    }

    void DeInit() {
        rpc_client_.DeInit();
    }

    int DevPowerOff() {
        return rpc_client_.DevPowerOff();
    }

    // IRpcClientCallback
    virtual void OnServerInfo(const ServerInfo &info) override {
        std::cout << "MyApp::OnServreInfo : has_speech = " << info.has_bidirection_speech << std::endl;
        std::cout << "MyApp::OnServreInfo : stream name = " << info.speech_stream_name << std::endl;
        std::cout << "MyApp::OnServreInfo : port =  " << info.speech_service_port << std::endl;
    }

    virtual void OnEvent(const UserEventData &event) override {
        UserEventData::EventType type = static_cast<UserEventData::EventType>(event.type);
        switch(type) {
        case UserEventData::EventType::PIR_WAKEUP: {
            std::cout << "MyApp::OnEvent() -- PIR_WAKEUP" << std::endl;
            break;
        }
        case UserEventData::EventType::KEY_WAKEUP: {
            std::cout << "MyApp::OnEvent() -- PIR_WAKEUP" << std::endl;
            break;
        }
        case UserEventData::EventType::STAY_ALARM: {
            std::cout << "MyApp::OnEvent() -- STAY_ALARM" << std::endl;
            break;
        }
        default: {
            std::cout << "MyApp::OnEvent() -- type not supported yet , type =" << static_cast<int>(event.type) << std::endl;
            return;
        }
        }

        std::cout << "MyApp::OnEvent() -- timestamp = " << event.timestamp_ms << std::endl;
        // check data
        std::cout << "MyApp::OnEvent() -- checking data, size = " << event.jpeg_size << " ... ";
        for (size_t i = 0; i < event.jpeg_size; i++){
            if (event.jpeg[i] != (i & 0xff)) {
                std::cout << "data error" << std::endl;
                return;
            }
        }
        std::cout << "done" << std::endl;
    }

  private:
    RpcClient rpc_client_;
    std::atomic<bool> connected_{false};
};

int main(int argc, char *argv[]) {
    std::string server_ip = "127.0.0.1";
    if (argc > 1) {
        server_ip = argv[1];
    }
    MyApp app;
    if (app.Init(server_ip, 9000) < 0) {
        return -1;
    }
    while(1) {
        int c = getchar();
        if (c == 'q') break;
        if (c == 'o') {
          std::cout << "send dev power off message" << std::endl;
          app.DevPowerOff();
        }
    }
    app.DeInit();
    return 0;
}
