#ifndef _USER_COMM_SERVER_H__
#define _USER_COMM_SERVER_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_msg.h"

class IServerCallback {
  public:
    virtual ~IServerCallback() {}
    virtual void OnMessage(const UserMessage *message) = 0;
    virtual void OnAEncFrameData(const AVEncFrameData &data) = 0;
};

class IInterruptCallback {
  public:
    virtual ~IInterruptCallback() {}
    virtual bool Exit() = 0;
};

// user-defined communication server (on rtsmart)
//
class InnerServerCallback;
class UserCommServer {
  public:
    friend class InnerServerCallback;
    UserCommServer();
    ~UserCommServer();

    int Init(const std::string &service_name, IServerCallback *callback, int port = 201);
    void DeInit();

    int SendEvent(const UserEventData &event, IInterruptCallback *callback = nullptr);
    int SendVideoData(const AVEncFrameData &data, IInterruptCallback *callback = nullptr);
    int SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback = nullptr);
    int SendMessage(void *msg, int len);

  private:
    UserCommServer(const UserCommServer &) = delete;
    UserCommServer& operator=(const UserCommServer &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _USER_COMM_SERVER_H__

