#ifndef _COMM_RPC_SERVER_H__
#define _COMM_RPC_SERVER_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_rpc_msg.h"

struct RpcRequest {
    enum class Type : int {
        INVALID,
        POWER_OFF,
        BOTTOM
    } type;
};

class IRpcServerCallback {
  public:
    virtual ~IRpcServerCallback() {}
    virtual void OnNewClient(int64_t conn_id, const std::string &remote_ip) = 0;
    virtual void OnDisconnect(int64_t conn_id, const std::string &reason) = 0;
    virtual void OnGetInfo(ServerInfo &info) = 0;
    virtual void OnRequest(const RpcRequest &req) = 0;
};

// rpc server (on peephole device)
//
class RpcServer {
  public:
    RpcServer();
    ~RpcServer();

    int Init(IRpcServerCallback *callback, unsigned short port = 9000);
    void DeInit();

    int SendEvent(const UserEventData &event);

  private:
    RpcServer(const RpcServer &) = delete;
    RpcServer& operator=(const RpcServer &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _COMM_RPC_SERVER_H__
