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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

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
static const k_s32 BLOCK_LEN = 252;
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

    printf("datafifo_init finish\n");

    return 0;
}

static k_bool s_bStop = K_FALSE;


struct ThreadArgs {
    const char *server_ip;
    int server_port;
};

void* read_more(void* args)
{
    // 将参数结构体转换回原始类型
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;

    // 使用传递过来的参数
    const char *server_ip = threadArgs->server_ip;
    int server_port = threadArgs->server_port;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return 1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    int isConnect = -1;
    while(K_FALSE == s_bStop & isConnect < 0)
    {            
        isConnect = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));    
    }

    if(isConnect >= 0)
    {
        printf("connect success\n");
    }

    clock_t start, finish;
	double Total_time;

    k_u32 readLen = 0;
    k_char* pBuf;
    k_s32 s32Ret = K_SUCCESS;

    int data_size = 252;
    int block_size = 256; 
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
            s32Ret = kd_datafifo_cmd(hDataFifo[READER_INDEX], DATAFIFO_CMD_READ_DONE, pBuf);
            if (K_SUCCESS != s32Ret)
            {
                printf("read done error:%x\n", s32Ret);
                break;
            }

            const char *data = (const char*)pBuf; 
            int sent_bytes = 0;

            start = clock();
            while ((K_FALSE == s_bStop) && (sent_bytes < data_size))
            {
                usleep(500);
                int bytes_to_send;
                if(block_size < data_size - sent_bytes)
                {
                    bytes_to_send = block_size;
                }
                else
                {
                    bytes_to_send = data_size - sent_bytes;
                }
                int bytes_sent = send(sock, data + sent_bytes, bytes_to_send, 0);
                if (bytes_sent < 0) {
                    return 1;
                }
                sent_bytes += bytes_sent;
            }
            finish = clock();
            Total_time = (double)(finish - start) / CLOCKS_PER_SEC * 1000; //单位换算成毫秒
            printf("%f ms\n", Total_time);
        }
    }
    close(sock);

    free(threadArgs);

    return NULL;
}

void datafifo_deinit()
{
    k_s32 s32Ret = K_SUCCESS;
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }

    kd_datafifo_close(hDataFifo[READER_INDEX]);

    printf("datafifo_deinit finish\n");
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        printf(stderr, "Usage: %s <ip_address> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]); 

    // 创建结构体来存储参数
    struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    if (args == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }

    // 将参数填充到结构体中
    args->server_ip = server_ip;
    args->server_port = server_port;

    k_s32 s32Ret = K_SUCCESS;
    k_char cmd[64];
    pthread_t readThread = -1;

    k_u64 phyAddr[2];
    sscanf(argv[3], "%lx", &phyAddr[READER_INDEX]);
    sscanf(argv[3], "%lx", &phyAddr[WRITER_INDEX]);
    s32Ret = datafifo_init(phyAddr[READER_INDEX], phyAddr[WRITER_INDEX]);

    if (s32Ret != 0)
    {
        return s32Ret;
    }

    s_bStop = K_FALSE;
    pthread_create(&readThread, NULL, read_more, (void *)args);

    do
    {
        printf("Input q to exit!\n");
    }
    while ( 0 != strncmp(fgets(cmd, 64, stdin), "q", 1) );

    s_bStop = K_TRUE;

    if (-1 != readThread)
    {
        pthread_join(readThread, NULL);
    }

    datafifo_deinit();

    return 0;
}


