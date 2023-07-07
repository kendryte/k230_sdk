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
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "k_datafifo.h"

#define READER_INDEX    0
#define WRITER_INDEX    1

static k_s32 g_s32Index = 0;
static const k_s32 BLOCK_LEN = 1024;
static k_datafifo_handle hDataFifo[2] = {K_DATAFIFO_INVALID_HANDLE, K_DATAFIFO_INVALID_HANDLE};

static void release(void* pStream)
{
    printf("release %p\n", pStream);
}

int datafifo_init(k_u64 reader_phyAddr, k_u64 writer_phyAddr)
{
    k_s32 s32Ret = K_SUCCESS;
    k_datafifo_params_s params_reader = {10, BLOCK_LEN, K_TRUE, DATAFIFO_READER};

    s32Ret = kd_datafifo_open_by_addr(&hDataFifo[READER_INDEX], &params_reader, reader_phyAddr);
    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    k_datafifo_params_s params_writer = {10, BLOCK_LEN, K_TRUE, DATAFIFO_WRITER};

    s32Ret = kd_datafifo_open_by_addr(&hDataFifo[WRITER_INDEX], &params_writer, writer_phyAddr);
    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, release);

    if (K_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init finish\n");

    return 0;
}

static k_bool s_bStop = K_FALSE;

void* read_more(void* arg)
{
    k_u32 readLen = 0;
    k_char* pBuf;
    k_s32 s32Ret = K_SUCCESS;

    while (K_FALSE == s_bStop)
    {
        readLen = 0;
        s32Ret = kd_datafifo_cmd(hDataFifo[READER_INDEX], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("get available read len error:%x\n", s32Ret);
            break;
        }

        if (readLen > 0)
        {
            s32Ret = kd_datafifo_read(hDataFifo[READER_INDEX], (void**)&pBuf);
            if (K_SUCCESS != s32Ret)
            {
                printf("read error:%x\n", s32Ret);
                break;
            }
            printf("receive:%s\n", pBuf);
            s32Ret = kd_datafifo_cmd(hDataFifo[READER_INDEX], DATAFIFO_CMD_READ_DONE, pBuf);
            if (K_SUCCESS != s32Ret)
            {
                printf("read done error:%x\n", s32Ret);
                break;
            }

            continue;
        }

        usleep(800000);
    }

    return NULL;
}

void* send_more(void* arg)
{
    k_char buf[BLOCK_LEN];
    k_s32 s32Ret = K_SUCCESS;

    while (K_FALSE == s_bStop)
    {
        k_u32 availWriteLen = 0;

        // call write NULL to flush
        s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
        }

        s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("get available write len error:%x\n", s32Ret);
            break;
        }

        if (availWriteLen >= BLOCK_LEN)
        {
            memset(buf, 0, BLOCK_LEN);
            snprintf(buf, BLOCK_LEN, "========%d========", g_s32Index);
            s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], buf);
            if (K_SUCCESS != s32Ret)
            {
                printf("write error:%x\n", s32Ret);
                break;
            }

            printf("send: %s\n", buf);

            s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_WRITE_DONE, NULL);
            if (K_SUCCESS != s32Ret)
            {
                printf("write done error:%x\n", s32Ret);
                break;
            }

            g_s32Index++;
        }
        else
        {
            //printf("no free space: %d\n", availWriteLen);
        }

        usleep(500000);
    }

    return NULL;
}

void datafifo_deinit()
{
    k_s32 s32Ret = K_SUCCESS;
    // call write NULL to flush and release stream buffer.
    s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }

    kd_datafifo_close(hDataFifo[READER_INDEX]);
    kd_datafifo_close(hDataFifo[WRITER_INDEX]);

    printf("datafifo_deinit finish\n");
}

int main(int argc, char** argv)
{
    k_s32 s32Ret = K_SUCCESS;
    k_char cmd[64];
    pthread_t readThread = -1;
    pthread_t sendThread = -1;

    if (3 != argc)
    {
        printf("Usage: %s PhyAddr0 PhyAddr1\n", argv[0]);
        return -1;
    }

    k_u64 phyAddr[2];
    sscanf(argv[1], "%lx", &phyAddr[READER_INDEX]);
    sscanf(argv[2], "%lx", &phyAddr[WRITER_INDEX]);
    s32Ret = datafifo_init(phyAddr[READER_INDEX], phyAddr[WRITER_INDEX]);

    if (s32Ret != 0)
    {
        return s32Ret;
    }

    printf("press any key to start. \n");
    getchar();

    s_bStop = K_FALSE;
    pthread_create(&readThread, NULL, read_more, NULL);
    usleep(250000);
    pthread_create(&sendThread, NULL, send_more, NULL);
    do
    {
        printf("Input q to exit: \n");
    }
    while ( 0 != strncmp(fgets(cmd, 64, stdin), "q", 1) );

    s_bStop = K_TRUE;

    if (-1 != readThread)
    {
        pthread_join(readThread, NULL);
    }
    if (-1 != sendThread)
    {
        pthread_join(sendThread, NULL);
    }

    printf("press any key to stop. \n");
    getchar();

    datafifo_deinit();

    return 0;
}


