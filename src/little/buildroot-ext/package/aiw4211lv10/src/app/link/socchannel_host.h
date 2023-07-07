/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: sample link file.
 * Author: CompanyName
 * Create: 2021-09-09
 */

#ifndef SOC_LINK_H
#define SOC_LINK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*****************************************************************************
  1 宏定义
*****************************************************************************/
#define SYSTEM_CMD_SIZE                 384     /* 小于这个值的数据报文通过高优先级通道传输 */
#define MAX_SEND_DATA_SIZE              1500    /* 小于这个值的数据报文通过低优先级通道传输 */
#define SYSTEM_NETDEV_NAME              "wlan0"

typedef void (*aich_channel_rx_func)(unsigned char *msg_data, int len);
/*****************************************************************************
  2 函数声明
*****************************************************************************/
int aich_channel_send_to_dev(unsigned char *buf, int len);

int aich_channel_init(void);

int aich_channel_deinit(void);

int aich_channel_register_rx_cb(aich_channel_rx_func cb);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif