/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-16     quanzhao     the first version
 */

#ifndef __DFS_UFS_H__
#define __DFS_UFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* file-related commands */
#define UFS_CMD_OPEN     0
#define UFS_CMD_CLOSE    1
#define UFS_CMD_IOCTL    2
#define UFS_CMD_READ     3
#define UFS_CMD_WRITE    4
#define UFS_CMD_FLUSH    5
#define UFS_CMD_LSEEK    6
#define UFS_CMD_GETDENTS 7
#define UFS_CMD_POLL     8

/* filesystem-related commands */
#define UFS_CMD_MOUNT    9
#define UFS_CMD_UNMOUNT  10
#define UFS_CMD_MKFS     11
#define UFS_CMD_STATFS   12

#define UFS_CMD_UNLINK   13
#define UFS_CMD_STAT     14
#define UFS_CMD_RENAME   15

/* miscellaneous commands */
#define UFS_CMD_FSTATE   16

#define UFS_CMD_NR       17

/* options for UFS_CMD_FSTATE */
#define UFS_FSTATE_GETSIZE  1
#define UFS_FSTATE_GETPOS   2

/* DFS command format */
#define UFS_CMD_MAX_ARGS    3

struct ufs_cmd
{
    uint32_t cmd;
    void *arg[UFS_CMD_MAX_ARGS];
};

#ifdef __cplusplus
}
#endif

#endif  /* __DFS_UFS_H__ */
