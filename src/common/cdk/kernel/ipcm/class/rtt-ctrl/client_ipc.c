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
#include "rc_common.h"


#define RC_IPC_BUF_SIZE    0x400
struct rc_client {
	int id;
	pthread_t thread;
};
static struct rc_client g_clients;
static int	  rc_ipc_exit = 0;
static int      fd = -1;

int rc_client_send(void *data, int len)
{
	ssize_t ret = write(fd, data, len);
	if (ret != len) {
		rc_error("write to [%d] failed\n", fd);
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

extern void rc_client_response(void *data, unsigned int count);

int rc_client_recv(void *handle, void *buf, unsigned int count)
{
	rc_client_response(buf, count);
	return 0;
}

static void *client_routine(void *arg)
{
	int state;
	int ret;
	fd_set rfds;
	struct timeval timeout;

	struct rc_client *client = arg;
	struct ipcm_handle_attr attr;
	void *read_buf = NULL;
	
	int open_count = 500; //open count timeout set
	while (1) {
		fd = open("/dev/ipcm_user", O_RDWR);
		if (fd >= 0)
			break;
		usleep(10000);
		if (!(--open_count)) {
			rc_error("open /dev/ipcm_user failed, timeout.\n");
			goto out;
		}
	}
	ioctl(fd, HI_IPCM_IOC_ATTR_INIT, &attr);
	attr.target = client->id;
	attr.port = RC_IPC_PORT;
	attr.priority = HANDLE_MSG_PRIORITY;
	ret = ioctl(fd, HI_IPCM_IOC_CONNECT, &attr);
	if (ret < 0) {
		goto close_ipcm;
	}
	read_buf = malloc(RC_IPC_BUF_SIZE + 1);
	if (NULL == read_buf) {
		rc_error("malloc for read buf failed\n");
		goto close_ipcm;
	}
	do {
		if (rc_ipc_exit)
			break;
		state = ioctl(fd, HI_IPCM_IOC_CHECK, NULL);
		if (HANDLE_DISCONNECTED == state) {
			(void)ioctl(fd, HI_IPCM_IOC_CONNECT, &attr);
		}
		timeout.tv_sec  = 0;
		timeout.tv_usec = 200000;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &timeout);
		if (ret < 0) {
			rc_error("select error\n");
			break;
		} else if (ret == 0) {
			continue;
		}
		ret = read(fd, read_buf, RC_IPC_BUF_SIZE);
		if (ret > 0)
			rc_client_recv(fd, read_buf, ret);
	} while (1);
	free(read_buf);
    if(ioctl(fd, HI_IPCM_IOC_DISCONNECT, NULL))
    {
        rc_error("disconnect error\n");
    }
close_ipcm:
	close(fd);
out:
	pthread_exit(0);
	return NULL;
}

int rc_ipc_init(void *arg)
{
	int i;

	rc_ipc_exit = 0;

    memset(&g_clients, 0, sizeof(struct rc_client));
    g_clients.id = 1;

    if (pthread_create(&g_clients.thread,
                NULL, &client_routine, &g_clients)) {
        rc_error("create thread for client failed\n");
        return -1;
    }

	return 0;
}

void rc_ipc_cleanup(void)
{
	int i;
	void *arg;
	rc_ipc_exit = 1;

    if (g_clients.thread)
        pthread_join(g_clients.thread, &arg);

}

int rc_ipc_connected(void)
{
    int state;
	if (fd < 0)
		return 0;
    state = ioctl(fd, HI_IPCM_IOC_CHECK, NULL);

    return (HANDLE_CONNECTED == state);
}