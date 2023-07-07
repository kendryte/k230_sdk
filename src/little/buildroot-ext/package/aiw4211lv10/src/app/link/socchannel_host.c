/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: sample cli file.
 * Author: CompanyName
 * Create: 2021-09-09
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/sockios.h>
#include <linux/wireless.h>

#include "securec.h"
#include "soc_base.h"
#include "socchannel_host.h"

/*****************************************************************************
  2 宏定义、全局变量
*****************************************************************************/
#define NETLINK_SOCKET_PORT_ID                  1100
#define NETLINK_CHANNEL_MODEID                  28
#undef NLMSG_ALIGNTO
#define NLMSG_ALIGNTO                           1
#define USLEEP_TIMES                             10

typedef struct {
    td_s32 skfd;
    pthread_t channel_thread;
    aich_channel_rx_func rx_func;
} netlink_monitor_s;
/*****************************************************************************
  3 枚举、结构体定义
*****************************************************************************/
static td_bool            g_channel_terminate = TD_FALSE;
static netlink_monitor_s *g_channel_monitor = TD_NULL;
/*****************************************************************************
  4 函数实现
*****************************************************************************/
static td_void *aich_channel_host_thread(td_void *args)
{
    td_s32 rev_len;
    td_s32 payload_len;
    td_char msg[SYSTEM_CMD_SIZE];
    struct nlmsghdr *nlh = TD_NULL;
    struct sockaddr_nl daddr;
    socklen_t len  = sizeof(struct sockaddr_nl);
    sample_unused(args);

    while (!g_channel_terminate) {
        (td_void)memset_s(&msg[0], sizeof(msg), 0, sizeof(msg));
        rev_len = recvfrom(g_channel_monitor->skfd, &msg[0], sizeof(msg),
            MSG_WAITALL, (struct sockaddr *)&daddr, &len);
        if (rev_len == -1) {
            if (errno == EINTR) {
                usleep(USLEEP_TIMES);
                continue;
            } else {
                sample_log_print("recvfrom error! fd:%d\n", g_channel_monitor->skfd);
                return TD_NULL;
            }
        }

        if (rev_len <= NLMSG_HDRLEN) {
            usleep(USLEEP_TIMES);
            continue;
        }

        nlh = (struct nlmsghdr *)msg;
        payload_len = rev_len - NLMSG_HDRLEN;
        sample_log_print("aich_channel_host_thread:%x,%d,%d,%d\n", nlh->nlmsg_type, payload_len, rev_len, NLMSG_HDRLEN);
        if (g_channel_monitor->rx_func != TD_NULL) {
            g_channel_monitor->rx_func((td_u8 *)NLMSG_DATA(nlh), payload_len);
        }
    }

    return TD_NULL;
}

td_s32 aich_channel_register_rx_cb(aich_channel_rx_func rx_func)
{
    if ((g_channel_monitor == TD_NULL) || (rx_func == TD_NULL)) {
        sample_log_print("aich_channel_register_rx_cb is fail\n");
        return EXT_FAILURE;
    }

    g_channel_monitor->rx_func = rx_func;
    return EXT_SUCCESS;
}

td_s32 aich_channel_send_to_dev(unsigned char *buf, int len)
{
    int ret;
    struct nlmsghdr *nlh = TD_NULL;
    struct sockaddr_nl daddr;

    if ((buf == TD_NULL) || (len <= 0) || (len > MAX_SEND_DATA_SIZE)) {
        sample_log_print("sendto sata len:%d\n", len);
        return EXT_FAILURE;
    }

    (td_void)memset_s(&daddr, sizeof(daddr), 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK; /* netlink id */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
    if (nlh == TD_NULL) {
        sample_log_print("malloc mem is fail\n");
        return EXT_FAILURE;
    }

    (td_void)memset_s(nlh, sizeof(nlh), 0x00, sizeof(nlh));
    nlh->nlmsg_len = NLMSG_SPACE(len);
    nlh->nlmsg_pid = NETLINK_SOCKET_PORT_ID;
    (td_void)memcpy_s(NLMSG_DATA(nlh), NLMSG_SPACE(len), buf, len);
    ret = sendto(g_channel_monitor->skfd, nlh, nlh->nlmsg_len, 0,
        (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if (ret == -1) {
        sample_log_print("sendto error:%s\n", strerror(errno));
        free(nlh);
        return EXT_FAILURE;
    }

    free(nlh);
    return 0;
}

td_s32 aich_channel_init(td_void)
{
    td_s32 ret;
    struct sockaddr_nl saddr = {0};

    if (g_channel_monitor != TD_NULL) {
        sample_log_print("aich_channel_init is fail\n");
        return EXT_FAILURE;
    }

    g_channel_monitor = (netlink_monitor_s *)malloc(sizeof(netlink_monitor_s));
    if (g_channel_monitor == TD_NULL) {
        return EXT_FAILURE;
    }

    g_channel_terminate = TD_FALSE;
    (td_void)memset_s(g_channel_monitor, sizeof(netlink_monitor_s), 0, sizeof(netlink_monitor_s));
    g_channel_monitor->skfd = -1;
    g_channel_monitor->skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_CHANNEL_MODEID);
    if (g_channel_monitor->skfd == -1) {
        sample_log_print("create is fail:%s\n", strerror(errno));
        goto deinit;
    }

    (td_void)memset_s(&saddr, sizeof(saddr), 0x00, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;          /* netlink id */
    saddr.nl_pid = NETLINK_SOCKET_PORT_ID; /* self pid */

    ret = bind(g_channel_monitor->skfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret != 0) {
        goto deinit;
    }

    ret = pthread_create(&g_channel_monitor->channel_thread, TD_NULL, aich_channel_host_thread, TD_NULL);
    if (ret != EXT_SUCCESS) {
        goto deinit;
    }

    return EXT_SUCCESS;
deinit:
    if (g_channel_monitor->skfd != -1) {
        close(g_channel_monitor->skfd);
        g_channel_monitor->skfd = -1;
    }

    if (g_channel_monitor != TD_NULL) {
        free(g_channel_monitor);
        g_channel_monitor = TD_NULL;
    }
    return EXT_FAILURE;
}

td_s32 aich_channel_deinit(td_void)
{
    if (g_channel_monitor == TD_NULL) {
        sample_log_print("aich_channel_deinit is fail\n");
        return EXT_FAILURE;
    }

    g_channel_terminate = TD_TRUE;

    if (g_channel_monitor->channel_thread) {
        pthread_cancel(g_channel_monitor->channel_thread);
        pthread_join(g_channel_monitor->channel_thread, TD_NULL);
    }

    if (g_channel_monitor->skfd != -1) {
        close(g_channel_monitor->skfd);
        g_channel_monitor->skfd = -1;
    }

    if (g_channel_monitor != TD_NULL) {
        free(g_channel_monitor);
        g_channel_monitor = TD_NULL;
    }

    return EXT_SUCCESS;
}

