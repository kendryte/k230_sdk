/*sharefs client, kernel space */
#include "stdio.h"

#include "ipcm_funcs.h"
#include "ipcm_osadapt.h"

#include "sfs_common.h"

static void *g_sfs_client_handle = NULL;
static int g_sfs_client_release = 0;

int sfs_client_send(void *buf, int count)
{
	int ret;

	if (!g_sfs_client_handle)
		return -1;
	ret = ipcm_vdd_sendmsg(g_sfs_client_handle, buf, count);
	if (ret != count) {
		sfs_error("write to [%d:%d] failed\n", SFS_SERVER_ID, SFS_IPC_PORT);
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

extern void sfs_client_response(void *buf, int count);
int sfs_client_recv(void *handle, void *buf, unsigned int count)
{
	sfs_client_response(buf, count);
	return 0;
}

void *sfs_ipc_init(void *param)
{
	struct ipcm_vdd_opt ops;
	void *handle;
	while (!ipcm_vdd_node_ready(SFS_SERVER_ID)) {
		__ipcm_msleep__(10);
	}

	handle = ipcm_vdd_open(SFS_SERVER_ID,
			SFS_IPC_PORT, __HANDLE_MSG_PRIORITY);
	if (!handle) {
		sfs_error("open ipcm [%d:%d] failed\n",
				SFS_SERVER_ID, SFS_IPC_PORT);
		return NULL;
	}
	ops.recv = &sfs_client_recv;
	ops.data = 0;
	ipcm_vdd_setopt(handle, &ops);
	ipcm_vdd_connect(handle, CONNECT_BLOCK);

	g_sfs_client_handle = handle;
	while (1) {
		if (__HANDLE_DISCONNECTED
				== ipcm_vdd_check_handle(g_sfs_client_handle)) {
			ipcm_vdd_connect(handle, CONNECT_NONBLOCK);
		}
		if (g_sfs_client_release)
			break;
		__ipcm_msleep__(100);
	}

	ipcm_vdd_disconnect(handle);
	ipcm_vdd_setopt(handle, NULL);
	ipcm_vdd_close(handle);

	g_sfs_client_handle = NULL;
	g_sfs_client_release = 0;
	return NULL;
}

void sfs_ipc_deinit(void)
{
	g_sfs_client_release = 1;
}

int sfs_ipc_connected(void)
{
	if (!g_sfs_client_handle)
		return 0;
	return (__HANDLE_CONNECTED == ipcm_vdd_check_handle(g_sfs_client_handle));
}
