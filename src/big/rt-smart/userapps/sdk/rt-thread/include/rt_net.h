/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-17     ShaoJinchun  the first version
 */

#ifndef  RT_NET_H__
#define  RT_NET_H__

#ifdef __cplusplus
extern "C" {
#endif

int closesocket(int s);
int socketpair(int domain, int type, int protocol, int sv[2]);

#ifdef __cplusplus
}
#endif

#endif  /*RT_NET_H__*/
