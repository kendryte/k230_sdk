#include <iostream>
#include <thread>
#include <mutex>
#include "../include/comm_rpc_server.h"
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

class RpcServer::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(IRpcServerCallback *callback, unsigned short port) {
        if (!callback) return -1;
        callback_ = callback;
        rpc_server_ = std::make_shared<rpc_server>(port, 2);
        if (!rpc_server_) {
            std::cout << "Failed to create rpc_server" << std::endl;
            return -1;
        }
        rpc_server_->set_network_err_callback(std::bind(&RpcServer::Impl::OnNetworkError, this, std::placeholders::_1, std::placeholders::_2));
        // std::bind is not supported yet, use lamda instead at the moment
        rpc_server_->register_handler("get_server_info", [this](rpc_conn conn) {
            callback_->OnNewClient(conn.lock()->conn_id(), conn.lock()->remote_address());

            RpcServerInfo server_info;
            callback_->OnGetInfo(server_info);

            ServerInfoImpl info;
            info.has_speech = server_info.has_bidirection_speech;
            info.speech_stream_name = server_info.speech_stream_name;
            info.speech_service_port = server_info.speech_service_port;
            return info;
        });
        rpc_server_->register_handler("power_off", [this](rpc_conn conn) {
            RpcRequest req;
            req.type = RpcRequest::Type::POWER_OFF;
            callback_->OnRequest(req);
        });

        // start rpc service
        rpc_server_->async_run();
        return 0;
    }

    void DeInit() {
        if (rpc_server_) {
            rpc_server_.reset();            
        }
    }

    int SendEvent(const RpcEventData &event) {
        if (!rpc_server_) return -1;
        UserEventDataImpl data;
        data.type = static_cast<int>(event.type);
        data.timestamp_ms = event.timestamp_ms;
        data.jpeg.ptr = reinterpret_cast<const char *>(event.jpeg);
        data.jpeg.size = event.jpeg_size;
        rpc_server_->publish("event_notify", data);
        return 0;
    }

  private:
    void OnNetworkError(std::shared_ptr<connection> conn, std::string reason) {
        callback_->OnDisconnect(conn->conn_id(), reason);
    }

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::shared_ptr<rpc_server> rpc_server_{nullptr};
    std::thread thread_;
    std::atomic<bool> exit_flag_{false};
    IRpcServerCallback *callback_{nullptr};
};

RpcServer::RpcServer() : impl_(std::make_unique<Impl>()) {}
RpcServer::~RpcServer() {}

int RpcServer::Init(IRpcServerCallback *callback, unsigned short port) {
    return impl_->Init(callback, port);
}

void RpcServer::DeInit() {
    impl_->DeInit();
}

int RpcServer::SendEvent(const RpcEventData &event) {
    return impl_->SendEvent(event);
}
