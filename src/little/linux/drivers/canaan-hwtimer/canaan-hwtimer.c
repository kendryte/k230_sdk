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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/io.h>
#include <asm/atomic.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/property.h>

#define K230_TIMER0_CHANL     (0x00)
#define K230_TIMER1_CHANL     (0x14)
#define K230_TIMER2_CHANL     (0x28)
#define K230_TIMER3_CHANL     (0x3c)
#define K230_TIMER4_CHANL     (0x50)
#define K230_TIMER5_CHANL     (0x64)
#define LOAD_COUNT            (0x00)
#define CURRENT_VALUE         (0x04)
#define CONTROL               (0x08)
#define EOI                   (0x0c)
#define INTR_STAT             (0x10)
#define CANAAN_DRIVER_NAME	"canaan-hwtimer"

static void __iomem *addr;
static unsigned int timeout = 1000; //ms
// static unsigned int new0, new1, new2, new3, new4, new5;
// static unsigned int old0, old1, old2, old3, old4, old5;

#define	SOFT_TIMEOUT		msecs_to_jiffies(timeout)
#define TMIOC_SET_TIMEOUT     _IOW('T', 0x20, int)


typedef struct _hwtimer_t {
    void __iomem    *base;
    struct device *dev;
    struct miscdevice mdev;
    struct timer_list softtimer;
    struct timespec64 ts_old, ts_new;
    unsigned int old, new;
    int index;
} hw_timer_t;

#define to_hwtimer(x)	container_of((x), hw_timer_t, mdev)

static bool first_init = true;

static void k230_hwtimer_reg_init(void __iomem *base)
{
    int value;
    /* stop all timer */
    writel(0x0, base + K230_TIMER0_CHANL + CONTROL);
    writel(0x0, base + K230_TIMER1_CHANL + CONTROL);
    writel(0x0, base + K230_TIMER2_CHANL + CONTROL);
    writel(0x0, base + K230_TIMER3_CHANL + CONTROL);
    writel(0x0, base + K230_TIMER4_CHANL + CONTROL);
    writel(0x0, base + K230_TIMER5_CHANL + CONTROL);

    /* set mode */
    value = readl(base + K230_TIMER0_CHANL + CONTROL);
    value &= ~0x2;
    value |= 0x0;

    writel(value, base + K230_TIMER0_CHANL + CONTROL);
    writel(value, base + K230_TIMER1_CHANL + CONTROL);
    writel(value, base + K230_TIMER2_CHANL + CONTROL);
    writel(value, base + K230_TIMER3_CHANL + CONTROL);
    writel(value, base + K230_TIMER4_CHANL + CONTROL);
    writel(value, base + K230_TIMER5_CHANL + CONTROL);

    writel(0xffffffff, base + K230_TIMER0_CHANL + LOAD_COUNT);
    writel(0xffffffff, base + K230_TIMER1_CHANL + LOAD_COUNT);
    writel(0xffffffff, base + K230_TIMER2_CHANL + LOAD_COUNT);
    writel(0xffffffff, base + K230_TIMER3_CHANL + LOAD_COUNT);
    writel(0xffffffff, base + K230_TIMER4_CHANL + LOAD_COUNT);
    writel(0xffffffff, base + K230_TIMER5_CHANL + LOAD_COUNT);
}

static void k230_hwtimer_start(hw_timer_t *timer, int enable)
{
    void __iomem *base = timer->base;
    int id = timer->index;
    char channel;
    unsigned int value;

    switch(id)
    {
        case 0:
            channel = K230_TIMER0_CHANL;
            break;
        case 1:
            channel = K230_TIMER1_CHANL;
            break;
        case 2:
            channel = K230_TIMER2_CHANL;
            break;
        case 3:
            channel = K230_TIMER3_CHANL;
            break;
        case 4:
            channel = K230_TIMER4_CHANL;
            break;
        case 5:
            channel = K230_TIMER5_CHANL;
            break;
        default:
            return;
    }

    if (enable)
    {
        value = readl(base + channel + CONTROL);
        value &= ~0x4;
        value |= 0x3;
        writel(value, base + channel + CONTROL);
    }
    else
    {
        value = readl(base + channel + CONTROL);
        value &= ~0x1;
        value |= 0x4;
        writel(value, base + channel + CONTROL);
    }

    return;
}

static void softtimer_func(struct timer_list *soft_timer)
{
	unsigned int value;
	hw_timer_t *timer = from_timer(timer, soft_timer, softtimer);
    void __iomem *reg = timer->base + (timer->index*0x14) + CURRENT_VALUE;
    
    struct timespec64 timetmp;
    //printk("f=%s l=%d\n", __func__, __LINE__);

    if(timer->index > 5)
        return;
    //local_irq_disable();
    value = readl(reg);
    ktime_get_boottime_ts64(&timetmp);
    //local_irq_enable();
    timer->old = timer->new;
    timer->ts_old = timer->ts_new;
    
    timer->new = value;
    timer->ts_new = timetmp;

    mod_timer(&timer->softtimer, jiffies + SOFT_TIMEOUT);
}

static int k230_hwtimer_open(struct inode *inode, struct file *file)
{
    hw_timer_t *timer = container_of(file->private_data,
					      hw_timer_t, mdev);

    k230_hwtimer_start(timer, 1);
	//timer_setup(&timer->softtimer, &softtimer_func, 0);
    mod_timer(&timer->softtimer, jiffies + SOFT_TIMEOUT);
	return 0;
}

static int k230_hwtimer_release(struct inode *inode, struct file *file)
{
	struct miscdevice *mdev = file->private_data;
	hw_timer_t *timer = to_hwtimer(mdev);

    //del_timer(&timer->softtimer);

    //k230_hwtimer_start(timer, 0);
    return 0;
}

static ssize_t k230_hwtimer_read(struct file *file, char __user *buf, size_t len,
			    loff_t *ppos)
{
    hw_timer_t *timer = container_of(file->private_data,
					      hw_timer_t, mdev);

    
    int *data = (int*)buf;
    u32 old = 0, new = 0;
    unsigned int freq = 0;
    
    struct timespec64 ts_diff = timespec64_sub(timer->ts_new, timer->ts_old);
    s64 time_diff = timespec64_to_ns(&ts_diff)/1000000; //ms;
   
    if ((len < sizeof(freq)) || (timer->index > 5))
        return -EINVAL;

     
    if( (time_diff > 0 ) && (time_diff < (timeout *2)  )){
        
        old = timer->old;
        new = timer->new; 
        if(new < old){
            freq = (1000*( old - new)) / time_diff ;   
        } else if (new > old ){
            freq =  (1000*(0xffffffff - new + old ))/time_diff;       
        }
        
    }
    // if(freq == 0) {
    //     printk("diff %llx timeout %x , old %x new %x f=%d \n", time_diff, timeout, timer->old, timer->new, freq);
    // }    
    if(copy_to_user(data, &freq, sizeof(freq)))
        return -EFAULT;

    return sizeof(freq);
}

static long k230_hwtimer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    hw_timer_t *timer = container_of(file->private_data,
					      hw_timer_t, mdev);
    unsigned int timeout_val;
    int id;
	void __user *argp= (void __user *)arg;

    if (timer == NULL)
        return -ENOENT;
    if (arg < 0)
        return -EINVAL;

    if (copy_from_user(&timeout_val, argp, sizeof(timeout_val)))
    {
        return -EINVAL;
    }

    id = timer->index;
    timeout_val *= 1000;    //ms
    if (timeout_val > 0xffffffff)
    {
        printk("timeout value exceeds the maximum\n");
        return -EINVAL;
    }
    timeout = timeout_val;

    if (cmd == TMIOC_SET_TIMEOUT)
    {
        k230_hwtimer_start(timer, 0);
        timer->old = 0;
        timer->new = 0;
        mod_timer(&timer->softtimer, jiffies + SOFT_TIMEOUT);
        k230_hwtimer_start(timer, 1);
    }
    else
    {
        printk("cmd error\n");
        return -EINVAL;
    }

    return 0;
}

static const struct file_operations hwtimer_fops = {
	.owner = THIS_MODULE,
	.open = k230_hwtimer_open,
	.read = k230_hwtimer_read,
    .unlocked_ioctl = k230_hwtimer_ioctl,
	.release = k230_hwtimer_release,
};

static int k230_hwtimer_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;
    char name[8];
    hw_timer_t *k230_timer;
    struct miscdevice *misc;

    if (first_init)
    {
        addr = devm_platform_ioremap_resource(pdev, 0);
        if (IS_ERR(addr))
            return PTR_ERR(addr);
        /* k230 hwtimer init */
        k230_hwtimer_reg_init(addr);
        first_init = false;
    }

    k230_timer = devm_kzalloc(dev, sizeof(*k230_timer), GFP_KERNEL);
	if (!k230_timer)
		return -ENOMEM;

    k230_timer->base = addr;
    k230_timer->dev = dev;
    device_property_read_u32(dev, "index", &k230_timer->index);
    dev_set_drvdata(&pdev->dev, k230_timer);

    misc = &k230_timer->mdev;
    memset(misc, 0, sizeof(*misc));
    sprintf(name, "timer%d", k230_timer->index);
    misc->minor = MISC_DYNAMIC_MINOR;
    misc->name = name;
    misc->fops = &hwtimer_fops;
    misc->parent = dev;

	ret = misc_register(misc);
	if(ret < 0){
		printk("hwtimer register cdev failed!\n");
		return ret;
	}

    misc->this_device = dev;
    timer_setup(&k230_timer->softtimer, &softtimer_func, 0);

    return 0;
}

static const struct of_device_id k230_of_match[] = {
	{ .compatible = "canaan,k230-hwtimer", .data = (void *)0},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, k230_of_match);

static struct platform_driver k230_hwtimer_driver = {
	.driver		= {
		.name	= CANAAN_DRIVER_NAME,
		.of_match_table = k230_of_match,
	},
	.probe		= k230_hwtimer_probe,
};
module_platform_driver(k230_hwtimer_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Canaan K230 hwtimer driver");
MODULE_ALIAS("platform:" CANAAN_DRIVER_NAME);
