#include "stdio.h"

#include "ipcm_funcs.h"
#include "ipcm_osadapt.h"

#include "rc_common.h"

static void *g_rc_server_handle = NULL;
static int g_rc_server_release = 0;

int rc_ipc_send(void *buf, int count)
{
	int ret;

	if (!g_rc_server_handle)
		return -1;
	ret = ipcm_vdd_sendmsg(g_rc_server_handle, buf, count);
	if (ret != count) {
		rc_error("write to [%d:%d] failed\n", RC_CLIENT_ID, RC_IPC_PORT);
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

extern int rc_server_dispatch_cmd(int fd, void *data, unsigned int count);

void rc_ipc_init(void *param)
{
	struct ipcm_vdd_opt ops;
	void *handle;
	while (!ipcm_vdd_node_ready(RC_CLIENT_ID)) {
		__ipcm_msleep__(10);
	}

	handle = ipcm_vdd_open(RC_CLIENT_ID,
			RC_IPC_PORT, __HANDLE_MSG_PRIORITY);
	if (!handle) {
		rc_error("open ipcm [%d:%d] failed\n",
				RC_CLIENT_ID, RC_IPC_PORT);
		return;
	}
	ops.recv = &rc_server_dispatch_cmd;
	ops.data = 0;
	ipcm_vdd_setopt(handle, &ops);
	ipcm_vdd_connect(handle, CONNECT_BLOCK);

	g_rc_server_handle = handle;
	while (1) {
		if (__HANDLE_DISCONNECTED
				== ipcm_vdd_check_handle(g_rc_server_handle)) {
			ipcm_vdd_connect(handle, CONNECT_NONBLOCK);
		}
		if (g_rc_server_release)
			break;
		__ipcm_msleep__(100);
	}

	ipcm_vdd_disconnect(handle);
	ipcm_vdd_setopt(handle, NULL);
	ipcm_vdd_close(handle);

	g_rc_server_handle = NULL;
	g_rc_server_release = 0;
}

void rc_ipc_deinit(void)
{
	g_rc_server_release = 1;
}


