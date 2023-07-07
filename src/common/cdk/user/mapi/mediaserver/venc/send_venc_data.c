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
#include "send_venc_data.h"
#include "mpi_venc_api.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>


#define DATAFIFO_CHN 4

static k_datafifo_handle hDataFifo[DATAFIFO_CHN] = {K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE};


static void release(void* pStream)//С�˶�����
{
    // printf("send_venc_data_init release %p\n", pStream);
    datafifo_msg * pmsg = (datafifo_msg *)pStream;
    int j = 0;
    k_s32 ret;
    if(pmsg->msg_type == MSG_KEY_TYPE)
    {
        k_venc_stream output;
        output.pack_cnt = pmsg->status.cur_packs;
        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);
        int chn = pmsg->chn;
        for(j = 0;j< pmsg->status.cur_packs;j++)
        {
            output.pack[j].len = pmsg->pack[j].len;
            output.pack[j].pts = pmsg->pack[j].pts;
            output.pack[j].type = pmsg->pack[j].type;
            output.pack[j].phys_addr = pmsg->pack[j].phys_addr;
            //printf("release datafifo[%d] phys_addr:0x%lx time[%ld]\n", chn, output.pack[j].phys_addr, get_big_currtime());
        }
        ret = kd_mpi_venc_release_stream(chn, &output);
        free(output.pack);
    }

}

k_datafifo_params_s writer_params = {128, BLOCKLEN, K_TRUE, DATAFIFO_WRITER};

static int IsInitok[DATAFIFO_CHN] = {0};

k_u64 send_venc_data_init(int chn)
{
    printf("send_venc_data_init chn:%d \n", chn);
    if(chn >= DATAFIFO_CHN)
    {
        printf("open datafifo chn:%d error\n", chn);
        return -1;
    }
    k_s32 s32Ret = K_SUCCESS;
    s32Ret = kd_datafifo_open(&hDataFifo[chn], &writer_params);
    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    k_u64 phyAddr = 0;
    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);
    if (K_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }
    printf("send_venc_data_init PhyAddr: %lx\n", phyAddr);
    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, release);
    if (K_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }
    IsInitok[chn] = 1;
    return phyAddr;
}


int callwriteNULLtoflush(int chn)
{
    if(chn >= DATAFIFO_CHN)
    {
        printf("callwriteNULLtoflush chn:%d error\n", chn);
        return -1;
    }
    if(IsInitok[chn] == 1)
    {
        k_s32 s32Ret = K_SUCCESS;
        // call write NULL to flush
        s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
            return -1;
        }
    }
    return 0;
}

int send_venc_data_to_little(int chn, k_char * buf )
{
    if(chn >= DATAFIFO_CHN)
    {
        printf("open datafifo_sen chn:%d error\n", chn);
        return -1;
    }
    if(IsInitok[chn] != 1)
    {
        //printf("not open datafifo chn:%d error\n", chn);
        return -2;
    }

    k_s32 s32Ret = K_SUCCESS;
    k_u32 availWriteLen = 0;
    int j = 0;

    // call write NULL to flush
    s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
        return -3;
    }

    s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
    if (K_SUCCESS != s32Ret)
    {
        printf("get available write len error:%x\n", s32Ret);
        return -4;
    }

    if (availWriteLen >= BLOCKLEN && availWriteLen <= (128*BLOCKLEN) )
    {
        datafifo_msg * pmsg = (datafifo_msg *)buf;
        //printf("send datafifo[%d] phys_addr:0x%lx time[%ld]\n", chn, pmsg->pack[0].phys_addr, get_big_currtime());
        s32Ret = kd_datafifo_write(hDataFifo[chn], buf);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
            return -5;
        }

        s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_WRITE_DONE, NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write done error:%x\n", s32Ret);
            return -6;
        }
        //printf("send end datafifo[%d] phys_addr:0x%lx time[%ld]\n", chn, pmsg->pack[0].phys_addr, get_big_currtime());
    }
    return 0;
}


int send_venc_data_deinit(int chn)
{
    if(chn >= DATAFIFO_CHN)
    {
        printf("send_venc_data_deinit chn:%d error\n", chn);
        return -1;
    }
    k_s32 s32Ret = K_SUCCESS;
    // call write NULL to flush and release stream buffer.
    s32Ret = kd_datafifo_write(hDataFifo[chn], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
        return -1;
    }
    printf(" kd_datafifo_close %lx\n", hDataFifo[chn]);
    kd_datafifo_close(hDataFifo[chn]);
    printf(" finish\n");
    IsInitok[chn] = 0;
    return 0;
}

unsigned long long get_big_currtime()
{
    static char funarr[64];
    memset(funarr, 0, sizeof(funarr));
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // printf("%.24s %ld Nanoseconds\n", ctime(&ts.tv_sec), ts.tv_nsec);
    unsigned long long curr_time = ts.tv_sec*1000 + ts.tv_nsec/(1000*1000);

    return curr_time;
}
