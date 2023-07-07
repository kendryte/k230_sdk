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
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/wireless.h>

#include "securec.h"
#include "soc_base.h"
#include "socchannel_host.h"
#include "socchannel_host_comm.h"
/* new fix: */
#include "soc_msg.h"

/*****************************************************************************
  2 宏定义、全局变量msg
*****************************************************************************/
static td_s32 sample_exit_cmd_process(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg);
static td_s32 sample_help_cmd_process(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg);

static const sample_cmd_entry_stru  g_sample_cmd[] = {
    {"help", sample_help_cmd_process},
    {"exit", sample_exit_cmd_process},
};

#define MAX_CMD_LEN 20
#define MAX_IPV4_LEN 16
#define WIFI_MAC_LEN 6
#define SLEEP_TIME 10
#define SAMPLE_CMD_NUM (sizeof(g_sample_cmd) / sizeof(g_sample_cmd[0]))
/*****************************************************************************
  3 枚举、结构体定义
*****************************************************************************/
typedef enum {
    /* commands */
    SAMPLE_CMD_IOCTL,
    SAMPLE_CMD_HELP,
    SAMPLE_CMD_EXIT,
    SAMPLE_CMD_BUTT,
} sample_cmd_e;

typedef enum {
    HOST_CMD_GET_MAC,
    HOST_CMD_GET_IP,
    HOST_CMD_SET_FILTER,
    HOST_CMD_TBTT,
}host_cmd_e;

/* command/event information */
typedef struct {
    sample_cmd_e what;
    td_u32 len;
    td_u8 obj[CMD_MAX_LEN];
} sample_message_s;

struct snode {
    sample_message_s message;
    struct snode *next;
};

struct squeue {
    struct snode *front;
    struct snode *rear;
};

typedef struct {
    pthread_mutex_t mut;
    pthread_cond_t cond;
    struct squeue cmd_queue;
    pthread_t sock_thread;
    td_s32 sockfd;
} sample_link_s;

static td_bool g_terminate = TD_FALSE;
static sample_link_s *g_sample_link = TD_NULL;
static td_char host_cmd[][MAX_CMD_LEN] = {"cmd_get_mac", "cmd_get_ip", "cmd_set_filter"};
/*****************************************************************************
  4 函数实现
*****************************************************************************/
static td_void sample_usage(td_void)
{
    printf("\nUsage:\n");
    printf("\tsample_cli  quit          quit sample_ap\n");
    printf("\tsample_cli  help          show this message\n");
}

td_s32 sample_str_cmd_process(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg)
{
    sample_unused(wdata);
    sample_message_s *msg = (sample_message_s *)pmsg;
    msg->what = SAMPLE_CMD_IOCTL;
    msg->len = len;
    (td_void)memcpy_s(msg->obj, CMD_MAX_LEN, param, len);
    msg->obj[CMD_MAX_LEN - 1] = '\0';
    return EXT_SUCCESS;
}

td_s32 sample_help_cmd_process(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg)
{
    sample_unused(wdata);
    sample_unused(param);
    sample_unused(len);
    sample_message_s *msg = (sample_message_s *)pmsg;
    msg->what = SAMPLE_CMD_HELP;
    return EXT_SUCCESS;
}

static td_s32 sample_exit_cmd_process(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg)
{
    sample_unused(wdata);
    sample_unused(param);
    sample_unused(len);
    sample_message_s *msg = (sample_message_s *)pmsg;
    msg->what = SAMPLE_CMD_EXIT;
    return EXT_SUCCESS;
}

static td_void sample_cleanup(td_void)
{
    sample_log_print("sample_cleanup enter\n");
    if (g_sample_link->sock_thread) {
        pthread_cancel(g_sample_link->sock_thread);
        pthread_join(g_sample_link->sock_thread, TD_NULL);
    }

    pthread_mutex_destroy(&g_sample_link->mut);
    pthread_cond_destroy(&g_sample_link->cond);

    if (g_sample_link->sockfd != -1) {
        close(g_sample_link->sockfd);
    }

    if (g_sample_link != TD_NULL) {
        free(g_sample_link);
        g_sample_link = TD_NULL;
    }
}

static td_void sample_terminate(td_s32 sig)
{
    sample_unused(sig);
    sample_cleanup();
    g_terminate = TD_TRUE;
    _exit(0);
}

static td_void sample_power(td_s32 sig)
{
    sample_unused(sig);
}

static td_s32 sample_wlan_init_up(td_void)
{
    td_s32 ret;
    td_char cmd[SYSTEM_CMD_SIZE] = {0};

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s up", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s up error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    sample_log_print("net device up success\n");
    return EXT_SUCCESS;
}

static td_s32 sample_wlan_init_mac(td_u8 *mac_addr, td_u8 len)
{
    td_s32 ret;
    td_char cmd[SYSTEM_CMD_SIZE] = {0};

    if (len != WIFI_MAC_LEN) {
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s down", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s down error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s hw ether %x:%x:%x:%x:%x:%x",
        /* 2, 3, 4, 5: MAC地址的第2, 3, 4, 5 Byte */
        SYSTEM_NETDEV_NAME, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s set mac error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s up", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s up error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    sample_log_print("net device set mac success\n");

    return EXT_SUCCESS;
}
/* new fix: */
static td_s32 sample_wlan_init_ip(td_u8 *ip_addr, td_u8 len)
{
    td_s32 ret;
    td_char cmd[SYSTEM_CMD_SIZE] = {0};

    sample_unused(len);
    if ((ip_addr[0] == 0) && (ip_addr[1] == 0) && (ip_addr[2] == 0) && (ip_addr[3] == 0)) { /* 1 2 3 ipaddr */
        return EXT_SUCCESS;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s down", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s down error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1,
        "ifconfig %s %d.%d.%d.%d netmask %d.%d.%d.%d",
        SYSTEM_NETDEV_NAME,
         /* 2, 3, 4, 5, 6, 7: 分别为IP地址的第2, 3, 4, 5, 6, 7 Byte */
        ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3], ip_addr[4], ip_addr[5], ip_addr[6], ip_addr[7]) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }

    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s set ip error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s up", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s up error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    sample_log_print("net device set ip success\n");

    return EXT_SUCCESS;
}

static td_void set_lo_ipaddr(td_void)
{
    td_s32 results;
    td_char cmd[SYSTEM_CMD_SIZE] = {0}; /* system Temporary variables */
    td_char *spawn_args[] = {"ifconfig", "lo", "127.0.0.1", TD_NULL};

    results = sprintf_s(cmd, sizeof(cmd), "%s %s %s", spawn_args[0], /* spawn_args[0]:ifconfig */
                        spawn_args[1], spawn_args[2]); /* spawn_args[1]:lo,spawn_args[2]:ipaddr */
    if (results < EOK) {
        sample_log_print("SAMPLE_STA: set lo ipaddr sprintf_s err!\n");
        return;
    }

    system(cmd);
}

static td_s32 sample_sock_create(td_void)
{
    if (g_sample_link == TD_NULL) {
        return EXT_FAILURE;
    }

    g_sample_link->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sample_link->sockfd == -1) {
        sample_log_print("error:%s", strerror(errno));
        return EXT_FAILURE;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SOCK_PORT);

    if (bind(g_sample_link->sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        sample_log_print("error:%s", strerror(errno));
        return EXT_FAILURE;
    }

    return EXT_SUCCESS;
}

static td_s32 sample_enqueue(struct squeue *pqueue, const sample_message_s *element)
{
    struct snode *pnew = TD_NULL;

    if (pqueue == TD_NULL || element == TD_NULL) {
        return -1;
    }
    /* Create a new node */
    pnew = malloc(sizeof(struct snode));
    if (pnew == TD_NULL) {
        return -1;
    }

    pnew->message = *element;
    pnew->next = TD_NULL;

    if (pqueue->rear == TD_NULL) {
        /* queue is empty, set front and rear points to new node */
        pqueue->front = pqueue->rear = pnew;
    } else {
        /* queue is not empty, set rear points to the new node */
        pqueue->rear = pqueue->rear->next = pnew;
    }

    return EXT_SUCCESS;
}

static td_s32 sample_dequeue(struct squeue *pqueue, sample_message_s *element)
{
    struct snode *p = TD_NULL;

    if (pqueue == TD_NULL || element == TD_NULL) {
        return EXT_FAILURE;
    }

    if (pqueue->front == TD_NULL) {
        return EXT_FAILURE;
    }

    *element = pqueue->front->message;
    p = pqueue->front;
    pqueue->front = p->next;
    /* if the queue is empty, set rear = NULL */
    if (pqueue->front == TD_NULL) {
        pqueue->rear = TD_NULL;
    }

    free(p);
    return EXT_SUCCESS;
}

static td_void *sample_sock_thread(td_void *args)
{
    td_char link_buf[SOCK_BUF_MAX];
    ssize_t recvbytes;
    sample_message_s message;
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    sample_unused(args);

    while (1) {
        /* 安全编程规则6.6例外(1) 对固定长度的数组进行初始化，或对固定长度的结构体进行内存初始化 */
        (td_void)memset_s(link_buf, sizeof(link_buf), 0, sizeof(link_buf));
        /* 安全编程规则6.6例外(1) 对固定长度的数组进行初始化，或对固定长度的结构体进行内存初始化 */
        (td_void)memset_s(&clientaddr, sizeof(struct sockaddr_in), 0, addrlen);

        recvbytes = recvfrom(g_sample_link->sockfd, link_buf, sizeof(link_buf), 0,
                             (struct sockaddr *)&clientaddr, &addrlen);
        if (recvbytes < 0) {
            if (errno == EINTR) {
                usleep(SLEEP_TIME);
                continue;
            } else {
                sample_log_print("recvfrom error! fd:%d\n", g_sample_link->sockfd);
                return TD_NULL;
            }
        }

        if (sample_sock_cmd_entry(g_sample_link, link_buf, recvbytes, (td_void *)&message) != EXT_SUCCESS) {
            sample_log_print("sample_str_cmd_process entry\n");
            sample_str_cmd_process(g_sample_link, link_buf, recvbytes, (td_void *)&message);
        }

        if (sendto(g_sample_link->sockfd, "OK", strlen("OK"), MSG_DONTWAIT, (const struct sockaddr *)&clientaddr,
                   addrlen) == -1) {
            sample_log_print("sendto error!fd:%d\n", g_sample_link->sockfd);
        }

        pthread_mutex_lock(&g_sample_link->mut);
        if (sample_enqueue(&g_sample_link->cmd_queue, &message) == EXT_SUCCESS) {
            pthread_cond_signal(&g_sample_link->cond);
        }
        pthread_mutex_unlock(&g_sample_link->mut);
    }
}

/* new fix: */
#if 0
void sample_link_rec_cb(unsigned char *msg_data, int len)
{
    td_s32 ret;
    td_s32 index;

    if ((len == 0) || (msg_data == TD_NULL)) {
        return;
    }

    index = msg_data[0];
    sample_log_print("soc_link_rec:%d\n", index);
    if (index == HOST_CMD_GET_MAC) {
        ret = sample_wlan_init_mac(&msg_data[1], WIFI_MAC_LEN);
        if (ret != EXT_SUCCESS) {
            sample_log_print("sample_wlan_init_mac is fail\n");
        }
    } else if (index == HOST_CMD_GET_IP) {
        ret = sample_wlan_init_ip(&msg_data[1], len - 1);
        if (ret != EXT_SUCCESS) {
            sample_log_print("sample_wlan_init_ip is fail\n");
        }
    } else {
        sample_log_print("soc_link_rec is fail\n");
    }
}
#endif
/*****************************************************************************
 功能描述  : 解析wifi传送的msg协议
 函数参数  : buf: 信息缓存区 (注: 该内存用户不可free，只可读)
            length: 信息长度
*****************************************************************************/
td_s32 kd_wifimsg_getmac(socchn_msg_t *msg, int length)
{
    td_s32 ret = -1;
    ret = sample_wlan_init_mac(msg->mac.mac, WIFI_MAC_LEN);
    if (ret != EXT_SUCCESS) {
        sample_log_print("sample_wlan_init_mac is fail\n");
    }

    return ret;
}

td_s32 kd_wifimsg_getip(socchn_msg_t *msg, int length)
{
    td_s32 ret;
    td_char cmd[SYSTEM_CMD_SIZE] = {0};

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s down", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s down error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1,
        "ifconfig %s %d.%d.%d.%d netmask %d.%d.%d.%d",
        SYSTEM_NETDEV_NAME,
         /* 2, 3, 4, 5, 6, 7: 分别为IP地址的第2, 3, 4, 5, 6, 7 Byte */
        msg->ip.ip[0], msg->ip.ip[1], msg->ip.ip[2], msg->ip.ip[3], 
        msg->ip.netmask[0], msg->ip.netmask[1], msg->ip.netmask[2], msg->ip.netmask[3]) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }

    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s set ip error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    (td_void)memset_s(cmd, SYSTEM_CMD_SIZE, 0, SYSTEM_CMD_SIZE);
    if (snprintf_s(cmd, SYSTEM_CMD_SIZE, SYSTEM_CMD_SIZE - 1, "ifconfig %s up", SYSTEM_NETDEV_NAME) == -1) {
        sample_log_print("snprintf_s fail\n");
        return EXT_FAILURE;
    }
    ret = system(cmd);
    if (ret == -1) {
        sample_log_print("%s up error\n", SYSTEM_NETDEV_NAME);
        return EXT_FAILURE;
    }

    sample_log_print("net device set ip success\n");

    return EXT_SUCCESS;
}

typedef td_s32 (*wifi_msg_callback_fn)(socchn_msg_t *msg, int length);

static wifi_msg_callback_fn wifimsg_cbs[SOCCHN_CMD_END] = {
    [SOCCHN_CMD_GET_MAC] = kd_wifimsg_getmac,
    [SOCCHN_CMD_GET_IP] = kd_wifimsg_getip,
};

void sample_link_rec_cb(unsigned char *buf, int length)
{
    td_s32 ret;

    if ((length == 0) || (buf == TD_NULL)) {
        return;
    }

    socchn_msg_t *msg = (socchn_msg_t *) buf;
    td_u32 crc_tmp = ~0U, crc = msg->header.crc;

    msg->header.crc = 0U;
    crc_tmp = crc_32(crc_tmp, msg, length);
    crc_tmp ^= ~0U;
    if (crc_tmp != crc)
    {
        printf("error: wifi msg crc invalid\n");
        return;
    }

    if (wifimsg_cbs[msg->header.cmd](msg, length) < 0)
    {
        printf("error: wifi msg %d failed\n", msg->header.cmd);
        return;
    }

    return;
}

void main_process(void)
{
    /* main loop */
    while (!g_terminate) {
        sample_message_s message;
        pthread_mutex_lock(&g_sample_link->mut);
        while (sample_dequeue(&g_sample_link->cmd_queue, &message) != EXT_SUCCESS) {
            pthread_cond_wait(&g_sample_link->cond, &g_sample_link->mut);
        }
        pthread_mutex_unlock(&g_sample_link->mut);
        sample_log_print("=====SAMPLE LOOP RECIEVE MSG:%d LEN:%d=====\n", message.what, message.len);
        fflush(stdout);

        switch (message.what) {
            case SAMPLE_CMD_HELP:
                sample_usage();
                break;
            case SAMPLE_CMD_EXIT:
                g_terminate = TD_TRUE;
                break;
            case SAMPLE_CMD_IOCTL:
                if (aich_channel_send_to_dev(message.obj, message.len) != EXT_SUCCESS) {
                    sample_log_print("sample_iwpriv_cmd send fail\n");
                }
                break;
            default:
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    td_s32 ret;
    sample_unused(argc);
    sample_unused(argv);
    set_lo_ipaddr();

    signal(SIGINT, sample_terminate);
    signal(SIGTERM, sample_terminate);
    signal(SIGPWR, sample_power);

    g_sample_link = (sample_link_s *)malloc(sizeof(sample_link_s));
    if (g_sample_link == TD_NULL) {
        return -1;
    }

    (void)memset_s(g_sample_link, sizeof(sample_link_s), 0, sizeof(sample_link_s));
    pthread_mutex_init(&g_sample_link->mut, TD_NULL);
    pthread_cond_init(&g_sample_link->cond, TD_NULL);

    if (sample_wlan_init_up() != EXT_SUCCESS) {
        sample_log_print("sample_wlan_init_up is fail\n");
    }

    if (aich_channel_init() != EXT_SUCCESS) {
        sample_log_print("aich_channel_init is fail\n");
    }

    aich_channel_register_rx_cb(sample_link_rec_cb);
    /* new fix: */
    //aich_channel_send_to_dev((td_u8 *)host_cmd[HOST_CMD_GET_MAC], (td_s32)strlen(host_cmd[HOST_CMD_GET_MAC]));
    socchn_msg_t msg;
    memset(&msg, 0x00, sizeof(socchn_msg_t));
    msg.header.magic = MAGIC_NUM;
    msg.header.cmd = SOCCHN_CMD_GET_MAC;
    msg.header.length = sizeof(socchn_msg_t);
    
    td_u32 crc = ~0U;
    crc = crc_32(crc, &msg, msg.header.length);
    crc ^= ~0U;
    msg.header.crc = crc;

    aich_channel_send_to_dev(&msg, sizeof(socchn_msg_t));

    memset(&msg, 0x00, sizeof(socchn_msg_t));
    msg.header.magic = MAGIC_NUM;
    msg.header.cmd = SOCCHN_CMD_GET_IP;
    msg.header.length = sizeof(socchn_msg_t);
    
    crc = ~0U;
    crc = crc_32(crc, &msg, msg.header.length);
    crc ^= ~0U;
    msg.header.crc = crc;

    aich_channel_send_to_dev(&msg, sizeof(socchn_msg_t));

    if (sample_register_cmd((sample_cmd_entry_stru *)&g_sample_cmd, SAMPLE_CMD_NUM) != EXT_SUCCESS) {
        sample_log_print("register wlan cmd is fail\n");
        goto link_out;
    }

    if (sample_sock_create() != EXT_SUCCESS) {
        sample_log_print("create sock is fail\n");
        goto link_out;
    }

    ret = pthread_create(&g_sample_link->sock_thread, TD_NULL, sample_sock_thread, TD_NULL);
    if (ret != EXT_SUCCESS) {
        sample_log_print("create sock thread is fail\n");
        goto link_out;
    }

    main_process();

link_out:
    sample_cleanup();
    return 0;
}