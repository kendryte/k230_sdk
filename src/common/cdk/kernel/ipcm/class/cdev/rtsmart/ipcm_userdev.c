/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG    "UART"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>

//#include "ipcm_ioctl.h"
#include "ipcm_osadapt.h"
#include "ipcm_nodes.h"
#include "ipcm_userdev.h"
#include "ipcm_funcs.h"

#ifdef RT_USING_USERSPACE
#include <lwp_user_mm.h>
#endif
#define IPCM_DEV_NAME	"ipcm_user"
struct kos_mem_list {
	struct kos_mem_list *prev;
	struct kos_mem_list *next;
	void *data;
	unsigned int len;
};

struct ipcm_dev_handle {
	unsigned long ipcm_handle;
	struct kos_mem_list mem_list;
	rt_wqueue_t *wait;
	rt_mutex_t mlist_mtx;
};

static void init_mem_list(struct kos_mem_list *head)
{
	head->prev = head;
	head->next = head;
}

void ipcm_handle_attr_init(struct ipcm_handle_attr *attr)
{
	int i;

	attr->target = -1;
	attr->port = -1;
	attr->priority = HANDLE_MSG_NORMAL;
	for (i = 0; i < IPCM_MAX_DEV_NR; i++)
		attr->remote_ids[i] = -1;
}

static void free_mem_list(struct kos_mem_list *head)
{
	struct kos_mem_list *mem, *next;

	while (head->next != head) {
		mem = head->next;
		next = mem->next;
		head->next = next;
		next->prev = head;
		free(mem);
	}
}

static void insert_mem_list(struct kos_mem_list *node,
		struct kos_mem_list *head)
{
	struct kos_mem_list *prev = head->prev;
	node->next = head;
	node->prev = prev;
	prev->next = node;
	head->prev = node;
}

static void delect_mem_list(struct kos_mem_list *node)
{
	struct kos_mem_list *prev = node->prev;
	struct kos_mem_list *next = node->next;
	prev->next = next;
	next->prev = prev;
	node->prev = NULL;
	node->next = NULL;
}

static int mem_list_empty(struct kos_mem_list *head)
{
	return head->next == head;
}

static int ipcm_dev_recv(void *vdd_handle, void *buf, unsigned int len)
{
	struct ipcm_dev_handle *phandle;
	struct ipcm_vdd_opt opt;
	struct kos_mem_list *mem;
	void *data;
	rt_wqueue_t *wait;
	unsigned long flags;

	ipcm_vdd_getopt(vdd_handle, &opt);
	phandle = (struct ipcm_dev_handle *)opt.data;

	data = rt_malloc(sizeof(struct kos_mem_list) + len);
	if (!data) {
		ipcm_err("mem alloc failed");
		return -ENOMEM;
	}

	mem = (struct kos_mem_list *)data;
	mem->data = (void *)((unsigned long)data + sizeof(struct kos_mem_list));
	memcpy(mem->data, buf, len);
	mem->len = len;


	flags =  rt_hw_interrupt_disable();
	insert_mem_list(mem, &phandle->mem_list);
	rt_hw_interrupt_enable(flags);

	wait = phandle->wait;
	rt_wqueue_wakeup(wait, (void*)POLLIN);

	return 0;
}

int ipcm_dev_open(struct dfs_fd *fd)
{
	struct ipcm_dev_handle *handle = NULL;
	rt_mutex_t mutex = NULL;
	rt_device_t device;

	handle = rt_malloc(sizeof(struct ipcm_dev_handle));
	if (!handle) {
		ipcm_err("no memory for ipcm_dev_handle");
		fd->data = NULL;
		return -RT_ENOMEM;
	}
	mutex =	rt_mutex_create("ipcm_user_mutex", RT_IPC_FLAG_PRIO);
	if (!mutex) {
		ipcm_err("ipcm user dev mutex create err");
		free(handle);
		return -RT_ENOMEM;
	}
	device = (rt_device_t)fd->fnode->data;
    RT_ASSERT(device != RT_NULL);

	init_mem_list(&handle->mem_list);
	handle->ipcm_handle = 0;
	handle->mlist_mtx = mutex;
	handle->wait = &device->wait_queue;
	fd->data = (void *)handle;
	return RT_EOK;
}

int ipcm_dev_close(struct dfs_fd *fd)
{
	struct ipcm_dev_handle *handle;
	void *vdd_handle;
	unsigned long flags;
	handle = (struct ipcm_dev_handle *)fd->data;
	if (!handle) {
		ipcm_err("try to close a invalid handle");
		return -RT_EINVAL;
	}
	vdd_handle = (void *)handle->ipcm_handle;
	if (vdd_handle) {
		ipcm_vdd_disconnect(vdd_handle);
		ipcm_vdd_setopt(vdd_handle, NULL);
		ipcm_vdd_close(vdd_handle);
	}
	flags =  rt_hw_interrupt_disable();
	free_mem_list(&handle->mem_list);
	rt_hw_interrupt_enable(flags);
	rt_mutex_delete(handle->mlist_mtx);
	free(handle);
	fd->data = NULL;
	return 0;

}

int ipcm_dev_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
	struct ipcm_dev_handle *handle;
	void *vdd_handle;
	struct ipcm_handle_attr attr;
	int check = HANDLE_DISCONNECTED;
	int local_id;
	struct ipcm_vdd_opt opt;
	unsigned int devices_nr;
	int ret;


	if (_IOC_TYPE(cmd) == 'M') {
		switch (_IOC_NR(cmd)) {
		case _IOC_NR(K_IPCM_IOC_ATTR_INIT):
			if(lwp_get_from_user((void *)&attr, args, sizeof(struct ipcm_handle_attr)) != sizeof(struct ipcm_handle_attr)) {
				ipcm_err("get from user err");
				return RT_ERROR;
			}
			ipcm_handle_attr_init(&attr);
			if(lwp_put_to_user(args, (void *)&attr, sizeof(struct ipcm_handle_attr)) != sizeof(struct ipcm_handle_attr)) {
				ipcm_err("put to user err");
				return RT_ERROR;
			}
			break;
		case _IOC_NR(K_IPCM_IOC_CONNECT):
		case _IOC_NR(K_IPCM_IOC_TRY_CONNECT):
			if(lwp_get_from_user((void *)&attr, args, sizeof(struct ipcm_handle_attr)) != sizeof(struct ipcm_handle_attr)) {
				ipcm_err("get from user err");
				return RT_ERROR;
			}
			handle = (struct ipcm_dev_handle *)fd->data;
			if (!handle) {
				ipcm_err("ipcm_dev_handle null");
				return -EINVAL;
			}

			if (handle->ipcm_handle) {
				vdd_handle = (void *)handle->ipcm_handle;
			} else {
				if ((attr.target < 0) || (attr.target >= MAX_NODES)) {
                    ipcm_err("ipcm target error");
                    return -EFAULT;
                }
				if ((_IOC_NR(K_IPCM_IOC_CONNECT)) == (_IOC_NR(cmd))) {
					while (!ipcm_vdd_node_ready(attr.target))
						__ipcm_msleep__(1);
                } else {
                    if (!ipcm_vdd_node_ready(attr.target))
                        return -EFAULT;
				}
				vdd_handle = ipcm_vdd_open(attr.target, attr.port,
						attr.priority);
				if (!vdd_handle) {
					ipcm_err("open ipcm vdd error");
					return -ENOMEM;
				}
				opt.recv = &ipcm_dev_recv;
				opt.data = (unsigned long)handle;
				ipcm_vdd_setopt(vdd_handle, &opt);
				handle->ipcm_handle = (unsigned long)vdd_handle;
			}
			if ((_IOC_NR(K_IPCM_IOC_CONNECT)) == (_IOC_NR(cmd)))
				ret = ipcm_vdd_connect(vdd_handle, CONNECT_BLOCK);
			else
				ret = ipcm_vdd_connect(vdd_handle, CONNECT_NONBLOCK);

			if (ret) {
				ipcm_trace(TRACE_MSG, "connect handle[%d:%d] failed!",
						attr.target, attr.port);
				return 1;
			}
			break;

		case _IOC_NR(K_IPCM_IOC_DISCONNECT):
			handle = (struct ipcm_dev_handle *)fd->data;
			if (!handle) {
				ipcm_err("ipcm_dev_handle null");
				return -EINVAL;
			}
			rt_mutex_take(handle->mlist_mtx, RT_WAITING_FOREVER);
			vdd_handle = (void *)handle->ipcm_handle;
			if (vdd_handle) {
				ipcm_vdd_disconnect(vdd_handle);
				ipcm_vdd_setopt(vdd_handle, NULL);
				ipcm_vdd_close(vdd_handle);
			}
			handle->ipcm_handle = 0;
			rt_mutex_release(handle->mlist_mtx);
			break;
		case _IOC_NR(K_IPCM_IOC_CHECK):
			handle = (struct ipcm_dev_handle *)fd->data;
			if (!handle) {
				ipcm_err("ipcm_dev_handle null");
				return -EINVAL;

			}
			vdd_handle = (void *)handle->ipcm_handle;
			if (vdd_handle) {
				check = ipcm_vdd_check_handle(vdd_handle);
			}
			return check;
		case _IOC_NR(K_IPCM_IOC_GET_LOCAL_ID):
			local_id = ipcm_vdd_localid();
			return local_id;
		case _IOC_NR(K_IPCM_IOC_GET_REMOTE_ID):
			devices_nr = ipcm_vdd_remoteids(attr.remote_ids);
			return devices_nr;
		case _IOC_NR(K_IPCM_IOC_GET_REMOTE_STS):
				return ipcm_vdd_node_ready(attr.target);
		case _IOC_NR(K_IPCM_IOC_NODE_RESET):
			{
				struct ipcm_node_reset reset;
				lwp_get_from_user((void *)&reset, args, sizeof(struct ipcm_node_reset) != sizeof(struct ipcm_node_reset));
				return ipcm_vdd_reset_node(reset.target, reset.timeout);
			}
		default:
			ipcm_err("ioctl unknow cmd!");
			break;
		}
	}
	return 0;
}
int ipcm_dev_write(struct dfs_fd *fd, const void *buf, size_t count)
{
	int ret = 0;
	struct ipcm_dev_handle *handle;
	void *vdd_handle;
	char *kbuf;

	handle = (struct ipcm_dev_handle *)fd->data;
	if (!handle || !buf) {
		ipcm_err("mem error");
		return -RT_ENOMEM;
	}
	vdd_handle = (void *)handle->ipcm_handle;
	if (!vdd_handle) {
		ipcm_err("invalid vdd_handle");
		return -RT_ENOMEM;
	}
#if 0
	kbuf = rt_malloc(count);
	ret = lwp_get_from_user(kbuf, (void *)buf, count);
	if(ret < 0) {
		ipcm_err("get from user err");
		return -RT_ERROR;
	}
#endif
	ret = ipcm_vdd_sendmsg(vdd_handle, buf, count);

	if (ret < 0)
		ipcm_err("sendto failed,ret:%d", ret);

	return ret;
}
int ipcm_dev_read(struct dfs_fd *fd, void *buf, size_t count)
{
	struct ipcm_dev_handle *handle;
	unsigned int len = 0;
	struct kos_mem_list *mem = NULL;
	unsigned long flags;

	handle = (struct ipcm_dev_handle *)fd->data;
	if (!handle) {
		ipcm_err("ipcm_dev_handle null");
		return -EINVAL;
	}
	flags =  rt_hw_interrupt_disable();
	if(!mem_list_empty(&handle->mem_list)) {
		mem = handle->mem_list.next;
		delect_mem_list(mem);
	}
	rt_hw_interrupt_enable(flags);
	if (mem) {
		len = mem->len;
		if (len > count)
			len = count;
#if 0
		lwp_put_to_user(buf, mem->data, len);
#else
		memcpy(buf, mem->data, len);
#endif
		free(mem);
	}
	return len;
}
int ipcm_dev_poll(struct dfs_fd *fd, struct rt_pollreq *req)
{
	struct ipcm_dev_handle *handle;
	unsigned int flags;
	handle = (struct ipcm_dev_handle*)fd->data;
	if (!handle) {
		ipcm_err("ipcm_dev_handle NULL!");
		return -EINVAL;
	}
	rt_poll_add(handle->wait, req);

	flags =  rt_hw_interrupt_disable();
	/* mem list empty means no data comming */
	if (!mem_list_empty(&handle->mem_list)) {
		rt_hw_interrupt_enable(flags);
		ipcm_trace(TRACE_DEV, "mem list not empty");
		return POLLIN | POLLRDNORM;
	}
	rt_hw_interrupt_enable(flags);

	return 0;
}
#ifdef RT_USING_DFS_PROCFS
static const struct dfs_file_ops ipcm_userdev_fops = {
	.open           = ipcm_dev_open,
	.close			= ipcm_dev_close,
	.ioctl			= ipcm_dev_ioctl,
	.write          = ipcm_dev_write,
	.read           = ipcm_dev_read,
	.poll           = ipcm_dev_poll,
};

int ipcm_open_proc(struct dfs_fd *fd)
{
	return 0;
}

int ipcm_close_proc(struct dfs_fd *fd)
{
	return 0;
}

int ipcm_read_proc(struct dfs_fd *fd, void *buf, size_t count)
{
	return __ipcm_read_proc__(NULL);
}

static const struct dfs_file_ops ipcm_proc_ops = {
    .open = ipcm_open_proc,
    .close = ipcm_close_proc,
    .read =  ipcm_read_proc,
};

static int ipcm_proc_init(void)
{
    int ret = 0;
    rt_proc_entry_t entry = rt_proc_entry_create(RT_Proc_Class_DeviceInfo, 0);
    ret = rt_proc_entry_register(entry,  IPCM_PROC_NAME);
    if(ret) {
        rt_kprintf("ipcm proc regisger fail \n");
        rt_proc_entry_destory(entry);
        return ret;
    }
    entry->fops  =  &ipcm_proc_ops;
    return 0;
}

static void ipcm_proc_exit(void)
{
    rt_proc_entry_t entry = rt_proc_entry_find(IPCM_PROC_NAME);
    if(entry) {
        rt_proc_entry_unregister(entry);
        rt_proc_entry_destory(entry);
    }
	return;
}

#else /* RT_USING_DFS_PROCFS */
static int ipcm_proc_init(void)
{
	return 0;
}
static void ipcm_proc_exit(void)
{
	return;
}
#endif

int _ipcm_vdd_init(void)
{
	rt_device_t ipcm_user_dev = NULL;
	int ret = ipcm_vdd_init();
	if (ret) {
		ipcm_err("module register failed");
		return ret;
	}
	ipcm_user_dev = rt_device_create(RT_Device_Class_Char, 0);
	if(!ipcm_user_dev) {
		ipcm_err("ipcm user dev create err");
	}
	ret = rt_device_register(ipcm_user_dev, IPCM_DEV_NAME, RT_DEVICE_FLAG_RDWR);
	if(ret)
	{
		ipcm_err("ipcm user dev register err:%d\n", ret);
		rt_device_destroy(ipcm_user_dev);
		return ret;
	}
    /* set fops */
    ipcm_user_dev->fops = &ipcm_userdev_fops;

	ipcm_proc_init();

	return ret;
}

void _ipcm_vdd_cleanup(void)
{
	rt_device_t dev = NULL;

	ipcm_proc_exit();

	dev = rt_device_find(IPCM_DEV_NAME);
	if(!dev) {
		rt_device_unregister(dev);
		dev->fops = NULL;
		rt_device_destroy(dev);
	}
	ipcm_vdd_cleanup();
}

#ifdef LOSCFG_SHELL
#endif /* LOSCFG_SHELL */
#endif
