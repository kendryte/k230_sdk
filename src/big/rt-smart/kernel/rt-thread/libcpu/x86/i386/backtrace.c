/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-28     JasonHu      first version
 */

#include <rtthread.h>
#include <backtrace.h>

static int rt_hw_backtrace(void **buffer, int size)
{
    int i = 0;
    int n = 0;
    unsigned int _ebp = 0;
    unsigned int _eip = 0;
    __asm__ __volatile__(" movl %%ebp, %0" :"=g" (_ebp)::"memory");
    for(i = 0; i < size && _ebp != 0 && *(unsigned int*)_ebp != 0 &&
            *(unsigned int *)_ebp != _ebp; i++) {
        _eip = (unsigned int)((unsigned int*)_ebp + 1);
        _eip = *(unsigned int*)_eip;
        _ebp = *(unsigned int*)_ebp;
        buffer[i] = (void*)_eip;
        n++;
    }
    return n;
}

void rt_hw_print_backtrace(void)
{
    void *buf[BACKTRACE_CNT] = {RT_NULL};
    int cnt = rt_hw_backtrace(buf, BACKTRACE_CNT);
    rt_kprintf("[!] Call backtrace:\n");
    int i;
    for (i = 0; i < cnt; i++)
    {
        rt_kprintf("%d: call %p\n", i, buf[i]);
    }
    rt_kprintf("[!] Call backtrace done.\n");
}
