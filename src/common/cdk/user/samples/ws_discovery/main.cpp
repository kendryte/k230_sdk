#include <unistd.h>
#include "ws_discovery.h"

class DeviceObserver:public IWsdObserver{
public:
    virtual void onDeviceChanged(const std::string &uuid, const std::string &ipaddr){
        printf("DeviceObserver::onDeviceChanged, uuid[%s], ipaddr[%s]\n", uuid.c_str(), ipaddr.empty() ? "NULL" : ipaddr.c_str());
    }
};

class ClientObserver: public IWsdClientObserver{
public:
    virtual void onClientChanged(const std::string &uuid, const std::string &ipaddr) {
        printf("ClientObserver::onClientChanged, uuid[%s], ipaddr[%s]\n", uuid.c_str(), ipaddr.empty() ? "NULL" : ipaddr.c_str());
    }
};


int main(int argc, char *argv[]) {
    if (argc > 1) {
        printf("WSD Client\n");
        std::string local_ipaddr;
        for(int i = 0; i < 10; i++) {
            if (!WSDiscoveryClient::GetInstance()->get_ipaddr("eth0", local_ipaddr)) {
                break;
            }
            usleep(1000 * 1000);
        }
        if (local_ipaddr.empty()) {
            printf("Failed to get local ip address");
            return -1;
        }
        printf("Local IP Address : %s\n", local_ipaddr.c_str());

        DeviceObserver *observer = new DeviceObserver();
        WSDiscoveryClient::GetInstance()->registerObserver(observer);
        WSDiscoveryClient::GetInstance()->start("eth0");
        while(1) {
            usleep(1000 * 1000);
            int c = getchar();
            if (c == 'q') {
                break;
            }
            if (c == 'd') {
                WSDiscoveryClient::GetInstance()->dumpDeviceList();
            }
        }
        WSDiscoveryClient::GetInstance()->Delete();
        delete observer;
    } else {
        printf("WSD Service\n");
        std::string local_ipaddr;
        for(int i = 0; i < 10; i++) {
            if (!WSDiscoveryClient::GetInstance()->get_ipaddr("eth0", local_ipaddr)) {
                break;
            }
            usleep(1000 * 1000);
        }
        if (local_ipaddr.empty()) {
            printf("Failed to get local ip address");
            return -1;
        }
        printf("Local IP Address : %s\n", local_ipaddr.c_str());

        ClientObserver *observer = new ClientObserver();
        WSDiscoveryService::GetInstance()->registerObserver(observer);
        WSDiscoveryService::GetInstance()->start("eth0");
        while(1) {
            usleep(1000 * 1000);
            int c = getchar();
            if (c == 'q') {
                break;
            }
            if (c == 'd') {
                WSDiscoveryService::GetInstance()->dumpClientList();
            }
        }
        WSDiscoveryService::GetInstance()->Delete();
        delete observer;
    }
    return 0;
}
