#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include "utest.h"

struct rt_semaphore sem;
struct rt_semaphore *psem;

struct rt_mutex mutex;
struct rt_mutex *pmutex;

struct rt_event evt;
struct rt_event *pevt;

struct rt_mailbox mbox;
struct rt_mailbox *pmbox;

struct rt_messagequeue mqueue;
struct rt_messagequeue *pmqueue;

static int all_thread_end = 2;

static void sndr_entry(void* parameter)
{
    rt_mutex_take(pmutex, RT_WAITING_FOREVER);
    rt_mutex_take(&mutex, RT_WAITING_FOREVER);

    rt_thread_mdelay(10);
    rt_sem_release(psem);
    rt_thread_mdelay(10);
    rt_sem_release(&sem);

    rt_thread_mdelay(10);
    rt_mutex_release(pmutex);
    rt_thread_mdelay(10);
    rt_mutex_release(&mutex);

    rt_thread_mdelay(10);
    rt_event_send(pevt, 1);
    rt_thread_mdelay(10);
    rt_event_send(&evt, 1);

    rt_thread_mdelay(10);
    rt_mb_send(pmbox, 0x01234567);
    rt_thread_mdelay(10);
    rt_mb_send(&mbox, 0x76543210);

    rt_thread_mdelay(10);
    rt_mq_send(pmqueue, "hello", sizeof("hello"));
    rt_thread_mdelay(10);
    rt_mq_send(&mqueue, "world", sizeof("world"));
    all_thread_end--;
}

char buff[10];
static void rcvr_entry(void* parameter)
{
    int ret;
    rt_ubase_t tmp;

    ret = rt_sem_take(psem, RT_WAITING_FOREVER);
    printf("rt_sem_take psem ok\n");
    uassert_int_equal(ret, RT_EOK);
    ret = rt_sem_take(&sem, RT_WAITING_FOREVER);
    printf("rt_sem_take &sem ok\n");
    uassert_int_equal(ret, RT_EOK);

    ret = rt_mutex_take(pmutex, RT_WAITING_FOREVER);
    printf("rt_mutex_take pmutex ok\n");
    uassert_int_equal(ret, RT_EOK);
    ret = rt_mutex_take(&mutex, RT_WAITING_FOREVER);
    printf("rt_mutex_take &mutex ok\n");
    uassert_int_equal(ret, RT_EOK);

    ret = rt_event_recv(pevt, 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, 0);
    printf("rt_event_recv pevt ok\n");
    uassert_int_equal(ret, RT_EOK);
    ret = rt_event_recv(&evt, 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, 0);
    printf("rt_event_recv &evt ok\n");
    uassert_int_equal(ret, RT_EOK);

    ret = rt_mb_recv(pmbox, &tmp, RT_WAITING_FOREVER);
    printf("rt_mb_recv pmbox ok, vaule = 0x%08x\n", (int)(size_t)tmp);
    uassert_int_equal(ret, RT_EOK);
    ret = rt_mb_recv(&mbox, &tmp, RT_WAITING_FOREVER);
    printf("rt_mb_recv &mbox ok, vaule = 0x%08x\n", (int)(size_t)tmp);
    uassert_int_equal(ret, RT_EOK);

    ret = rt_mq_recv(pmqueue, buff, 10, RT_WAITING_FOREVER);
    printf("rt_mq_recv pmqueue ok, info = %s\n", buff);
    uassert_int_equal(ret, RT_EOK);
    ret = rt_mq_recv(&mqueue, buff, 10, RT_WAITING_FOREVER);
    printf("rt_mq_recv &mqueue ok, info = %s\n", buff);
    uassert_int_equal(ret, RT_EOK);

    printf("delete psem\n");
    ret = rt_sem_delete(psem);
    uassert_int_equal(ret, RT_EOK);
    printf("detach &sem\n");
    ret = rt_sem_detach(&sem);
    uassert_int_equal(ret, RT_EOK);
    printf("delete pmutex\n");
    ret = rt_mutex_delete(pmutex);
    uassert_int_equal(ret, RT_EOK);
    printf("detach &mutex\n");
    ret = rt_mutex_detach(&mutex);
    uassert_int_equal(ret, RT_EOK);
    printf("delete pevt\n");
    ret = rt_event_delete(pevt);
    uassert_int_equal(ret, RT_EOK);
    printf("detach &evt\n");
    ret = rt_event_detach(&evt);
    uassert_int_equal(ret, RT_EOK);
    printf("delete pmbox\n");
    ret = rt_mb_delete(pmbox);
    uassert_int_equal(ret, RT_EOK);
    printf("detach &mbox\n");
    ret = rt_mb_detach(&mbox);
    uassert_int_equal(ret, RT_EOK);
    printf("delete pmqueue\n");
    ret = rt_mq_delete(pmqueue);
    uassert_int_equal(ret, RT_EOK);
    printf("detach &mqueue\n");
    ret = rt_mq_detach(&mqueue);
    uassert_int_equal(ret, RT_EOK);
    all_thread_end--;
}

static void test_user_ipc(void)
{
    rt_thread_t tid1, tid2;

    tid1 = rt_thread_create("sndr", sndr_entry, RT_NULL,
            4096, 5, 10);
    tid2 = rt_thread_create("rcvr", rcvr_entry, RT_NULL,
            4096, 5, 10);
    if (tid1 && tid2)
    {
        rt_thread_startup(tid1);
        rt_thread_startup(tid2);
    }
    while(all_thread_end > 0)
        rt_thread_mdelay(1);//Wait for the end of the thread
}

static rt_err_t utest_tc_init(void)
{
    psem = rt_sem_create("tsem", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&sem, "tsem", 0, RT_IPC_FLAG_FIFO);

    pmutex = rt_mutex_create("tmutex", RT_IPC_FLAG_FIFO);
    rt_mutex_init(&mutex, "tmutex", RT_IPC_FLAG_FIFO);

    pevt = rt_event_create("tevt", RT_IPC_FLAG_PRIO);
    rt_event_init(&evt, "tevt", RT_IPC_FLAG_PRIO);

    pmbox = rt_mb_create("mbox", 1, RT_IPC_FLAG_FIFO);
    rt_mb_init(&mbox, "mbox", RT_NULL, 1, RT_IPC_FLAG_FIFO);

    pmqueue = rt_mq_create("mqueue", 10, 2, RT_IPC_FLAG_FIFO);
    rt_mq_init(&mqueue, "mqueue", RT_NULL, 10, 25, RT_IPC_FLAG_FIFO);

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_user_ipc);
}

UTEST_TC_EXPORT(testcase, "utest.user_ipc.user_ipc_tc", utest_tc_init, utest_tc_cleanup, 500);
