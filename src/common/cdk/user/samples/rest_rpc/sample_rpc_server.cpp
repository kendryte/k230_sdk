#include <iostream>
#include <atomic>
#include "comm_rpc_server.h"

class MyApp : public IRpcServerCallback {
  public:
    MyApp() {}

    int Init(unsigned short port) {
        return rpc_server_.Init(this, port);
    }

    void DeInit() {
        rpc_server_.DeInit();
    }

    int SendEvent(const UserEventData &event) {
        if (!connected_)  return -1;
        return rpc_server_.SendEvent(event);
    }

    // IRpcServerCallback
    virtual void OnNewClient(int64_t conn_id, const std::string &remote_ip) override {
        std::cout << "MyApp::OnNewClient(): conn_id = " << conn_id << ", remote_ip = " << remote_ip << std::endl;
        connected_.store(true);
    }

    virtual void OnDisconnect(int64_t conn_id, const std::string &reason) override {
        std::cout << "MyApp::OnDisconnect(): conn_id = " << conn_id << ", reason = " << reason << std::endl;
        connected_.store(false);
    }

    virtual void OnGetInfo(ServerInfo &info) override {
        // fill server info
        info.has_bidirection_speech = true;
        info.speech_stream_name = "peephole_speech";
        info.speech_service_port = 8554;
    }

    virtual void OnRequest(const RpcRequest &req) override {
        if (req.type == RpcRequest::Type::POWER_OFF) {
            std::cout << "MyApp::OnRequest(): POWER_OFF" << std::endl;
        }
    }

    int test_send_event() {
        uint8_t buf[256];
        static size_t size = 100;
        if (size++ > 200) size = 100;
        for (size_t i = 0; i < size; i++) buf[i] = (i & 0xff);

        UserEventData event;
        event.type = UserEventData::EventType::PIR_WAKEUP;
        event.timestamp_ms = 10000 + size;
        event.jpeg = buf;
        event.jpeg_size = size;
        return SendEvent(event);
    }

  private:
    RpcServer rpc_server_;
    std::atomic<bool> connected_{true};
};

int main() {
    MyApp app;
    if (app.Init(9000) < 0) {
      return -1;
    }

    while(1) {
        int c = getchar();
        if (c == 'q') break;
        if (c == 'e') {
            std::cout << "send event to rpc_client" << std::endl;
            app.test_send_event();
            std::cout << "send event to rpc_client, done" << std::endl;
        }
    }

    app.DeInit();
    return 0;
}
