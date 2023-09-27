#ifndef _USER_COMM_CLIENT_H__
#define _USER_COMM_CLIENT_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_msg.h"

class IClientCallback {
  public:
    virtual ~IClientCallback() {}
    virtual void OnMessage(const UserMessage *message) = 0;
    virtual void OnEvent(const UserEventData &event) = 0;
    virtual void OnVEncFrameData(const AVEncFrameData &data) = 0;
    virtual void OnAEncFrameData(const AVEncFrameData &data) = 0;
};

class IInterruptCallback {
  public:
    virtual ~IInterruptCallback() {}
    virtual bool Exit() = 0;
};

// user-defined communication client (on linux)
//
class UserCommClient {
  public:
    UserCommClient();
    ~UserCommClient();

    int Init(const std::string &service_name, IClientCallback *callback, int port = 201);
    void DeInit();
    int Start();
    void Stop();

    // for backchannel audio
    int SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback = nullptr);

    int EnterPlaybackMode();
    int KeyPressed(); // doorbell key pressed

  private:
    UserCommClient(const UserCommClient &) = delete;
    UserCommClient& operator=(const UserCommClient &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _USER_COMM_CLIENT_H
