#ifndef __IPCM_NODES_H__
#define __IPCM_NODES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#include "device_config.h"

#ifndef MAX_NODES
#define MAX_NODES	8
#endif
#define MAX_PORTS	1024

enum ipcm_node_state {
	NODE_ALIVE,
	NODE_READY,
	NODE_HALT,
	NODE_RESET
};

enum ipcm_handle_state {
	STATE_INIT = 0,     //first state when module init
	CONNECT_REQUESTING,  //send requesting when connecting
	CONNECT_REQUESTED,   //received requesting when connecting
	CONNECT_ACKING,   //send acking when connecting
	CONNECT_ACKED,   //received acking when connecting
};

/*
 * Be noted that the size of the following struct
 * is within 32 bytes. Please do not edit this
 * struct if unsure.
 */
struct ipcm_node_desc {
	volatile unsigned int state;
	volatile unsigned int recvbuf_pfn[MAX_NODES - 1];
};

struct ipcm_node {
	unsigned int id;
	struct ipcm_node_desc *desc;
	unsigned int irq;
	struct ipcm_shared_zone *sendbuf;
	struct ipcm_shared_zone *recvbuf;
	int state;
	struct ipcm_transfer_handle **handlers;
	int *handlers_state;
};

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
