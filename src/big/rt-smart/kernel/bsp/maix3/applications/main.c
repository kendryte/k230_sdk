/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>
#include <string.h>
#include <dfs_fs.h>
#include "msh.h"

#ifndef RT_SHELL_PATH
#define RT_SHELL_PATH "/bin/init.sh"
#endif

int main(void)
{
    int result;
    struct statfs buffer;
    printf("RT-SMART Hello RISC-V.\n");

    char path[64];
    strcpy(path, RT_SHELL_PATH);
    strrchr(path, '/')[0] = 0;
    if(!strcmp(path, "/sdcard") || !strcmp(path, "/sharefs"))
    {
        while(dfs_statfs(path, &buffer) != 0)
        {
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
    }

    msh_exec(RT_SHELL_PATH, strlen(RT_SHELL_PATH)+1);

    return 0;
}
