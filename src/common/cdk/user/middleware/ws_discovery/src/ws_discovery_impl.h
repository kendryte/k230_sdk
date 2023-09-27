#ifndef WS_DISCOVERY_IMPL_H_
#define WS_DISCOVERY_IMPL_H_

#include <string>

class IWsdDeviceNotify {
  public:
    virtual ~IWsdDeviceNotify() {}
    virtual int add(const std::string &uuid, const std::string &ipaddr) = 0;
    virtual int remove(const std::string &uuid) = 0;
};

class IWsdClientNotify{
  public:
    virtual ~IWsdClientNotify() {}
    virtual int addClient(const std::string &uuid, const std::string &ipaddr) = 0;
};

class IProbeMatch{
  public:
    virtual ~IProbeMatch() {}
    virtual int probe_match(const std::string &ipaddr) = 0;
};

class WsdInstance : public IProbeMatch {
  public:
    WsdInstance(IWsdDeviceNotify *o, IWsdClientNotify *c);

  public:
    struct content_t{
        std::string uuid;
        std::string ipaddr;
    };
    struct message_t {
        std::string type;
        std::string device;
        content_t content;
    };

    int initialize(const std::string &interface);
    int uninitialize();

    int hello();
    int bye();
    int probe();
    virtual int probe_match(const std::string &ipaddr);
    int on_hello(content_t *content);
    int on_bye(content_t *content);
    int on_probe(content_t *content);
    int on_probe_match(content_t *content);

    int parse_message(char *msg, int len, message_t *message);
    int create_recv_socket();
    int close_recv_socket(int sock_fd);
    int udp_recv(int sock_fd, char *buf, int len);

  private:
    int udp_muliticast_send(char *buf, int len);
    int udp_unicast_send(char *buf, int len, const char *dst_ipaddr);
  
  private:
    std::string net_interface_;
    std::string local_ipaddr_;
    std::string local_macaddr_;
    IWsdDeviceNotify *service_observer_{nullptr};
    IWsdClientNotify *client_observer_{nullptr};
};

int get_ipaddr(const char *interface, char *ipaddr);

#endif // WS_DISCOVERY_IMPL_H_
