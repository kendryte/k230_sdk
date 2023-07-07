/**
 * @file usage.c
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2022-11-18
 *
 * @copyright Copyright (c) 2022 Canaan Inc.
 *
 */
#include <rtthread.h>
#include <rthw.h>
#include <lwp.h>
#include "usage.h"

#define USAGE_ACC_MAX_COUNT     10
#define USAGE_CAL_PERIOD        1000         /*ms*/
#define MAX_USAGE_CAL_PERIOD    2000         /*ms*/
#define MIN_USAGE_CAL_PERIOD    100          /*ms*/
#define USAGE_RECORD_DEPTH      5
#define TIME_MAX_VALUE          0xFFFFFFFFFFFFFFFFUL
#define THREAD_NBR_MAX          100  // max the thread number
#define INVALID_USAGE           101

#ifdef RT_USING_SMP
    static struct rt_spinlock g_lock;
#else
    static rt_uint32_t g_lock;
#endif

static volatile uint64_t time_elapsed = 0;

static uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

typedef struct
{
    char        *lwp_name;
    char        *thread_name;   // thead name
    rt_thread_t thread;
    int         pid;
    int         tid;
    rt_ubase_t  time;
    rt_uint8_t  usage;  // thread usage percent 100%
}thread_usage_info;

static rt_ubase_t schedule_last_time;
static thread_usage_info thread_info[THREAD_NBR_MAX] = {0};
static rt_ubase_t total_time_last = 0;
static int thread_info_index = 0;
static rt_timer_t g_usage_timer = RT_NULL;
static int g_usage_period = 0;      /*ms*/
static rt_bool_t top_command_enable = RT_FALSE;

void init_cal_usage_time(void)
{
    schedule_last_time = get_ticks();
}

void thread_stats_scheduler_hook(struct rt_thread *from, struct rt_thread *to)
{
    volatile rt_ubase_t time;
    RT_ASSERT(schedule_last_time != 0);
    time = get_ticks();
    if(time > schedule_last_time) {
        from->user_data += (time - schedule_last_time);
    } else {
        rt_kprintf("%s %lx\n", __func__, schedule_last_time);
        from->user_data += (TIME_MAX_VALUE - schedule_last_time) + time;
    }
    schedule_last_time = time;
}

static void thread_stats_print(void)
{
    thread_usage_info *p_info;

    rt_kprintf("pid\tlwp\t\ttid\tthread\t\tusage\t\n");
    for(rt_ubase_t i = 0; i < thread_info_index; i++) {
        p_info = &thread_info[i];
        if(p_info->thread_name != RT_NULL) {
            rt_kprintf("%3d\t%-16s%3d\t%-16s",  p_info->pid, p_info->lwp_name, p_info->tid, p_info->thread_name);
            if(p_info->usage > 0) {
                rt_kprintf("%2u%%\n", p_info->usage);
            } else {
                rt_kprintf("<1%%\n", p_info->usage);
            }
        } else {
            continue;;
        }
    }
}

static void thread_cal_usage()
{

    volatile rt_ubase_t time, total_time;
    struct rt_list_node *node;
    struct rt_list_node *list;
    struct rt_thread *thread;
    struct rt_thread *cur_thread;

    rt_ubase_t i;
    rt_base_t level;

    level = rt_spin_lock_irqsave(&g_lock);
    time = get_ticks();

    if(time > total_time_last) {
        total_time = time - total_time_last;
    } else {
        rt_kprintf("total_time_last:%lx\n", total_time_last);
        total_time = (TIME_MAX_VALUE - total_time_last) + time;
    }

    cur_thread = rt_thread_self();
    cur_thread->user_data += time - schedule_last_time;

    list = &(rt_object_get_information(RT_Object_Class_Thread)->object_list);
    for(i = 0, node = list->next; (node != list) && i < THREAD_NBR_MAX; node = node->next, i++) {
        thread = rt_list_entry(node, struct rt_thread, list);
        thread_info[i].thread = thread;
        thread_info[i].thread_name = thread->name;
        thread_info[i].lwp_name = RT_NULL;
        thread_info[i].pid = 0;
#ifdef RT_USING_USERSPACE
        struct rt_lwp *lwp = thread->lwp;
        if(lwp != RT_NULL) {
            thread_info[i].lwp_name = lwp->cmd;
            thread_info[i].pid = lwp->pid;
        }
#endif
        thread_info[i].time = thread->user_data;
        thread_info[i].tid = thread->tid;
        thread->user_data = 0;
    }

    total_time_last = get_ticks();
    schedule_last_time = total_time_last;
    rt_spin_unlock_irqrestore(&g_lock, level);
    thread_info_index = i;
    total_time /= 100;
    if(total_time > 0) {
        for(rt_ubase_t j = i, i = 0; i < j; i++) {
            thread_info[i].usage = thread_info[i].time / total_time;
        }
    }
}

static void usage_cal_time_func(void *arg)
{
    thread_cal_usage();
    if(top_command_enable) {
        rt_kprintf("\e[1;1H\e[2J");
        thread_stats_print();
    }
}

rt_uint8_t sys_cpu_usage(rt_uint8_t cpu_id)
{
    thread_usage_info *p_info;
    char idle_thread_name[RT_NAME_MAX] = {0};
    rt_base_t level;
    sprintf(idle_thread_name, "tidle%d", cpu_id);
    level = rt_spin_lock_irqsave(&g_lock);
    for(rt_ubase_t i = 0; i < thread_info_index; i++) {
        p_info = &thread_info[i];
        if(!strcmp(p_info->thread_name, idle_thread_name)) {
            rt_spin_unlock_irqrestore(&g_lock, level);
            return 100 - p_info->usage;
        } else {
            continue;
        }
    }
    rt_spin_unlock_irqrestore(&g_lock, level);
    rt_kprintf("no idle thread :%s???\n", idle_thread_name);
    return INVALID_USAGE;
}

rt_uint8_t sys_thread_usage(rt_thread_t thread)
{
    thread_usage_info *p_info;
    rt_base_t level;
    RT_ASSERT(thread != RT_NULL)
    level = rt_spin_lock_irqsave(&g_lock);
    for(rt_ubase_t i = 0; i < thread_info_index; i++) {
        p_info = &thread_info[i];
        if(p_info->thread == thread) {
            rt_spin_unlock_irqrestore(&g_lock, level);
            return p_info->usage;
        } else {
            continue;;
        }
    }
    rt_spin_unlock_irqrestore(&g_lock, level);
    return INVALID_USAGE;
}

rt_uint8_t sys_process_usage(int pid)
{
    thread_usage_info *p_info;
    rt_uint8_t usage = 0;
    rt_base_t level;
    level = rt_spin_lock_irqsave(&g_lock);
    for(rt_ubase_t i = 0; i < thread_info_index; i++) {
        p_info = &thread_info[i];
        if(p_info->pid == pid) {
            usage += p_info->usage;
        }
    }
    rt_spin_unlock_irqrestore(&g_lock, level);
    RT_ASSERT(usage < INVALID_USAGE);
    return usage != 0 ? usage : INVALID_USAGE;
}

rt_err_t sys_set_usage_period(int mill_sec)
{
    rt_ubase_t tick = 0;
    rt_base_t level;

    if(g_usage_timer && (mill_sec <= MAX_USAGE_CAL_PERIOD) && (mill_sec >= MIN_USAGE_CAL_PERIOD)) {
        g_usage_period = mill_sec;
        tick = rt_tick_from_millisecond(mill_sec);
        rt_timer_control(g_usage_timer, RT_TIMER_CTRL_SET_TIME, &tick);
        level = rt_spin_lock_irqsave(&g_lock);
        for(rt_ubase_t i = 0; i < thread_info_index; i++) {
            rt_memset(&thread_info[i], 0, sizeof(thread_info[i]));
        }
        thread_info_index = 0;
        rt_spin_unlock_irqrestore(&g_lock, level);
        rt_kprintf("%s  mill_sec:%d success\n", __func__, mill_sec);
        return RT_EOK;
    }
    rt_kprintf("%s  mill_sec:%d failed\n", __func__, mill_sec);
    return RT_ERROR;
}

rt_int32_t cpu_usage_init(void)
{
    rt_scheduler_sethook(thread_stats_scheduler_hook);
    g_usage_period = USAGE_CAL_PERIOD;
    g_usage_timer = rt_timer_create("usage_timer", usage_cal_time_func, RT_NULL,
                rt_tick_from_millisecond(USAGE_CAL_PERIOD), RT_TIMER_FLAG_PERIODIC);
    RT_ASSERT(g_usage_timer != RT_NULL);
    total_time_last = get_ticks();
    rt_timer_start(g_usage_timer);
}
INIT_COMPONENT_EXPORT(cpu_usage_init);

int usage_show(int argc, char**argv)
{
    thread_stats_print();
}
MSH_CMD_EXPORT(usage_show, show all thread usage);

int top(void)
{
    top_command_enable = RT_TRUE;
}
MSH_CMD_EXPORT(top, show all thread usage at set intervals);

int top_exit(void)
{
    top_command_enable = RT_FALSE;
}
MSH_CMD_EXPORT(top_exit, stop show all thread usage at set intervals);

void process_usage(int argc, char**argv)
{
    int pid;
    rt_uint8_t usage;
    if(argc < 2) {
        rt_kprintf("please input process pid\n");
        return;
    }
    pid = atoi(argv[1]);
    usage  = sys_process_usage(pid);
    if(usage != INVALID_USAGE) {
        if(usage > 0) {
            rt_kprintf("process pid:%d usage:%d%%\n", pid, usage);
        } else {
            rt_kprintf("process pid:%d usage:<1%%\n", pid, usage);
        }
    } else {
        rt_kprintf("process pid:%d can not get usage\n", pid);
    }
}
MSH_CMD_EXPORT(process_usage, get process usage);

void sys_usage(void)
{
    rt_uint8_t usage;
    usage = sys_cpu_usage(0);
    if(usage > 0) {
        rt_kprintf("sys usage:%d%%\n", usage);
    } else {
        rt_kprintf("sys usage:<1%%\n", usage);
    }
}
MSH_CMD_EXPORT(sys_usage, get sys usage);

void set_usage_period(int argc, char**argv)
{
    rt_uint32_t mill_sec;
    if(argc < 2) {
        rt_kprintf("please input period(ms)<%d ~ %d>\n", MIN_USAGE_CAL_PERIOD, MAX_USAGE_CAL_PERIOD);
        return;
    }
    mill_sec = atoi(argv[1]);
    sys_set_usage_period(mill_sec);
}
MSH_CMD_EXPORT(set_usage_period, set the period of the system usage ms);

/**test code*/
#ifdef RT_USING_UTEST
#include "utest.h"

#define TEST_VAL_USAGE_FULL 0xFFF
#define THREAD_STACK_SIZE  2048
#define THREAD_TIMESLICE   10
#define THREAD_PRIORITY    18
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

static void usage_test_thread1(void *param)
{
    volatile rt_uint32_t val = TEST_VAL_USAGE_FULL;
    while (val != 0) {
        val--;
        if(val == 0) {
            rt_thread_mdelay(10);
            val = TEST_VAL_USAGE_FULL;
        }
    }
    return;
}

static void usage_test_thread2(void *param)
{
    volatile rt_uint32_t val = TEST_VAL_USAGE_FULL / 2;
    while (val != 0) {
        val--;
        if(val == 0) {
            rt_thread_mdelay(10);
            val = TEST_VAL_USAGE_FULL / 2;
        }
    }
    return;
}

void test_usage_create_thread1(void)
{
    rt_err_t ret_startup = -RT_ERROR;
    tid1 = rt_thread_create("thread1",
                            usage_test_thread1,
                            (void *)1,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE - 5);
    if (tid1 == RT_NULL)
    {
        uassert_false(tid1 == RT_NULL);
        goto __exit;
    }
    ret_startup = rt_thread_startup(tid1);
    if (ret_startup != RT_EOK)
    {
        uassert_false(ret_startup != RT_EOK);
        goto __exit;
    }
    rt_kprintf("%s ok\n", __func__);
    return;
__exit:
    if (tid1 != RT_NULL)
    {
        rt_thread_delete(tid1);
    }
    return;
}

void test_usage_create_thread2(void)
{
    rt_err_t ret_startup = -RT_ERROR;
    tid2 = rt_thread_create("thread2",
                            usage_test_thread2,
                            (void *)2,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE - 5);
    if (tid2 == RT_NULL)
    {
        uassert_false(tid2 == RT_NULL);
        goto __exit;
    }

    ret_startup = rt_thread_startup(tid2);
    if (ret_startup != RT_EOK)
    {
        uassert_false(ret_startup != RT_EOK);
        goto __exit;
    }
    rt_kprintf("%s ok\n", __func__);
    return;
__exit:
    if (tid2 != RT_NULL)
    {
        rt_thread_delete(tid2);
    }
    return;
}

static void test_usage_thread1_usage(void)
{
    rt_err_t ret;
    if(tid1 == RT_NULL)
    {
        uassert_false(tid1 == RT_NULL);
        return;
    }

    ret = sys_thread_usage(tid1);
    if(ret == INVALID_USAGE)
    {
        uassert_false(ret == INVALID_USAGE);
        return;
    }
    rt_kprintf("thread:%s tid:%d usage:%d%%\n", tid1->name, tid1->tid, ret);
}

static void test_usage_thread2_usage(void)
{
    rt_err_t ret;
    if(tid2 == RT_NULL)
    {
        uassert_false(tid2 == RT_NULL);
        return;
    }

    ret = sys_thread_usage(tid2);
    if(ret == INVALID_USAGE)
    {
        uassert_false(ret == INVALID_USAGE);
        return;
    }
    rt_kprintf("thread:%s tid:%d usage:%d%%\n", tid2->name, tid2->tid, ret);
}

static void test_usage_sys_usage(void)
{
    rt_err_t ret;
    ret = sys_cpu_usage(0);
    if(ret == INVALID_USAGE)
    {
        uassert_false(ret == INVALID_USAGE);
        return;
    }
    rt_kprintf("sys usage:%d%%\n", ret);
}

static void test_usage_set_period(void)
{
    rt_err_t ret;
    ret = sys_set_usage_period(MAX_USAGE_CAL_PERIOD);
    if(ret != RT_EOK)
    {
        uassert_false(ret != RT_EOK);
        return;
    }
    rt_kprintf("sys_set_usage_period ret:%d\n", ret);
}

static rt_err_t usage_test_init(void)
{
    return RT_EOK;
}

static rt_err_t usage_test_cleanup(void)
{
    if (tid1 != RT_NULL)
    {
        rt_thread_delete(tid1);
    }
    if (tid2 != RT_NULL)
    {
        rt_thread_delete(tid2);
    }
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_usage_create_thread1);
    UTEST_UNIT_RUN(test_usage_create_thread2);
    rt_thread_mdelay(2 * g_usage_period);
    UTEST_UNIT_RUN(test_usage_thread1_usage);
    UTEST_UNIT_RUN(test_usage_thread2_usage);
    UTEST_UNIT_RUN(test_usage_sys_usage);
    UTEST_UNIT_RUN(test_usage_set_period);
}

UTEST_TC_EXPORT(testcase, "testcases.kernel.usage", usage_test_init, usage_test_cleanup, 1000);
#endif
