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

#include "msg_proc.h"
#include "ui_common.h"
#include <iostream>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "k_ipcmsg.h"

using namespace std;

#define UI_MSG_QUEUE_MAX_COUNT 100
#define _STR(s) #s
#define STR(s) _STR(s)

typedef struct {
    std::mutex mtx;
    std::queue<ui_msg_t *> msg_q;
} msg_mgt_t;

static msg_mgt_t msg_mgt;
static int ipcmsg_handle = -1;

static ui_msg_t *ui_msg_alloc(uint32_t size);
static int ui_msg_put(ui_msg_t *pmsg);

static int common_msg_proc_helper(ui_cmd_e cmd, int8_t *pdata)
{
    ui_msg_t *pmsg = ui_msg_alloc(sizeof(ui_msg_t));
    if (pmsg == NULL) {
        printf("%s no mem\n", __func__);
        return -1;
    }
    pmsg->cmd = cmd;
    pmsg->result = *pdata;

    return ui_msg_put(pmsg);
}

static void msg_recv(int handle, k_ipcmsg_message_t* msg)
{
    switch (msg->u32CMD) {
    case MSG_CMD_SIGNUP_RESULT:
        common_msg_proc_helper(UI_CMD_SIGNUP_RESULT, (int8_t *)(msg->pBody));
    break;
    case MSG_CMD_IMPORT_RESULT:
        common_msg_proc_helper(UI_CMD_IMPORT_RESULT, (int8_t *)(msg->pBody));
    break;
    case MSG_CMD_DELETE_RESULT:
        common_msg_proc_helper(UI_CMD_DELETE_RESULT, (int8_t *)(msg->pBody));
    break;
    case MSG_CMD_FEATURE_SAVE: {
        uint32_t phyaddr = *((uint32_t *)(msg->pBody));
        uint32_t length = *(((uint32_t *)(msg->pBody)) + 1);
        feature_db_save(phyaddr, length);
        break;
    }
    default:
    break;
    }
}

static void* thread_ipcmsg(void* arg)
{
    int handle;
    int fd = 0;
    printf("Connecting IPCMSG.");
    while(1)
    {
        fd = open("/dev/ipcm_user", O_RDWR);
        if(fd  < 0)
        {
            printf(".");
            usleep(10 * 1000);
        } else {
            close(fd);
            break;
        }
    }
    if(kd_ipcmsg_connect(&handle, "door_lock", msg_recv))
    {
        printf("failed\n.");
    } else {
        printf("Success\n.");
    }
    ipcmsg_handle = handle;
    kd_ipcmsg_run(ipcmsg_handle);
    return NULL;
}


static int msg_send_data(uint32_t cmd, void *payload, uint32_t payload_len)
{
    k_ipcmsg_message_t* pReq;

    if (ipcmsg_handle < 0)
        return -1;

    pReq = kd_ipcmsg_create_message(0, cmd, payload,
        payload_len);
    kd_ipcmsg_send_only(ipcmsg_handle, pReq);
    kd_ipcmsg_destroy_message(pReq);

    return 0;
}


static ui_msg_t *ui_msg_alloc(uint32_t size)
{
    ui_msg_t *msg;

    msg = (ui_msg_t *)malloc(sizeof(ui_msg_t) + size);

    return msg;
}

static int ui_msg_free(ui_msg_t *pmsg)
{
    if (pmsg) {
        free(pmsg);
        pmsg = NULL;
    }

    return 0;
}

static int ui_msg_get(ui_msg_t **ppmsg)
{
    int ret = 0;

    msg_mgt.mtx.lock();
    if (msg_mgt.msg_q.empty()) {
        ret = -1;
    } else {
        *ppmsg = msg_mgt.msg_q.front();
        msg_mgt.msg_q.pop();
    }
    msg_mgt.mtx.unlock();

    return ret;
}

static int ui_msg_put(ui_msg_t *pmsg)
{
    int ret = 0;

    msg_mgt.mtx.lock();
    if (msg_mgt.msg_q.size() >= UI_MSG_QUEUE_MAX_COUNT) {
        printf("%s ui msg more than max\n", __func__);
        ui_msg_free(pmsg);
        ret = -1;
    } else {
        msg_mgt.msg_q.push(pmsg);
    }
    msg_mgt.mtx.unlock();

    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

int msg_proc_init(void)
{
    int ret;
    k_ipcmsg_connect_t stConnectAttr;

    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = 101;
    stConnectAttr.u32Priority = 0;
    kd_ipcmsg_add_service("door_lock", &stConnectAttr);

    pthread_t tid_thread_ipcmsg;
    pthread_attr_t tattr_thread_ipcmsg;
    pthread_attr_init(&tattr_thread_ipcmsg);
    int max_prio = sched_get_priority_max(SCHED_RR);
    struct sched_param sp = {max_prio};
    pthread_attr_setschedpolicy(&tattr_thread_ipcmsg, SCHED_RR);
    pthread_attr_setschedparam(&tattr_thread_ipcmsg, &sp);
    pthread_attr_setdetachstate(&tattr_thread_ipcmsg,
                                PTHREAD_CREATE_DETACHED);
    pthread_create(&tid_thread_ipcmsg, &tattr_thread_ipcmsg,
                   thread_ipcmsg, NULL);
    pthread_attr_destroy(&tattr_thread_ipcmsg);

    return 0;
}

int msg_send_cmd(uint32_t cmd)
{
    char tmp = 0;
    return msg_send_data(cmd, &tmp, 1);
}

int msg_send_cmd_with_data(uint32_t cmd, void *payload, uint32_t payload_len)
{
    return msg_send_data(cmd, payload, payload_len);
}

int ui_msg_proc(void)
{
    int ret;
    ui_msg_t *pmsg;

    ret = ui_msg_get(&pmsg);
    if (ret != 0)
        return ret;

    switch (pmsg->cmd) {
    case UI_CMD_SIGNUP_RESULT:
    case UI_CMD_IMPORT_RESULT:
    case UI_CMD_DELETE_RESULT:
        scr_main_display_result(pmsg->result);
    break;
    }
    ui_msg_free(pmsg);

    return ret;
}

#ifdef __cplusplus
}
#endif
