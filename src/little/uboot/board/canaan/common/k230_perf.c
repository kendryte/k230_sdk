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
#include <asm/asm.h>
#include <asm/io.h>
#include <asm/types.h>
#include <lmb.h>
#include <cpu_func.h>
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <image.h>
#include <gzip.h>
#include <asm/spl.h>
#include "sysctl.h"

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
#include "k230_board_common.h"

#define TIME_TO_MS(a) ((a)/(27*1000))
#define CYCLE_TO_MS(a) ((a)/(800*1000))

struct st_rtinfo{
    ulong t,c,name;
};

static int record_time_count=0;
static struct st_rtinfo rt[100];


static uint64_t perf_get_smodecycles(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdcycle %0" : "=r"(cnt)
    );
    return cnt;
}

static __maybe_unused uint64_t perf_get_sinstret(void)
{
    uint64_t cnt;

    __asm__ __volatile__ (
        "rdinstret %0" : "=r"(cnt)
    );

    return cnt;
}

static uint64_t perf_get_times(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(cnt));
    return cnt;
}


/**
 * @brief  记录当前时间到sram 
 * 
 * @param prompt 
 */
void record_boot_time_info_to_sram(char *prompt)
{
    struct st_rtinfo *prt;
    #ifdef CONFIG_SPL_BUILD
    prt = (struct st_rtinfo *)0x80200000;
    #else 
    prt = (struct st_rtinfo *)0x80208000;
    #endif 

    while(prt->c)prt++;

    prt->t=perf_get_times();
    prt->c=perf_get_smodecycles();
    snprintf((char *)&prt->name,7, "%s", prompt);
    prt++;
    prt->c=0;
}


/**
 * @brief 记录当前时间到rt全局变量里面
 * 
 * @param prompt 
 */
void record_boot_time_info(char *prompt)
{
    struct st_rtinfo *prt;
    //printf("record_boot_time_info\n");
    //return 0;
    //原因gzip要占用sram.所以sram里面的时间统计需要移到rt全局变量里面；
    if(0 == record_time_count) {   
        memset(rt,0, sizeof(rt)); 
        
        for(prt = (struct st_rtinfo *)0x80200000; prt->c; prt++,record_time_count++){
            rt[record_time_count].t =prt->t;
            rt[record_time_count].c =prt->c;
            rt[record_time_count].name = prt->name;
            prt->c = 0;
        }

        for(prt = (struct st_rtinfo *)0x80208000; prt->c; prt++,record_time_count++){
            rt[record_time_count].t =prt->t;
            rt[record_time_count].c =prt->c;
            rt[record_time_count].name = prt->name;
            prt->c = 0;
        }
    }
    if(record_time_count > 99) return ;

    rt[record_time_count].t = perf_get_times();   
    rt[record_time_count].c= perf_get_smodecycles();
    snprintf((char *)&(rt[record_time_count].name),7,"%s", prompt);
    record_time_count++;
    rt[record_time_count].c=0;
}
/**
 * @brief 显示时间统计信息
 * 
 * @param cmdtp 
 * @param flag 
 * @param argc 
 * @param argv 
 * @return int 
 */
__maybe_unused int do_timeinfo(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
    int i=0;
    record_boot_time_info("last");
    for(i=0; i < record_time_count; i++ ){
        if(i)
            printf("i=%02d t=0x%016lx c=0x%016lx ,tds=0x%016lx  cds=0x%016lx, tds=%ldms tdp==%ldms cds=%ldms cdp=%ldms prom=%s \n",
                    i, rt[i].t, rt[i].c, rt[i].t-rt[0].t, rt[i].c-rt[i-1].c, 
                    TIME_TO_MS(rt[i].t-rt[0].t), TIME_TO_MS(rt[i].t-rt[i-1].t),
                    CYCLE_TO_MS(rt[i].c-rt[0].c), CYCLE_TO_MS(rt[i].c-rt[i-1].c), (char*)&rt[i].name);
        else 
            printf("i=%02d t=0x%016lx c=0x%016lx ,tds=0x%016lx  cds=0x%016lx, tds=%ldms tdp==%ldms cds=%ldms cdp=%ldms prom=%s  \n",
                    i, rt[i].t, rt[i].c, rt[i].t-rt[0].t, rt[i].c-rt[0].c,
                    TIME_TO_MS(rt[i].t), TIME_TO_MS(rt[i].t), 
                    CYCLE_TO_MS( rt[i].c), CYCLE_TO_MS(rt[i].c), (char*)&rt[i].name);
    }
    
    return 0;
}

#ifndef CONFIG_SPL_BUILD

static int do_gtime(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
    int ret = 0;
    printf("cycle 0x%llx  time 0x%llx \n", perf_get_smodecycles(), perf_get_times());
    return ret;
}

U_BOOT_CMD_COMPLETE(
	gtime, 4, 0, do_gtime,
	"gtime",
	"gtime  cycle time", NULL
);

#endif 