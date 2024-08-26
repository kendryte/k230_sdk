#ifndef  __VIRT_TTY_HEADER__
#define  __VIRT_TTY_HEADER__

#include "device_config.h"
#ifdef MAX_NODES
#define MAX_CLIENTS   MAX_NODES
#else
#define MAX_CLIENTS   8 
#endif
#define MAX_CLIENT_NAME_LEN   16
#define VIRT_TTY_PORT    5

enum message_cmd {
	VIRT_TTY_CMD_DATA_ADDR,
	VIRT_TTY_CMD_STR_NAME,
	VIRT_TTY_CMD_STR_CMD
};

struct virt_tty_msg {
	int cmd;
	unsigned long long data[2];
};

struct virt_tty_shared_buffer {
	unsigned long phys;
	unsigned long size;
	unsigned long base;

	volatile char *data;
	unsigned int len;
	volatile unsigned int *wp;
	volatile unsigned int *rp;
	volatile int *circled;
};

enum client_state {
	CLIENT_HALT,
	CLIENT_ALIVE,
	CLIENT_CONNECTING,
	CLIENT_CONNECTED,
	CLIENT_REGISTERED
};

struct virt_tty_client {
	void *handle;
	char name[MAX_CLIENT_NAME_LEN];
	struct virt_tty_shared_buffer *buffer;
	int state;
	int id;
};

#endif
