/*sharefs server, user space */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#include "ipcm_userdev.h"
#include "sfs_common.h"

#define SFS_IPC_BUF_SIZE    0x400
struct sfs_client {
	int id;
	pthread_t thread;
};
static struct sfs_client g_clients[SFS_CLIENTS_CNT];
static int	  sfs_ipc_exit = 0;

int sfs_server_send(int ipc_fd, void *data, int len)
{
	ssize_t ret = write(ipc_fd, data, len);
	if (ret != len) {
		sfs_error("write to [%d] failed\n", ipc_fd);
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

extern int sfs_server_dispatch_cmd(int fd, void *data, unsigned int count);
static void *client_routine(void *arg)
{
	int state;
	int ret;
	fd_set rfds;
	struct timeval timeout;

	struct sfs_client *client = arg;
	struct ipcm_handle_attr attr;
	void *read_buf = NULL;
	int fd;
	int open_count = 500; //open count timeout set
	while (1) {
		fd = open("/dev/ipcm_user", O_RDWR);
		if (fd >= 0)
			break;
		usleep(10000);
		if (!(--open_count)) {
			sfs_error("open /dev/ipcm_user failed, timeout.\n");
			goto out;
		}
	}
	ioctl(fd, HI_IPCM_IOC_ATTR_INIT, &attr);
	attr.target = client->id;
	attr.port = SFS_IPC_PORT;
	attr.priority = HANDLE_MSG_PRIORITY;
	ret = ioctl(fd, HI_IPCM_IOC_CONNECT, &attr);
	if (ret < 0) {
		goto close_ipcm;
	}
	read_buf = malloc(SFS_IPC_BUF_SIZE + 1);
	if (NULL == read_buf) {
		sfs_error("malloc for read buf failed\n");
		goto close_ipcm;
	}
	do {
		if (sfs_ipc_exit)
			break;
		state = ioctl(fd, HI_IPCM_IOC_CHECK, NULL);
		if (HANDLE_DISCONNECTED == state) {
			(void)ioctl(fd, HI_IPCM_IOC_TRY_CONNECT, &attr);
		}
		timeout.tv_sec  = 10;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &timeout);
		if (ret < 0) {
			sfs_error("select error\n");
			break;
		} else if (ret == 0) {
			continue;
		}
		ret = read(fd, read_buf, SFS_IPC_BUF_SIZE);
		if (ret > 0)
			sfs_server_dispatch_cmd(fd, read_buf, ret);
	} while (1);
	free(read_buf);
close_ipcm:
	close(fd);
out:
	pthread_exit(0);
	return NULL;
}

int sfs_ipc_init(void *arg)
{
	int i;
	int clients = SFS_CLIENTS;
	sfs_ipc_exit = 0;

	for (i=0; i < SFS_CLIENTS_CNT; i++) {
		memset(&g_clients[i], 0, sizeof(struct sfs_client));
		g_clients[i].id = ffs(clients) - 1;
		clients &= ~(1<<(ffs(clients)-1));
		if (pthread_create(&g_clients[i].thread,
					NULL, &client_routine, &g_clients[i])) {
			sfs_error("create thread for %d client failed\n", i);
			return -1;
		}
	}
	return 0;
}

void sfs_ipc_cleanup(void)
{
	int i;
	void *arg;
	sfs_ipc_exit = 1;
	for (i=0; i < SFS_CLIENTS_CNT; i++) {
		if (g_clients[i].thread)
			pthread_join(g_clients[i].thread, &arg);
	}
}
