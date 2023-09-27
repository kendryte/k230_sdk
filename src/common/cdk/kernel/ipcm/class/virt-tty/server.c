#include "ipcm_osadapt.h"
#include "ipcm_funcs.h"

#include "virt-tty_config.h"
#include "virt-tty.h"

#define VIRT_TTY_SERVER_THREAD    "Hi_virt-tty_server"

struct virt_tty_server {
	struct virt_tty_client clients[MAX_CLIENTS];
};

struct virt_tty_server monitor;
struct ipcm_task *virt_tty_server_task = NULL;

static int virt_tty_server_release = 0;

static int is_client_alive(int client_id)
{
	return ipcm_vdd_node_ready(client_id);
}

static int client_addr_parse(struct virt_tty_client *client, struct virt_tty_msg *msg)
{
	struct virt_tty_shared_buffer *buffer;

	buffer = __ipcm_mem_alloc__(sizeof(struct virt_tty_shared_buffer));
	if (!buffer) {
		ipcm_err("buffer alloc failed!");
		return -1;
	}
	buffer->phys = msg->data[0];
	buffer->size = msg->data[1];
	buffer->base = (unsigned long)__ipcm_io_mapping__(buffer->phys,
			buffer->size);
	if (!buffer->base) {
		__ipcm_mem_free__(buffer);
		ipcm_err("buffer mapping failed!");
		return -1;
	}
	buffer->wp = (unsigned int *)buffer->base;
	buffer->rp = (unsigned int *)(buffer->base + 0x04);
	buffer->circled = (int *)(buffer->base + 0x08);
	buffer->data = (char *)(buffer->base + 0x10);
	buffer->len = buffer->size - 0x10;
	client->buffer = buffer;
	return 0;
}

extern int virt_tty_server_register_cdev(void *data, const char *name, int id);
static int client_name_parse(struct virt_tty_client *client,
		struct virt_tty_msg *msg)
{
	int name_len;
	name_len = msg->data[0];
	if (name_len >= MAX_CLIENT_NAME_LEN) {
		ipcm_err("client name len: %d ,will be cut off!",
				name_len);
		name_len = MAX_CLIENT_NAME_LEN - 1;
	}
	__memcpy__(client->name, (void *)&msg->data[1], name_len);
	virt_tty_server_register_cdev(client, client->name, client->id);
	client->state = CLIENT_REGISTERED;
	return 0;
}

static int server_recv(void *handle, void *buf, unsigned int len)
{
	int ret;
	struct ipcm_vdd_opt opt;
	struct virt_tty_client *client;
	struct virt_tty_msg *msg = buf;

	ipcm_vdd_getopt(handle, &opt);
	client = (struct virt_tty_client *)opt.data;
	if (!client || !msg) {
		ipcm_err("invalid recv messages.");
		return -1;
	}
	switch(msg->cmd) {
	case VIRT_TTY_CMD_DATA_ADDR:
		ret = client_addr_parse(client, msg);
		break;
	case VIRT_TTY_CMD_STR_NAME:
		ret = client_name_parse(client, msg);
		break;

	default:
		ipcm_err("unsupported cmd.");
		ret = -1;
		break;
	}
	return ret;
}

extern void virt_tty_server_unregister_cdev(int id);
static void client_cleanup(struct virt_tty_client *client)
{
	if (CLIENT_REGISTERED == client->state)
		virt_tty_server_unregister_cdev(client->id);
	client->state = CLIENT_HALT;
	__memset__(client->name, 0, MAX_CLIENT_NAME_LEN);
	if (client->buffer) {
		__ipcm_io_unmapping__((void *)client->buffer->base);
		__ipcm_mem_free__(client->buffer);
		client->buffer = NULL;
	}
	if (client->handle) {
		ipcm_vdd_disconnect(client->handle);
		ipcm_vdd_setopt(client->handle, NULL);
		ipcm_vdd_close(client->handle);
		client->handle = NULL;
	}
}

static int is_client_connected(struct virt_tty_client *client)
{
	if (client->handle)
		return (__HANDLE_CONNECTED == ipcm_vdd_check_handle(client->handle));
	else
		return 0;
}

static int connecting_client(struct virt_tty_client *client)
{
	struct ipcm_vdd_opt opt;

	client->handle = ipcm_vdd_open(client->id, VIRT_TTY_PORT, __HANDLE_MSG_NORMAL);
	if (!client->handle) {
		ipcm_err("open ipcm [%d:%d] failed", client->id, VIRT_TTY_PORT);
		return -1;
	}
	opt.data = (unsigned long)client;
	opt.recv = &server_recv;
	ipcm_vdd_setopt(client->handle, &opt);
	ipcm_vdd_connect(client->handle, CONNECT_NONBLOCK);
	client->state = CLIENT_CONNECTING;
	return 1;
}

static int try_connect_client(struct virt_tty_client *client)
{
	if (!is_client_alive(client->id)) {
		return -1;
	}
	if (is_client_connected(client)) {
		if (CLIENT_CONNECTING == client->state)
			client->state = CLIENT_CONNECTED;
		return 0;
	}
	if (CLIENT_CONNECTING != client->state)
		return connecting_client(client);
	return 1;
}

static int virt_tty_server_monitor(void *data)
{
	int i = 0;
	int server_id;
	struct virt_tty_client *client;

	server_id = ipcm_vdd_localid();
	do {
		for (i=0; i<MAX_CLIENTS; i++) {
			if (i == server_id)
				continue;
			client = &monitor.clients[i];

			if (CLIENT_REGISTERED != client->state) {
				try_connect_client(client);
			} else {
				if (!is_client_connected(client)
						|| !is_client_alive(client->id)) {
					client_cleanup(client);
				}
			}
			__ipcm_msleep__(100);
		}
	} while (!__ipcm_thread_check_stop__(virt_tty_server_task));

	if (1 == virt_tty_server_release) {
		for (i=0; i<MAX_CLIENTS; i++) {
			if (i == server_id)
				continue;
			client = &monitor.clients[i];
			client_cleanup(client);
		}
	}
	return 0;
}

int virt_tty_server_read(void *data, void *buf, unsigned int len)
{
	unsigned long phead;
	unsigned int wp, rp;
	struct virt_tty_shared_buffer *buffer;
	struct virt_tty_client *client;

	client = data;
	if (NULL == client) {
		ipcm_err("invalid param!");
		return -1;
	}
	buffer = client->buffer;
	if (NULL == buffer) {
		ipcm_err("client buffer not sync!");
		return -1;
	}
	wp = *buffer->wp;
	rp = *buffer->rp;
	phead = (unsigned long)buffer->data + rp;
	if (!(*buffer->circled) && (wp >= rp)) {
		unsigned int copy_len = (len<(wp-rp)) ? len : (wp-rp);
		__memcpy__(buf, (void *)phead, copy_len);
		*buffer->rp = rp + copy_len;
		return copy_len;
	} else {
		if (len < (buffer->len - rp)) {
			__memcpy__(buf, (void *)phead, len);
			*buffer->rp =rp + len;
			return len;
		} else {
			unsigned int copy_len = buffer->len - rp;
			unsigned int left_len = len - copy_len;
			__memcpy__(buf, (void *)phead, copy_len);
			*buffer->rp = rp = 0;
			*buffer->circled = 0;
			phead = (unsigned long)buffer->data + rp;
			left_len = (left_len<(wp-rp)) ? left_len : (wp-rp);
			__memcpy__((void *)((unsigned long)buf + copy_len),
					(void *)phead, left_len);
			*buffer->rp = rp + left_len;
			return (copy_len + left_len);
		}
	}
	return -1;
}

int virt_tty_server_write(void *data, void const *buf, unsigned int len)
{
	struct virt_tty_client *client = data;
	if (NULL == client) {
		ipcm_err("invalid param!");
		return -1;
	}
	if (!is_client_connected(client)) {
		ipcm_err("client disconnected!");
		return -1;
	}
	return ipcm_vdd_sendmsg(client->handle, buf, len);
}

int virt_tty_server_init(void)
{
	int ret = 0;
	int i;
	struct virt_tty_client *client;

	for (i=0; i<MAX_CLIENTS; i++) {
		client = &monitor.clients[i];
		client->handle = NULL;
		client->buffer = NULL;
		__memset__(client->name, 0, MAX_CLIENT_NAME_LEN);
		client->state = CLIENT_HALT;
		client->id = i;
	}

	virt_tty_server_task = __ipcm_thread_create__(VIRT_TTY_SERVER_THREAD,
			virt_tty_server_monitor, NULL);
	if (NULL == virt_tty_server_task) {
		ipcm_err("create virt_tty server failed!");
		ret = -1;
	}
	return ret;
}

void virt_tty_server_cleanup(void)
{
	virt_tty_server_release = 1;

	if (virt_tty_server_task)
		__ipcm_thread_destroy__(virt_tty_server_task);
}
