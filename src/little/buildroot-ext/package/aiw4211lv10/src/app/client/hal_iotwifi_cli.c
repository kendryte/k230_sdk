/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "soc_types.h"
#include "securec.h"
#include "soc_base.h"

#include <getopt.h>
#include "soc_msg.h"
#include "hal_iotwifi_cli.h"

static int kd_connect_to_svr(void)
{
    int sockfd = -1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        sample_log_print("create socket failed: %s", strerror(errno));
        return EXT_FAILURE;
    }

    return sockfd;
}

static void kd_disconnect_from_svr(int *link)
{
    if (link && *link > 0)
    {
        close(*link);
        *link = -1;
    }
}

static int kd_send_msg_to_svr(int link, int cmd, void *msg, int length)
{
    int ret = -1;
    struct sockaddr_in servaddr;
    ssize_t recvbytes;
    char buf[SOCK_BUF_MAX];
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    socchn_msg_t __msg;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(SOCK_PORT);

    if (cmd < 0 || cmd >= SOCCHN_CMD_END)
    {
        printf("cmd %d invalid\n", cmd);
        return -1;
    }

    memset(&__msg, 0x00, sizeof(socchn_msg_t));
    __msg.header.magic = MAGIC_NUM;
    __msg.header.cmd = cmd;
    __msg.header.length = sizeof(socchn_msg_t);

    if (msg && (length > 0 && length <= sizeof(socchn_msg_t)))
        memcpy(__msg.dat, msg, length);
 
    unsigned int crc = ~0U;
    crc = ~0U;    
    crc = crc_32(crc, &__msg, __msg.header.length);
    crc ^= ~0U;
    __msg.header.crc = crc;

    ret = sendto(link, &__msg, __msg.header.length, MSG_DONTWAIT, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret != __msg.header.length)
    {
        sample_log_print("sendto error:%s\n", strerror(errno));
        return -1;
    }

    memset(buf, 0x00, sizeof(buf));
    recvbytes = recvfrom(link, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *)&clientaddr, &addrlen);
    if (recvbytes < 0) {
        sample_log_print("recvfrom error:%s\n", strerror(errno));
        return -1;
    }

    if (strncmp("OK", buf, strlen("OK")) != 0) {
        sample_log_print("sendto cmd error, buf: %s\n", buf);
        return -1;
    }
    sample_log_print("send msg ok!\n");
    return 0;
}

static int kd_send_cfg_to_svr(int link, int cmd, void *cfg, int length)
{
    int ret = -1;
    struct sockaddr_in servaddr;
    ssize_t recvbytes;
    char buf[SOCK_BUF_MAX];
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    socchn_cfg_t __cfg;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(SOCK_PORT);

    if (cmd < 0 || cmd >= SOCCHN_CMD_END)
    {
        printf("cmd %d invalid\n", cmd);
        return -1;
    }

    memset(&__cfg, 0x00, sizeof(socchn_cfg_t));
    __cfg.header.magic = MAGIC_NUM;
    __cfg.header.cmd = cmd;
    __cfg.header.length = sizeof(socchn_cfg_t);

    if (cfg && (length > 0 && length <= sizeof(socchn_cfg_t)))
        memcpy(&__cfg.config, cfg, length);

    unsigned int crc = ~0U;
    crc = ~0U;    
    crc = crc_32(crc, &__cfg, __cfg.header.length);
    crc ^= ~0U;
    __cfg.header.crc = crc;

    ret = sendto(link, &__cfg, __cfg.header.length, MSG_DONTWAIT, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret != __cfg.header.length)
    {
        sample_log_print("sendto error:%s\n", strerror(errno));
        return -1;
    }

    memset(buf, 0x00, sizeof(buf));
    recvbytes = recvfrom(link, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *)&clientaddr, &addrlen);
    if (recvbytes < 0) {
        sample_log_print("recvfrom error:%s\n", strerror(errno));
        return -1;
    }

    if (strncmp("OK", buf, strlen("OK")) != 0) {
        sample_log_print("sendto cmd error, buf: %s\n", buf);
        return -1;
    }
    sample_log_print("send msg ok!\n");
    return 0;
}

int kd_set_wifi_connect(wifi_connect_t *conn)
{
    int ret = -1;
    int link = -1;

    if (!conn)
    {
        printf("conn is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_CONNECT, conn, sizeof(wifi_connect_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_set_wifi_keepalive(wifi_keepalive_t *keepalive)
{
    int ret = -1;
    int link = -1;

    if (!keepalive)
    {
        printf("keepalive is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_KEEPALIVE, keepalive, sizeof(wifi_keepalive_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_set_wifi_sleep(wifi_sleep_t *sleep)
{
    int ret = -1;
    int link = -1;

    if (!sleep)
    {
        printf("sleep is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_SLEEP, sleep, sizeof(wifi_sleep_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_set_wifi_mac(wifi_mac_t *mac)
{
    int ret = -1;
    int link = -1;

    if (!mac)
    {
        printf("mac is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_MAC, mac, sizeof(wifi_mac_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_get_wifi_mac(wifi_mac_t *mac)
{
    int ret = -1;
    int link = -1;

    if (!mac)
    {
        printf("mac is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_GET_MAC, mac, sizeof(wifi_mac_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_get_wifi_ip(wifi_ip_t *ip)
{
    int ret = -1;
    int link = -1;

    if (!ip)
    {
        printf("ip is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_GET_IP, ip, sizeof(wifi_ip_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_set_wifi_ip(wifi_ip_t *ip)
{
    int ret = -1;
    int link = -1;

    if (!ip)
    {
        printf("ip is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_IP, ip, sizeof(wifi_ip_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}


int kd_wifi_config(wifi_config_t *config)
{
    int ret = -1;
    int link = -1;

    if (!config)
    {
        printf("config is NULL\n");
        return -1;
    }

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_cfg_to_svr(link, SOCCHN_CMD_SET_CONFIG, config, sizeof(wifi_config_t));
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
}

int kd_wifi_sleep(void)
{

    //return kd_set_wifi_sleep(NULL);
#if 1
    int ret = -1;
    int link = -1;

    link = kd_connect_to_svr();
    if (link <= 0)
    {
        ret = -1;
        goto EXIT;
    }

    ret = kd_send_msg_to_svr(link, SOCCHN_CMD_SET_SLEEP, NULL, 0);
    if (ret != 0)
    {
        goto EXIT;
    }

    ret = 0;
EXIT:
    kd_disconnect_from_svr(&link);
    return ret;
#endif
}