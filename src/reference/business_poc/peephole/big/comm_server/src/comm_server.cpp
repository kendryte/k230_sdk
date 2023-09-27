#include <iostream>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <atomic>
#include "comm_server.h"
#include "comm_msg.h"
#include "k_type.h"
#include "k_ipcmsg.h"
#include "event_writer.h"
#include "avdata_writer.h"
#include "avdata_reader.h"

class IServerMessageCallback {
  public:
    virtual ~IServerMessageCallback() {}
    virtual void OnMessage(k_s32 s32Id, k_ipcmsg_message_t* msg) = 0;
};

static IServerCallback *s_callback_{nullptr};
std::atomic<bool> s_client_ready_{false};
static IServerMessageCallback *s_msg_callback_{nullptr};

void handle_message (k_s32 s32Id, k_ipcmsg_message_t* msg) {
    if(msg->u32BodyLen >= sizeof(UserMessage)) {
        UserMessage *message = (UserMessage*)msg->pBody;
        if (message->type <= static_cast<int>(UserMsgType::UserMsgMax)) {
            if (s_callback_) s_callback_->OnMessage(message);
        } else {
            if(s_msg_callback_) s_msg_callback_->OnMessage(s32Id, msg);
            return;
        }
    } else {
        std::cout << "UseMessage length is too small" << std::endl;
    }
    k_s32 s32Ret = 0;
    char content[64];
    memset(content, 0, sizeof(content));
    switch(msg->u32Module) {
        case SEND_SYNC_MODULE_ID:
            snprintf(content, 64, "module:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SEND_ASYNC_MODULE_ID:
            snprintf(content, 64, "module:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SEND_ONLY_MODULE_ID:
            /*
            If a reply message is created for kd_ipcmsg_send_only,
            it will trigger the "Sync msg is too late" alert on the other side..
            */
            printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
            return;
        default:
            snprintf(content, 64, "module:%d, cmd:%08x, is not found.", msg->u32Module, msg->u32CMD);
            s32Ret = -1;
    }
    k_ipcmsg_message_t *respMsg = kd_ipcmsg_create_resp_message(msg, s32Ret, content, sizeof(content));
    kd_ipcmsg_send_async(s32Id, respMsg, NULL);
    kd_ipcmsg_destroy_message(respMsg);
    // printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
}

class InnerServerCallback;
class UserCommServer::Impl {
  public:
    Impl() { }
    ~Impl() { DeInit(); }

    int Init(const std::string &service_name, IServerCallback *callback, int port = 201);
    void DeInit();
    int SendEvent(const UserEventData &event, IInterruptCallback *callback);
    int SendVideoData(const AVEncFrameData &data, IInterruptCallback *callback);
    int SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback);

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
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::string service_name_;
    k_s32 ipcmsg_id_{-1};
    std::thread thread_;
    std::mutex mutex_;
    bool initialized_{false};
    InnerServerCallback *msg_handler_{nullptr};
    EventWriter event_writer_;
    AVDataWriter vdata_writer_;
    AVDataWriter adata_writer_;
    AVDataReader adata_reader_;

  public:
    uint64_t event_df_phy_addr_{0};
    uint64_t event_data_phy_addr_{0};
    uint64_t event_mem_size_{0};
    uint64_t vdata_df_phy_addr_{0};
    uint64_t vdata_data_phy_addr_{0};
    uint64_t vdata_mem_size_{0};
    uint64_t adata_df_phy_addr_{0};
    uint64_t adata_data_phy_addr_{0};
    uint64_t adata_mem_size_{0};
    uint64_t radata_df_phy_addr_{0};
    uint64_t radata_data_phy_addr_{0};
    uint64_t radata_mem_size_{0};
};

class InnerServerCallback : public IServerMessageCallback {
  public:
    InnerServerCallback(UserCommServer::Impl &proxy) : proxy_(proxy) {}
    virtual void OnMessage(k_s32 s32Id, k_ipcmsg_message_t* ipc_msg) override {
        uint8_t tmp[256];
        memset(tmp, 0, sizeof(tmp));
        UserMessage *msg = (UserMessage*)tmp;

        UserMessage *message = (UserMessage*)ipc_msg->pBody;
        int base = static_cast<int>(UserMsgType::UserMsgMax);
        if (message->type == base + 1) {
            // CLIENT_GET_PHY_ADDRS
            msg->type = message->type;
            msg->len = 0;
            uint64_t *addr = (uint64_t*)msg->data;
            addr[0] = proxy_.event_df_phy_addr_;  msg->len += sizeof(uint64_t);
            addr[1] = proxy_.event_data_phy_addr_; msg->len += sizeof(uint64_t);
            addr[2] = proxy_.event_mem_size_; msg->len += sizeof(uint64_t);
            addr[3] = proxy_.vdata_df_phy_addr_;  msg->len += sizeof(uint64_t);
            addr[4] = proxy_.vdata_data_phy_addr_; msg->len += sizeof(uint64_t);
            addr[5] = proxy_.vdata_mem_size_; msg->len += sizeof(uint64_t);
            addr[6] = proxy_.adata_df_phy_addr_;  msg->len += sizeof(uint64_t);
            addr[7] = proxy_.adata_data_phy_addr_; msg->len += sizeof(uint64_t);
            addr[8] = proxy_.adata_mem_size_; msg->len += sizeof(uint64_t);
            addr[9] = proxy_.radata_df_phy_addr_;  msg->len += sizeof(uint64_t);
            addr[10] = proxy_.radata_data_phy_addr_; msg->len += sizeof(uint64_t);
            addr[11] = proxy_.radata_mem_size_; msg->len += sizeof(uint64_t);
        }
        // std::cout << "InnerServerCallback : msg_len = " << msg->len << std::endl;
        k_ipcmsg_message_t *respMsg = kd_ipcmsg_create_resp_message(ipc_msg, 0, msg, msg->len + sizeof(int) * 2);
        kd_ipcmsg_send_async(s32Id, respMsg, NULL);
        kd_ipcmsg_destroy_message(respMsg);
    }
  private:
    UserCommServer::Impl& proxy_;
};

int UserCommServer::Impl::Init(const std::string &service_name, IServerCallback *callback, int port) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (initialized_) return 0;
    // initialize writers & readers
    if (event_writer_.Init(event_df_phy_addr_, event_data_phy_addr_, event_mem_size_) < 0) {
        return -1;
    }
    std::cout << "event_writer phy_addrs: " << event_df_phy_addr_ << ", " << event_data_phy_addr_ << std::endl;
    if (vdata_writer_.Init(vdata_df_phy_addr_, vdata_data_phy_addr_, vdata_mem_size_) < 0) {
        return -1;
    }
    std::cout << "vdata_writer phy_addrs: " << vdata_df_phy_addr_ << ", " << vdata_data_phy_addr_ << std::endl;
    if (adata_writer_.Init(adata_df_phy_addr_, adata_data_phy_addr_, adata_mem_size_) < 0) {
        return -1;
    }
    std::cout << "adata_writer phy_addrs: " << adata_df_phy_addr_ << ", " << adata_data_phy_addr_ << std::endl;

    if (adata_reader_.Init(radata_df_phy_addr_, radata_data_phy_addr_, radata_mem_size_, callback) < 0) {
        return -1;
    }
    std::cout << "adata_reader phy_addrs: " << radata_df_phy_addr_ << ", " << radata_data_phy_addr_ << std::endl;

    // start service
    k_ipcmsg_connect_t stConnectAttr;
    memset(&stConnectAttr, 0, sizeof(k_ipcmsg_connect_t));
    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = port;
    stConnectAttr.u32Priority = 0;
    k_s32 ret = kd_ipcmsg_add_service(service_name.c_str(), &stConnectAttr);
    if (ret != K_SUCCESS) {
        return -1;
    }
    std::cout << "UserCommServer::Impl::Init() ...1" << std::endl;
    service_name_ = service_name;
    s_callback_ = callback;
    msg_handler_ = new InnerServerCallback(*this);
    s_msg_callback_ = msg_handler_;
    // kd_ipcmsg_connect() will block until comm_client connect...
    //     it is weired...the comm-sever can not exit if comm-client not do that
    if (0 != kd_ipcmsg_connect(&ipcmsg_id_, service_name_.c_str(), handle_message)) {
        return -1;
    }
    std::cout << "UserCommServer::Impl::Init() ...2" << std::endl;

    thread_ = std::thread([this]() {
        kd_ipcmsg_run(ipcmsg_id_);
    });
    initialized_ = true;
    std::cout << "UserCommServer::Impl::Init() ...Done" << std::endl;
    return 0;
}

void UserCommServer::Impl::DeInit() {
    std::unique_lock<std::mutex> lck(mutex_);
    if (!initialized_) return;
    std::cout << "UserCommServer::Impl::DeInit() ..." << std::endl;
    event_writer_.DeInit();
    vdata_writer_.DeInit();
    adata_writer_.DeInit();
    adata_reader_.DeInit();
    std::cout << "UserCommServer::Impl::DeInit() ...1" << std::endl;
    if (ipcmsg_id_ != -1) {
        kd_ipcmsg_disconnect(ipcmsg_id_);
        ipcmsg_id_ = -1;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    std::cout << "UserCommServer::Impl::DeInit() ...2" << std::endl;
    if (!service_name_.empty()) {
      kd_ipcmsg_del_service(service_name_.c_str());
      service_name_.clear();
    }
    usleep(1000 * 1000 * 2);
    if (msg_handler_) delete msg_handler_, msg_handler_ = nullptr;
    s_client_ready_.store(false);
    initialized_ = false;
    std::cout << "UserCommServer::Impl::DeInit() --- done" << std::endl;
}

int UserCommServer::Impl::SendEvent(const UserEventData &event, IInterruptCallback *callback) {
    return event_writer_.Write(event, callback);
}

int UserCommServer::Impl::SendVideoData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return vdata_writer_.Write(data, callback);
}

int UserCommServer::Impl::SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return adata_writer_.Write(data, callback);
}

UserCommServer::UserCommServer() : impl_(std::make_unique<Impl>()) {}
UserCommServer::~UserCommServer() {}

int UserCommServer::Init(const std::string &service_name, IServerCallback *callback, int port) {
    return impl_->Init(service_name, callback, port);
}

void UserCommServer::DeInit() {
    impl_->DeInit();
}

int UserCommServer::SendEvent(const UserEventData &event, IInterruptCallback *callback) {
    return impl_->SendEvent(event, callback);
}

int UserCommServer::SendVideoData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return impl_->SendVideoData(data, callback);
}

int UserCommServer::SendAudioData(const AVEncFrameData &data, IInterruptCallback *callback) {
    return impl_->SendAudioData(data, callback);
}

int UserCommServer::SendMessage(void *msg, int len) {
    return impl_->SendMessage(msg, len);
}
