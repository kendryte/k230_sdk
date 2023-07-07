/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: sample common file.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "soc_types.h"
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define SAMPLE_CMD_MAX_LEN 1500

/*****************************************************************************
  3 枚举、结构体定义
*****************************************************************************/
typedef td_s32(*sample_cmd_func)(td_void *wdata, td_char *param, td_u32 len, td_void *pmsg);
typedef struct {
    td_char           *cmd_name;    /* 命令字符串 */
    sample_cmd_func      func;        /* 命令对应处理函数 */
} sample_cmd_entry_stru;

typedef struct {
    sample_cmd_entry_stru *cmd_tbl;   /* 命令表 */
    td_u32               count;     /* 命令总数 */
} sample_cmd_common;

/*****************************************************************************
  4 函数声明
*****************************************************************************/
td_s32 sample_get_cmd_one_arg(const td_char *pc_cmd, td_char *pc_arg, td_u32 pc_arg_len, td_u32 *pul_cmd_offset);

td_s32 sample_parse_cmd(td_void *wdata, td_char *cmd, ssize_t len, td_void *msg);

td_s32 sample_sock_cmd_entry(td_void *wdata, const char *cmd, ssize_t len, td_void *msg);

td_s32 sample_register_cmd(sample_cmd_entry_stru *cmd_tbl, td_u32 num);

#endif

