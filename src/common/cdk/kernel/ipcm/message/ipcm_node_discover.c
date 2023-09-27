#include "ipcm_desc.h"
#include "ipcm_osadapt.h"
#include "device_config.h"
#include "ipcm_nodes.h"
#include "ipcm_buffer.h"
#include "ipcm_config_common.h"

extern struct ipcm_node_desc *g_nodes_desc;
extern struct ipcm_task *node_detect_task;
extern struct ipcm_node *ipcm_get_node(int id);

/*  nodes desc state bits definition
 *  31    30      ------|--------|--------|--------
 *  alive recover 000000|reset   |ready   |connect
 */

void set_local_alive(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	desc[LOCAL_ID].state |= (1<<31);
}
void clear_local_alive(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	desc[LOCAL_ID].state &= ~(1<<31);
}
int is_node_alive(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1<<31)) {
		ipcm_trace(TRACE_DESC, "node %d is alive!", nid);
		return 1;
	}
	return 0;
}


void set_local_recover(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	desc[LOCAL_ID].state |= (1<<30);
}
void clear_local_recover(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	desc[LOCAL_ID].state &= ~(1<<30);
}
int is_local_recover(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (desc[LOCAL_ID].state & (1<<30))
		return 1;
	return 0;
}
int is_node_recover(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1<<30)) {
		ipcm_trace(TRACE_DESC, "node %d is recover!", nid);
		return 1;
	}
	return 0;
}

/* only master can do reset operation, and the reset bits are located
 * in desc[0] only.
 * */
void set_node_reset(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[0].state |= (1<<(nid + RESET_STATE_SHIFT));
}
void clear_node_reset(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[0].state &= ~(1<<(nid + RESET_STATE_SHIFT));
}
int is_node_reset(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[0].state & (1<<(nid + RESET_STATE_SHIFT))) {
		ipcm_trace(TRACE_DESC, "node %d is reset!", nid);
		return 1;
	}
	return 0;
}

void set_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[LOCAL_ID].state |= (1<<(nid + READY_STATE_SHIFT));
}
void clear_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[LOCAL_ID].state &= ~(1<<(nid + READY_STATE_SHIFT));
}
int is_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[LOCAL_ID].state & (1<<(nid + READY_STATE_SHIFT))) {
		ipcm_trace(TRACE_DESC, "local for node %d is ready!", nid);
		return 1;
	}
	return 0;
}
int is_node_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1<<(LOCAL_ID + READY_STATE_SHIFT))) {
		ipcm_trace(TRACE_DESC, "node %d is ready!", nid);
		return 1;
	}
	return 0;
}

int is_local_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[LOCAL_ID].state & (1 << nid))
		return 1;
	return 0;
}
int is_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1 << LOCAL_ID))
		return 1;
	return 0;
}
int set_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	struct ipcm_node *node;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return -1;
	}

	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return -1;
	}

	node->handlers = __ipcm_mem_alloc__(MAX_PORTS * sizeof(void *));
	if (NULL == node->handlers) {
		ipcm_err("alloc mem for node %d handlers failed!", nid);
		return -1;
	}
	__memset__(node->handlers, 0, MAX_PORTS * sizeof(void *));

	node->handlers_state = __ipcm_mem_alloc__(MAX_PORTS * sizeof(int));
	if (NULL == node->handlers_state) {
		ipcm_err("alloc mem for node %d handlers_state failed!", nid);
		__ipcm_mem_free__(node->handlers);
		return -1;
	}
	__memset__(node->handlers_state, 0, MAX_PORTS * sizeof(int));

	desc[LOCAL_ID].state |= (1<<nid);
	/* wait for the remote set the connect flag */
	while(!is_node_connected(nid))
		__ipcm_msleep__(1);

	node->state = NODE_READY;
	return 0;
}

extern int wait_handlers_release(struct ipcm_node *node, int timeout);
void clear_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	struct ipcm_node *node;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return;
	}

	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return;
	}
	desc[LOCAL_ID].state &= ~(1<<nid);

	if (NODE_READY != node->state)
		return;

	node->state = NODE_HALT;
	wait_handlers_release(node, 0);
	if (NULL != node->handlers) {
		__ipcm_mem_free__(node->handlers);
		node->handlers = NULL;
	}
	if (NULL != node->handlers_state) {
		__ipcm_mem_free__(node->handlers_state);
		node->handlers_state = NULL;
	}
}

void set_desc_info(int nid, int hnid, unsigned int pfn, unsigned int size)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

	ipcm_trace(TRACE_DESC, "nid %d hnid %d, pfn 0x%x, size 0x%x",
			nid, hnid, pfn, size);
	ipcm_trace(TRACE_DESC, "addr of desc[%d].recvbuf_pfn[%d] %p",
			hnid, nid, &desc[hnid].recvbuf_pfn[nid]);

	if (hnid > nid)
		desc[hnid].recvbuf_pfn[nid] = pfn;
	else if (hnid < nid)
		desc[hnid].recvbuf_pfn[nid - 1] = pfn;
	else
		ipcm_err("hnid = nid = %d", nid);
}

void clear_desc_info(int nid, int hnid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

	if (nid < 0 || hnid < 0) {
		ipcm_err("nid or hnid is error!");
		return;
	}

	if (hnid > nid)
		desc[hnid].recvbuf_pfn[nid] = 0;
	else if (hnid < nid)
		desc[hnid].recvbuf_pfn[nid - 1] = 0;
	else
		ipcm_err("hnid = nid = %d", nid);
}

void get_desc_info(int nid, int hnid, unsigned int *pfn, unsigned int *size)
{
	unsigned long zone_base;
	struct ipcm_node_desc *desc = g_nodes_desc;
	char *addr;
	int val;

	if (nid < hnid) {
		*pfn = desc[hnid].recvbuf_pfn[nid];
		ipcm_trace(TRACE_DESC, "pfn 0x%x, desc %p [%d%d] &pfn %p",
			*pfn, desc, hnid, nid, &desc[hnid].recvbuf_pfn[nid]);
	} else if (nid > hnid) {
		*pfn = desc[hnid].recvbuf_pfn[nid - 1];
		ipcm_trace(TRACE_DESC, "pfn 0x%x, desc %p [%d%d] &pfn %p",
			*pfn, desc, hnid, nid, &desc[hnid].recvbuf_pfn[nid - 1]);
	} else {
		*pfn = 0;
		*size = 0;
		ipcm_err("nid == hnid");
		return;
	}

	zone_base = (*pfn) << PAGE_SHIFT;
	ipcm_trace(TRACE_DESC, "zonebase 0x%lx", zone_base);
	addr = __ipcm_io_mapping__(zone_base, 0x1000);
	if (NULL == addr) {
		ipcm_err("zone addr ioremap failed!");
		return;
	}
	val = *(int *)(addr);
	if (ZONE_MAGIC != val) {
		ipcm_err("zone magic err[0x%x/0x%x]!", val, ZONE_MAGIC);
	}

	*size = *(int *)(addr + 4);

	__ipcm_io_unmapping__(addr);
}

/*
 * Initialization for zone that contains messages sent from
 * node nid to local.
 * Only if local is slave, would this function be called.
 * If local is master, the initialization needs to be done
 * by node nid, and the master will call get_zone_from_node
 * instead.
 */
static int init_zone_from_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (0 == LOCAL_ID) {
		/*
		 * master does not actually init any shared zone
		 * by itself, slaves do it instead.
		 */
		return 0;
	}

	/*
	 * get zone information from configuration files.
	 * and supposed that 0 phys is illegal here, roughly.
	 */
	cfg_2local_shm_phys(nid, phys);
	if (phys == 0) {
		ipcm_trace(TRACE_MEM, "cfg err: phys = 0");
		return -1;
	}

	cfg_2local_shm_size(nid, size);
	if (size == 0) {
		ipcm_trace(TRACE_MEM, "cfg err: size = 0");
		return -1;
	}

	ipcm_info("phys 0x%x, size 0x%x", phys, size);
	ret = ipcm_shared_zone_init(nid, phys, size, 0);
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}

	pfn = phys >> PAGE_SHIFT;
	set_desc_info(nid, LOCAL_ID, pfn, size);

	return 0;
}

/*
 * Initialization for zone that contains messages sent from
 * local to node nid.
 * Only if remote is master, would this function be called,
 * because slaves will always initialize its message-receive
 * zone by itself.
 */
static int init_zone_to_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned long phys;
	unsigned int size;

	if (0 == nid) {
		cfg_2remote_shm_phys(nid, phys);
		if (phys == 0) {
			ipcm_trace(TRACE_MEM, "cfg err: phys = 0");
			return -1;
		}

		cfg_2remote_shm_size(nid, size);
		if (size == 0) {
			ipcm_trace(TRACE_MEM, "cfg err: size = 0");
			return -1;
		}

		ret = ipcm_shared_zone_init(nid, phys, size, 1);
		if (ret) {
			ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
			return -1;
		}

		pfn = phys >> PAGE_SHIFT;
		//ipcm_info("pfn 0x%x, size 0x%x", pfn, size);
		set_desc_info(LOCAL_ID, nid, pfn, size);
	}

	return 0;
}

/*
 * Clear information of zone which contains messages sent from
 * node nid to local.
 */
static void release_zone_from_node(int nid)
{
	if (0 != LOCAL_ID) {
		/* master does not clear any shared zone */
		clear_desc_info(nid, LOCAL_ID);
	}
	ipcm_shared_zone_release(nid, 0);
}

/*
 * Clear information of zone which contains messages sent from
 * local to node nid.
 */
static void release_zone_to_node(int nid)
{
	if (0 == nid) {
		/* clear master shared zone */
		clear_desc_info(LOCAL_ID, nid);
	}
	ipcm_shared_zone_release(nid, 1);
}

/*
 * Get information of zone which contains messages sent from
 * node nid to local.
 * The information was prepare by node nid, and the initialization
 * of the zone was also done by node nid.
 * Only if local is master, should this function be called, otherwise
 * init_zone_from_node would be called instead.
 */
static int get_zone_from_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (nid == 0) {
		ipcm_err("nid should never be 0");
		return -1;
	}

	if (LOCAL_ID != 0) {
		ipcm_err("local id should be 0");
		return -1;
	}

	get_desc_info(nid, 0, &pfn, &size);

	phys = ((unsigned long)pfn) << PAGE_SHIFT;
	ret = ipcm_shared_zone_init(nid, phys, size, 0);
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}
	//ipcm_info("from %d phys 0x%x, size 0x%x", nid, phys, size);

	return 0;
}

/*
 * Get information of zone contains messages sent from local
 * to node nid.
 * The information is prepare by node nid.
 * Only if remote is slave, would this function be called.
 * If node nid is master, then init_zone_to_node should be
 * called instead.
 */
static int get_zone_to_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (nid == 0) {
		ipcm_err("nid should never be 0");
		return -1;
	}

	get_desc_info(LOCAL_ID, nid, &pfn, &size);
	phys = pfn << PAGE_SHIFT;
	ret = ipcm_shared_zone_init(nid, phys, size, 1);
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}
	//ipcm_info("to %d phys 0x%x, size 0x%x", nid, phys, size);

	return 0;
}

void do_cleanup(int nid)
{
	if (is_local_connected(nid)) {
		clear_node_connected(nid);
	}
	if (is_local_ready(nid)) {
		release_zone_from_node(nid);
		release_zone_to_node(nid);
		clear_local_ready(nid);
	}
}

void do_reset_node(int nid)
{
	set_node_reset(nid);
	do_cleanup(nid);
}

int do_connecting(int nid)
{
	int ret;

	if (!is_local_ready(nid) && !is_node_ready(nid)) {
		if (0 == LOCAL_ID)
			return 0;
		ret = init_zone_from_node(nid);
		if (ret) {
			ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
					nid, LOCAL_ID);
			return -1;
		}

		if (0 == nid) {
			ret =init_zone_to_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
				return -1;
			}
		}
		set_local_ready(nid);
		return 0;
	}

	if (!is_local_ready(nid) && is_node_ready(nid)) {
		if (0 != LOCAL_ID) {
			ret = init_zone_from_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
						nid, LOCAL_ID);
				return -1;
			}
		} else {
			ret = get_zone_from_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "get msg zone[from %d to %d] failed!",
						nid, LOCAL_ID);
				return -1;
			}
		}

		ret = get_zone_to_node(nid);
		if (ret) {
			ipcm_err("get msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
			return -1;
		}

		set_local_ready(nid);
		set_node_connected(nid);
		return 0;
	}

	if (is_local_ready(nid) && is_node_ready(nid)) {
		if (LOCAL_ID == 0) {
			/* normally this is not going to happen. */
			set_node_connected(nid);
			return 0;
		}

		if (nid == 0) {
			set_node_connected(nid);
			return 0;
		}

		ret = get_zone_to_node(nid);
		if (ret) {
			ipcm_err("get msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
			return -1;
		}
		set_node_connected(nid);
	}

	/*
	 * last case : local ready && node not ready
	 * in this case, we can do nothing, just return 0;
	 */
	return 0;
}

int check_node_reset(int nid)
{
	if (0 == LOCAL_ID) {
		if (is_node_reset(nid)) {
			if (is_node_recover(nid)) {
				clear_node_reset(nid);
				return 0;
			}
			return 1;
		} else {
			struct ipcm_node *node;
			/* only master can reset node */
			node = ipcm_get_node(nid);
			if (NULL == node) {
				ipcm_err("invalid node %d", nid);
				return -1;
			}
			if (NODE_RESET == node->state) {
				do_reset_node(nid);
				return 1;
			}
			return 0;
		}
	} else {
		/* slave recovered */
		if (is_node_reset(LOCAL_ID)) {
			if (!is_local_recover())
				set_local_recover();
		} else {
			if (is_local_recover())
				clear_local_recover();
		}
		return 0;
	}
}

int try_connect_node(int nid)
{
	if (check_node_reset(nid))
		return 0;

	if (nid == LOCAL_ID)
		return 0;

	if (!is_node_alive(nid)) {
		if (is_local_connected(nid))
			do_cleanup(nid);
		return 0;
	}
	if (is_local_connected(nid))
		return 0;

	if (do_connecting(nid)) {
		ipcm_trace(TRACE_INIT, "connected to node %d failed!", nid);
		return -1;
	}

	return 0;
}

static int node_release = 0;

int ipcm_nodes_detecting(void *p)
{
	unsigned int nid;

	for (nid = 0; nid < MAX_NODES; nid++) {
		clear_local_ready(nid);
		clear_node_connected(nid);
	}
	set_local_alive();

	//ipcm_info("ipcm detecting thread running!\n");
	do {
		for (nid = 0; nid < MAX_NODES; nid++) {
			try_connect_node(nid);
		}
		__ipcm_msleep__(10);

	} while (!__ipcm_thread_check_stop__(node_detect_task));

	if (node_release) {
		for (nid = 0; nid < MAX_NODES; nid++)
			do_cleanup(nid);
		clear_local_alive();
	}
	ipcm_info("Nodes discovery thread quit!");
	return 0;
}

void ipcm_node_release(int release)
{
	node_release = release;
}
