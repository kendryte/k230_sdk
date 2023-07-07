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
#include <pthread.h>
#include <unistd.h>
#include "k_type.h"
#include "k_ipcmsg.h"
#include "sample_define.h"

#define SEND_TIME_INTERVAL_US   (1000 * 1000)

static k_bool s_bStopSend = K_FALSE;

void handle_message1(k_s32 s32Id, k_ipcmsg_message_t* msg)
{
    printf("receive msg form %d: %s\n", s32Id, (char*)msg->pBody);
}

void handle_message2(k_s32 s32Id, k_ipcmsg_message_t* msg)
{
    printf("receive msg form %d: %s\n", s32Id, (char*)msg->pBody);
}

void handle_resp(k_ipcmsg_message_t* pResp)
{
    printf("receive async resp: %s, return:%x\n\n", (char*)pResp->pBody, pResp->s32RetVal);
}

void* receive_thread(void* arg)
{
    k_s32* pId = (k_s32*)arg;
    printf("Run...\n");
    kd_ipcmsg_run(*pId);
    printf("after Run...\n");
    return NULL;
}



void* sendasync_thread(void* arg)
{
    k_s32* ps32Id = (k_s32*)arg;
    k_u32  u32Cmd = 0;
    static k_u32 s_u32Index = 0;
    char content[16];

    while (K_FALSE == s_bStopSend)
    {
        memset(content, 0, 16);
        snprintf(content, 16, "async %u", s_u32Index++);
        u32Cmd = (*ps32Id << 24) | s_u32Index;
        k_ipcmsg_message_t* pReq = kd_ipcmsg_create_message(SNED_ASYNC_MODULE_ID, u32Cmd, content, 16);
        printf("sendasync modle:%x cmd:%08x \n", 2, u32Cmd);
        kd_ipcmsg_send_async(*ps32Id, pReq, handle_resp);
        kd_ipcmsg_destroy_message(pReq);

        usleep(SEND_TIME_INTERVAL_US * 3);
    }

    return NULL;
}

void* sendonly_thread(void* arg)
{
    k_s32* ps32Id = (k_s32*)arg;
    k_u32  u32Cmd = 0;
    static k_u32 s_u32Index = 0;
    char content[16];

    while (K_FALSE == s_bStopSend)
    {
        memset(content, 0, 16);
        snprintf(content, 16, "only %u", s_u32Index++);
        u32Cmd = (*ps32Id << 24) | s_u32Index;
        k_ipcmsg_message_t* pReq = kd_ipcmsg_create_message(SEND_ONLY_MODULE_ID, u32Cmd, content, 16);
        printf("sendonly modle:%x cmd:%08x \n", 2, u32Cmd);
        kd_ipcmsg_send_only(*ps32Id, pReq);
        kd_ipcmsg_destroy_message(pReq);

        usleep(SEND_TIME_INTERVAL_US * 3);
    }

    return NULL;
}


void* sendsync_thread(void* arg)
{
    k_s32* ps32Id = (k_s32*)arg;
    static k_u32 s_u32Index = 0;
    k_u32 u32Cmd;
    char content[16];
    k_ipcmsg_message_t* pReq = NULL;
    k_ipcmsg_message_t* pResp = NULL;
    struct timespec ts_send_start;
    struct timespec ts_send_end;
    k_u64 time_use;

    while (K_FALSE == s_bStopSend)
    {
        memset(content, 0, 16);
        snprintf(content, 16, "sync %d", s_u32Index++);
        u32Cmd = (*ps32Id << 24) | s_u32Index;
        pReq = kd_ipcmsg_create_message(SEND_SYNC_MODULE_ID, u32Cmd, content, 16);
        if(NULL == pReq)
        {
            printf("kd_ipcmsg_create_message return null.\n");
            return NULL;
        }

        clock_gettime(CLOCK_MONOTONIC, &ts_send_start);
        k_s32 retVal = kd_ipcmsg_send_sync(*ps32Id, pReq, &pResp, 2000);
        clock_gettime(CLOCK_MONOTONIC, &ts_send_end);
        if(ts_send_end.tv_nsec > ts_send_start.tv_nsec)
            time_use = ts_send_end.tv_nsec - ts_send_start.tv_nsec;
        else {
            time_use = 999999999 - ts_send_start.tv_nsec + ts_send_end.tv_nsec;
        }
        k_u64 time_use_us = time_use / 1000 % 1000;
        printf("id:%d send sync: module:%d cmd:%08x time use:%ld.%ldms\n", *ps32Id, 1, u32Cmd, time_use / 1000000, time_use_us);

        if (0 == retVal)
        {
            printf("receive sync resp: %s, return:%x\n\n", (char*)pResp->pBody, pResp->s32RetVal);
            kd_ipcmsg_destroy_message(pResp);
        }
        else if (K_IPCMSG_ETIMEOUT == retVal)
        {
            printf("SendSync timeout\n");
        }
        else
        {
            printf("Send fail. Unknow Error\n");
        }

        kd_ipcmsg_destroy_message(pReq);

        usleep(SEND_TIME_INTERVAL_US * 3);

    }

    return NULL;
}

int sample_send(int argc, char** argv)
{
    pthread_t receivePid0, receivePid1;
    pthread_t sendPid0, sendPid1, sendPid2, sendPid3, sendPid4, sendPid5;

    k_s32 s32Id1;
    k_s32 s32Id2;

    if (0 != kd_ipcmsg_try_connect(&s32Id1, "Test1", handle_message1))
    {
        printf("Connect fail\n");
        return K_FAILED;
    }

    printf("-----Connect: %d\n", kd_ipcmsg_is_connect(s32Id1));

    if (0 != kd_ipcmsg_try_connect(&s32Id2, "Test2", handle_message2))
    {
        printf("Connect fail\n");
        return K_FAILED;
    }

    printf("-----Connect: %d\n", kd_ipcmsg_is_connect(s32Id2));


    if (0 != pthread_create(&receivePid0, NULL, receive_thread, &s32Id1))
    {
        printf("pthread_create receive_thread fail\n");
        return K_FAILED;
    }
    if (0 != pthread_create(&sendPid0, NULL, sendsync_thread, &s32Id1))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);
    if (0 != pthread_create(&sendPid1, NULL, sendasync_thread, &s32Id1))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);
    if (0 != pthread_create(&sendPid2, NULL, sendonly_thread, &s32Id1))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);

    if (0 != pthread_create(&receivePid1, NULL, receive_thread, &s32Id2))
    {
        printf("pthread_create receive_thread fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);
    if (0 != pthread_create(&sendPid3, NULL, sendsync_thread, &s32Id2))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);
    if (0 != pthread_create(&sendPid4, NULL, sendasync_thread, &s32Id2))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }
    usleep(SEND_TIME_INTERVAL_US);
    if (0 != pthread_create(&sendPid5, NULL, sendonly_thread, &s32Id2))
    {
        printf("pthread_create fun fail\n");
        return K_FAILED;
    }

    k_char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    s_bStopSend = K_TRUE;
    pthread_join(sendPid0, NULL);
    pthread_join(sendPid1, NULL);
    pthread_join(sendPid2, NULL);
    pthread_join(sendPid3, NULL);
    pthread_join(sendPid4, NULL);
    pthread_join(sendPid5, NULL);

    kd_ipcmsg_disconnect(s32Id1);
    kd_ipcmsg_disconnect(s32Id2);

    pthread_join(receivePid0, NULL);
    pthread_join(receivePid1, NULL);

    printf("exit\n");

    return K_SUCCESS;
}


int main(int argc, char** argv)
{
    k_ipcmsg_connect_t stConnectAttr;
    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = 201;
    stConnectAttr.u32Priority = 0;
    kd_ipcmsg_add_service("Test1", &stConnectAttr);

    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = 202;
    stConnectAttr.u32Priority = 1;
    kd_ipcmsg_add_service("Test2", &stConnectAttr);

    sample_send(argc, argv);

    kd_ipcmsg_del_service("Test1");
    kd_ipcmsg_del_service("Test2");

    return K_SUCCESS;
}
