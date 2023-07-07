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
#include <sys/prctl.h>
#include <stdio.h>
#include <pthread.h>
#include "k_type.h"
#include "k_ipcmsg.h"
#include "sample_define.h"

void* rcv_thread(void* arg)
{
    prctl(PR_SET_NAME, "rcv_thread", 0, 0, 0);
    k_s32* s32Id = (k_s32*)arg;
    printf("start receive msg  from %d\n", *s32Id);
    kd_ipcmsg_run(*s32Id);
    return NULL;
}

void handle_message1(k_s32 s32Id, k_ipcmsg_message_t* msg)
{
    k_s32 s32Ret = 0;
    char content[64];

    memset(content, 0, 64);
    switch(msg->u32Module)
    {
        case SEND_SYNC_MODULE_ID:
            snprintf(content, 64, "modle:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SNED_ASYNC_MODULE_ID:
            snprintf(content, 64, "modle:%d, cmd:%08x, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SEND_ONLY_MODULE_ID:
            /*
            If a reply message is created for kd_ipcmsg_send_only,
            it will trigger the "Sync msg is too late" alert on the other side..
            */
            printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
            return;
        default:
            snprintf(content, 64, "modle:%d, cmd:%08x, is not found.", msg->u32Module, msg->u32CMD);
            s32Ret = -1;
    }

    k_ipcmsg_message_t *respMsg = kd_ipcmsg_create_resp_message(msg, s32Ret, content, 64);
    kd_ipcmsg_send_async(s32Id, respMsg, NULL);
    kd_ipcmsg_destroy_message(respMsg);
    printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
}

void handle_message2(k_s32 s32Id, k_ipcmsg_message_t* msg)
{
    k_s32 s32Ret = 0;
    char content[64];

    memset(content, 0, 64);
    switch(msg->u32Module)
    {
        case SEND_SYNC_MODULE_ID:
            snprintf(content, 64, " modle:%d, cmd:%08x, done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SNED_ASYNC_MODULE_ID:
            snprintf(content, 64, " modle:%d, cmd:%08x, done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case SEND_ONLY_MODULE_ID:
            /*
            If a reply message is created for kd_ipcmsg_send_only,
            it will trigger the "Sync msg is too late" alert on the other side..
            */
            printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
            return;
        default:
            snprintf(content, 64, " modle:%d, cmd:%08x, fail.", msg->u32Module, msg->u32CMD);
            s32Ret = -1;
    }
    k_ipcmsg_message_t *respMsg = kd_ipcmsg_create_resp_message(msg, s32Ret, content, 64);
    kd_ipcmsg_send_async(s32Id, respMsg, NULL);
    kd_ipcmsg_destroy_message(respMsg);
    printf("receive msg from %d: %s, len: %d\n", s32Id, (char*)msg->pBody, msg->u32BodyLen);
}

void receive_msg(void)
{
    k_s32 s32Id1;
    k_s32 s32Id2;

    int ret = 0;
    k_ipcmsg_connect_t stConnectAttr;

    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = 201;
    stConnectAttr.u32Priority = 0;
    kd_ipcmsg_add_service("Test1", &stConnectAttr);

    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = 202;
    stConnectAttr.u32Priority = 1;
    kd_ipcmsg_add_service("Test2", &stConnectAttr);

    if(ret != 0)
    {
        printf("kd_ipcmsg_add_service return err:%x\n", ret);
    }

    if (0 != kd_ipcmsg_connect(&s32Id1, "Test1", handle_message1))
    {
        printf("Connect fail\n");
        return;
    }

    if (0 != kd_ipcmsg_connect(&s32Id2, "Test2", handle_message2))
    {
        printf("Connect fail\n");
        return;
    }

    pthread_t threadid1;
    pthread_t threadid2;

    if (0 != pthread_create(&threadid1, NULL, rcv_thread, &s32Id1))
    {
        printf("pthread_create rcv_thread fail\n");
        return;
    }

    if (0 != pthread_create(&threadid2, NULL, rcv_thread, &s32Id2))
    {
        printf("pthread_create rcv_thread fail\n");
        return;
    }


    k_char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Press q to quit\n");
    }

    kd_ipcmsg_disconnect(s32Id1);
    kd_ipcmsg_disconnect(s32Id2);

    pthread_join(threadid1, NULL);
    pthread_join(threadid2, NULL);

    kd_ipcmsg_del_service("Test1");
    kd_ipcmsg_del_service("Test2");

    printf("quit\n");
}

int main(int argc, char** argv)
{

    receive_msg();

    return K_SUCCESS;
}




