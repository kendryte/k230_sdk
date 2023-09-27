#include "device_config.h"
#include "ipcm_nodes.h"
#include "ipcm_osadapt.h"
#include "ipcm_buffer.h"
#include "ipcm_funcs.h"
#include "ipcm_desc.h"

extern struct ipcm_dev *get_dev(int id);
extern struct ipcm_dev *local_dev(void);
extern struct ipcm_node *ipcm_get_node(int id);

static int __memcpy_to_area(struct ipcm_shared_area *area,
		const void *buf, int len, struct ipcm_transfer_head *head)
{
	unsigned long phead, pbody, ptail;
	unsigned int rp, wp;
	unsigned int total_size = 0;
	int ret;
	rp = *area->rp;
	wp = *area->wp;

	ipcm_trace(TRACE_MEM, "rp/wp:0x%x/0x%x", rp, wp);
	phead = (unsigned long)area->data + wp;
	pbody = phead + __MSG_HEAD_SIZE;
	ptail = __MSG_ALIGNED(pbody + len);
	total_size = ptail - phead;

	if (wp >= rp) {
		if ((wp + total_size) < (area->len - __MSG_HEAD_SIZE)) {
			__memcpy__((void *)pbody, buf, len);
			__memset__((void *)(pbody + len), 0xa9, ptail -(pbody + len));
			__memcpy__((void *)phead, head, __MSG_HEAD_SIZE);
			__barrier__();
			*area->wp = wp + total_size;
			ret = len;
		} else if (rp > total_size) {
			struct ipcm_transfer_head pad;
			__memcpy__(&pad, head, __MSG_HEAD_SIZE);
			pad.type = __MSG_JUMP_MARK__;
			__memcpy__((void *)phead, &pad, __MSG_HEAD_SIZE);

			phead = (unsigned long)area->data;
			pbody = phead + __MSG_HEAD_SIZE;
			ptail = __MSG_ALIGNED(pbody + len);
			__memcpy__((void *)pbody, buf, len);
			__memset__((void *)(pbody + len), 0xa9, ptail -(pbody + len));
			__memcpy__((void *)phead, head, __MSG_HEAD_SIZE);
			__barrier__();
			*area->wp = total_size;
			ret = len;
		} else {
			ipcm_trace(TRACE_MEM, "shared buf full!");
			ret = -1;
		}

	} else if ((rp - wp) > total_size) {
		__memcpy__((void *)pbody, buf, len);
		__memset__((void *)(pbody + len), 0xa9, ptail -(pbody + len));
		__memcpy__((void *)phead, head, __MSG_HEAD_SIZE);
		__barrier__();
		*area->wp = wp + total_size;
		ret = len;
	} else {
		ipcm_trace(TRACE_MEM, "shared buf full!");
		ret = -1;
	}

	ipcm_trace(TRACE_MEM, "next wp 0x%x", *area->wp);
	return ret;
}

int ipcm_send_message(struct ipcm_transfer_handle *handle,
		const void *buf, unsigned int len, unsigned int flag)
{
	int ret = 0;
	struct ipcm_node *node;
	struct ipcm_shared_area *area;
	struct ipcm_transfer_head head;
	unsigned long irq_save;

	ipcm_trace(TRACE_MEM, "target [%d:%d],len %d\n",
			handle->target, handle->port, len);
	node = ipcm_get_node(handle->target);
	if (node == NULL) {
		ipcm_err("invalid node %d(null)\n", handle->target);
		return -1;
	}

	if (!is_node_connected(handle->target)) {
		ipcm_err("node desc %d not connected !", handle->target);
		return -1;
	}

	/* if node is not ready yet, just ignore the send */
	if (NODE_READY != node->state)
		return 0;

	if (len > MAX_TRANSFER_LEN) {
		ipcm_err("over size message<%d bytes>!", MAX_TRANSFER_LEN);
		return -1;
	}

	ipcm_trace(TRACE_MEM, "nid %d &sendbuf %p",
			handle->target, node->sendbuf);

	if (!__interrupt_context__()
			&& (flag == MESSAGE_NORMAL))
		area = &node->sendbuf->normal_area;
	else
		area = &node->sendbuf->prio_area;

	__memset__(&head, 0, __MSG_HEAD_SIZE);
	head.target = handle->target;
	head.port = handle->port;
	head.source = ipcm_vdd_localid();
	head.type = flag;
	head.len = len;

	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&area->lock);
	ret = __memcpy_to_area(area, buf, len, &head);
	__ipcm_unlock__(&area->lock);
	__ipcm_irq_restore__(irq_save);
	if (ret != len) {
		ipcm_info("message send failed!");
		return -1;
	}
	if ((flag == MESSAGE_URGENT)
			|| (flag == MESSAGE_NORMAL)) {
		__ipcm_atomic_inc__(&handle->send_count);
		if ((int)len > __ipcm_atomic_read__(&handle->max_send_len))
			__ipcm_atomic_set__(&handle->max_send_len, (int)len);
	}
	return ret;
}

static void __fetch_one_msg(struct ipcm_shared_area *area,
		void **buf, int *len)
{
	unsigned int rp, wp;
	struct ipcm_transfer_head *head;

	rp = *area->rp;
	wp = *area->wp;
	ipcm_trace(TRACE_MEM, "in: &rp %p=0x%x, &wp %p=0x%x",
			area->rp, rp, area->wp, wp);
	if (rp == wp) {
		*buf = NULL;
		*len = 0;
		return;
	}

	head = (struct ipcm_transfer_head *)((unsigned long)area->data + rp);
	ipcm_trace(TRACE_MEM, "head->type 0x%x", head->type);
	if (head->type == __MSG_JUMP_MARK__) {
		if (wp) {
			ipcm_trace(TRACE_MEM, "turning back to the start");
			head = (struct ipcm_transfer_head *)area->data;
			*area->rp = 0;
		} else {
			ipcm_err("shared mem err!");
			IPCM_BUG();
		}
	}

	*len = head->len;
	*buf = (void *)head;
	ipcm_trace(TRACE_MEM, "message[head %p,len 0x%x]", head, head->len);
}

extern void msg_notify(struct ipcm_transfer_handle *handle);

static void ack_connect_msg(struct ipcm_node *node,
		struct ipcm_transfer_head *head)
{
	char buffer[8];
	int ret;
	struct ipcm_transfer_handle *handle;
	unsigned long irq_save;

	handle = node->handlers[head->port];
	/*
	 * FIXME: the 'handlers_state' may be modified by ipcm_vdd_connect
	 *        and ack_connect_msg. but it dosen't be locked.
	 */
	irq_save = __ipcm_irq_save__();
	switch (head->type) {
	case MESSAGE_CONNECT_REQUEST:
		if ((CONNECT_REQUESTING == node->handlers_state[head->port])
				&& handle) {
			__ipcm_lock__(&handle->lock);
#ifndef NO_MULTITASKS
			if (!handle->recv) {
				__ipcm_unlock__(&handle->lock);
				break;
			}
#endif
			__memset__(buffer, 0xa5, 8);
			ret = ipcm_send_message(handle,
					buffer, 8, MESSAGE_CONNECT_ACK);
			if (8 != ret)
				IPCM_BUG();
			msg_notify(handle);
			node->handlers_state[head->port] = CONNECT_ACKING;
			__ipcm_unlock__(&handle->lock);
		} else {
			node->handlers_state[head->port] = CONNECT_REQUESTED;
		}
		break;

	case MESSAGE_CONNECT_ACK:
		if ((CONNECT_ACKING == node->handlers_state[head->port])
				&& handle) {
			__ipcm_lock__(&handle->lock);
#ifndef NO_MULTITASKS
			if (!handle->recv) {
				__ipcm_unlock__(&handle->lock);
				break;
			}
#endif
			node->handlers_state[head->port] = CONNECT_ACKED;
			handle->state = __HANDLE_CONNECTED;
			__ipcm_unlock__(&handle->lock);
		}
		break;

	case MESSAGE_DISCONNECT_REQUEST:
		if (handle) {
			__ipcm_lock__(&handle->lock);
			handle->state = __HANDLE_DISCONNECTED;
			__ipcm_unlock__(&handle->lock);
		}
		node->handlers_state[head->port] = STATE_INIT;
		break;

	default:
		break;
	}
	__ipcm_irq_restore__(irq_save);
}

void __recv_messages(struct ipcm_node *node, struct ipcm_shared_area *area)
{
	void *buf;
	int len;
	struct ipcm_transfer_head *head;
	struct ipcm_transfer_handle *handle;

	if (NULL == node->handlers)
		return;

	while (1) {
		/*
		 * FIXME: the area may be released by ipcm_nodes_detecting,
		 * while it is being access here. so need lock it.
		 */
		__fetch_one_msg(area, &buf, &len);
		if (buf == NULL)
			break;

		head = buf;

		if ((MESSAGE_CONNECT_REQUEST == head->type)
				|| (MESSAGE_CONNECT_ACK == head->type)
				|| (MESSAGE_DISCONNECT_REQUEST == head->type)) {
			ack_connect_msg(node, head);
			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			continue;
		}

		handle = node->handlers[head->port];
		if (handle) {
			//__ipcm_lock__(&handle->lock);
			if (handle->recv
					&& (__HANDLE_CONNECTED == handle->state)) {

				handle->recv(handle, (void *)(head + 1), head->len);
				__ipcm_atomic_inc__(&handle->recv_count);
				if ((int)head->len > __ipcm_atomic_read__(&handle->max_recv_len))
					__ipcm_atomic_set__(&handle->max_recv_len, (int)head->len);
			}
			//__ipcm_unlock__(&handle->lock);
		}

		*area->rp = __MSG_ALIGNED(*area->rp + __MSG_HEAD_SIZE + head->len);
		ipcm_trace(TRACE_MEM, "updated rp 0x%x", *area->rp);
	}
}

int ipcm_process_messages(struct ipcm_node *node)
{
	struct ipcm_shared_area *area;
	unsigned long irq_save;

	area = &node->recvbuf->prio_area;
	irq_save = __ipcm_irq_save__();
	__ipcm_lock__(&area->lock);
	__recv_messages(node, area);
	__ipcm_unlock__(&area->lock);
	__ipcm_irq_restore__(irq_save);

	area = &node->recvbuf->normal_area;
	__recv_messages(node, area);

	return 0;
}

int ipcm_irq_messages(struct ipcm_node *node)
{
	struct ipcm_shared_area *area;

	area = &node->recvbuf->prio_area;
	__ipcm_lock__(&area->lock);
	__recv_messages(node, area);
	__ipcm_unlock__(&area->lock);

	return 0;
}


static int __get_msg_from_area(struct ipcm_shared_area *area,
				int *port, char *buf, int len)
{
	struct ipcm_transfer_head *head;
	struct ipcm_node *node;
	struct ipcm_transfer_handle *handle;
	void *message = NULL;
	int size;

	do {
		__fetch_one_msg(area, &message, &size);
		if (!message)
			return 0;
		head = (struct ipcm_transfer_head *)message;
		node = ipcm_get_node(head->source);

		if (!node || (NULL == node->handlers)) {
			break;
		}
		if ((MESSAGE_CONNECT_REQUEST == head->type)
				|| (MESSAGE_CONNECT_ACK == head->type)
				|| (MESSAGE_DISCONNECT_REQUEST == head->type)) {
			ack_connect_msg(node, head);
			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			continue;
		}

		handle = node->handlers[head->port];
		if (!handle) {
			ipcm_err("invalid message from %d:%d",
					head->source, head->port);
			/* just ignore this message */
			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			continue;
		}

		if ((MESSAGE_NORMAL == head->type)
				|| (MESSAGE_URGENT == head->type)) {
			if (__HANDLE_CONNECTED != handle->state) {
				ipcm_err("invalid message from %d:%d",
						head->source, head->port);
				/* just ignore this message */
				*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
				continue;
			}
			if (len < size) {
				ipcm_err("buffer too small[%d/%d]!!",
						len, size);
				/* message still not received!!! */
				return -1;
			}
			__memcpy__(buf,
					(void *)((unsigned long)message + __MSG_HEAD_SIZE),
					size);
			*port = head->port;

			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			//ipcm_err("## >> rp 0x%x", *area->rp);
			return size;
		}

		IPCM_BUG();
	} while (1);

	return 0;
}

#ifdef NO_MULTITASKS
/*
 * wait until the connect request message from node comes.
 * return: 1, connected; 0 on a disconnect message received.
 */

int wait_for_connected(struct ipcm_transfer_handle *handle)
{
	struct ipcm_transfer_head *head;
	struct ipcm_node *node = ipcm_get_node(handle->target);
	struct ipcm_shared_area *area = &node->recvbuf->prio_area;
	void *message = NULL;
	int size;
	int delay_loop;

	do {
		__fetch_one_msg(area, &message, &size);
		if (!message) {
			//ack_connect_msg(node, head);
			if (handle->state == __HANDLE_CONNECTED)
				return 1;
			delay_loop = 0x90000;
			while(delay_loop--);
			return 1;
			//continue;
		}
		ipcm_err("msg comes ......");
		head = (struct ipcm_transfer_head *)message;
		if (head->source != node->id) {
			ipcm_err("unexpected message from [%d:%d],len %d",
				head->source, head->port, head->len);
			/* just ignore this message */
			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			continue;
		}
		if ((MESSAGE_CONNECT_REQUEST == head->type)
				|| (MESSAGE_CONNECT_ACK == head->type)
				|| (MESSAGE_DISCONNECT_REQUEST == head->type)) {
			ack_connect_msg(node, head);
			*area->rp = __MSG_ALIGNED(*area->rp
					+ __MSG_HEAD_SIZE + head->len);
			return (MESSAGE_CONNECT_ACK == head->type);
		}
		ipcm_err("unexpected message from [%d:%d],len %d, ignored",
				head->source, head->port, head->len);
		*area->rp = __MSG_ALIGNED(*area->rp
				+ __MSG_HEAD_SIZE + head->len);
	} while (1);

	return 0;
}
#endif

int get_one_message(struct ipcm_node *node, int *port, char *buf, int len)
{
	int ret;
	struct ipcm_shared_area *area = &node->recvbuf->prio_area;
	ret = __get_msg_from_area(area, port, buf, len);
	if (ret > 0)
		return ret;
	if (ret < 0) {
		ipcm_err("recv message error!");
		return -1;
	}

	area = &node->recvbuf->normal_area;
	ret = __get_msg_from_area(area, port, buf, len);
	if (ret < 0) {
		ipcm_err("recv message error!");
		return -1;
	}
	return ret;
}

void mem_region_align(struct mem_region *region, int align)
{
	unsigned long base = (region->base + align - 1) & (~(align - 1));
	unsigned long end = (region->base + region->size) & (~(align - 1));

	if (end <= base) {
		ipcm_err("region[base:0x%lx,sz:0x%x] invalid!",
				region->base, region->size);
		return;
	}
	ipcm_trace(TRACE_INIT, "region align [0x%lx, 0x%x] >> [0x%lx, 0x%x]",
			region->base, region->size, base, end - base);

	region->base = base;
	region->size = end - base;
}

int shared_mem_area_init(struct ipcm_shared_zone *zone, int nid, int send)
{
	struct ipcm_shared_area *area;
	struct mem_region *region;

	/*
	 * Mark zone magic and size.
	 * In slave-to-slave connection, each node will initialize its
	 * receive zone only, and In slave-to-master connection, slave
	 * will initialize both its send zone and receive zone, Master
	 * will do nothing.
	 */
	if ((0 == nid) || ((0 != LOCAL_ID) && !send)) {
		__memset__((void *)(zone->region.base), 0, zone->region.size);
		*(int *)zone->region.base = ZONE_MAGIC;
		*(int *)(zone->region.base + 4) = zone->region.size;
	}

	/*
	 * the base address and size of each zone
	 * need to be at least 4Kbytes aligned.
	 * but we're not going to check it here.
	 */
	area = &zone->prio_area;
	region = &area->region;
	region->size = zone->region.size / 2 - 0x10;
	region->base = zone->region.base + 0x10;
	mem_region_align(region, 0x10);
	area->rp = (unsigned int *)region->base;
	area->wp = (unsigned int *)(region->base + 0x4);
	area->data = (char *)(region->base + 0x10);
	area->len = region->size - 0x10;
	ipcm_trace(TRACE_INIT, "prio_area: base %p, data %p, len 0x%p",
			area->rp, area->data, area->len);

	if ((0 == nid) || ((0 != LOCAL_ID) && !send)) {
		*area->rp = 0;
		*area->wp = 0;
		*(unsigned int *)(region->base + 0xc) = region->size;
	}
	if(__ipcm_lock_init__(&area->lock)) {
		return -1;
	}

	area = &zone->normal_area;
	region = &area->region;
	region->size = zone->region.size - zone->prio_area.region.size - 0x10;
	region->base = zone->region.base + zone->prio_area.region.size + 0x10;
	mem_region_align(region, 0x10);
	area->rp = (unsigned int *)region->base;
	area->wp = (unsigned int *)(region->base + 0x4);
	area->data = (char *)(region->base + 0x10);
	area->len = region->size - 0x10;
	ipcm_trace(TRACE_INIT, "normal_area: base %p, data %p, len 0x%x",
			area->rp, area->data, area->len);

	if ((0 == nid) || ((0 != LOCAL_ID) && !send)) {
		*area->rp = 0;
		*area->wp = 0;
		*(unsigned int *)(region->base + 0xc) = region->size;
	}
	if (__ipcm_lock_init__(&area->lock)) {
		area = &zone->prio_area;
		__ipcm_lock_free__(&area->lock);
		return -1;
	}
	return 0;
}

void shared_mem_area_release(struct ipcm_shared_zone *zone)
{
	struct ipcm_shared_area *area;

	area = &zone->prio_area;
	__ipcm_lock_free__(&area->lock);
	area = &zone->normal_area;
	__ipcm_lock_free__(&area->lock);
}

int ipcm_shared_zone_init(int nid, unsigned long base,
			unsigned long size, int sendbuf)
{
	struct ipcm_node *node;
	struct ipcm_shared_zone *zone;
	struct mem_region *region;

	ipcm_trace(TRACE_INIT, "local %d nid %d, buf[0x%lx, 0x%lx, %s]",
			LOCAL_ID, nid, base,
			size, sendbuf ? "send":"recv");
	if (nid == LOCAL_ID)
		return 0;

	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return -1;
	}

	zone = __ipcm_mem_alloc__(sizeof(struct ipcm_shared_zone));
	if (NULL == zone) {
		ipcm_err("mem alloc for node %d %s zone struct failed!",
				nid, sendbuf?"send":"recv");
		return -1;
	}

	if (sendbuf)
		node->sendbuf = zone;
	else
		node->recvbuf = zone;

	region = &zone->region;
	/* we need virtual address here hence */
	region->base = (unsigned long)__ipcm_io_mapping__(base, size);
	if (0 == region->base) {
		__ipcm_mem_free__(zone);
		ipcm_err("io mapping failed!");
		return -1;
	}

	ipcm_trace(TRACE_INIT, "zone->base 0x%lx", region->base);
	region->size = size;
	mem_region_align(region, PAGE_SIZE);

	return shared_mem_area_init(zone, nid, sendbuf);
}

void ipcm_shared_zone_release(int nid, int sendbuf)
{
	struct ipcm_node *node;
	struct ipcm_shared_zone *zone;
	struct mem_region *region;

	if (LOCAL_ID == nid)
		return;
	node = ipcm_get_node(nid);
	if (NULL == node)
		return;

	if (sendbuf)
		zone = node->sendbuf;
	else
		zone = node->recvbuf;

	if ((0 == nid) || ((0 != LOCAL_ID) && !sendbuf))
		shared_mem_area_release(zone);

	region = &zone->region;
	__ipcm_io_unmapping__((void *)region->base);

	region->base = 0;
	region->size = 0;

	__ipcm_mem_free__(zone);
}

