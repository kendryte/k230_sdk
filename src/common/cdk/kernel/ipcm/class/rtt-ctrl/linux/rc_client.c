#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "rc_client.h"
#include "rc_client.h"
#include "rc_command.h"
#include "rc_common.h"

static int exiting = 0;
static int resp_code;

extern int rc_client_send(void* data, int len);

void rc_client_response(void* data, unsigned int count)
{
    struct rc_response* resp_cmd = data;
    struct rc_response_exec* resp_exec = (struct rc_response_exec*)resp_cmd->response;
    printf("response cmd:%d id:%d ret:%d\n", resp_cmd->cmd, resp_cmd->id, resp_exec->ret);
    resp_code = resp_exec->ret;
    exiting = 1;
}

static void __exit(int sig)
{
    resp_code = -EINTR;
    exiting = 1;
}

int rc_client_init(void)
{
    return rc_ipc_init();
}

void rc_client_deinit(void)
{
    rc_ipc_cleanup();
}

int main(int argc, char* argv[])
{
    int ret = 0;
    struct rc_request* req_cmd;
    struct rc_request_exec* req_exec;
    struct rc_response* resp_cmd;
    struct rc_response_exec* resp_exec;
    int cmd_len;

    if (argc < 2) {
        rc_error("rtt-ctl cmd");
        return -EINVAL;
    }
    if (strlen(argv[1]) == 0) {
        rc_error("rtt-ctl cmd");
        return -EINVAL;
    }

    cmd_len = sizeof(struct rc_request) + sizeof(struct rc_request_exec);
    req_cmd = malloc(cmd_len);
    if (!req_cmd) {
        rc_error("alloc for open failed");
        return -ENOMEM;
    }
    memset(req_cmd, 0, cmd_len);
    req_cmd->cmd = RC_CMD_EXEC;
    req_exec = (struct rc_request_exec*)req_cmd->request;
    req_exec->len = strlen(argv[1]);
    if (req_exec->len >= sizeof(req_exec->cmd)) {
        rc_error("cmd is too long");
        free(req_cmd);
        return -EINVAL;
    }
    strcpy(req_exec->cmd, argv[1]);

    rc_client_init();
    (void)signal(SIGINT, __exit);

    for (int to = 0; to < 1000 && !rc_ipc_connected(); to++)
        usleep(1000);
    if (!rc_ipc_connected()) {
        rc_error("connect timeout");
        ret = -EBUSY;
        goto exit;
    }
    rc_client_send(req_cmd, cmd_len);
    for (int to = 0; to < 1000 && !exiting; to++)
        usleep(1000);
    if (!exiting) {
        rc_error("cmd resp timeout");
        ret = -ETIME;
        goto exit;
    }
    ret = resp_code;
exit:
    free(req_cmd);
    rc_client_deinit();
    return ret;
}
