#ifndef __IPCM_FUNCS_H__
#define __IPCM_FUNCS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

enum message_type {
	MESSAGE_NORMAL,
	MESSAGE_URGENT,
	MESSAGE_CONNECT_REQUEST,
	MESSAGE_CONNECT_ACK,
	MESSAGE_DISCONNECT_REQUEST,
};
enum connect_block {
	CONNECT_NONBLOCK,
	CONNECT_BLOCK
};

enum __handle_state {
	__HANDLE_DISCONNECTED,
	__HANDLE_CONNECTING,
	__HANDLE_CONNECTED
};

enum __message_priority {
	__HANDLE_MSG_NORMAL,
	__HANDLE_MSG_PRIORITY
};

typedef int (*recv_notifier)(void *handle, void *buf, unsigned int len);

typedef struct ipcm_vdd_opt {
	recv_notifier recv;
	unsigned long data;
} ipcm_vdd_opt_t;

void *ipcm_vdd_open(int target, int port, int priority);
void ipcm_vdd_close(void *handle);
int ipcm_vdd_sendmsg(void *handle, const void *buf, unsigned int len);
int ipcm_vdd_connect(void *handle, int is_block);
void ipcm_vdd_disconnect(void *handle);
int ipcm_vdd_recvmsg(int *source, int *port, char *pbuf, int len);
int ipcm_vdd_check_handle(void *handle);
int ipcm_vdd_getopt(void *handle, struct ipcm_vdd_opt *opt);
int ipcm_vdd_setopt(void *handle, struct ipcm_vdd_opt *opt);

int ipcm_vdd_localid(void);
int ipcm_vdd_remoteids(int *ids);
int ipcm_vdd_init(void);
void ipcm_vdd_cleanup(void);

int ipcm_vdd_node_ready(int target);
int ipcm_vdd_reset_node(int target, int timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
