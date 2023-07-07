/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: header file for Wi-Fi Station component
 * Author: CompanyName
 * Create: 2021-09-09
 */

#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__
#include "soc_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CMD_MAX_LEN         1500
#define SOCK_BUF_MAX        1500
#define SOCK_PORT           8822

#define sample_log_print(fmt, args...) \
        printf("[%s][%s][l:%d]," fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args);

#define sample_unused(x) ((x) = (x))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

