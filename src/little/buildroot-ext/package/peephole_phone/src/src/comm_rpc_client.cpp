#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include "comm_rpc_client.h"
#include "rest_rpc.hpp"

using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

struct ServerInfoImpl {
    bool has_speech = false;
    std::string speech_stream_name;
    int speech_service_port = 8554;
    MSGPACK_DEFINE(has_speech, speech_stream_name, speech_service_port);
};

struct UserEventDataImpl {
    int32_t type{-1};
    int64_t timestamp_ms{0};
    msgpack::type::raw_ref jpeg;
    MSGPACK_DEFINE(type, timestamp_ms, jpeg);
};

class RpcClient::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(IRpcClientCallback *callback, const std::string &dev_ip, unsigned short port) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!callback || dev_ip.empty()) return -1;
        callback_ = callback;
        dev_ip_addr_ = dev_ip;
        port_ = port;

        rpc_client_ = std::make_shared<rpc_client>();
        if (CheckAndConnect() < 0) return -1;
        rpc_client_->subscribe("event_notify", std::bind(&RpcClient::Impl::OnEvent, this, std::placeholders::_1));

        ServerInfoImpl info = rpc_client_->call<ServerInfoImpl>("get_server_info");
        ServerInfo cb_info;
        cb_info.has_bidirection_speech = info.has_speech;
        cb_info.speech_stream_name = info.speech_stream_name;
        cb_info.speech_service_port = info.speech_service_port;
        callback_->OnServerInfo(cb_info);
        return 0;
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        if(rpc_client_) {
            rpc_client_.reset(), rpc_client_ = nullptr;
        }
    }

    int DevPowerOff() {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!rpc_client_) return -1;
        if (CheckAndConnect() < 0) return -1;
        return rpc_client_->call<int>("power_off");
    }

  private:
    void OnEvent(string_view data) {
        if(!callback_) return;
        msgpack_codec codec;
        UserEventDataImpl e = codec.unpack<UserEventDataImpl>(data.data(), data.size());

        UserEventData event;
        event.type = static_cast<UserEventData::EventType>(e.type);
        event.timestamp_ms = e.timestamp_ms;
        event.jpeg_size = e.jpeg.size;
        event.jpeg = reinterpret_cast<uint8_t*>(const_cast<char *>(e.jpeg.ptr));
        callback_->OnEvent(event);
    }

  private:
    int CheckAndConnect(int timeout_seconds = 2) {
        if(rpc_client_->has_connected()) return 0;
        rpc_client_->enable_auto_reconnect(); // automatic reconnect
        rpc_client_->enable_auto_heartbeat(); // automatic heartbeat
        bool r = rpc_client_->connect(dev_ip_addr_, port_);
        int count = 0;
        while (true) {
            if (rpc_client_->has_connected()) {
                std::cout << "connected ok\n";
                break;
            } else {
                std::cout << "connected failed: " << count++ << "\n";
                if (count > timeout_seconds) {
                    std::cout << "Connect failed" << std::endl;
                    return -1;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        return 0;
    }

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::mutex mutex_;
    std::shared_ptr<rpc_client> rpc_client_{nullptr};
    IRpcClientCallback *callback_{nullptr};
    std::string dev_ip_addr_;
    unsigned short port_{9000};
};

RpcClient::RpcClient() : impl_(std::make_unique<Impl>()) {}
RpcClient::~RpcClient() {}

int RpcClient::Init(IRpcClientCallback *callback, const std::string &dev_ip, unsigned short port) {
    return impl_->Init(callback, dev_ip, port);
}

void RpcClient::DeInit() {
    impl_->DeInit();
}

int RpcClient::DevPowerOff() {
    return impl_->DevPowerOff();
}
