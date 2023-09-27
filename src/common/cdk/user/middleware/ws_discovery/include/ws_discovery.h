#ifndef WS_DISCOVERY_H_
#define WS_DISCOVERY_H_

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>

//--------------------------------------------------------------------------
// WsdDiscoveryClient
//--------------------------------------------------------------------------
class IWsdObserver{
public:
    virtual ~IWsdObserver() {}
    // ipaddr.empty() indicates device service is not available, 
    //    otherwise device service is added or updated.
    virtual void onDeviceChanged(const std::string &uuid, const std::string &ipaddr) = 0;
};

class WSDiscoveryClient {
  public:
    static WSDiscoveryClient *GetInstance();
    int Delete();
    int registerObserver(IWsdObserver *o);
    int removeObserver(IWsdObserver *o);
    int get_ipaddr(const std::string &net_itf, std::string &ipaddr);
    int start(const std::string &net_itf);
    int add(const std::string &uuid, const std::string &ipaddr);
    int remove(const std::string &uuid);
    int dumpDeviceList();

  private:
    WSDiscoveryClient();
    ~WSDiscoveryClient();
    static WSDiscoveryClient *instance_;
    void notifyObservers(const std::string &uuid, const std::string &ipaddr);
    void checkDevices();
    void checkNotify();

  private:
    std::string net_itf_;
    std::thread thread_id_;
    std::atomic<bool> exit_flag_{false};
    int routine();

    std::recursive_mutex observer_mutex_;
    std::list<IWsdObserver*> ob_list_;

    std::mutex device_mutex_;
    struct DeviceNode{
        std::string uuid;
        std::string ipaddr;
        int64_t time;
        int notified;
        bool operator == (const DeviceNode& rhs) const {
            return  uuid == rhs.uuid && ipaddr == rhs.ipaddr
                 && time == rhs.time && notified == rhs.notified;
        }
    };
    std::list<DeviceNode> device_list_;
};


//--------------------------------------------------------------------------
//WsdDiscoveryService
//--------------------------------------------------------------------------
class IWsdClientObserver{
  public:
    virtual ~IWsdClientObserver() {}
    // ipaddr.empty() indicates client is not available, 
    //   otherwise client is added or updated.
    virtual void onClientChanged(const std::string &uuid, const std::string &ipaddr) = 0;
};

class IProbeMatch;
class WSDiscoveryService{
  public:
    static WSDiscoveryService *GetInstance();
    int Delete();
    int registerObserver(IWsdClientObserver *c);
    int removeObserver(IWsdClientObserver *c);
    int get_ipaddr(const std::string &net_itf, std::string &ipaddr);
    int start(const std::string &net_itf);
    int addClient(const std::string &uuid, const std::string &ipaddr);
    int dumpClientList();

  private:
    WSDiscoveryService();
    ~WSDiscoveryService();
    static WSDiscoveryService *instance_;
    void notifyObservers(const std::string &uuid, const std::string &ipaddr);
    void checkClients();
    void checkReply(IProbeMatch *pm);

  private:
    std::string net_itf_;
    std::thread thread_id_;
    std::atomic<bool> exit_flag_{false};
    int routine();

    std::recursive_mutex observer_mutex_;
    std::list<IWsdClientObserver*> ob_list_;

    std::mutex client_mutex_;
    struct ClientNode {
        std::string uuid;
        std::string ipaddr;
        int64_t time;
        int replied{0};
        int notified{0};
        bool operator == (const ClientNode& rhs) const {
            return  uuid == rhs.uuid && ipaddr == rhs.ipaddr
                 && time == rhs.time && notified == rhs.notified
                 &&  replied == rhs.replied;
        }
    };
    std::list<ClientNode> client_list_;
};

#endif // WS_DISCOVERY_H_
