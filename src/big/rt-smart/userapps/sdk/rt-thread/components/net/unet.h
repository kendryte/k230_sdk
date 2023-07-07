#ifndef __UNET_H__
#define __UNET_H__

#include <rtthread.h>
#include <rtdef.h>
#include <lwp_shm.h>

#include <unet_eth.h>

/* The following are common IPC channel methods for unet. Note that the
 * argument 'cmd' might be a pointer or a handler of shared-memory. It depends
 * on whether the macro 'RT_USING_USERSPACE' is defined. */
rt_inline void *unet_cmd_send_recv(int channel, void *cmd)
{
    RT_ASSERT(channel >= 0);

    /* wrap the command and data into an IPC message */
    struct rt_channel_msg unet_msg;
    unet_msg.type   = RT_CHANNEL_RAW;
    unet_msg.u.d    = cmd;

    /* send the command and wait for the result */
    rt_channel_send_recv(channel, &unet_msg, &unet_msg);

    /* Watch this, we use a void pointer to transfer the returned value. */
    return unet_msg.u.d;
}

rt_inline void unet_cmd_send(int channel, void *cmd)
{
    RT_ASSERT(channel >= 0);

    /* wrap the command and data into an IPC message */
    struct rt_channel_msg unet_msg;
    unet_msg.type   = RT_CHANNEL_RAW;
    unet_msg.u.d    = cmd;

    /* send the command and not wait for the result */
    rt_channel_send(channel, &unet_msg);
}

rt_inline void *unet_cmd_recv(int channel)
{
    RT_ASSERT(channel >= 0);

    struct rt_channel_msg unet_msg;
    unet_msg.type = RT_CHANNEL_RAW;
    rt_channel_recv(channel, &unet_msg);
    return unet_msg.u.d;
}

rt_inline void unet_cmd_reply(int channel, void *cmd)
{
    RT_ASSERT(channel >= 0);

    struct rt_channel_msg unet_msg;
    unet_msg.type = RT_CHANNEL_RAW;
    unet_msg.u.d = cmd;
    rt_channel_reply(channel, &unet_msg);
}

#define UNET_CMD_MAX_ARGS   6   /* maximum number of arguments in syscall */
struct unet_cmd
{
    uint32_t cmd;
    void *argv[UNET_CMD_MAX_ARGS];
};

/* When shared-memory is used to transfer the command and data at the same time,
 * the command is put at the begining while the data follows at the offset
 * 'UNET_CMD_OFFSET'. */
#define UNET_CMD_OFFSET    sizeof(struct unet_cmd)

/* In microkernel, shared-memory is used to transfer received data in pages. For
 * now, we limite the total size of command and data to one page. */
#define UNET_RECV_DATA_MAXLEN    (4096-UNET_CMD_OFFSET)

extern int compose_cmd(uint32_t cmd, void *arg0, void *arg1, void *arg2,
        void *arg3, void *arg4, void *arg5, size_t datalen);
/* conventional wrappers */
#define compose_cmd0(c, l)              \
    compose_cmd(c, RT_NULL, RT_NULL, RT_NULL, RT_NULL, RT_NULL, RT_NULL, l)
#define compose_cmd1(c, a0, l)          \
    compose_cmd(c, a0, RT_NULL, RT_NULL, RT_NULL, RT_NULL, RT_NULL, l)
#define compose_cmd2(c, a0, a1, l)      \
    compose_cmd(c, a0, a1, RT_NULL, RT_NULL, RT_NULL, RT_NULL, l)
#define compose_cmd3(c, a0, a1, a2, l)          \
    compose_cmd(c, a0, a1, a2, RT_NULL, RT_NULL, RT_NULL, l)
#define compose_cmd4(c, a0, a1, a2, a3, l)      \
    compose_cmd(c, a0, a1, a2, a3, RT_NULL, RT_NULL, l)
#define compose_cmd5(c, a0, a1, a2, a3, a4, l)  \
    compose_cmd(c, a0, a1, a2, a3, a4, RT_NULL, l)

/*
 * UNET requests the user network stack to change the network state. All the
 * operations are handled by the user network stack, no reply is required.
 */
#define UNET_SRV_SET_HW_STATUS      1
#define UNET_SRV_SET_ADDR_INFO      2
#define UNET_SRV_SET_DNS_SERVER     3
#define UNET_SRV_SET_DHCP           4
#define UNET_SRV_PING               5
#define UNET_SRV_NETSTAT            6
#define UNET_SRV_SET_DEFAULT        7

#define UNET_SRV_DATA_INPUT         8   /* data received */

/*
 * The user network requests UNET to change the netdev state, so that network
 * state in the user network stack keeps identical with the one in kernel.
 */
#define UNET_NETDEV_SETIPADDR       10
#define UNET_NETDEV_SETNETMASK      11
#define UNET_NETDEV_SETGW           12
#define UNET_NETDEV_SET_LINKSTATUS  13
#define UNET_NETDEV_SET_DNS         14
#define UNET_NETDEV_SET_STATUS      15
#define UNET_NETDEV_SET_ADDRINFO    16
#define UNET_NETDEV_SET_DHCP        17

/* UNET socket interfaces, implemented by the user network stack */
#define UNET_SRV_CMD_SOCKET         20
#define UNET_SRV_CMD_CLOSE          21
#define UNET_SRV_CMD_BIND           22
#define UNET_SRV_CMD_LISTEN         23
#define UNET_SRV_CMD_CONNECT        24
#define UNET_SRV_CMD_ACCEPT         25
#define UNET_SRV_CMD_SENDTO         26
#define UNET_SRV_CMD_RECVFROM       27
#define UNET_SRV_CMD_GETSOCKOPT     28
#define UNET_SRV_CMD_SETSOCKOPT     29
#define UNET_SRV_CMD_SHUTDOWN       30
#define UNET_SRV_CMD_GETPEERNAME    31
#define UNET_SRV_CMD_GETSOCKNAME    32
#define UNET_SRV_CMD_FCNTL          33
#define UNET_SRV_CMD_IOCTL          34

/* the retured structure includes other pointers */
#define UNET_SRV_CMD_GETHOSTBYNAME  35
#define UNET_SRV_CMD_GETHOSTBYNAME_R    36

#define UNET_SRV_CMD_GETADDRINFO    37
#define UNET_SRV_CMD_FREEADDRINFO   38
#define UNET_SRV_CMD_POLL           39

/* the commands from unet to kernel */
#define UNET_ETH_CHANNEL_NAME      "unet_eth"

#define UNET_TCPIP_STACK_ATTACH     40   /* attach tcpip stack to unet */
#define UNET_TCPIP_STACK_DETACH     41
#define UNET_NETDEV_NETIF_ATTACH    42
#define UNET_NETDEV_NETIF_DETACH    43
#define UNET_WAKEUP_REQUEST         45

#endif  /* __UNET_H__ */
