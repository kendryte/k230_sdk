#include "stdio.h"
#include <rtthread.h>
#include <rthw.h>
#include <msh.h>
#include "rc_command.h"
#include "rc_common.h"

extern void rc_ipc_init(void* arg);
extern void rc_ipc_deinit(void);
extern int rc_ipc_send(void* buf, int count);

static int rc_ipc_exit = 0;
static rt_mailbox_t rc_mb;

static int send_resp(struct rc_request* req, int resp)
{
    int ret, resp_len;
    struct rc_response* resp_cmd;
    struct rc_response_exec* resp_exec;

    resp_len = sizeof(struct rc_response) + sizeof(struct rc_response_exec);
    resp_cmd = rt_malloc(resp_len);
    if (!resp_cmd) {
        rc_error("malloc for cmd[%d:%d] failed", req->cmd, req->id);
        return -1;
    }
    resp_cmd->cmd = req->cmd;
    resp_cmd->id = req->id;
    resp_exec = (struct rc_response_exec*)resp_cmd->response;
    resp_exec->ret = resp;
    ret = rc_ipc_send(resp_cmd, resp_len);
    rt_free(resp_cmd);

    return ret;
}

static int rc_s_exec(struct rc_request* req)
{
    int ret;
    struct rc_request_exec* req_exec;

    if (req->cmd != RC_CMD_EXEC) {
        rc_error("not supported CMD: %d", req->cmd);
        return send_resp(req, -EPERM);
    }
    req_exec = (struct rc_request_exec*)req->request;
    ret = msh_exec(req_exec->cmd, req_exec->len);
    ret = send_resp(req, ret);

    return ret;
}

static void rc_ipc_exec(void* arg)
{
    struct rc_request* req;

    while (!rc_ipc_exit) {
        rt_mb_recv(rc_mb, &req, RT_WAITING_FOREVER);
        rc_s_exec(req);
        rt_free(req);
    }
    rt_mb_delete(rc_mb);
}

int rc_server_dispatch_cmd(int fd, void* data, unsigned int count)
{
    void* msg = rt_malloc(count);

    if (msg == NULL) {
        rc_error("malloc for msg failed");
        return send_resp(data, -ENOMEM);
    }
    rt_memcpy(msg, data, count);
    if (RT_EOK != rt_mb_send(rc_mb, msg)) {
        rc_error("mailbox is full");
        return send_resp(data, -EBUSY);
    }

    return 0;
}

int rc_server_init(void)
{
    int ret;
    rt_thread_t rc_thread;

    rc_mb = rt_mb_create("rc_mb", 8, RT_IPC_FLAG_FIFO);
    if (rc_mb == RT_NULL) {
        rc_error("create rc mailbox failed");
        return -1;
    }

    rc_thread = rt_thread_create("rc_server_exec", rc_ipc_exec, RT_NULL, 0x4000, 5, 5);
    if (rc_thread == RT_NULL) {
        rc_error("create rc exec thread failed");
        return -1;
    }

    ret = rt_thread_startup(rc_thread);
    if (ret) {
        rc_error("rc exec thread startup failed");
        return ret;
    }

    rc_thread = rt_thread_create("rc_server", rc_ipc_init, RT_NULL, 0x1000, 5, 5);
    if (rc_thread == RT_NULL) {
        rc_error("create rc thread failed");
        return -1;
    }

    ret = rt_thread_startup(rc_thread);
    if (ret) {
        rc_error("rc thread startup failed");
        return ret;
    }

    return 0;
}

void rc_server_deinit(void)
{
    rc_ipc_deinit();
    rc_ipc_exit = 1;
}
