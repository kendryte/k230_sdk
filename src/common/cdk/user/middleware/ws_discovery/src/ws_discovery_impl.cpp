#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <thread>
#include "ws_discovery_impl.h"

// #define WSD_DEBUG
int get_ipaddr(const char *interface, char *ipaddr){
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("get_ipaddr() -- socket create failed...!\n");
        return -1;
    }

    struct sockaddr_in *sin;
    struct ifreq ifr_ip;

    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, interface, sizeof(ifr_ip.ifr_name) - 1);

    if(ioctl( sock_fd, SIOCGIFADDR, &ifr_ip) < 0 ){
        printf("get_ipaddr() -- SIOCGIFADDR ioctl error\n");
        close(sock_fd);
        return -1;
    }
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));
    close(sock_fd);
    return 0;
}

static int get_macaddr(const char *interface,char *macaddr){
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("get_macaddr() -- socket create failed...!\n");
        return -1;
    }
    struct ifreq ifr_mac;
    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, interface, sizeof(ifr_mac.ifr_name)-1);
    if(ioctl(sock_fd, SIOCGIFHWADDR, &ifr_mac) < 0){
        printf("get_macaddr() -- SIOCGIFHWADDR ioctl error\n");
        close(sock_fd);
        return -1;
    }
    sprintf(macaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
    close(sock_fd);
    return 0;
}

static int av_find_info_tag(char *arg, int arg_size, const char *tag1, const char *info) {
    const char *p;
    char tag[128], *q;

    p = info;
    if (*p == '?')
        p++;
    for (;;) {
        q = tag;
        while (*p != '\0' && *p != '=' && *p != '&') {
            if ((q - tag) < sizeof(tag) - 1)
                *q++ = *p;
            p++;
        }
        *q = '\0';
        q = arg;
        if (*p == '=') {
            p++;
            while (*p != '&' && *p != '\0') {
                if ((q - arg) < arg_size - 1) {
                    if (*p == '+')
                        *q++ = ' ';
                    else
                        *q++ = *p;
                }
                p++;
            }
        }
        *q = '\0';
        if (!strcmp(tag, tag1))
            return 1;
        if (*p != '&')
            break;
        p++;
    }
    return 0;
}

//
WsdInstance::WsdInstance(IWsdDeviceNotify *o, IWsdClientNotify *c) 
  : service_observer_(o), client_observer_(c) {
    time_t t;
    srand((unsigned) time(&t));
}

int WsdInstance::initialize(const std::string &interface){
    net_interface_ = interface;
    char local_ipaddr[256];
    char local_macaddr[128];
    if (get_ipaddr(net_interface_.c_str(),(char*)local_ipaddr) < 0){
        return -1;
    }
    if (get_macaddr(net_interface_.c_str(),(char*)local_macaddr) < 0){
        return -1;
    }
    local_ipaddr_ = local_ipaddr;
    local_macaddr_ = local_macaddr;
	return 0;
}

int WsdInstance::uninitialize(){
    return 0;
}

int WsdInstance::hello(){
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"type=WSD_HELLO&device=NetworkVideoTransmitter&uuid=%s&ipaddr=%s", \
        local_macaddr_.c_str(), local_ipaddr_.c_str());
    return udp_muliticast_send(buffer,sizeof(buffer));
}

int WsdInstance::bye(){
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"type=WSD_BYE&device=NetworkVideoTransmitter&uuid=%s&ipaddr=%s", \
        local_macaddr_.c_str(), local_ipaddr_.c_str());
    return udp_muliticast_send(buffer,sizeof(buffer));
}

int WsdInstance::probe(){
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"type=WSD_PROBE&device=NetworkVideoClient&uuid=%s&ipaddr=%s", \
        local_macaddr_.c_str(), local_ipaddr_.c_str());
    return udp_muliticast_send(buffer,sizeof(buffer));
}

int WsdInstance::probe_match(const std::string &ipaddr){
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"type=WSD_PROBEMATCH&device=NetworkVideoTransmitter&uuid=%s&ipaddr=%s", \
        local_macaddr_.c_str(), local_ipaddr_.c_str());
    return udp_unicast_send(buffer,sizeof(buffer), ipaddr.c_str());
}

int WsdInstance::on_hello(WsdInstance::content_t *content){
    // printf("wsd_on_hello --- uuid[%s],ipaddr[%s] \n",content->uuid.c_str(), content->ipaddr.c_str());
    if (service_observer_ && content) {
        service_observer_->add(content->uuid,content->ipaddr);
    }
    return 0;
}

int WsdInstance::on_bye(WsdInstance::content_t *content){
    // printf("wsd_on_bye ---uuid[%s],ipaddr[%s]\n",content->uuid.c_str(), content->ipaddr.c_str());
    if (service_observer_ && content) {
        service_observer_->remove(content->uuid);
    }
    return 0;
}

int WsdInstance::on_probe(WsdInstance::content_t *content){
    // printf("wsd_on_probe ---uuid[%s], ipaddr[%s]\n",content->uuid.c_str(), content->ipaddr.c_str());
    if (client_observer_ && content) {
        client_observer_->addClient(content->uuid,content->ipaddr);
    }
    return 0;
}

int WsdInstance::on_probe_match(WsdInstance::content_t *content){
    // printf("wsd_probe_match ---uuid[%s], ipaddr[%s]\n",content->uuid.c_str(), content->ipaddr.c_str());
    if (service_observer_ && content) {
        service_observer_->add(content->uuid,content->ipaddr);
    }
    return 0;
}

int WsdInstance::parse_message(char *msg,int len, WsdInstance::message_t *message){
    message->type.clear();
    message->device.clear();
    message->content.uuid.clear();
    message->content.ipaddr.clear();
    char buf[256];
    const char *p = msg;
    if (av_find_info_tag(buf, sizeof(buf), "type", p)) {
        message->type = buf;
    }
    if (av_find_info_tag(buf, sizeof(buf), "device", p)) {
        message->device = buf;
    }
    if (av_find_info_tag(buf, sizeof(buf), "uuid", p)) {
        message->content.uuid  = buf;
    }
    if (av_find_info_tag(buf, sizeof(buf), "ipaddr", p)) {
        message->content.ipaddr = buf;
    }
	if (message->type.empty()
      ||message->device.empty()
      ||message->content.uuid.empty()
      ||message->content.ipaddr.empty()){
      	printf("WsdInstance::parse_message() failed\n");
        return -1;
    }
    /*
    printf("type= %s, device = %s, uuid = %s, ipaddr = %s\n", \
        message->type.c_str(), message->device.c_str(), message->content.uuid.c_str(), message->content.ipaddr.c_str());
    */
    return 0;
}
int WsdInstance::create_recv_socket(){
    int sock_fd;
    /* Create a datagram socket on which to receive. */
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0){
        return -1;
    }

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    int reuse = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
        close(sock_fd);
        return -1;
    }
    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    struct sockaddr_in localSock;
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(3702);
    localSock.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_fd, (struct sockaddr*)&localSock, sizeof(localSock))){
        close(sock_fd);
	    return -1;
    }
    /* Join the multicast group on the local interface */
    /* Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    struct ip_mreq group;
    group.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    group.imr_interface.s_addr = inet_addr(local_ipaddr_.c_str());
    if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0){
        close(sock_fd);
        return -1;
    }
    return sock_fd;
}

int WsdInstance::close_recv_socket(int sock_fd){
    close(sock_fd);
    return 0;
}

int WsdInstance::udp_recv(int sock_fd, char *buf, int len){
    struct sockaddr_in from;
    int from_len = sizeof(from);
    //inet_ntoa(from.sin_addr)
    while (1) {
        int recv_len = recvfrom(sock_fd, buf,len, 0,(struct sockaddr *) &from,(socklen_t *)&from_len);
	    if (recv_len < 0){
            int error = errno;
            if(error ==  EAGAIN || error == EINTR){
               continue;
            }
            return -EIO;
        }
        return recv_len;
    }
}
static int do_udp_send(int sock_fd,char *buf,int len,struct sockaddr_in *dst){
    while (1) {
        if(sendto(sock_fd, buf, len, 0, (struct sockaddr*)dst, sizeof(*dst)) < 0){
            if(errno == EINTR){
                continue;
            }
            return -1;
        }
        break;
    }
    return 0;
}

static int udp_send(int sock_fd,char *buf,int len,struct sockaddr_in *dst,int unicast){
    //Retry and back-off, according to SOAP over UDP
    int repeat = unicast ? 1:2;
    do_udp_send(sock_fd, buf, len, dst);
    repeat--;
    int T =  50 + rand()%200;
    while (1) {
        usleep(T * 1000);
        do_udp_send(sock_fd,buf,len,dst);
        if(repeat <= 0){
            return 0;
        }
        repeat--;
        T *=2;
        if(T > 500) T = 500;
    }
}

static int do_udp_muliticast_send(char *buf, int len, const char *local_ipaddr){
    struct sockaddr_in groupSock;
    /* Initialize the group sockaddr structure*/
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr("239.255.255.250");
    groupSock.sin_port = htons(3702);

	int sock_fd;
    /* Create a datagram socket on which to send. */
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0){
        return -1;
    }

    /* Disable loopback so you do not receive your own datagrams.*/
    #ifndef WSD_DEBUG
    char loopch = 0;
    if (setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0) {
        close(sock_fd);
        return -1;
    }
    #endif

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    struct in_addr localInterface;
    localInterface.s_addr = inet_addr(local_ipaddr);
    if (setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0){
        close(sock_fd);
        return -1;
    }

    /* Send a message to dst specified by the (*dst) sockaddr structure. */
    udp_send(sock_fd,buf,len,&groupSock,0);

    close(sock_fd);
    //printf("multicast --send %s\n",buf);
    return 0;
}

static int do_udp_unicast_send(char *buf, int len, const char *dst_ipaddr){
    struct sockaddr_in unicastSock;
    memset((char *) &unicastSock, 0, sizeof(unicastSock));
    unicastSock.sin_family = AF_INET;
    unicastSock.sin_addr.s_addr = inet_addr(dst_ipaddr);
    unicastSock.sin_port = htons(3702);

    int sock_fd;
    /* Create a datagram socket on which to send. */
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0){
        return -1;
    }
    /* Send a message to dst specified by the (*dst) sockaddr structure. */
    udp_send(sock_fd,buf,len,&unicastSock,1);
    //
    close(sock_fd);
    //printf("unicast --send %s\n",buf);
    return 0;
}

struct send_param_t{
    char *buf;
    int len;
    std::string ipaddr;
    int unicast;
};
static int async_udp_send(char *buf,int len,const char *ipaddr, int unicast){
    send_param_t *param  = new send_param_t;
    if(!param){
        return -1;
    }
    param->buf = new char[len];
    param->len = len;
    if(!param->buf){
        delete param;
        return -1;
    }
    memcpy(param->buf,buf,len);
    param->ipaddr = ipaddr;
    param->unicast = unicast;

    std::thread([param](){
        if (param->unicast){
            do_udp_unicast_send(param->buf,param->len, param->ipaddr.c_str());
        } else {
            do_udp_muliticast_send(param->buf,param->len, param->ipaddr.c_str());
        }
        delete []param->buf;
        delete param;
    }).detach();
    return 0;
}

int WsdInstance::udp_muliticast_send(char *buf, int len){
    //return do_udp_muliticast_send(buf,len,local_ipaddr);
    return async_udp_send(buf, len, local_ipaddr_.c_str(), 0);
}
int WsdInstance::udp_unicast_send(char *buf, int len, const char *dst_ipaddr){
    //return do_udp_unicast_send(buf,len,dst_ipaddr);
    return async_udp_send(buf, len, dst_ipaddr, 1);
}
