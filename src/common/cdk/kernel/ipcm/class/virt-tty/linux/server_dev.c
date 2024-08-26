#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include "virt-tty.h"
#include "virt-tty_config.h"
#include "server.h"

static dev_t virt_tty_devt;

static char *virt_tty_devnode(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "virt-tty/%s", dev_name(dev));
}

struct class virt_tty_class = {
	.name       = "virt-tty",
	.devnode    = virt_tty_devnode,
};

struct virt_tty_server_device {
	int id;
	int count;
	void *handle;
	struct device dev;
	struct cdev cdev;
};
static struct virt_tty_server_device *devices[MAX_CLIENTS];

static int virt_tty_dev_open(struct inode *inode, struct file *file)
{
	struct virt_tty_server_device *virt_ttydev = container_of(inode->i_cdev,
			struct virt_tty_server_device, cdev);
	if (virt_ttydev->count) {
		printk(KERN_ERR "huh, you can't open me more than once.\n");
		return -EPERM;
	}
	virt_ttydev->count++;
	file->private_data = virt_ttydev->handle;
	return 0;
}

static ssize_t virt_tty_dev_read(struct file *file, char __user *buf,
		size_t count, loff_t *f_pos)
{
	ssize_t ret;
	void *handle = file->private_data;
	void *vbuf;

	if (!handle) {
		printk(KERN_ERR"mem error");
		return -ENOMEM;
	}
	vbuf = vmalloc(count);
	if (!vbuf) {
		printk(KERN_ERR"mem alloc failed");
		return -ENOMEM;
	}
	ret = virt_tty_server_read(handle, vbuf, count);
	if (ret < 0) {
		goto out;
	}
	if (copy_to_user(buf, vbuf, count)) {
		printk(KERN_ERR"copy_to_user error");
		ret = -EFAULT;
	}

out:
	vfree(vbuf);
	return ret;
}

static ssize_t virt_tty_dev_write(struct file *file, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	ssize_t ret;
	void *handle = file->private_data;
	void *vbuf;

	if (!handle) {
		printk(KERN_ERR"mem error");
		return -ENOMEM;
	}
	vbuf = vmalloc(count);
	if (!vbuf) {
		printk(KERN_ERR"mem alloc failed,count=%lx\n",count);
		return -ENOMEM;
	}
	if (!copy_from_user(vbuf, buf, count)) {
		ret = virt_tty_server_write(handle, vbuf, count);
	} else {
		printk(KERN_ERR"copy_from_user error");
		ret = -EFAULT;
	}

	vfree(vbuf);
	return ret;
}

static int virt_tty_dev_release(struct inode *inode, struct file *file)
{
	struct virt_tty_server_device *virt_ttydev = container_of(inode->i_cdev,
			struct virt_tty_server_device, cdev);
	virt_ttydev->count--;

	return 0;
}

static long virt_tty_dev_ioctl(struct file *file,
		unsigned int cmd, unsigned long args)
{
	return 0;
}

static const struct file_operations virt_ttydev_fops = {
	.owner          = THIS_MODULE,
	.open           = virt_tty_dev_open,
	.release        = virt_tty_dev_release,
	.unlocked_ioctl = virt_tty_dev_ioctl,
	.write          = virt_tty_dev_write,
	.read           = virt_tty_dev_read
};

static void virt_ttydev_free(struct device *dev)
{
	struct virt_tty_server_device *virt_ttydev = container_of(dev,
			struct virt_tty_server_device, dev);
	kfree(virt_ttydev);
}

int virt_tty_server_register_cdev(void *data, const char *name, int id)
{
	struct virt_tty_server_device *virt_ttydev;
	int ret;

	virt_ttydev = kzalloc(sizeof(struct virt_tty_server_device), GFP_KERNEL);
	if (!virt_ttydev)
		return -ENOMEM;

	dev_set_name(&virt_ttydev->dev, name);
	virt_ttydev->handle = data;
	virt_ttydev->id = id;
	virt_ttydev->count = 0;
	virt_ttydev->dev.class = &virt_tty_class;
	virt_ttydev->dev.devt = MKDEV(MAJOR(virt_tty_devt), virt_ttydev->id);
	virt_ttydev->dev.release = virt_ttydev_free;
	device_initialize(&virt_ttydev->dev);

	cdev_init(&virt_ttydev->cdev, &virt_ttydev_fops);
	ret = cdev_add(&virt_ttydev->cdev, virt_ttydev->dev.devt, 1);
	if (ret) {
		printk(KERN_ERR"failed to add virt_tty char device %d:%d\n",
				MAJOR(virt_tty_devt), virt_ttydev->id);
		goto cleanup_virt_ttydev;
	}

	ret = device_add(&virt_ttydev->dev);
	if (ret) {
		printk(KERN_ERR"failed to add virt_tty device\n");
		goto del_cdev;
	}
	devices[id] = virt_ttydev;
	return 0;

del_cdev:
	cdev_del(&virt_ttydev->cdev);
cleanup_virt_ttydev:
	put_device(&virt_ttydev->dev);

	return ret;
}

void virt_tty_server_unregister_cdev(int id)
{
	struct virt_tty_server_device *virt_ttydev = devices[id];

	if (!virt_ttydev)
		return;
	device_del(&virt_ttydev->dev);
	cdev_del(&virt_ttydev->cdev);
	put_device(&virt_ttydev->dev);
	devices[id] = NULL;
}

static int __init virt_tty_server_dev_init(void)
{
	int ret;
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
		devices[i] = NULL;
	ret = virt_tty_server_init();
	if (ret) {
		printk("init server virt_tty failed\n");
		return -1;
	}
	ret = class_register(&virt_tty_class);
	if (ret) {
		printk("unable to register virt_tty_dev class\n");
		goto cleanup_virt_tty;
	}
	ret = alloc_chrdev_region(&virt_tty_devt, 0, MAX_CLIENTS, "virt-tty");
	if (ret < 0) {
		printk("alloc char dev for virt_tty failed\n");
		goto cleanup_class;
	}
	return 0;

cleanup_class:
	class_unregister(&virt_tty_class);
cleanup_virt_tty:
	virt_tty_server_cleanup();

	return ret;
}

static void virt_tty_server_dev_cleanup(void)
{
	virt_tty_server_cleanup();
	unregister_chrdev_region(virt_tty_devt, MAX_CLIENTS);
	class_unregister(&virt_tty_class);
}

module_init(virt_tty_server_dev_init);
module_exit(virt_tty_server_dev_cleanup);

MODULE_AUTHOR("Ou Jinsong");
MODULE_DESCRIPTION("virt-tty for multi OS debug, log.");
MODULE_LICENSE("GPL");
