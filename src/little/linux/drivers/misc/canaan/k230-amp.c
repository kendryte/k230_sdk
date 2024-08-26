#include <linux/module.h>
#include <linux/init.h>

#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>


#define AMP_CMD_BOOT             _IOWR('q', 1, unsigned long)

#define SYSCTL_BASE         0x91100000ULL
#define CPU1_HART_RSTVEC    0x2104
#define CPU1_RST_CTL        0x100c

struct amp_plat {
    void __iomem    *sysctl_base;
    unsigned int    p_amp_reset_vec;
    unsigned int    size;
    int             major;
    int             minor;
    struct class    *class;
    struct cdev     cdev;
};

static struct amp_plat *plat;

static int amp_boot(unsigned long v_amp_reset_vec)
{
    unsigned long flags;
    local_irq_save(flags);
    __asm__ __volatile__(
        "li t6, 0x00040000\t\n"
        "csrs sstatus, t6\t\n"
    );
    dcache_wb_range(v_amp_reset_vec, v_amp_reset_vec + plat->size);
    __asm__ __volatile__(
        "li t6, 0x00040000\t\n"
        "csrc sstatus, t6\t\n"
    );
    local_irq_restore(flags);
    iowrite32(plat->p_amp_reset_vec, plat->sysctl_base + CPU1_HART_RSTVEC);
    
    iowrite32(0x10001000, plat->sysctl_base + CPU1_RST_CTL);
    iowrite32(0x10001, plat->sysctl_base + CPU1_RST_CTL);
    iowrite32(0x10000, plat->sysctl_base + CPU1_RST_CTL);

    return 0;
}

static int setup_chrdev_region(void)
{
    dev_t dev = 0;
    int err;

    if (plat->major == 0) {
        err = alloc_chrdev_region(&dev, 0, 1, "k230-amp");
        plat->major = MAJOR(dev);

        if (err) {
            printk("k230-amp: can't get major %d\n", plat->major);
            return err;
        }
    }
    return 0;
}

static int create_module_class(void)
{
    plat->class = class_create(THIS_MODULE, "k230_amp_class");

    if (IS_ERR(plat->class))
        return PTR_ERR(plat->class);

    return 0;
}

static void clean_up_amp_cdev(void)
{
    cdev_del(&plat->cdev);
}

static int amp_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int amp_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long amp_ioctl(struct file *filp, unsigned int cmd,
                             unsigned long arg)
{
    switch (cmd) {
    case AMP_CMD_BOOT:
        return amp_boot(arg);
    default:
        printk("amp_ioctl: Unknown ioctl: 0x%.8X\n", cmd);
        return -EINVAL;
    }
}

static int amp_mmap(struct file *file, struct vm_area_struct *vma)
{
    // mmap时 reset 大核，保证内存空间不被修改。
    iowrite32(0x10001, plat->sysctl_base + CPU1_RST_CTL);
    
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);//标志该内存区不能被换出
    if(remap_pfn_range(vma,
                       vma->vm_start,
                       plat->p_amp_reset_vec>>PAGE_SHIFT,
                       vma->vm_end - vma->vm_start,
                       vma->vm_page_prot))
    {  
        return -EAGAIN;
    }
    plat->size = vma->vm_end - vma->vm_start;
    return 0;
}

const struct file_operations amp_fops = {
    .owner          = THIS_MODULE,
    .open           = amp_open,
    .release        = amp_release,
    .unlocked_ioctl = amp_ioctl,
    .mmap           = amp_mmap,
};

int setup_amp_cdev(const char *device_name)
{
    struct device *device;
    int err, devno =
        MKDEV(plat->major, plat->minor);

    cdev_init(&plat->cdev, &amp_fops);
    plat->cdev.owner = THIS_MODULE;
    err = cdev_add(&plat->cdev, devno, 1);
    if (err) {
        printk("amp: Error %d adding amp device number %d \n", err, plat->minor);
        return err;
    }

    if (device_name != NULL) {
        device = device_create(plat->class, NULL, devno, NULL,
                                device_name);
        if (IS_ERR(device)) {
            printk("amp: device not created\n");
            clean_up_amp_cdev();
            return PTR_ERR(device);
        }
    }

    return 0;
}

static int amp_probe(struct platform_device *pdev)
{
    u32 sysctl_phy_addr;

    plat = kzalloc(sizeof(struct amp_plat), GFP_KERNEL);
    if (!plat) {
        printk("amp_probe: kzalloc failed \n");
        return -ENOMEM;
    }
    
    plat->sysctl_base = ioremap(SYSCTL_BASE, 0xffff);
    if(!plat->sysctl_base) {
        printk("amp_probe: ioremap failed \n");
        kfree(plat);
        return -ENOMEM;
    }

    device_property_read_u32(&pdev->dev, "amp-reset-vec", &plat->p_amp_reset_vec);
    plat->size = 0;

    plat->major = 0;
    plat->minor = 0;

    setup_chrdev_region();
    create_module_class();

    setup_amp_cdev("k230-amp");
    printk("amp_probe \n");
	return 0;
}

static int amp_remove(struct platform_device *pdev)
{
        dev_t dev = MKDEV(plat->major, plat->minor);

        device_destroy(plat->class, dev);
        clean_up_amp_cdev();
        kfree(plat);

        return 0;
}


static const struct of_device_id k230_amp_ids[] = {
	{ .compatible = "canaan,k230-amp" },
	{}
};

static struct platform_driver k230_amp_driver = {
    .probe          = amp_probe,
    .remove         = amp_remove,
    .driver         = {
        .name           = "k230-amp",
        .of_match_table = of_match_ptr(k230_amp_ids),
    },
};

int amp_module_init(void)
{
        int ret;

        ret = platform_driver_register(&k230_amp_driver);

        return ret;
}

void amp_module_deinit(void)
{
        platform_driver_unregister(&k230_amp_driver);
}

module_init(amp_module_init);
module_exit(amp_module_deinit);
MODULE_LICENSE("GPL v2");
