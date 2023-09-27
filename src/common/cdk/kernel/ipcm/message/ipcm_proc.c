#include "ipcm_osadapt.h"
#include "ipcm_desc.h"
#include "ipcm_nodes.h"
#include "ipcm_funcs.h"
#include "ipcm_buffer.h"
#include "device_config.h"
#include "ipcm_config_common.h"

extern struct ipcm_node_desc *g_nodes_desc;
extern struct ipcm_node *ipcm_get_node(int id);
extern int ipcm_node_ready(int target);

static unsigned long get_phys_from_desc(int nid, int hnid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	unsigned int pfn;

	if (nid < hnid) {
		pfn = desc[hnid].recvbuf_pfn[nid];
	} else if (nid > hnid) {
		pfn = desc[hnid].recvbuf_pfn[nid - 1];
	} else {
		return 0;
	}
	return pfn<<PAGE_SHIFT;
}


int __ipcm_read_proc__(void * data)
{
	int nid, j;
	int flag = 0;
	struct ipcm_node *node;
	unsigned int size;
	unsigned long phys;
	struct ipcm_transfer_handle *handle;
	const char *state;

	for (nid=0; nid < MAX_NODES; nid++) {
		if (NULL == (node = ipcm_get_node(nid)))
			continue;

		switch (node->state) {
			case NODE_HALT:
				state = "HALT";
				break;
			case NODE_ALIVE:
				state = "ALIVE";
				break;
			case NODE_READY:
				state = "READY";
				break;
			case NODE_RESET:
				state = "RESET";
				break;
			default:
				state = "FAULT";
				break;
		}
		/* node information */
		if (nid == LOCAL_ID) {
			__ipcm_proc_printf__(data, "\n*---LOCAL  NODE: ID=%d, STATE: %s\n",
					node->id, state);
			continue;
		} else {
			__ipcm_proc_printf__(data, "\n*---REMOTE NODE: ID=%d, STATE: %s\n",
					node->id, state);
		}

		/* receive buffer information */
		__ipcm_proc_printf__(data, " |-RECV BUFFER,");
		if (0 == LOCAL_ID) {
			if (ipcm_node_ready(nid)) {
				phys = get_phys_from_desc(nid, 0);
				size = node->recvbuf->region.size;
			} else {
				phys = 0;
				size = 0;
			}
		} else {
			cfg_2local_shm_phys(nid, phys);
			cfg_2local_shm_size(nid, size);
		}
		if (phys)
			__ipcm_proc_printf__(data, " PHYS<0x%016X, 0x%08X>\n",
					phys, size);
		else
			__ipcm_proc_printf__(data, " PHYS<NULL, 0x%08X>\n",
					size);

		/* send buffer information */
		__ipcm_proc_printf__(data, " |-SEND BUFFER,");
		if (0 == nid) {
			cfg_2remote_shm_phys(nid, phys);
			cfg_2remote_shm_size(nid, size);
		} else {
			if (ipcm_node_ready(nid)) {
				phys = get_phys_from_desc(LOCAL_ID, nid);
				size = node->sendbuf->region.size;
			} else {
				phys = 0;
				size = 0;
			}
		}
		if (phys)
			__ipcm_proc_printf__(data, " PHYS<0x%016X, 0x%08X>\n",
					phys, size);
		else
			__ipcm_proc_printf__(data, " PHYS<NULL, 0x%08X>\n",
					size);

		if ((nid == LOCAL_ID) || (!ipcm_node_ready(nid))) {
			continue;
		}
		/* every handler information which is opened */
		flag = 0;
		for (j=0; j<MAX_PORTS; j++) {
			if (NULL == node->handlers)
				break;
			handle = node->handlers[j];
			if (!handle)
				continue;
			if (!flag) {
				flag = 1;
				__ipcm_proc_printf__(data, "  |-Port | State       ");
				__ipcm_proc_printf__(data, " | Send Count | Recv Count");
				__ipcm_proc_printf__(data, " | Max Send Len | Max Recv Len\n");
			}
			switch(handle->state) {
				case __HANDLE_CONNECTED:
					state = "Connected";
					break;
				case __HANDLE_CONNECTING:
					state = "Connecting";
					break;
				case __HANDLE_DISCONNECTED:
					state = "Disconnected";
					break;
				default:
					state = "Fault";
					break;
			}
			__ipcm_proc_printf__(data, "    %-4d   %-12s",
					handle->port, state);
			__ipcm_proc_printf__(data, "   %-10d   %-10d",
					__ipcm_atomic_read__(&handle->send_count),
					__ipcm_atomic_read__(&handle->recv_count));
			__ipcm_proc_printf__(data, "   %-12d   %-12d\n",
					__ipcm_atomic_read__(&handle->max_send_len),
					__ipcm_atomic_read__(&handle->max_recv_len));
		}
	}
	return 0;
}


