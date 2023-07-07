/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: sample common file.
 * Author: CompanyName
 * Create: 2021-08-04
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "securec.h"
#include "socchannel_host_comm.h"
#include "soc_base.h"

/*****************************************************************************
  2 宏定义、全局变量
*****************************************************************************/
static sample_cmd_common g_cmd_com = {0};

/*****************************************************************************
  4 函数实现
*****************************************************************************/
td_s32 sample_get_cmd_one_arg(const td_char *pc_cmd, td_char *pc_arg, td_u32 pc_arg_len, td_u32 *pul_cmd_offset)
{
    const td_char *pc_cmd_copy = TD_NULL;
    td_u32   pos = 0;

    if ((pc_cmd == TD_NULL) || (pc_arg == TD_NULL) || (pul_cmd_offset == TD_NULL)) {
        sample_log_print("pc_cmd/pc_arg/pul_cmd_offset null ptr error %pK, %pK, %pK!\n", \
            pc_cmd, pc_arg, pul_cmd_offset);
        return EXT_FAILURE;
    }

    pc_cmd_copy = pc_cmd;

    while (*pc_cmd_copy != '\0' && !((*(pc_cmd_copy) == ',') && (*(pc_cmd_copy - 1) != '\\'))) {
        if ((*(pc_cmd_copy + 1) == ',') && (*(pc_cmd_copy) == '\\')) {
            ++pc_cmd_copy;
            continue;
        }
        pc_arg[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (pos >= pc_arg_len) {
            sample_log_print("ul_pos >= WLAN_CMD_NAME_MAX_LEN, ul_pos %d!\n", pos);
            return EXT_FAILURE;
        }
    }

    pc_arg[pos]  = '\0';

    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        sample_log_print("return param pc_arg is null!}\r\n");
        return EXT_FAILURE;
    }
    *pul_cmd_offset = (td_u32)(pc_cmd_copy - pc_cmd);

    return EXT_SUCCESS;
}

td_s32 sample_parse_cmd(td_void *wdata, td_char *cmd, ssize_t len, td_void *msg)
{
    td_u8                  cmd_id;
    td_u32                 off_set = 0;
    td_char                wlan_name[SAMPLE_CMD_MAX_LEN] = {0};

    if (cmd == TD_NULL) {
        return EXT_FAILURE;
    }

    if (sample_get_cmd_one_arg(cmd, wlan_name, SAMPLE_CMD_MAX_LEN, &off_set) != EXT_SUCCESS) {
        return EXT_FAILURE;
    }
    cmd += (off_set + 1);

    for (cmd_id = 0; cmd_id < g_cmd_com.count; cmd_id++) {
        if (strcmp(g_cmd_com.cmd_tbl[cmd_id].cmd_name, wlan_name) == 0) {
            if (g_cmd_com.cmd_tbl[cmd_id].func(wdata, cmd, len, msg) != EXT_SUCCESS) {
                sample_log_print("cmd exec fail!\n");
                return EXT_FAILURE;
            }
            return EXT_SUCCESS;
        }
    }

    return EXT_FAILURE;
}

td_s32 sample_sock_cmd_entry(td_void *wdata, const char *cmd, ssize_t len, td_void *msg)
{
    td_char *pcmd = TD_NULL;
    td_char *pcmd_tmp = TD_NULL;
    if (len > SAMPLE_CMD_MAX_LEN) {
        sample_log_print("command len > %d!\n", SAMPLE_CMD_MAX_LEN);
        return EXT_FAILURE;
    }
    pcmd = malloc(SAMPLE_CMD_MAX_LEN);
    if (pcmd == TD_NULL) {
        return EXT_FAILURE;
    }
    if (cmd != TD_NULL) {
        if (memcpy_s(pcmd, len, cmd, len) != EOK) {
            sample_log_print("command memcpy_s failed!\n");
            free(pcmd);
            return EXT_FAILURE;
        }
    }

    pcmd[len] = '\0';
    pcmd_tmp = pcmd;
    if (sample_parse_cmd(wdata, pcmd_tmp, len, msg) != EXT_SUCCESS) {
        free(pcmd);
        return EXT_FAILURE;
    }
    free(pcmd);
    return EXT_SUCCESS;
}

td_s32 sample_register_cmd(sample_cmd_entry_stru *cmd_tbl, td_u32 num)
{
    td_u32 i;
    sample_cmd_common *tmp_list = TD_NULL;
    tmp_list = (sample_cmd_common *)&g_cmd_com;
    for (i = 0; i < num; i++) {
        if (cmd_tbl[i].cmd_name == TD_NULL || cmd_tbl[i].func == TD_NULL) {
            sample_log_print("SAMPLE_COMMON: register cmd table failed!\n");
            return EXT_FAILURE;
        }
    }
    tmp_list->cmd_tbl = cmd_tbl;
    tmp_list->count = num;
    return EXT_SUCCESS;
}

