#include <rthw.h>
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ipcm_osadapt.h"

//#include "asm/hal_platform_ints.h"
#include <rtthread.h>
#include <ioremap.h>

void pr(const char *fmt, ...)
{
	rt_kprintf(fmt);
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

int __interrupt_context__(void)
{
	return 0;	/*判断是否处于中断上下文*/
}

void *__ipcm_mem_alloc__(int size)
{
	if (size > 0)
		return malloc(size);
	else
		return NULL;
}

void __ipcm_mem_free__(void *mem)
{
	free(mem);
	mem = NULL;
}

unsigned long __ipcm_irq_save__(void)
{
	unsigned long flags;
	flags =  rt_hw_interrupt_disable();
	return flags;
}

void __ipcm_irq_restore__(unsigned long flags)
{
	rt_hw_interrupt_enable((unsigned long)flags);
}

int __ipcm_lock_init__(struct ipcm_lock *ilock)
{
	int ret = 0;
#ifdef __IPCM_MUTEX_USED__

	ilock ->os_lock =  rt_mutex_create("ipcm_lock", RT_IPC_FLAG_PRIO);
	if(!ilock->os_lock)
	{
		rt_kprintf("mutex create failed\n");
		IPCM_BUG();
		return
	}
/*
	ret = LOS_MuxCreate(&ilock->os_lock);
	if (LOS_OK != ret) {
		rt_kprintf("mutex create failed\n");
		IPCM_BUG();
	}
*/
#endif
	return ret;
}

int __ipcm_lock__(struct ipcm_lock *ilock)
{
	int ret = 0;
#ifdef __IPCM_MUTEX_USED__
	ret = rt_mutex_take ((rt_mutex_t)ilock->os_lock, RT_WAITING_FOREVER);
	if (LOS_OK != ret) {
		rt_kprintf("mutex pend error!\n");
		IPCM_BUG();
	}
/*	ret = LOS_MuxPend(ilock->os_lock, LOS_WAIT_FOREVER);
	if (LOS_OK != ret) {
		rt_kprintf("mutex pend error!\n");
		IPCM_BUG();
	}
*/
#endif
	return ret;
}

int __ipcm_unlock__(struct ipcm_lock *ilock)
{
	int ret = 0;
#ifdef __IPCM_MUTEX_USED__
	ret = rt_mutex_release((rt_mutex_t)ilock->os_lock);
	//ret = LOS_MuxPost(ilock->os_lock);
	if (LOS_OK != ret) {
		rt_kprintf("mutex post error!\n");
		IPCM_BUG();
	}
#endif
	return ret;
}

void __ipcm_lock_free__(struct ipcm_lock *ilock)
{
#ifdef __IPCM_MUTEX_USED__
	int ret;
	ret = rt_mutex_delete((rt_mutex_t)ilock->os_lock);
	//ret = LOS_MuxDelete(ilock->os_lock);
	if (LOS_OK != ret) {
		rt_kprintf("mutex delete error!\n");
		IPCM_BUG();
	}
#endif
}

void *__ipcm_io_mapping__(unsigned long addr, unsigned int sz)
{
	return rt_ioremap_nocache((void *)addr, sz);
}

void __ipcm_io_unmapping__(void *addr)
{
	rt_iounmap(addr);
}

void __ipcm_msleep__(unsigned int ms)
{
	rt_thread_mdelay(ms);
}

#define IPCM_THREAD_STACK_SIZE 	4096
#define IPCM_THREAD_PRIORITY	5
#define IPCM_THREAD_TIMESLICE	5

struct ipcm_task *__ipcm_thread_create__(char *name,
		int (*fn)(void *p),
		void *data)
{
	rt_thread_t los_task = RT_NULL;
	struct ipcm_task *ptask = malloc(sizeof(struct ipcm_task));
	if (NULL == ptask) {
		rt_kprintf("task alloc failed\n");
		return NULL;
	}
	memset(ptask, 0, sizeof(struct ipcm_task));
	los_task = rt_thread_create(name, (void (*)(void *))fn, RT_NULL, IPCM_THREAD_STACK_SIZE,
				IPCM_THREAD_PRIORITY, IPCM_THREAD_TIMESLICE);
	if(!los_task) {
		free(ptask);
		return RT_NULL;
	}
	ptask->os_task = (long)los_task;
	rt_thread_startup(los_task);
	return ptask;
}

int __ipcm_thread_check_stop__(struct ipcm_task *ptask)
{
	int stop;

	if (NULL == ptask) {
		return 0;
	}
	stop = ptask->priv;
	if (1 == stop) {
		ptask->priv = 2;
	}
	return (1 == stop);
}

void __ipcm_thread_destroy__(struct ipcm_task *ptask)
{
	int stop;

	if (NULL == ptask) {
		return;
	}
	ptask->priv = 1;
	do {
		rt_thread_mdelay(10);
		stop = ptask->priv;
	} while (2 != stop);
	rt_thread_delete((rt_thread_t)ptask->os_task);
	free(ptask);
	ptask = NULL;
}

int __ipcm_timer_create__(struct ipcm_timer *itimer,
		void (*callback)(unsigned long),
		int ms,
		void *data)
{
	int ret;
	rt_timer_t ipcm_timer = NULL;
	rt_tick_t tick;
	tick = rt_tick_from_millisecond(ms);
	ipcm_timer = rt_timer_create("ipcm_timer", (void (*)(void *))callback, NULL, tick, RT_TIMER_FLAG_PERIODIC);
	if (!ipcm_timer) {
		rt_kprintf("timer create failed\n");
		return -1;
	}
	itimer->os_timer = (long)ipcm_timer;

	ret = rt_timer_start(ipcm_timer);
	if ( RT_EOK!= ret) {
		rt_kprintf("timer start error\n");
	}
	return ret;
}

void __ipcm_timer_free__(struct ipcm_timer *itimer)
{
	rt_timer_stop((rt_timer_t)(itimer->os_timer));
	rt_timer_delete((rt_timer_t)(itimer->os_timer));
}

void __ipcm_timer_restart__(struct ipcm_timer *itimer, int ms)
{
	/*
	 *	Since we are using a periodic timer, this function does not need to be implemented for now
	*/
}

int __ipcm_event_init__(struct ipcm_event *ievent)
{
	rt_event_t event = NULL;
	event = rt_event_create("ipcm_event", RT_IPC_FLAG_PRIO);
	if (NULL == event) {
		rt_kprintf("event create failed\n");
		return -ENOMEM;
	}
	ievent->os_wait = (long)event;
	return 0;
}

void __ipcm_event_free__(struct ipcm_event *ievent)
{
	rt_event_delete((rt_event_t)(ievent->os_wait));
	ievent->os_wait = 0;
}

int __ipcm_wait_event__(struct ipcm_event *ievent)
{
	rt_event_t event = (rt_event_t)(ievent->os_wait);
	rt_event_recv(event, 0x01, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
				RT_WAITING_FOREVER, NULL);
	return 0;
}

void __ipcm_wakeup_event__(struct ipcm_event *ievent)
{
	rt_event_t event = (rt_event_t)(ievent->os_wait);
	rt_event_send(event, 0x01);
}

#ifdef RT_USING_DFS_PROCFS
#include <stdarg.h>
int __ipcm_proc_printf__(void *data, const char *f, ...)
{
	va_list args;

	va_start(args, f);

	vprintf(f, args);

	va_end(args);;
}
#else
int __ipcm_proc_printf__(void *data, const char *f, ...)
{
	return 0;
}
#endif /* RT_USING_DFS_PROCFS */


int __ipcm_atomic_read__(ipcm_atomic_t *v)
{
	return (*((volatile typeof(v))(v)));
}

void __ipcm_atomic_set__(ipcm_atomic_t *v, int i)
{
	//FIXME:
	//LOS_AtomicXchg32bits((volatile uint32_t *)v, i);
	*(volatile uint32_t *)v = i;
}

void __ipcm_atomic_inc__(ipcm_atomic_t *v)
{
	//FIXME:
	//LOS_AtomicInc((volatile INT32 *)(v));
	(*(volatile uint32_t *)v)++;
}

void __ipcm_atomic_dec__(ipcm_atomic_t *v)
{
	//FIXME:
	//LOS_AtomicDec((volatile INT32 *)(v));
	(*(volatile uint32_t *)v)--;
}

