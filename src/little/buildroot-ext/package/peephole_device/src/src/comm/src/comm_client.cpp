#include <iostream>
#include <thread>
#include <mutex>
#include "comm_client.h"
#include "k_type.h"
#include "k_ipcmsg.h"
#include "event_reader.h"
#include "avdata_reader.h"
#include "avdata_writer.h"

static IClientCallback *s_callback_ = nullptr;

class UserCommClient::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(const std::string &service_name, IClientCallback *callback, int port = 201);
    void DeInit();

    int SendMessageReady();
    int ExitService();

    int SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback);

    int EnterPlaybackMode();
    int KeyPressed(); // doorbell key pressed

    int SendMessage(void *msg, int len) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (ipcmsg_id_ < 0 || len < sizeof(UserMessage)) {
            return -1;
        }
        UserMessage *message = (UserMessage*)msg;
        k_u32 cmd = message->type;
        k_ipcmsg_message_t* req = kd_ipcmsg_create_message(SEND_SYNC_MODULE_ID, cmd, msg, len);
        if (!req)  {
            return -1;
        }
        k_ipcmsg_message_t* resp = NULL;
        if (kd_ipcmsg_send_sync(ipcmsg_id_, req, &resp, 2000) != K_SUCCESS) {
            return -1;
        }
        // printf("%s\n", (char*)resp->pBody);
        kd_ipcmsg_destroy_message(resp);
        kd_ipcmsg_destroy_message(req);
        return 0;
    }

  private:
    int get_phy_addrs();

  private:
    static void handle_message(k_s32 s32Id, k_ipcmsg_message_t* msg);

  private:
    int SendMessage(UserMsgType type) {
        UserMessage message;
        message.type = static_cast<int>(type);
        message.len = 0;
        return SendMessage(&message, sizeof(message));
    };

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::string service_name_;
    k_s32 ipcmsg_id_{-1};
    std::thread thread_;
    std::mutex mutex_;
    bool initialized_{false};

    // datafifo phy addrs
    uint64_t event_df_phy_addr_;
    uint64_t event_data_phy_addr_;
    uint64_t event_mem_size_;
    EventReader event_reader_;

    uint64_t vdata_df_phy_addr_;
    uint64_t vdata_data_phy_addr_;
    uint64_t vdata_mem_size_;
    AVDataReader vdata_reader_;

    uint64_t adata_df_phy_addr_;
    uint64_t adata_data_phy_addr_;
    uint64_t adata_mem_size_;
    AVDataReader adata_reader_;

    uint64_t wadata_df_phy_addr_;
    uint64_t wadata_data_phy_addr_;
    uint64_t wadata_mem_size_;
    AVDataWriter adata_writer_;
};

int UserCommClient::Impl::Init(const std::string &service_name, IClientCallback *callback, int port) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (initialized_) return 0;
    k_ipcmsg_connect_t stConnectAttr;
    memset(&stConnectAttr, 0, sizeof(k_ipcmsg_connect_t));
    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = port;
    stConnectAttr.u32Priority = 0;
    if (kd_ipcmsg_add_service(service_name.c_str(), &stConnectAttr) != K_SUCCESS) {
        std::cout << "UserCommClient::Impl::Init kd_ipcmsg_add_service fail" << std::endl;
        return -1;
    }
    service_name_ = service_name;
    s_callback_ = callback;

    if (kd_ipcmsg_try_connect(&ipcmsg_id_, service_name_.c_str(), UserCommClient::Impl::handle_message) != K_SUCCESS){
        std::cout << "UserCommClient::Impl::Init kd_ipcmsg_try_connect fail" << std::endl;
        return -1;
    }
    thread_ = std::thread([this](){
        kd_ipcmsg_run(ipcmsg_id_);
    });
    lck.unlock();

    // get datafifo phyaddrs
    if (get_phy_addrs() < 0) {
        return -1;
    }
    // create readers
    if (event_df_phy_addr_ && event_data_phy_addr_  && event_mem_size_)
    if (event_reader_.Init(event_df_phy_addr_, event_data_phy_addr_, event_mem_size_, callback) < 0) {
        std::cout << "UserCommClient::Impl::Init event_reader_ fail" << std::endl;
        return -1;
    }
    if (vdata_df_phy_addr_ && vdata_data_phy_addr_  && vdata_mem_size_)
    if (vdata_reader_.Init(vdata_df_phy_addr_, vdata_data_phy_addr_, vdata_mem_size_, callback) < 0) {
        std::cout << "UserCommClient::Impl::Init vdata_reader_ fail" << std::endl;
        return -1;
    }
    if (adata_df_phy_addr_ && adata_data_phy_addr_  && adata_mem_size_)
    if (adata_reader_.Init(adata_df_phy_addr_, adata_data_phy_addr_, adata_mem_size_, callback) < 0) {
        std::cout << "UserCommClient::Impl::Init adata_reader_ fail" << std::endl;
        return -1;
    }
    if (wadata_df_phy_addr_ && wadata_data_phy_addr_  && wadata_mem_size_)
    if (adata_writer_.Init(wadata_df_phy_addr_, wadata_data_phy_addr_, wadata_mem_size_) < 0) {
        std::cout << "UserCommClient::Impl::Init adata_writer_ fail" << std::endl;
        return -1;
    }
    initialized_ = true;
    return 0;
}

int UserCommClient::Impl::SendMessageReady() {
    if (SendMessage(UserMsgType::CLIENT_READY) < 0) {
        std::cout << "UserCommClient::Impl::Init SendMessage(CLIENT_READY) fail" << std::endl;
        return -1;
    }
    return 0;
}

int UserCommClient::Impl::get_phy_addrs() {
    std::unique_lock<std::mutex> lck(mutex_);
    UserMessage message;
    message.type = static_cast<int>(UserMsgType::UserMsgMax) + 1;
    message.len = 0;

    k_u32 cmd = message.type;
    k_ipcmsg_message_t* req = kd_ipcmsg_create_message(SEND_SYNC_MODULE_ID, cmd, &message, sizeof(message));
    if (!req)  {
        return -1;
    }
    k_ipcmsg_message_t* resp = NULL;
    if (kd_ipcmsg_send_sync(ipcmsg_id_, req, &resp, 2000) != K_SUCCESS) {
        return -1;
    }
    // printf("%s\n", (char*)resp->pBody);
    UserMessage *msg  = (UserMessage*)resp->pBody;
    uint64_t *addr = (uint64_t *)msg->data;
    event_df_phy_addr_ = addr[0];
    event_data_phy_addr_ = addr[1];
    event_mem_size_ = addr[2];
    vdata_df_phy_addr_ = addr[3];
    vdata_data_phy_addr_ = addr[4];
    vdata_mem_size_ = addr[5];
    adata_df_phy_addr_ = addr[6];
    adata_data_phy_addr_ = addr[7];
    adata_mem_size_ = addr[8];
    wadata_df_phy_addr_ = addr[9];
    wadata_data_phy_addr_ = addr[10];
    wadata_mem_size_ = addr[11];
    // std::cout << "event phyaddrs: " << event_df_phy_addr_ << "," << event_data_phy_addr_ << std::endl;
    // std::cout << "video phyaddrs: " << vdata_df_phy_addr_ << "," << vdata_data_phy_addr_ << std::endl;
    // std::cout << "raudio phyaddrs: " << adata_df_phy_addr_ << "," << adata_data_phy_addr_ << std::endl;
    // std::cout << "waudio phyaddrs: " << wadata_df_phy_addr_ << "," << wadata_data_phy_addr_ << std::endl;
    kd_ipcmsg_destroy_message(resp);
    kd_ipcmsg_destroy_message(req);
    return 0;
}

void UserCommClient::Impl::DeInit() {
    std::unique_lock<std::mutex> lck(mutex_);
    if(!initialized_) return;
    event_reader_.DeInit();
    vdata_reader_.DeInit();
    adata_reader_.DeInit();
    adata_writer_.DeInit();
    if (ipcmsg_id_ != -1) {
        kd_ipcmsg_disconnect(ipcmsg_id_);
        ipcmsg_id_ = -1;            
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    if (!service_name_.empty()) {
        kd_ipcmsg_del_service(service_name_.c_str());
        service_name_.clear();
    }
    initialized_ = false;
}

void UserCommClient::Impl::handle_message(k_s32 s32Id, k_ipcmsg_message_t* msg) {
    if (!s_callback_) return;
    if (msg->u32BodyLen >= sizeof(UserMessage)) {
        UserMessage *message = (UserMessage*)msg->pBody;
        s_callback_->OnMessage(message);
    } else {
        std::cout << "UseMessage length is too small" << std::endl;
    }
}

int UserCommClient::Impl::SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return adata_writer_.Write(data, callback);
}

int UserCommClient::Impl::EnterPlaybackMode() {
    return SendMessage(UserMsgType::PLAYBACK);
}

int UserCommClient::Impl::KeyPressed() {
    return SendMessage(UserMsgType::KEY_PRESSED);
}

int UserCommClient::Impl::ExitService() {
    return SendMessage(UserMsgType::EXIT);
}

UserCommClient::UserCommClient() : impl_(std::make_unique<Impl>()) {}
UserCommClient::~UserCommClient() {}

int UserCommClient::Init(const std::string &service_name, IClientCallback *callback, int port) {
    return impl_->Init(service_name, callback, port);
}

void UserCommClient::DeInit() {
    impl_->DeInit();
}

int UserCommClient::Start() {
    return impl_->SendMessageReady();
}

void UserCommClient::Stop() {
    impl_->ExitService();
}

int UserCommClient::SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return impl_->SendAudioData(data, callback);
}

int UserCommClient::EnterPlaybackMode() {
    return impl_->EnterPlaybackMode();
}

int UserCommClient::KeyPressed() {
    return impl_->KeyPressed();
}

