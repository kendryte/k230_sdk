#include "ipcm_osadapt.h"
#include "ipcm_nodes.h"
#include "ipcm_funcs.h"
#include "device_config.h"
#include "ipcm_buffer.h"
#include "ipcm_platform.h"
#include "ipcm_desc.h"


struct ipcm_timer recv_timer;
struct ipcm_event recv_event;

struct ipcm_node_desc *g_nodes_desc;
struct ipcm_node g_ipcm_nodes[MAX_NODES] = {0};
struct ipcm_task *node_detect_task = NULL;
struct ipcm_task *message_recv_task = NULL;

struct ipcm_node *local_node(void)
{
	return &g_ipcm_nodes[LOCAL_ID];
}

unsigned int get_nodes_number(void)
{
	return MAX_NODES;
}

struct ipcm_node *ipcm_get_node(int id)
{
	if ((id < 0) || (id > (MAX_NODES - 1)))
		return NULL;

	return &g_ipcm_nodes[id];
}

int ipcm_node_ready(int target)
{
	if ((target < 0) || (target >= MAX_NODES)) {
		ipcm_err("invalid target[%d]!", target);
		return 0;
	}

	if (NODE_READY == g_ipcm_nodes[target].state)
		return 1;
	return 0;
}

int wait_handlers_release(struct ipcm_node *node, int timeout)
{
	int port = 0;
	int wait_forever = 0;
	int ret = 0;

	if (NODE_READY != node->state)
		return 0;

	if (!timeout)
		wait_forever = 1;
	/* convert second to 10ms unit */
	timeout = timeout * 100;
	while (1) {
		port=0;
		while (port < MAX_PORTS) {
			if (node->handlers[port])
				break;
			port++;
		}
		if ((MAX_PORTS == port)
				|| (!wait_forever && !(--timeout)))
			break;
		__ipcm_msleep__(10);
	}

	for (; port < MAX_PORTS; port++) {
		if (node->handlers[port]) {
			ipcm_err("port[%d:%d] is not closed!", node->id, port);
		}
		ret = -1;
	}
	return ret;
}

int ipcm_vdd_reset_node(int target, int timeout)
{
	struct ipcm_node *node;

	if ((target < 0) || (target >= MAX_NODES)) {
		ipcm_err("invalid target[%d]!", target);
		return -1;
	}
	if (0 != LOCAL_ID) {
		ipcm_err("only master<id=0> can reset node!");
		return -1;
	}
	node = ipcm_get_node(target);
	if (NULL == node)
		return -1;

	if (wait_handlers_release(node, timeout))
		return -1;

	node->state = NODE_RESET;
	return 0;
}

static struct ipcm_transfer_handle *get_handle(int target, int port)
{

	if ((target < 0) || (target >= MAX_NODES)) {
		ipcm_err("invalid target[%d:%d]!", target, port);
		return NULL;
	}

	if ((port < 0) || (port >= MAX_PORTS)) {
		ipcm_err("invalid port[%d:%d]!", target, port);
		return NULL;
	}

	if (g_ipcm_nodes[target].handlers)
		return g_ipcm_nodes[target].handlers[port];

	return NULL;
}

int ipcm_vdd_localid(void)
{
	return LOCAL_ID;
}

int ipcm_vdd_check_handle(void *data)
{
	struct ipcm_transfer_handle *handle = data;
	if (NULL == handle) {
		ipcm_err("handle is null");
		return -1;
	}

	return handle->state;
}

int ipcm_vdd_remoteids(int *ids)
{
	int i;
	int ready = 0;

	for (i = 0; i < MAX_NODES; i++) {
		if (ipcm_node_ready(i)) {
			if (ids)
				ids[i] = 1;
			ready++;
		} else {
			if (ids)
				ids[i] = 0;
		}
	}
	return ready;
}

int ipcm_vdd_node_ready(int target)
{
	return ipcm_node_ready(target);
}

void *ipcm_vdd_open(int target, int port, int priority)
{
	struct ipcm_transfer_handle *handle;

	ipcm_trace(TRACE_DEV, "handle[%d:%d]", target, port);
	if ((target < 0) || (target >= MAX_NODES)) {
		ipcm_err("invalid target[%d:%d]!", target, port);
		return NULL;
	}
	if ((port < 0) || (port >= MAX_PORTS)) {
		ipcm_err("invalid port[%d:%d]!", target, port);
		return NULL;
	}

	if (LOCAL_ID == target) {
		ipcm_err("invalid target %d!", target);
		return NULL;
	}

	handle = get_handle(target, port);
	if (handle) {
		ipcm_err("handle[%d:%d]%p already opened!",
				target, port, handle);
		return NULL;
	}

	if (!ipcm_node_ready(target)) {
		ipcm_err("node[%d] not ready !", target);
		return NULL;
	}

	handle = __ipcm_mem_alloc__(sizeof(struct ipcm_transfer_handle));
	__memset__(handle, 0, sizeof(struct ipcm_transfer_handle));
	if (!handle) {
		ipcm_err("handle alloc failed!");
		return NULL;
	}

	if (__ipcm_lock_init__(&handle->lock)) {
		ipcm_err("init handle lock failed!");
		__ipcm_mem_free__(handle);
		return NULL;
	}

	handle->target = target;
	handle->port = port;
	handle->data = 0;
	handle->recv = NULL;
	handle->state = __HANDLE_DISCONNECTED;
	handle->priority = priority;

	__ipcm_atomic_set__(&handle->send_count, 0);
	__ipcm_atomic_set__(&handle->recv_count, 0);
	__ipcm_atomic_set__(&handle->max_send_len, 0);
	__ipcm_atomic_set__(&handle->max_recv_len, 0);
	__ipcm_event_init__(&handle->connect_event);
	g_ipcm_nodes[target].handlers[port] = handle;

	return handle;
}

void ipcm_vdd_close(void *data)
{
	int target, port;
	struct ipcm_transfer_handle *handle = data;
	if (handle) {
		target = handle->target;
		port = handle->port;
		if ((target < 0) || (target >= MAX_NODES)) {
			ipcm_err("invalid target[%d:%d]!", target, port);
			return;
		}
		if ((port < 0) || (port >= MAX_PORTS)) {
			ipcm_err("invalid port[%d:%d]!", target, port);
			return;
		}
		ipcm_trace(TRACE_DEV, "target [%d:%d]", target, port);
		__ipcm_event_free__(&handle->connect_event);
		__ipcm_lock_free__(&handle->lock);
		__ipcm_mem_free__(handle);
		if (g_ipcm_nodes[target].handlers) {
			g_ipcm_nodes[target].handlers[port] = NULL;
		}
	} else {
		ipcm_err("try to close an invalid handle[NULL]!");
	}
}

int ipcm_vdd_getopt(void *data, struct ipcm_vdd_opt *ops)
{
	struct ipcm_transfer_handle *handle = data;

	/*
	 * don't want to do too much legality checking.
	 */
	if (!handle) {
		ipcm_err("invalid handle!");
		return -1;
	}

	ops->data = handle->data;
	ops->recv = handle->recv;

	return 0;
}

int ipcm_vdd_setopt(void *data, struct ipcm_vdd_opt *ops)
{
	struct ipcm_transfer_handle *handle = data;
	unsigned long irq_save;

	if (!handle) {
		ipcm_err("invalid handle!");
		return -1;
	}

	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&handle->lock);
	if (ops) {
		handle->data = ops->data;
		handle->recv = ops->recv;
	} else {
		handle->data = 0;
		handle->recv = NULL;
	}
	__ipcm_unlock__(&handle->lock);
	__ipcm_irq_restore__(irq_save);

	return 0;
}

void msg_notify(struct ipcm_transfer_handle *handle)
{
	struct ipcm_node *node = ipcm_get_node(handle->target);
	if (node)
		__interrupt_trigger__(node->id, node->irq);
}

int ipcm_vdd_sendmsg(void *data,
		const void *buf,
		unsigned int len)
{
	int ret;
	struct ipcm_transfer_handle *handle = data;
	unsigned long irq_save;
	int flag;

	if (NULL == handle) {
		ipcm_err("handle is null");
		return -1;
	}
	if(!ipcm_node_ready(handle->target)) {
		ipcm_err("node %d not ready", handle->target);
		return -1;
	}

	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&handle->lock);
	if (__HANDLE_CONNECTED != handle->state) {
		ipcm_err("handle is not connected");
		__ipcm_unlock__(&handle->lock);
		__ipcm_irq_restore__(irq_save);
		return -2;
	}

	if (handle->priority == __HANDLE_MSG_PRIORITY)
		flag = MESSAGE_URGENT;
	else
		flag = MESSAGE_NORMAL;
	ret = ipcm_send_message(handle, buf, len, flag);

	__ipcm_unlock__(&handle->lock);
	__ipcm_irq_restore__(irq_save);

	if ((ret == len) && (MESSAGE_URGENT == flag))
		msg_notify(handle);

	return ret;
}

/*
 * ipcm_vdd_connect() - connect handle before send messages
 * @data:     Pointer to ipcm_transfer_handle from ipcm_vdd_open.
 * @is_block: CONNECT_NONBLOCK try to connect the handle without check
 *            CONNECT_BLOCK connect the handle and be sure it connected
 * return value: return 0 if connect successfully,
 *		 return 1 if it's needed to check later,
 *		 return -1 if the connect failed.
 */
int ipcm_vdd_connect(void *data, int is_block)
{
	int ret = 0;
	int target, port;
	char buffer[8];
	struct ipcm_transfer_handle *handle = data;
	struct ipcm_node *node;
	unsigned long irq_save;

	ipcm_trace(TRACE_INIT, "connect...");
	if (NULL == handle) {
		ipcm_err("handle is null");
		return -1;
	}
	__memset__(buffer, 0xa5, 8);

	target = handle->target;
	port = handle->port;
	if(!ipcm_node_ready(target)) {
		ipcm_err("node %d not ready", target);
		return -1;
	}

	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&handle->lock);

	if (__HANDLE_CONNECTED == handle->state) {
		ipcm_err(TRACE_MSG, "handle is connected");
		__ipcm_unlock__(&handle->lock);
		__ipcm_irq_restore__(irq_save);
		return 0;
	}
	node = ipcm_get_node(target);
	if (NULL == node) {
		ipcm_err("invalid node %d", target);
		__ipcm_unlock__(&handle->lock);
		__ipcm_irq_restore__(irq_save);
		return -1;
	}
	/*
	 * make sure the MESSAGE_CONNECT_REQUEST msg is sent successfully.
	 */
	ret = ipcm_send_message(handle, buffer, 8, MESSAGE_CONNECT_REQUEST);
	if (8 != ret)
		IPCM_BUG();

	handle->state = __HANDLE_CONNECTING;

#ifndef NO_MULTITASKS
	if ((CONNECT_REQUESTED == node->handlers_state[port])
			&& handle->recv) {
#else
	if (CONNECT_REQUESTED == node->handlers_state[port]) {
#endif
		ret = ipcm_send_message(handle, buffer, 8, MESSAGE_CONNECT_ACK);
		if (8 != ret)
			IPCM_BUG();
		node->handlers_state[port] = CONNECT_ACKING;
	} else {
		node->handlers_state[port] = CONNECT_REQUESTING;
	}
	msg_notify(handle);
	__ipcm_unlock__(&handle->lock);
	__ipcm_irq_restore__(irq_save);

	if (CONNECT_NONBLOCK == is_block)
		return 1;

#ifndef NO_MULTITASKS
	if (__HANDLE_DISCONNECTED == handle->state) {
		ipcm_err("handle is disconnected\n");
		return -1;
	}

	if(__HANDLE_CONNECTED == handle->state) {
		ipcm_info("handle is CONNECTED \n");
		return 0;
	}

	/* wait until the handle is connected */
	__ipcm_wait_event__(&handle->connect_event);

#else
	return 1;
#endif
	return 0;
}

void ipcm_vdd_disconnect(void *data)
{
	int ret = 0;
	int target, port;
	char buffer[8];
	struct ipcm_transfer_handle *handle = data;
	struct ipcm_node *node;
	unsigned long irq_save;

	ipcm_trace(TRACE_INIT, "disconnect...");
	if (NULL == handle) {
		ipcm_err("handle is null");
		return;
	}
	target = handle->target;
	port = handle->port;
	node = ipcm_get_node(target);
	if (NULL == node) {
		ipcm_err("invalid node %d", target);
		return;
	}
	if(!ipcm_node_ready(target)) {
		return;
	}

	__memset__(buffer, 0xa5, 8);
	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&handle->lock);
	if (__HANDLE_DISCONNECTED == handle->state) {
		__ipcm_unlock__(&handle->lock);
		__ipcm_irq_restore__(irq_save);
		return;
	}

	/*
	 * make sure the MESSAGE_DISCONNECT_REQUEST msg is sent successfully.
	 */
	ret = ipcm_send_message(handle, buffer, 8, MESSAGE_DISCONNECT_REQUEST);
	if (8 != ret)
		IPCM_BUG();
	msg_notify(handle);

	node->handlers_state[port] = STATE_INIT;
	handle->state = __HANDLE_DISCONNECTED;
	__ipcm_unlock__(&handle->lock);
	__ipcm_irq_restore__(irq_save);
}

/*
 * get a message from a connected node. the message will be stored in
 * pbuf on success. the caller has to assure the size of pbuf is large
 * enough to stored any comming messages from node target.
 * the receive will fail if the message is too large to be stored in
 * pbuf.
 * input:
 * 	source: the node id that message is from;
 * 	port: the connect port that message is from;
 *	pbuf: a prepared buffer to store the comming message if any.
 *	len: size of the given buffer(pbuf).
 * return:
 * 	> 0: the actual size of the comming message.
 * 	= 0: no message from this target.
 * 	< 0: on error.
 */
int ipcm_vdd_recvmsg(int *source, int *port, char *pbuf, int len)
{
	int ret;
	int i;
	struct ipcm_node *node = NULL;

	for (i = 0; i < MAX_NODES; i++) {
		if (LOCAL_ID == i)
			continue;
		node = ipcm_get_node(i);
		if (node && (NODE_READY == node->state)) {
			ret = get_one_message(node, port, pbuf, len);
			if (0 > ret) {
				ipcm_err("receive message from %d failed[%d]", i, ret);
				return -1;
			}
			*source = i;
			return ret;
		}
	}
	return 0;
}

extern int ipcm_process_messages(struct ipcm_node *node);
extern int ipcm_irq_messages(struct ipcm_node *node);
static int message_recv_thread(void *data)
{
	int i;
	struct ipcm_node *node;

	//ipcm_info("##message recv thread run");
	do {
		for (i = 0; i < MAX_NODES; i++) {
			if (LOCAL_ID == i)
				continue;
			node = ipcm_get_node(i);
			if (node && (NODE_READY == node->state)) {
				ipcm_trace(TRACE_MSG, "process msg[id:%d]", i);
				ipcm_process_messages(node);
			}
		}
		__ipcm_wait_event__(&recv_event);
	} while (!__ipcm_thread_check_stop__(message_recv_task));

	ipcm_info("##message recv thread exit");
	return 0;
}

int irq_callback(int irq, void *data)
{
#if 0
	int i;
	struct ipcm_node *node;
	ipcm_trace(TRACE_MSG, "irq %d tri", irq);

	for (i = 0; i < MAX_NODES; i++) {
		if (LOCAL_ID == i)
			continue;
		node = ipcm_get_node(i);
		if (node && (NODE_READY == node->state)) {
			ipcm_irq_messages(node);
		}
	}
#else
	__ipcm_wakeup_event__(&recv_event);
#endif
	return 0;

}

void timer_callback(unsigned long data)
{
	__ipcm_wakeup_event__(&recv_event);
	__ipcm_timer_restart__(&recv_timer, 10);
}

int ipcm_vdd_init(void)
{
	int ret = 0;
	int i;

	/*
	 * architeture-related initialization
	 * May be this function should be called within
	 * discovery thread.
	 */
	__arch_init__();
#ifndef __NODES_DESC_MEM_OFFSET__
#define __NODES_DESC_MEM_OFFSET__ 0
#endif
	g_nodes_desc = (void *)((unsigned long)(__ipcm_io_mapping__(__NODES_DESC_MEM_BASE__, 0x1000)) + __NODES_DESC_MEM_OFFSET__);
	if (NULL == g_nodes_desc) {
		ipcm_err("io mapping for g_nodes_desc failed!");
		return -1;
	}

#ifdef USE_RT_SMART
	unsigned int *p = (int *)g_nodes_desc;
	for(i = 0; i < (0x1000 / 4); i++)
	{
		p[i] = 0;
	}
#endif

	for (i = 0; i < MAX_NODES; i++) {
		g_ipcm_nodes[i].id = i;

		if (LOCAL_ID == i) {
			g_ipcm_nodes[i].irq = IRQ_NUM;
			g_ipcm_nodes[i].state = NODE_ALIVE;
		} else
			g_ipcm_nodes[i].state = NODE_HALT;
		g_ipcm_nodes[i].desc = &g_nodes_desc[i];
	}

	ret = __ipcm_event_init__(&recv_event);
	if (ret) {
		ipcm_err("recv event init failed!");
		goto free_desc;
	}

#ifdef NO_MULTITASKS
	ipcm_info("create recv task: no multi-tasks env!");
#else
	message_recv_task = __ipcm_thread_create__("ipcm-recv",
					message_recv_thread, NULL);
	if (NULL == message_recv_task) {
		ipcm_err("create recv-task failed!");
		goto free_revent;
	}
#endif

	ret = __ipcm_timer_create__(&recv_timer, timer_callback, 10, NULL);
	if (ret) {
		ipcm_err("recv timer create failed!");
		goto free_rtask;
	}

	node_detect_task = __ipcm_thread_create__("ipcm-discovery",
					ipcm_nodes_detecting, NULL);
	if (NULL == node_detect_task) {
		ipcm_err("create discovery task failed!");
		goto free_timer;
	}
	return 0;

free_timer:
	__ipcm_timer_free__(&recv_timer);
free_rtask:
#ifdef NO_MULTITASKS
	ipcm_info("free_rtask:no multi-tasks env!");
#else
	__ipcm_thread_destroy__(message_recv_task);
#endif

free_revent:
	__ipcm_event_free__(&recv_event);

free_desc:
	for (i = 0; i < MAX_NODES; i++) {
		g_ipcm_nodes[i].id = 0xffff;
		g_ipcm_nodes[i].irq = 0;
		g_ipcm_nodes[i].state = NODE_HALT;
		g_ipcm_nodes[i].desc = NULL;
	}
	__ipcm_io_unmapping__(g_nodes_desc);
	ipcm_err("module init failed");

	return ret;
}

void ipcm_vdd_cleanup(void)
{
	int i;

	__ipcm_thread_destroy__(message_recv_task);
	ipcm_node_release(1);
	__ipcm_thread_destroy__(node_detect_task);
	__ipcm_timer_free__(&recv_timer);
	__ipcm_event_free__(&recv_event);

	for (i = 0; i < MAX_NODES; i++) {
		/* wait for all handlers release */
		wait_handlers_release(&g_ipcm_nodes[i], 0);

		g_ipcm_nodes[i].id = 0xffff;
		if (LOCAL_ID == i) {
			g_ipcm_nodes[i].irq = 0;
		}
		g_ipcm_nodes[i].state = NODE_HALT;
		g_ipcm_nodes[i].desc = NULL;
	}
	__ipcm_io_unmapping__(g_nodes_desc);
	__arch_free__();
}

#ifdef __KERNEL__
#include <linux/module.h>
EXPORT_SYMBOL(ipcm_vdd_open);
EXPORT_SYMBOL(ipcm_vdd_close);
EXPORT_SYMBOL(ipcm_vdd_sendmsg);
EXPORT_SYMBOL(ipcm_vdd_getopt);
EXPORT_SYMBOL(ipcm_vdd_setopt);
EXPORT_SYMBOL(ipcm_vdd_localid);
EXPORT_SYMBOL(ipcm_vdd_remoteids);
EXPORT_SYMBOL(ipcm_vdd_disconnect);
EXPORT_SYMBOL(ipcm_vdd_connect);
EXPORT_SYMBOL(ipcm_vdd_check_handle);
EXPORT_SYMBOL(ipcm_vdd_reset_node);
EXPORT_SYMBOL(ipcm_vdd_node_ready);
#endif

