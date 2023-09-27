#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include "ws_discovery.h"
#include "ws_discovery_impl.h"

static const int US_TO_S = 1000000;

static int64_t ws_gettime(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int64_t)tv.tv_sec * US_TO_S + tv.tv_usec;
}

//----------------------------------------------------------------------------------
//WSD Client
//----------------------------------------------------------------------------------
static std::mutex mutex_;
WSDiscoveryClient *WSDiscoveryClient::instance_ = nullptr;
WSDiscoveryClient *WSDiscoveryClient::GetInstance() {
    std::unique_lock<std::mutex> lck(mutex_);
    if(instance_ == NULL){
        instance_ = new WSDiscoveryClient();
    }
    return instance_;
}

int WSDiscoveryClient::Delete() {
    std::unique_lock<std::mutex> lck(mutex_);
    if (instance_) {
        delete instance_, instance_ = nullptr;
    }
    return 0;
}

WSDiscoveryClient::WSDiscoveryClient() {
    exit_flag_.store(false);
}

WSDiscoveryClient::~WSDiscoveryClient() {
    if(thread_id_.joinable()){
        exit_flag_.store(true);
        thread_id_.join();
    }

    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    ob_list_.clear();
    lck.unlock();

    std::unique_lock<std::mutex> lck2(device_mutex_);
    device_list_.clear();
    lck2.unlock();
}

int WSDiscoveryClient::get_ipaddr(const std::string &net_itf, std::string &ipaddr) {
    char local_ipaddr[256];
    if (::get_ipaddr(net_itf.c_str(), local_ipaddr) < 0) {
        return -1;
    }
    ipaddr = local_ipaddr;
    return 0;
}

int WSDiscoveryClient::start(const std::string &net_itf) {
    net_itf_ = net_itf;
    thread_id_ = std::thread([this](){
        routine();
    });
    return 0;
}
int WSDiscoveryClient::registerObserver(IWsdObserver *o) {
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    for (auto & it: ob_list_) {
        if (it == o) {
            return 0;
        }
    }
    ob_list_.push_back(o);
    return 0;
}

int WSDiscoveryClient::removeObserver(IWsdObserver *o){
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    ob_list_.remove(o);
    return 0;
}

void WSDiscoveryClient::notifyObservers(const std::string &uuid, const std::string &ipaddr) {
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    for(auto &it: ob_list_) {
        it->onDeviceChanged(uuid,ipaddr);
    }
}

int WSDiscoveryClient::dumpDeviceList(){
    printf("WSDiscovery -- devices found:\n");
    std::unique_lock<std::mutex> lck(device_mutex_);
    for (auto &it : device_list_) {
        printf("\tDevice: uuid[%s], ipaddr[%s]\n", it.uuid.c_str(), it.ipaddr.c_str());
    }
    return 0;
}

int WSDiscoveryClient::add(const std::string &uuid, const std::string &ipaddr) {
    std::unique_lock<std::mutex> lck(device_mutex_);
    for (auto &it : device_list_) {
        if (it.uuid == uuid) {
            if (it.ipaddr != ipaddr){
                 //ipaddr changed,update and notify observers
                 it.ipaddr = ipaddr;
                 it.time = ws_gettime();
                 it.notified = 0;
                 // printf("WSDiscovery addDevice--ipaddr changed-- uuid[%s], ipaddr[%s]\n", curr->uuid.c_str(), curr->ipaddr.c_str());
                 return 0;
            } else {
                 //duplicate device
                 it.time = ws_gettime();
                 return 0;
            }
        }
    }
    //new device, add to list and notify observers
    DeviceNode node;
    node.uuid = uuid;
    node.ipaddr  = ipaddr;
    node.time = ws_gettime();
    node.notified = 0;
    // printf("WSDiscovery addDevice--new - uuid[%s], ipaddr[%s]\n", node->uuid.c_str(), node->ipaddr.c_str());
    device_list_.push_back(node);
    return 0;
}

int WSDiscoveryClient::remove(const std::string &uuid) {
    std::unique_lock<std::mutex> lck(device_mutex_);
    for ( auto it = device_list_.begin(); it != device_list_.end(); it ++) {
        if (it->uuid == uuid) {
            it = device_list_.erase(it);
            lck.unlock();
            std::string ip_addr;
            notifyObservers(uuid, ip_addr);
            return 0;
        }
    }
    printf("WARNING --- device not registered\n");
    return 0;
}

void WSDiscoveryClient::checkDevices() {
    std::unique_lock<std::mutex> lck(device_mutex_);
    int64_t  now = ws_gettime();
    for (auto it = device_list_.begin(); it != device_list_.end(); ) {
        if( (now - it->time)/US_TO_S >= 4){
            std::string ip_addr;
            notifyObservers(it->uuid, ip_addr);
            it = device_list_.erase(it);
        } else {
            ++ it;
        }
    }
}

void WSDiscoveryClient::checkNotify(){
    std::unique_lock<std::mutex> lck(device_mutex_);
    for (auto &it : device_list_) {
        if (!it.notified) {
            notifyObservers(it.uuid, it.ipaddr);
            it.notified = 1;
        }
    }
}

//How to implement heart-beat?
//Now we will multicast probe periodically.
int WSDiscoveryClient::routine() {
    class Notifier: public IWsdDeviceNotify {
      public:
        explicit Notifier(WSDiscoveryClient &client) : client_(client) {}
        virtual int add(const std::string &uuid, const std::string &ipaddr) override {
            return client_.add(uuid, ipaddr);
        }
        virtual int remove(const std::string &uuid) override {
            return client_.remove(uuid);
        }
      private:
        WSDiscoveryClient &client_;
    } notifier(*this);
    WsdInstance wsd(&notifier, nullptr);
    if (wsd.initialize(net_itf_) < 0) {
       return -1;
    }
    int sock_fd = wsd.create_recv_socket();
    if (sock_fd < 0) {
      return -1;
    }

    int64_t last_cmd_time = ws_gettime();
    int64_t last_notify_time = ws_gettime();
    int check_notify = 1;
    wsd.probe();

    char buf[1024];
    int len = sizeof(buf);
    while (!exit_flag_) {
        int recv_len,n;
        struct pollfd p[1] = {{sock_fd, POLLIN, 0}};
        n = poll(p, 1, 100);
        if (n > 0){
            if (p[0].revents & POLLIN){
                recv_len = wsd.udp_recv(sock_fd ,buf,len);
                if (recv_len < 0){
                    printf("wsd_target_service udp_recv EIO\n");
                    break;
                } else {
                    WsdInstance::message_t message;
                    if (!wsd.parse_message(buf,recv_len,&message)){
                        if (message.type == "WSD_HELLO") {
                            wsd.on_hello(&message.content);
                        } else if (message.type == "WSD_BYE") {
                            wsd.on_bye(&message.content);
                        } else if (message.type == "WSD_PROBEMATCH") {
                            wsd.on_probe_match(&message.content);
                        }
                    }
                }
            }
        } else if(n < 0) {
            if(errno == EINTR) continue;
            break;//EIO
        } else {
             //TIMEOUT
        }
        if (check_notify  && (ws_gettime() - last_notify_time)/US_TO_S >= 1){
            checkNotify();
            check_notify = 0;
        }
        if ((ws_gettime() - last_cmd_time)/US_TO_S >= 2){
            checkDevices();
            last_notify_time = ws_gettime();
            last_cmd_time = ws_gettime();
            wsd.probe();
            check_notify = 1;
        }
    }
    wsd.close_recv_socket(sock_fd);
    wsd.uninitialize();
    return 0;
}

//------------------------------------------------------------------------------------
//WSD Service
//------------------------------------------------------------------------------------
static std::mutex service_mutex;
WSDiscoveryService *WSDiscoveryService::instance_ = nullptr;
WSDiscoveryService *WSDiscoveryService::GetInstance() {
    std::unique_lock<std::mutex> lck(service_mutex);
    if (instance_ == NULL) {
        instance_ = new WSDiscoveryService();
    }
    return instance_;
}

int WSDiscoveryService::Delete(){
    std::unique_lock<std::mutex> lck(service_mutex);
    if (instance_) {
      delete instance_;
      instance_ = nullptr;
    }
    return 0;
}

WSDiscoveryService::WSDiscoveryService(){
    exit_flag_.store(false);
}

WSDiscoveryService::~WSDiscoveryService(){
    if (thread_id_.joinable()) {
        exit_flag_.store(true);
	    thread_id_.join();
    }
    std::unique_lock<std::mutex> lck(client_mutex_);
    client_list_.clear();
}

int WSDiscoveryService::get_ipaddr(const std::string &net_itf, std::string &ipaddr) {
    char local_ipaddr[256];
    if (::get_ipaddr(net_itf.c_str(), local_ipaddr) < 0) {
        return -1;
    }
    ipaddr = local_ipaddr;
    return 0;
}

int WSDiscoveryService::registerObserver(IWsdClientObserver *c){
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    for (auto &it : ob_list_) {
        if (it == c) return 0;
    }
    ob_list_.push_back(c);
    return 0;
}

int WSDiscoveryService::removeObserver(IWsdClientObserver *c) {
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    ob_list_.remove(c);
    return 0;
}

void WSDiscoveryService::notifyObservers(const std::string &uuid, const std::string &ipaddr) {
    std::unique_lock<std::recursive_mutex> lck(observer_mutex_);
    for (auto &it : ob_list_) {
        it->onClientChanged(uuid, ipaddr);
    }
}

int WSDiscoveryService::start(const std::string &net_itf) {
    net_itf_ = net_itf;
    thread_id_ = std::thread([this]() {
        routine();
    });
    return 0;
}

int WSDiscoveryService::dumpClientList(){
    printf("WSDiscovery clients found:\n");
    std::unique_lock<std::mutex> lck(client_mutex_);
    for (auto &it : client_list_) {
        printf("\tClient: uuid[%s], ipaddr[%s]\n", it.uuid.c_str(), it.ipaddr.c_str());
    }
    return 0;
}

int WSDiscoveryService::addClient(const std::string &uuid, const std::string &ipaddr){
    std::unique_lock<std::mutex> lck(client_mutex_);
    for (auto &it : client_list_) {
        if (it.uuid == uuid) {
            if (it.ipaddr != ipaddr) {
                 it.ipaddr = ipaddr;
                 it.time = ws_gettime();
                 it.replied = 0;
                 it.notified = 0;
            } else if(it.replied) {
                 it.replied = 0;//reset when replied but a new probe command received.
                 it.time = ws_gettime();
            }
            return 0;
        }
    }
    //new client, add to list
    ClientNode node;
    node.uuid = uuid;
    node.ipaddr = ipaddr;
    node.time = ws_gettime();
    node.replied = 0;
    client_list_.push_back(node);
    // printf("WSDiscoveryService addClient,uuid[%s], ipaddr[%s]\n", node->uuid.c_str(), node->ipaddr.c_str());
    return 0;
}

void WSDiscoveryService::checkClients(){
    std::unique_lock<std::mutex> lck(client_mutex_);
    int64_t  now = ws_gettime();
    
    for (auto it = client_list_.begin(); it != client_list_.end();) {
        if ((now - it->time)/US_TO_S >= 5) {
            // printf("WSDiscoveryService remove client,uuid[%s], ipaddr[%s]\n", curr->uuid.c_str(), curr->ipaddr.c_str());
            std::string ip_addr;
            notifyObservers(it->uuid, ip_addr);
            it = client_list_.erase(it);
        } else {
            ++ it;
        }
    }
}

void WSDiscoveryService::checkReply(IProbeMatch *pm){
    std::unique_lock<std::mutex> lck(client_mutex_);
    for (auto &it : client_list_) {
        int64_t now = ws_gettime();
        if ((!it.replied) && ((now - it.time)/US_TO_S >= 1)){
            pm->probe_match(it.ipaddr);//TODO
            if (!it.notified) {
                notifyObservers(it.uuid, it.ipaddr);
                it.notified = 1;
            }
            it.replied = 1;
        }
    }
}

// How to implement heart-beat?
// Now client will multicast probe periodically.
int WSDiscoveryService::routine(){
    class Notifier: public IWsdClientNotify {
      public:
        explicit Notifier(WSDiscoveryService &service) : service_(service) {}
        virtual int addClient(const std::string &uuid, const std::string &ipaddr) override {
            return service_.addClient(uuid, ipaddr);
        }
      private:
        WSDiscoveryService &service_;
    } notifier(*this);

    WsdInstance wsd(nullptr, &notifier);
    if (wsd.initialize(net_itf_) < 0){
       return -1;
    }

    int sock_fd = wsd.create_recv_socket();
    if (sock_fd < 0){
       return -1;
    }
    int64_t last_check_time = ws_gettime();
    wsd.hello();

    char buf[1024];
    int len = sizeof(buf);
    while (!exit_flag_){
        struct pollfd p[1] = {{sock_fd, POLLIN, 0}};
        int n = poll(p, 1, 100);
        if (n > 0){
            if (p[0].revents & POLLIN) {
                int recv_len = wsd.udp_recv(sock_fd, buf, len);
                if (recv_len < 0){
                    printf("wsd_target_service udp_recv EIO\n");
                    break;
                } else {
                    WsdInstance::message_t message;
                    if (!wsd.parse_message(buf,recv_len,&message)){
                        if (message.type == "WSD_PROBE"){
                           wsd.on_probe(&message.content);
                        }
                    }
                }
            }
        } else if (n < 0) {
            if(errno == EINTR) continue;
            break;//EIO
        } else {
             //TIMEOUT
        }
        checkReply(&wsd);
        if ((ws_gettime() - last_check_time)/US_TO_S >= 2){
            checkClients();
            last_check_time = ws_gettime();
        }
    }
    wsd.close_recv_socket(sock_fd);
    wsd.uninitialize();
    return 0;
}
