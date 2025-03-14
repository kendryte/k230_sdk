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
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "read_dpu_data.h"
#include "mapi_sys_api.h"
#include <stdlib.h>


#define BLOCKLEN (512)
#define DATAFIFO_CHN 4

static k_datafifo_handle hDataFifo[DATAFIFO_CHN] = {K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE,K_DATAFIFO_INVALID_HANDLE};
k_datafifo_params_s dpu_read_parames = {128, BLOCKLEN, K_TRUE, DATAFIFO_READER};

typedef struct kdreaddatafifo
{
    pthread_t read_tid;
    int chn;
    k_bool is_start;
}datafifo_pthread_s;

datafifo_pthread_s reader_pth[DATAFIFO_CHN] ;


k_u64 little_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void * virt_addr= NULL;
    // k_u32 size = SIZEPIC;
#ifdef USE_RT_SMART
    virt_addr = (void *)kd_mpi_sys_mmap(phys_addr, size);
#else
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);

    static k_s32 mmap_fd_tmp = 0;
    if(mmap_fd_tmp == 0)
    {
        mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
        if (mmap_fd_tmp > 0) {
            printf("mmap fd open success.\n");
        }
    }

    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd_tmp, phys_addr & ~page_mask);
    if(mmap_addr != (void *)-1) {
        virt_addr = (void *)((char *)mmap_addr + (phys_addr & page_mask));
    } else {
        printf("mmap addr error: %d %s.\n", mmap_addr, strerror(errno));;
    }

#endif
    return (k_u64)virt_addr;
}


k_s32 little_sys_munmap(k_u64 phy_addr, k_u64 virt_addr, k_u32 size)
{
    k_u32 ret;
#ifdef USE_RT_SMART
    ret = kd_mpi_sys_munmap((void *)virt_addr, size);
#else
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phy_addr & page_mask) + page_mask) & ~(page_mask);
    ret = munmap((void *)((k_u64)(virt_addr) & ~page_mask), mmap_size);
    if (ret == -1) {
        printf("munmap error.\n");
    }
#endif
    return 0;
}



void* read_dpu_data_pth(void* arg)
{
    pthread_detach(pthread_self());
    int chn = *((int *)arg);
    k_u32 readLen = 0;
    k_char* pBuf;
    k_s32 s32Ret = K_SUCCESS;
    int j = 0;
    kd_dpu_data_s dpu_data;
    memset(&dpu_data,0,sizeof(kd_dpu_data_s));
    k_char * pdata = NULL;
    k_u32 width, height;

    while(reader_pth[chn].is_start)
    {
        readLen = 0;
        s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            // printf("get available read len error:%x\n", s32Ret);
            usleep(1*1000);
            continue;
        }

        if (readLen > 0 && readLen <= (128*BLOCKLEN))
        {
            s32Ret = kd_datafifo_read(hDataFifo[chn], (void**)&pBuf);
            if (K_SUCCESS != s32Ret)
            {
                printf("read_dpu_data_pth read error:%x\n", s32Ret);
                usleep(1*1000);
                continue;
            }

            if(readLen >= sizeof(datafifo_msg))
            {
                int len = 0;
                datafifo_msg * pmsg = (datafifo_msg *)pBuf;
                if(pmsg->msg_type == MSG_KEY_TYPE)
                {
                    pfn_dpu_dataproc datafunc = (pfn_dpu_dataproc)pmsg->upfunc;
                    if(pmsg->result_type == DPU_RESULT_TYPE_IMG)
                    {
                        width = pmsg->dpu_result.img_result.width;
                        height = pmsg->dpu_result.img_result.height;
                        kd_dpu_data_s *p_dpu_data = malloc(sizeof(kd_dpu_data_s));
                        memset(p_dpu_data, 0, sizeof(kd_dpu_data_s));
                        p_dpu_data->temperature = pmsg->temperature;

                        memcpy(&p_dpu_data->dpu_result.img_result, &pmsg->dpu_result.img_result, sizeof(k_video_frame));

                        k_video_frame *p_img_result = &p_dpu_data->dpu_result.img_result;
                        if(p_img_result->pixel_format == PIXEL_FORMAT_RGB_888_PLANAR)
                        {
                            for(int i = 0; i < 3; i++)
                            {
                                p_img_result->virt_addr[i] = little_sys_mmap(p_img_result->phys_addr[i], width * height);
                            }
                            datafunc(pmsg->dev_num, p_dpu_data, pmsg->puserdata);
                            for(int i = 0; i < 3; i++)
                            {
                                little_sys_munmap(p_img_result->phys_addr[i],  p_img_result->virt_addr[i], width * height);
                            }
                        }
                        else if(p_img_result->pixel_format == PIXEL_FORMAT_YUV_SEMIPLANAR_420)
                        {
                            // printf("%s>img phys_addr 0x%08x 0x%08x 0x%08x\n", __FUNCTION__, p_img_result->phys_addr[0], p_img_result->phys_addr[1], p_img_result->phys_addr[2]);
                            p_img_result->virt_addr[0] = little_sys_mmap(p_img_result->phys_addr[0], width * height);
                            p_img_result->virt_addr[1] = little_sys_mmap(p_img_result->phys_addr[1],  width * height/2);
                            // p_img_result->virt_addr[2] = little_sys_mmap(p_img_result->phys_addr[2],  width * height/4);
                            datafunc(pmsg->dev_num, p_dpu_data, pmsg->puserdata);
                            little_sys_munmap(p_img_result->phys_addr[0],  p_img_result->virt_addr[0], width * height);
                            little_sys_munmap(p_img_result->phys_addr[1],  p_img_result->virt_addr[1], width * height/2);
                            // little_sys_munmap(p_img_result->phys_addr[2],  p_img_result->virt_addr[2], width * height/4);
                        }
                    }
                    else if(pmsg->result_type == DPU_RESULT_TYPE_LCN)
                    {
                        kd_dpu_data_s *p_dpu_data = malloc(sizeof(kd_dpu_data_s));

                        memset(p_dpu_data, 0, sizeof(kd_dpu_data_s));
                        p_dpu_data->temperature = pmsg->temperature;

                        memcpy(&p_dpu_data->dpu_result.lcn_result, &pmsg->dpu_result.lcn_result, sizeof(k_dpu_chn_lcn_result_t));

                        k_dpu_chn_lcn_result_t *p_lcn_result = &p_dpu_data->dpu_result.lcn_result;
                        p_dpu_data->dpu_result.lcn_virt_addr = (void*)little_sys_mmap(p_lcn_result->depth_out.depth_phys_addr, p_lcn_result->depth_out.length);
                        datafunc(pmsg->dev_num, p_dpu_data, pmsg->puserdata);
                        little_sys_munmap(p_lcn_result->depth_out.depth_phys_addr, (k_u64)p_dpu_data->dpu_result.lcn_virt_addr, p_lcn_result->depth_out.length);
                    }
                }
            }
            s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_READ_DONE, pBuf);
            if (K_SUCCESS != s32Ret)
            {
                printf("read done error:%x\n", s32Ret);
            }
        }
        usleep(200);
    }

    return NULL;
}


int read_dpu_data_init(int chn, k_u64 read_phyAddr)
{
    if(chn >= DATAFIFO_CHN)
    {
        printf("read_dpu_data_init error %d \n",chn);
        return -1;
    }
    printf("read_datafifo_init[%d] read_phyAddr:%lx \n", chn, read_phyAddr);
    if(chn >= DATAFIFO_CHN)
    {
        printf("fifo chn error \n");
        return -1;
    }
    k_s32 s32Ret = K_SUCCESS;

    s32Ret = kd_datafifo_open_by_addr(&hDataFifo[chn], &dpu_read_parames, read_phyAddr);
    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init[%d] finish\n",chn);

    reader_pth[chn].chn = chn;
    reader_pth[chn].is_start = K_TRUE;
    pthread_create(&reader_pth[chn].read_tid, NULL, read_dpu_data_pth, &reader_pth[chn].chn);

    return 0;
}

k_s32 read_dpu_data_clean_all_datafifo_data(int chn)
{
    k_s32 s32Ret = K_FAILED;
    k_u32 readLen = 0;
    void *pdata = NULL;
    k_s32 ret;
    while (K_TRUE)
    {
        s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("%s get available read len error:%x\n", __FUNCTION__, s32Ret);
            break;
        }

        if (readLen > 0)
        {
            s32Ret = kd_datafifo_read(hDataFifo[chn], &pdata);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read error:%x\n", __FUNCTION__, s32Ret);
                break;
            }

            s32Ret = kd_datafifo_cmd(hDataFifo[chn], DATAFIFO_CMD_READ_DONE, pdata);
            if (K_SUCCESS != s32Ret)
            {
                printf("%s read done error:%x\n", __FUNCTION__, s32Ret);
                break;
            }
        }
        else
        {
            usleep(500*1000);//wait all datafifo release
            break;
        }
    }
    return K_SUCCESS;
}


void read_dpu_data_deinit(int chn)
{
    if(chn >= DATAFIFO_CHN)
    {
        printf("read_dpu_data_deinit error %d \n",chn);
        return ;
    }

    reader_pth[chn].is_start = K_FALSE;
    pthread_join(reader_pth[chn].read_tid,NULL);
    reader_pth[chn].read_tid = 0;

    read_dpu_data_clean_all_datafifo_data(chn);

    kd_datafifo_close(hDataFifo[chn]);

    printf("datafifo_deinit[%d] finish\n",chn);
}


