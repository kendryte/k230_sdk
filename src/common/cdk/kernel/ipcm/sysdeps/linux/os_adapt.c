#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "ipcm_osadapt.h"
#include "device_config.h"

void pr(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
	return;
}

int __interrupt_context__(void)
{
	return in_interrupt();
}

void *__memcpy__(void *dest, const void *src, unsigned int n)
{
	return memcpy(dest, src, n);
}

void *__memset__(void *s, int c, unsigned int n)
{
	return memset(s, c, n);
}

unsigned int __strlen__(const char *s)
{
	return strlen(s);
}

void *__ipcm_mem_alloc__(int size)
{
	if (size > 0)
		return kmalloc(size, GFP_KERNEL);
	else
		return NULL;
}

void __ipcm_mem_free__(void *mem)
{
	kfree(mem);
	mem = NULL;
}

unsigned long __ipcm_irq_save__(void)
{
	unsigned long flags;
	local_irq_save(flags);
	return flags;
}

void __ipcm_irq_restore__(unsigned long flags)
{
	local_irq_restore((unsigned long)flags);
}

int __ipcm_lock_init__(struct ipcm_lock *ilock)
{
	spinlock_t *lock = kmalloc(sizeof(spinlock_t), GFP_KERNEL);
	if (NULL == lock) {
		printk(KERN_ERR "spinlock alloc failed\n");
		return -ENOMEM;
	}
	spin_lock_init(lock);
	ilock->os_lock = (long)lock;
	return 0;
}

int __ipcm_lock__(struct ipcm_lock *ilock)
{
	spin_lock((spinlock_t *)ilock->os_lock);

	return 0;
}

int __ipcm_unlock__(struct ipcm_lock *ilock)
{
	spin_unlock((spinlock_t *)ilock->os_lock);

	return 0;
}

void __ipcm_lock_free__(struct ipcm_lock *ilock)
{
	kfree((void *)ilock->os_lock);
	ilock->os_lock = 0;
}

void *__ipcm_io_mapping__(unsigned long addr, unsigned int sz)
{
	void *virt = ioremap_wc(addr, sz);
	return virt;
}

void __ipcm_io_unmapping__(void *addr)
{
	iounmap(addr);
}

void __ipcm_msleep__(unsigned int ms)
{
	msleep_interruptible(ms);
}

struct ipcm_task *__ipcm_thread_create__(char *name,
		int (*fn)(void *p),
		void *data)
{
	struct ipcm_task *ptask;
	struct task_struct *task;
	ptask = kmalloc(sizeof(struct ipcm_task), GFP_ATOMIC);
	if (NULL == ptask) {
		printk(KERN_ERR "task alloc failed\n");
		return NULL;
	}

	task = kthread_run(fn, data, name);
	if (IS_ERR(task)) {
		printk(KERN_ERR "create new task failed!\n");
		kfree(ptask);
		return NULL;
	}
	ptask->os_task = (long)task;

	return ptask;
}

int __ipcm_thread_check_stop__(struct ipcm_task *ptask)
{
	return kthread_should_stop();
}

void __ipcm_thread_destroy__(struct ipcm_task *ptask)
{
	struct task_struct *task;

	if (NULL == ptask)
		return;
	task = (struct task_struct *)ptask->os_task;
	kthread_stop(task);
	kfree(ptask);
	ptask = NULL;
}

static void (*tmp_callback)(unsigned long) = NULL;

static void time_callback(struct timer_list *timer) {
	if(tmp_callback)
		tmp_callback(0);
}

int __ipcm_timer_create__(struct ipcm_timer *itimer,
		void (*callback)(unsigned long),
		int ms,
		void *data)
{
	struct timer_list *timer;
	timer = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
	if (NULL == timer) {
		printk(KERN_ERR "timer alloc failed\n");
		return -ENOMEM;
	}
#if 0
	init_timer(timer);
	timer->function = callback;
	timer->data = (unsigned long)data;
#else
	timer_setup(timer, time_callback, 0);
	tmp_callback = callback;
#endif
	timer->expires = jiffies + msecs_to_jiffies(ms);
	itimer->os_timer = (long)timer;
	add_timer(timer);
    /* the third argument may include TIMER_* flags */
    /* ... */
	return 0;
}

void __ipcm_timer_free__(struct ipcm_timer *itimer)
{
	struct timer_list *timer = (struct timer_list *)itimer->os_timer;
	del_timer(timer);
	kfree(timer);
}

void __ipcm_timer_restart__(struct ipcm_timer *itimer, int ms)
{
	struct timer_list *timer = (struct timer_list *)itimer->os_timer;

	timer->expires = jiffies + msecs_to_jiffies(ms);
	add_timer(timer);
}

int __ipcm_event_init__(struct ipcm_event *ievent)
{
	wait_queue_head_t *wait;
	wait = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
	if (NULL == wait) {
		printk(KERN_ERR "alloc event failed\n");
		return -ENOMEM;
	}
	init_waitqueue_head(wait);
	ievent->os_wait = (long)wait;
	return 0;
}

void __ipcm_event_free__(struct ipcm_event *ievent)
{
	wait_queue_head_t *wait = (wait_queue_head_t *)ievent->os_wait;
	kfree(wait);
	ievent->os_wait = 0;
}

int __ipcm_wait_event__(struct ipcm_event *ievent)
{
	int ret = 0;
	wait_queue_head_t *wait = (wait_queue_head_t *)ievent->os_wait;

	if (!ievent->state) {
		DEFINE_WAIT(__wait);
		for (;;) {
			prepare_to_wait(wait, &__wait, TASK_INTERRUPTIBLE);
			if (ievent->state)
				break;
			if (!signal_pending(current)) {
				schedule();
				continue;
			}
			ret = -ERESTARTSYS;
			break;
		}
		finish_wait(wait, &__wait);
	}
	ievent->state = 0;
	return ret;
}

void __ipcm_wakeup_event__(struct ipcm_event *ievent)
{
	wait_queue_head_t *wait = (wait_queue_head_t *)ievent->os_wait;
	ievent->state = 1;
	wake_up(wait);
}

int __ipcm_proc_printf__(void *data, const char *f, ...)
{
	seq_printf((struct seq_file *)data, f);
	return 0;
}

int __ipcm_atomic_read__(ipcm_atomic_t *v)
{
	return atomic_read((atomic_t *)(v));
}

void __ipcm_atomic_set__(ipcm_atomic_t *v, int i)
{
	atomic_set((atomic_t *)(v), i);
}

void __ipcm_atomic_inc__(ipcm_atomic_t *v)
{
	atomic_inc((atomic_t *)(v));
}

void __ipcm_atomic_dec__(ipcm_atomic_t *v)
{
	atomic_dec((atomic_t *)(v));
}

