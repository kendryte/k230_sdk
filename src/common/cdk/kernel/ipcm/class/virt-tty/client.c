#include "ipcm_osadapt.h"
#include "ipcm_funcs.h"

#include "virt-tty_config.h"
#include "virt-tty.h"

#define VIRT_TTY_CLIENT_THREAD    "virt-tty_client"

static struct virt_tty_client g_client = {0};
static struct virt_tty_shared_buffer send_buf = {0};
static struct ipcm_task *virt_tty_client_task = NULL;
static int server_id;

#define BUFF_LEN     0x1000
struct circle_buffer {
	int wp;
	int rp;
	int circled;
	char data[BUFF_LEN];
	int len;
};
static struct circle_buffer recv_buf = {0};

void circle_buf_init(struct circle_buffer *buf)
{
	buf->wp = 0;
	buf->rp = 0;
	buf->circled = 0;
	buf->len = BUFF_LEN;
}

int is_circle_buf_empty(struct circle_buffer *buf)
{
	return (buf->circled)? 0: (buf->wp == buf->rp);
}

int circle_buf_write(struct circle_buffer *buf, const char *data, int len)
{
	unsigned long phead = (unsigned long)buf->data + buf->wp;
	int wp = buf->wp;
	int rp = buf->rp;

	if (len <= (buf->len - wp)) {
		__memcpy__((void *)phead, data, len);
		buf->wp = wp + len;
		if ((buf->circled) && (buf->wp > rp))
			buf->rp = buf->wp;
		return len;
	} else {
		unsigned int copy_len = buf->len - wp;
		unsigned int left_len = len - copy_len;
		__memcpy__((void *)phead, data, copy_len);
		buf->wp = wp = 0;
		buf->circled = 1;
		phead = (unsigned long)buf->data;
		if (left_len > buf->len) {
			/* circle buf overflow! */
			left_len = buf->len;
		}
		__memcpy__((void *)phead,
				(void *)((unsigned long)data + copy_len), left_len);
		buf->wp = wp + left_len;
		if (buf->wp > rp)
			buf->rp = buf->wp;
		return (copy_len + left_len);
	}
	return -1;
}

int circle_buf_read(struct circle_buffer *buf, char *data, int len)
{
	unsigned long phead = (unsigned long)buf->data + buf->rp;
	int wp = buf->wp;
	int rp = buf->rp;

	if (!(buf->circled) && (wp >= rp)) {
		if (len > (wp - rp))
			len = wp - rp;
		__memcpy__(data, (void *)phead, len);
		buf->rp = rp + len;
		return len;
	} else {
		if (len < (buf->len - rp)) {
			__memcpy__(data, (void *)phead, len);
			buf->rp = rp + len;
			return len;
		} else {
			unsigned int copy_len = buf->len - rp;
			unsigned int left_len = len - copy_len;
			__memcpy__(data, (void *)phead, copy_len);
			buf->rp = rp = 0;
			buf->circled = 0;
			phead = (unsigned long)buf->data;
			if (left_len > (wp - rp))
				left_len = wp - rp;
			__memcpy__((void *)((unsigned long)data + copy_len),
					(void *)phead, left_len);
			buf->rp = rp + left_len;
			return (copy_len + left_len);
		}
	}
	return -1;
}

static int is_server_alive(void)
{
	return ipcm_vdd_node_ready(server_id);
}

extern void virt_tty_client_recv_notify(void);
static int client_recv(void *handle, void *buf, unsigned int len)
{
	circle_buf_write(&recv_buf, buf, len);
	/* notify */
	virt_tty_client_recv_notify();
	return 0;
}

static int is_server_connected(struct virt_tty_client *client)
{
	if (client->handle)
		return (__HANDLE_CONNECTED == ipcm_vdd_check_handle(client->handle));
	else
		return 0;
}

static void disconnect_server(struct virt_tty_client *client)
{
	if (client->handle) {
		ipcm_vdd_disconnect(client->handle);
		ipcm_vdd_setopt(client->handle, NULL);
		ipcm_vdd_close(client->handle);
	}
	client->handle = NULL;
	client->state = CLIENT_ALIVE;
}

static int connecting_server(struct virt_tty_client *client)
{
	struct ipcm_vdd_opt opt;

	client->handle = ipcm_vdd_open(server_id, VIRT_TTY_PORT, __HANDLE_MSG_NORMAL);
	if (!client->handle) {
		ipcm_err("open ipcm [%d:%d] failed", server_id, VIRT_TTY_PORT);
		return -1;
	}
	opt.data = (unsigned long)client;
	opt.recv = &client_recv;
	ipcm_vdd_setopt(client->handle, &opt);
	ipcm_vdd_connect(client->handle, CONNECT_NONBLOCK);
	client->state = CLIENT_CONNECTING;
	return 1;
}

static int try_connect_server(struct virt_tty_client *client)
{
	if (!is_server_alive())
		return -1;

	if (is_server_connected(client)) {
		if (CLIENT_CONNECTING == client->state)
			client->state = CLIENT_CONNECTED;
		return 0;
	}
	if (CLIENT_CONNECTING != client->state)
		return connecting_server(client);
	return 1;
}

static int virt_tty_client_send_addr(struct virt_tty_client *client)
{
	struct virt_tty_msg msg;

	msg.cmd = VIRT_TTY_CMD_DATA_ADDR;
	msg.data[0] = (unsigned long long)client->buffer->phys;
	msg.data[1] = (unsigned long long)client->buffer->size;

	return ipcm_vdd_sendmsg(client->handle, &msg, sizeof(struct virt_tty_msg));
}

static int virt_tty_client_send_name(struct virt_tty_client *client)
{
	int ret;
	int name_len;
	int msg_len;
	struct virt_tty_msg *msg = NULL;

	name_len = __strlen__(client->name);
	msg_len = sizeof(struct virt_tty_msg) + name_len;

	msg = __ipcm_mem_alloc__(msg_len);
	if (NULL == msg) {
		ipcm_err("malloc message failed.");
		return -1;
	}

	msg->cmd = VIRT_TTY_CMD_STR_NAME;
	msg->data[0] = name_len;
	__memcpy__((void *)&msg->data[1], client->name, name_len);
	ret = ipcm_vdd_sendmsg(client->handle, msg, msg_len);
	if (ret == msg_len) {
		client->state = CLIENT_REGISTERED;
		ret = 0;
	}
	__ipcm_mem_free__(msg);
	return ret;
}

static int virt_tty_client_thread(void *data)
{
	int ret;
	struct virt_tty_client *client = data;

	do {
		if (CLIENT_REGISTERED != client->state) {
			if (!try_connect_server(client)) {
				virt_tty_client_send_addr(client);
				virt_tty_client_send_name(client);
			}
			__ipcm_msleep__(1);
		} else {
			if (!is_server_connected(client)
					|| !is_server_alive()) {
				disconnect_server(client);
				client->state = CLIENT_HALT;
			}
			__ipcm_msleep__(1000);
		}
	} while (!__ipcm_thread_check_stop__(virt_tty_client_task));

	disconnect_server(client);

	return 0;
}

int virt_tty_client_buffer_init(void);
int virt_tty_client_send(void const *buf, unsigned int len)
{
	unsigned long phead;
	unsigned int wp, rp;
	struct virt_tty_shared_buffer *buffer;
	struct virt_tty_client *client = &g_client;

	if (!client->buffer && virt_tty_client_buffer_init())
		return -1;

	buffer = client->buffer;
	wp = *buffer->wp;
	rp = *buffer->rp;
	phead = (unsigned long)buffer->data + wp;

	if (len <= (buffer->len - wp)) {
		__memcpy__((void *)phead, buf, len);
		*buffer->wp = wp + len;
		if ((*buffer->circled) && (*buffer->wp > rp))
			*buffer->rp = *buffer->wp;
		return len;
	} else {
		unsigned int copy_len = buffer->len - wp;
		unsigned int left_len = len - copy_len;
		__memcpy__((void *)phead, buf, copy_len);
		*buffer->wp = wp = 0;
		*buffer->circled = 1;
		phead = (unsigned long)buffer->data + wp;
		if (left_len > buffer->len) {
			/* circle buf overflow! */
			left_len = buffer->len;
		}
		__memcpy__((void *)phead,
				(void *)((unsigned long)buf + copy_len), left_len);
		*buffer->wp = wp + left_len;
		if (*buffer->wp > rp)
			*buffer->rp = wp;
		return (copy_len + left_len);
	}
}

int virt_tty_client_received(void)
{
	return !is_circle_buf_empty(&recv_buf);
}

int virt_tty_client_read(void *buf, unsigned int len)
{
	return circle_buf_read(&recv_buf, buf, len);
}

int virt_tty_client_buffer_init(void)
{
	struct virt_tty_shared_buffer *buffer = &send_buf;
	struct virt_tty_client *client = &g_client;
	char *name = VIRT_TTY_NAME;
	unsigned long phys = VIRT_TTY_PHYS;
	unsigned long size = VIRT_TTY_SIZE;
	int name_len;

	server_id = VIRT_TTY_SERVER_ID;
	if (!phys || !size) {
		ipcm_err("invalid phys or size!");
		return -1;
	}
	name_len = __strlen__(name);
	if (name_len <= 0 || name_len >= MAX_CLIENT_NAME_LEN) {
		ipcm_err("invalid client name length");
		return -1;
	}
	__memcpy__(client->name, name, name_len);

	circle_buf_init(&recv_buf);
	buffer->phys = phys;
	buffer->size = size;
	buffer->base = (unsigned long)__ipcm_io_mapping__(phys, size);
	if (!buffer->base) {
		ipcm_err("buffer mapping failed!");
		return -1;
	}
	__memset__((void *)buffer->base, 0x00, buffer->size);
	buffer->wp = (unsigned int *)(buffer->base);
	buffer->rp = (unsigned int *)(buffer->base + 0x04);
	buffer->circled = (int *)(buffer->base + 0x08);
	buffer->data = (char *)(buffer->base + 0x10);
	buffer->len = buffer->size - 0x10;

	client->buffer = buffer;
	return 0;
}

int virt_tty_client_init(void)
{
	int ret;
	struct virt_tty_client *client = &g_client;

	client->handle = NULL;
	client->state = CLIENT_HALT;
	client->id = 0;

	if (!client->buffer && virt_tty_client_buffer_init())
		return -1;

	virt_tty_client_task = __ipcm_thread_create__(VIRT_TTY_CLIENT_THREAD,
			virt_tty_client_thread, client);
	if (NULL == virt_tty_client_task) {
		ipcm_err("create virt_tty client thread failed!");
		return -1;
	}
	return 0;
}

void virt_tty_client_cleanup(void)
{
	struct virt_tty_client *client = &g_client;

	__ipcm_thread_destroy__(virt_tty_client_task);

	__memset__(client->name, 0, MAX_CLIENT_NAME_LEN);
	client->state = CLIENT_HALT;
	if (client->buffer) {
		__ipcm_io_unmapping__((void *)client->buffer->base);
		client->buffer = NULL;
	}
}
