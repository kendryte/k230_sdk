#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/syscore_ops.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "ipcm_osadapt.h"
#include "ipcm_nodes.h"
#include "ipcm_funcs.h"
#include "ipcm_userdev.h"

struct kos_mem_list {
	struct list_head head;
	void *data;
	unsigned int len;
};

struct ipcm_dev_handle {
	unsigned long ipcm_handle;
	struct list_head mem_list;
	wait_queue_head_t wait;
	spinlock_t lock;
};

void ipcm_handle_attr_init(struct ipcm_handle_attr *attr)
{
	int i;

	attr->target = -1;
	attr->port = -1;
	attr->priority = HANDLE_MSG_NORMAL;
	for (i = 0; i < IPCM_MAX_DEV_NR; i++)
		attr->remote_ids[i] = -1;
}

static void del_mem_list(struct ipcm_dev_handle *handle)
{
	struct list_head *entry, *next;
	struct kos_mem_list *mem;
	if (!list_empty(&handle->mem_list)) {
		list_for_each_safe(entry, next, &handle->mem_list) {
			mem = list_entry(entry, struct kos_mem_list, head);
			list_del(&mem->head);
			kfree(mem);
		}
	}
}

static int ipcm_dev_recv(void *vdd_handle, void *buf, unsigned int len)
{
	struct ipcm_dev_handle *phandle;
	struct ipcm_vdd_opt opt;
	struct kos_mem_list *mem;
	void *data;
	unsigned long flags;

	ipcm_vdd_getopt(vdd_handle, &opt);
	phandle = (struct ipcm_dev_handle *)opt.data;

	data = kmalloc(sizeof(struct kos_mem_list) + len, GFP_ATOMIC);
	if (!data) {
		ipcm_err("mem alloc failed");
		return -ENOMEM;
	}

	mem = (struct kos_mem_list *)data;
	mem->data = (void *)((long)data + sizeof(struct kos_mem_list));
	__memcpy__(mem->data, buf, len);
	mem->len = len;

	spin_lock_irqsave(&phandle->lock, flags);
	list_add_tail(&mem->head, &phandle->mem_list);
	spin_unlock_irqrestore(&phandle->lock, flags);

	wake_up_interruptible(&phandle->wait);

	return 0;
}

static int ipcm_dev_open(struct inode *inode, struct file *file)
{
	struct ipcm_dev_handle *handle;

	handle = kmalloc(sizeof(struct ipcm_dev_handle), GFP_ATOMIC);
	if (!handle) {
		file->private_data = NULL;
		return -EFAULT;
	}
	INIT_LIST_HEAD(&handle->mem_list);
	init_waitqueue_head(&handle->wait);
	spin_lock_init(&handle->lock);
	handle->ipcm_handle = 0;

	file->private_data = (void *)handle;
	return 0;
}

static int ipcm_dev_release(struct inode *inode, struct file *file)
{
	struct ipcm_dev_handle *handle;
	void *vdd_handle;
	unsigned long flags;

	handle = (struct ipcm_dev_handle *)file->private_data;
	if (!handle) {
		ipcm_err("try to close a invalid handle");
		return -EFAULT;
	}
	vdd_handle = (void *)handle->ipcm_handle;

	if (vdd_handle) {
		ipcm_vdd_disconnect(vdd_handle);
		ipcm_vdd_setopt(vdd_handle, NULL);
		ipcm_vdd_close(vdd_handle);
	}

	/* reclaim mem list */
	spin_lock_irqsave(&handle->lock, flags);
	if (!list_empty(&handle->mem_list))
		del_mem_list(handle);
	spin_unlock_irqrestore(&handle->lock, flags);

	file->private_data = NULL;
	kfree(handle);
	return 0;
}

static ssize_t ipcm_dev_read(struct file *file, char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct ipcm_dev_handle *handle;
	struct list_head *entry, *next;
	unsigned int len = 0;
	int read = 0;
	struct kos_mem_list *mem = NULL;
	unsigned long flags;

	handle = (struct ipcm_dev_handle *)file->private_data;
	if (!handle) {
		ipcm_err("ipcm_dev_handle null");
		return -EINVAL;
	}

	spin_lock_irqsave(&handle->lock, flags);
	if (!list_empty(&handle->mem_list)) {
		list_for_each_safe(entry, next, &handle->mem_list) {
			mem = list_entry(entry, struct kos_mem_list, head);
			len = mem->len;
			list_del(&mem->head);
			break;
		}
	}
	spin_unlock_irqrestore(&handle->lock, flags);

	if (mem) {
		if (len > count)
			len = count;
		read = copy_to_user(buf, mem->data, len);
		if (read < 0)
			ipcm_err("copy to user err");
		kfree(mem);
	}
	return len;
}

static ssize_t ipcm_dev_write(struct file *file, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	int ret = 0;
	struct ipcm_dev_handle *handle;
	void *vdd_handle;
	char *kbuf;

	handle = (struct ipcm_dev_handle *)file->private_data;
	if (!handle) {
		ipcm_err("mem error");
		return -ENOMEM;
	}
	vdd_handle = (void *)handle->ipcm_handle;
	if (!vdd_handle) {
		ipcm_err("invalid vdd_handle");
		return -EFAULT;
	}
	kbuf = kmalloc(count, GFP_ATOMIC);
	if (NULL == kbuf) {
		ipcm_err("mem error");
		return -ENOMEM;
	}
	ret = copy_from_user(kbuf, buf, count);
	if (ret < 0) {
		ipcm_err("copy from user err");
		ret = -EFAULT;
		goto write_out;
	}
	ret = ipcm_vdd_sendmsg(vdd_handle, kbuf, count);
	if (ret < 0)
		ipcm_err("sendto failed,ret:%d", ret);

write_out:
	kfree(kbuf);
	return ret;
}

static long ipcm_dev_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
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
			if (copy_from_user((void *)&attr, (void *)arg,
						sizeof(struct ipcm_handle_attr))) {
				ipcm_err("copy from user err");
				return -EFAULT;
			}
			ipcm_handle_attr_init(&attr);
			if (copy_to_user((void *)arg,
				(void *)&attr,
				sizeof(struct ipcm_handle_attr))) {
				ipcm_err("copy to user err");
				return -EFAULT;
			}
			break;
		case _IOC_NR(K_IPCM_IOC_CONNECT):
		case _IOC_NR(K_IPCM_IOC_TRY_CONNECT):
			handle = (struct ipcm_dev_handle *)file->private_data;
			if (!handle) {
				ipcm_err("ipcm_dev_handle null");
				return -EINVAL;
			}

			if (handle->ipcm_handle) {
				vdd_handle = (void *)handle->ipcm_handle;
			} else {
				if (copy_from_user((void *)&attr, (void *)arg,
							sizeof(struct ipcm_handle_attr))) {
					ipcm_err("copy from user err");
					return -EFAULT;
				}
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
				return ret;
			}
			break;
		case _IOC_NR(K_IPCM_IOC_DISCONNECT):
			handle = (struct ipcm_dev_handle *)file->private_data;
			if (!handle) {
				ipcm_err("ipcm_dev_handle null");
				return -EINVAL;
			}
			vdd_handle = (void *)handle->ipcm_handle;
			if (vdd_handle) {
				ipcm_vdd_disconnect(vdd_handle);
				ipcm_vdd_setopt(vdd_handle, NULL);
				ipcm_vdd_close(vdd_handle);
			}
			handle->ipcm_handle = 0;
			break;
		case _IOC_NR(K_IPCM_IOC_CHECK):
			handle = (struct ipcm_dev_handle *)file->private_data;
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
			if (copy_from_user((void *)&attr, (void *)arg,
						sizeof(struct ipcm_handle_attr))) {
				ipcm_err("copy from user err");
				return -EFAULT;
			}
			devices_nr = ipcm_vdd_remoteids(attr.remote_ids);
			if (copy_to_user((void *)arg, (void *)&attr,
					sizeof(struct ipcm_handle_attr))) {
				ipcm_err("copy to user err");
				return -EFAULT;
			}
			return devices_nr;
		case _IOC_NR(K_IPCM_IOC_GET_REMOTE_STS):
			return ipcm_vdd_node_ready(arg);
		case _IOC_NR(K_IPCM_IOC_NODE_RESET):
			{
				struct ipcm_node_reset reset;
				if (copy_from_user((void *)&reset, (void *)arg,
							sizeof(struct ipcm_node_reset))) {
					ipcm_err("copy from user err");
					return -EFAULT;
				}
				return ipcm_vdd_reset_node(reset.target, reset.timeout);
			}
		default:
			ipcm_err("ioctl unknow cmd!");
			break;
		}
	}

	return 0;
}

static unsigned int ipcm_dev_poll(struct file *file, struct poll_table_struct *table)
{
	struct ipcm_dev_handle *handle;
	unsigned long flags;

	handle = (struct ipcm_dev_handle*)file->private_data;
	if (!handle) {
		ipcm_err("ipcm_dev_handle null");
		return -EINVAL;
	}
	poll_wait(file, &handle->wait, table);

	/* if mem list empty means no data comming */
	spin_lock_irqsave(&handle->lock, flags);
	if (!list_empty(&handle->mem_list)) {
		spin_unlock_irqrestore(&handle->lock, flags);
		ipcm_trace(TRACE_DEV, "mem list not empty");
		return POLLIN | POLLRDNORM;
	}
	spin_unlock_irqrestore(&handle->lock, flags);

	return 0;
}

static void ipcm_syscore_shutdown(void)
{
	ipcm_vdd_cleanup();
}

static const struct file_operations ipcm_userdev_fops = {
	.owner          = THIS_MODULE,
	.open           = ipcm_dev_open,
	.release        = ipcm_dev_release,
	.unlocked_ioctl = ipcm_dev_ioctl,
	.write          = ipcm_dev_write,
	.read           = ipcm_dev_read,
	.poll           = ipcm_dev_poll,
};

static struct miscdevice ipcm_userdev = {
	.minor  = MISC_DYNAMIC_MINOR,
	.fops   = &ipcm_userdev_fops,
	.name   = "ipcm_user"
};

static struct syscore_ops ipcm_syscore_ops = {
	.shutdown   = ipcm_syscore_shutdown
};

/* proc interface setup */
static void *ipcm_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter;
	if (*pos == 0)
		return &counter;
	else {
		*pos = 0;
		return NULL;
	}
}

static void *ipcm_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return NULL;
}

static void ipcm_seq_stop(struct seq_file *s, void *v)
{
}

static int ipcm_seq_show(struct seq_file *sfile, void *v)
{
	return __ipcm_read_proc__((void *)sfile);
}

static const struct seq_operations ipcm_seq_ops = {
	.start = ipcm_seq_start,
	.next  = ipcm_seq_next,
	.stop  = ipcm_seq_stop,
	.show  = ipcm_seq_show
};

static int ipcm_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ipcm_seq_ops);
}

#if 0
static const struct file_operations ipcm_proc_fops = {
	.owner = THIS_MODULE,
	.open = ipcm_proc_open,
	.read = seq_read,
	.release = seq_release
};
#endif

static const struct proc_ops ipcm_proc_fops = {
	.proc_open = ipcm_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = seq_release
};

static struct proc_dir_entry *entry = NULL;

int ipcm_dev_init(void)
{
	int ret;
	ret = misc_register(&ipcm_userdev);
	if (ret) {
		ipcm_err("module register failed");
		return ret;
	}

	ret = ipcm_vdd_init();
	if (ret) {
		ipcm_err("module register failed");
		misc_deregister(&ipcm_userdev);
		return ret;
	}
	register_syscore_ops(&ipcm_syscore_ops);

	entry = proc_create(IPCM_PROC_NAME, 0, NULL, &ipcm_proc_fops);
	if (!entry){
		printk(KERN_ERR "Create ipcm proc fail!\n");
	}
	pr_info("%s OK\n", __func__);

	return ret;
}

void ipcm_dev_cleanup(void)
{
	if (entry)
		proc_remove(entry);
	unregister_syscore_ops(&ipcm_syscore_ops);
	misc_deregister(&ipcm_userdev);
	ipcm_vdd_cleanup();
}

module_init(ipcm_dev_init);
module_exit(ipcm_dev_cleanup);

MODULE_AUTHOR("anonymous");
MODULE_DESCRIPTION("Something Huge");
MODULE_LICENSE("GPL");
