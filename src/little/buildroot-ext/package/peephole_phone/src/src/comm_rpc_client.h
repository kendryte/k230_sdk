#ifndef _COMM_RPC_CLIENT_H__
#define _COMM_RPC_CLIENT_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_rpc_msg.h"

class IRpcClientCallback {
  public:
    virtual ~IRpcClientCallback() {}
    virtual void OnServerInfo(const ServerInfo &info) = 0;
    virtual void OnEvent(const UserEventData &event) = 0;
};
 
// rpc client (on Phone)
//
class RpcClient {
  public:
    RpcClient();
    ~RpcClient();

    int Init(IRpcClientCallback *callback, const std::string &dev_ip, unsigned short port = 9000);
    void DeInit();
    int DevPowerOff();

  private:
    RpcClient(const RpcClient &) = delete;
    RpcClient& operator=(const RpcClient &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _COMM_RPC_CLIENT_H__
