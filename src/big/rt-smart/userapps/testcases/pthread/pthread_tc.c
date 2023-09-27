#include <stdio.h>
#include <rtthread.h>
#include <pthread.h>


#define CLEANUP_NOTCALLED 0
#define CLEANUP_CALLED 1
#define INTHREAD 0
#define INMAIN 1
/*
pthread_attr_getscope 支持获取当前配置。 预期完成时间：9月30日
pthread_attr_getstackaddr 支持获取栈信息。 预期完成时间：9月30日
pthread_attr_setstackaddr 不支持，线程内部栈地址自动分配，栈大小可以自动扩充，建议应用代码中屏蔽这部分。
pthread_attr_setschedparam 支持设置优先级。 预期完成时间：10月15日
pthread_setschedparam 支持设置优先级。 预期完成时间：10月15日
pthread_getschedparam 支持获取当前配置。 预期完成时间：9月30日
pthread_attr_setschedpolicy 仅支持按优先级调度。 预期完成时间：10月15日
pthread_cancel 支持取消线程，并可以设置cleanup函数。预期完成时间：10月15日
pthread_setcanceltype 不支持
pthread_cond_broadcast 新加支持，预期完成时间：11月15日
pthread_cond_timedwait 新加支持，预期完成时间：11月15日
pthread_mutex_setprioceiling 不支持
pthread_mutexattr_getprioceiling 不支持
sem_unlink 
*/


void* thread_task_cancel(void* arg)
{
    int rc = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (rc != 0) {
		printf("pthread_setcancelstate error rc = %d\n", rc);
	}
    rc = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (rc != 0) {
		printf("pthread_setcanceltype error rc = %d\n", rc);
	}
    while(1)
    {
        rt_thread_mdelay(100);
    }
    return NULL;
}

int test_pthread_cancel(void)
{
    pthread_t thread;
    void * mess;
    pthread_create(&thread,NULL,thread_task_cancel,NULL);
    rt_thread_mdelay(500);
    int res = pthread_cancel(thread);
    if (res != 0) {
        printf("pthread cancel fail\n");
        return 0;
    }
    res = pthread_join(thread, &mess);
    if (res != 0) {
        printf("pthread join fail\n");
        return 0;
    }
    if (mess == PTHREAD_CANCELED) {
        printf("thread canceled\n");
        return 0;
    }
    printf("thread cancel mess error\n");
    return 0;
}



static void * thread_task_schedparam(void* arg)
{
	struct sched_param sparam;
	int policy,priority;
	int rc;
	policy = SCHED_FIFO;
	priority = sched_get_priority_min(policy);
	sparam.sched_priority = priority;

    rc = pthread_setschedparam(pthread_self(), policy, &sparam);
	if (rc != 0) {
		printf("pthread_setschedparam error rc = %d\n", rc);
	}

	rc = pthread_getschedparam(pthread_self(), &policy, &sparam);
	if (rc != 0) {
		printf("pthread_getschedparam error rc = %d\n", rc);
	}
	return NULL;
}

int test_schedparam(void)
{
	pthread_t thread;
	int rc;

	rc = pthread_create(&thread, NULL, thread_task_schedparam, NULL);
	if (rc) {
		printf("pthread_create error rc = %d\n", rc);
        return 0;
	}

	pthread_join(thread, NULL);
	printf("test_schedparam Test PASSED\n");
	return 0;
}

void test_pthread_attr_destroy(void)
{
    pthread_attr_t new_attr;
    if (pthread_attr_init(&new_attr) != 0) {
		printf("test_pthread_attr_destroy / Cannot initialize attribute object\n");
        return;
	}

	/* Destroy attribute */
	if (pthread_attr_destroy(&new_attr) != 0) {
		printf("test_pthread_attr_destroy / Cannot destroy the attribute object\n");
        return;
	}
    printf("pthread_attr_destroy sucess");
}

void test_pthread_attr_inheritsched(void)
{
    int rc = 0;
	pthread_attr_t attr;
    int inheritsched;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf("pthread_attr_init error rc = %d\n", rc);
        return;
	}

	rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rc != 0) {
		printf("pthread_attr_setinheritsched error rc = %d\n", rc);
        return;
	}
	rc = pthread_attr_getinheritsched(&attr, &inheritsched);
	if (rc != 0) {
		printf("pthread_attr_getinheritsched error rc = %d\n", rc);
        return;
	}
	if (inheritsched != PTHREAD_EXPLICIT_SCHED) {
        printf("got wrong inheritsched param");
        return;
	}

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		printf("pthread_attr_destroy error rc = %d\n", rc);
		return;
	}
	printf("test_pthread_attr_inheritsched PASSED\n");
}

void test_pthread_attr_stacksize(void)
{
    pthread_attr_t attr;
	size_t stack_size;
	size_t ssize;
	// void *saddr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf("pthread_attr_init error rc = %d\n", rc);
		return;
	}

	/* Get the default stack_addr and stack_size value */
	rc = pthread_attr_getstacksize(&attr, &stack_size);
	if (rc != 0) {
		printf("pthread_attr_getstacksize error rc = %d\n", rc);
		return;
	}
	/* printf("stack_size = %lu\n", stack_size); */

	stack_size = 1024;

	// if (posix_memalign(&saddr, sysconf(_SC_PAGE_SIZE), stack_size) != 0) {
	// 	printf("out of memory while "
	// 	       "allocating the stack memory");
	// 	return;
	// }
	/* printf("stack_size = %lu\n", stack_size); */

	rc = pthread_attr_getstacksize(&attr, &ssize);
	if (rc != 0) {
		printf("pthread_attr_getstacksize error rc = %d\n", rc);
		return;
	}
	/* printf("ssize = %lu\n", ssize); */

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		printf("pthread_attr_destroy error rc = %d\n", rc);
		return;
	}

	printf("test_pthread_attr_stacksize PASSED\n");
}

static void *thread_scope_func()
{
	pthread_exit(0);
	return NULL;
}

void test_pthread_attr_scope(void)
{
    pthread_t new_th;
	pthread_attr_t attr;
	int cscope;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf("pthread_attr_init error");
		return;
	}

	rc = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	if (rc != 0) {
		printf("PTHREAD_SCOPE_SYSTEM is not supported");
		return;
	}

	rc = pthread_create(&new_th, &attr, thread_scope_func, NULL);
	if (rc != 0) {
		printf("pthread_create error");
		return;
	}

	rc = pthread_attr_getscope(&attr, &cscope);
	if (rc != 0) {
		printf("pthread_attr_getscope error");
		return;
	}

	if (cscope != PTHREAD_SCOPE_SYSTEM) {
		printf("The contentionscope is not correct \n");
		return;
	}

	rc = pthread_join(new_th, NULL);
	if (rc != 0) {
		printf("pthread_join error");
		return;
	}

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		printf("pthread_attr_destroy error");
		return;
	}

	printf("test_pthread_attr_scope PASSED\n");
}


static int cleanup_sem1;			/* Manual semaphore */
static int cleanup_flag;

static void a_cleanup_func(void *flag_val)
{
	cleanup_flag = (long)flag_val;
	return;
}

/* Function that the thread executes upon its creation */
static void *cleanup_thread_func()
{
	pthread_cleanup_push(a_cleanup_func, (void *)CLEANUP_CALLED);
	pthread_cleanup_pop(1);

	/* Tell main that the thread has called the pop function */
	cleanup_sem1 = INMAIN;

	/* Wait for main to say it's ok to continue (as it checks to make sure that
	 * the cleanup handler was called */
	while (cleanup_sem1 == INMAIN)
		rt_thread_mdelay(500);

	pthread_exit(0);
	return NULL;
}

void test_pthread_cleanup()
{
    pthread_t new_th;

	/* Initializing values */
	cleanup_sem1 = INTHREAD;
	cleanup_flag = CLEANUP_NOTCALLED;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, cleanup_thread_func, NULL) != 0) {
		printf("Error creating thread\n");
		return;
	}

	/* Wait for thread to call the pthread_cleanup_pop */
	while (cleanup_sem1 == INTHREAD)
		rt_thread_mdelay(500);

	/* Check to verify that the cleanup handler was called */
	if (cleanup_flag != CLEANUP_CALLED) {
		printf("Test FAILED: Cleanup handler was not called\n");
		return;
	}

	/* Tell thread it can keep going now */
	cleanup_sem1 = INTHREAD;

	/* Wait for thread to end execution */
	if (pthread_join(new_th, NULL) != 0) {
		printf("Error in pthread_join()\n");
		return;
	}

	printf("test_pthread_cleanup PASSED\n");
}



void test_pthread_cond_init_destroy()
{
    pthread_cond_t cond1, cond2;
    pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
    pthread_condattr_t condattr;
	int rc;

	/* Initialize a condition variable attribute object */
	if ((rc = pthread_condattr_init(&condattr)) != 0) {
		printf("Error at pthread_condattr_init(), rc=%d\n",rc);
		return;
	}

	/* Initialize cond1 with the default condition variable attribute */
	if ((rc = pthread_cond_init(&cond1, &condattr)) != 0) {
		printf("Fail to initialize cond1, rc=%d\n", rc);
		return;
	}

	/* Initialize cond2 with NULL attributes */
	if ((rc = pthread_cond_init(&cond2, NULL)) != 0) {
		printf("Fail to initialize cond2, rc=%d\n", rc);
		return;
	}

	/* Destroy the condition variable attribute object */
	if ((rc = pthread_condattr_destroy(&condattr)) != 0) {
		printf("Error at pthread_condattr_destroy(), rc=%d\n",
			rc);
		return;
	}

	/* Destroy cond1 */
	if ((rc = pthread_cond_destroy(&cond1)) != 0) {
		printf("Fail to destroy cond1, rc=%d\n", rc);
		printf("Test FAILED\n");
		return;
	}

	/* Destroy cond2 */
	if ((rc = pthread_cond_destroy(&cond2)) != 0) {
		printf("Fail to destroy cond2, rc=%d\n", rc);
		printf("Test FAILED\n");
		return;
	}

	/* Destroy cond3 */
	if ((rc = pthread_cond_destroy(&cond3)) != 0) {
		printf("Fail to destroy cond3, rc=%d\n", rc);
		printf("Test FAILED\n");
		return;
	}

	printf("test_pthread_cond_init_destroy PASSED\n");
}


#define THREAD_NUM  3

static struct signal_testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} signal_td;

static pthread_t thread[THREAD_NUM];

static int signal_start_num = 0;
static int signal_waken_num = 0;
static void alarm_handler(int signo)
{
	int i;
	printf("Error: failed to wakeup all threads\n");
	for (i = 0; i < THREAD_NUM; i++) {	/* cancel threads */
		pthread_cancel(thread[i]);
	}

	return;
}
static void *signal_thr_func(void *arg)
{
	int rc;
	pthread_t self = pthread_self();
	if (pthread_mutex_lock(&signal_td.mutex) != 0) {
		printf("[Thread 0x%p] failed to acquire the mutex\n",
			(void *)self);
		return NULL;
	}
	signal_start_num++;
	printf("[Thread 0x%p] started and locked the mutex\n",
		(void *)self);

	printf("[Thread 0x%p] is waiting for the cond\n",
		(void *)self);
	rc = pthread_cond_wait(&signal_td.cond, &signal_td.mutex);
	if (rc != 0) {
		printf("pthread_cond_wait return %d\n", rc);
		return NULL;
	}
	signal_waken_num++;
	printf("[Thread 0x%p] was wakened and acquired the mutex again\n",
		(void *)self);

	if (pthread_mutex_unlock(&signal_td.mutex) != 0) {
		printf("[Thread 0x%p] failed to release the mutex\n",
			(void *)self);
		return NULL;
	}
	fprintf(stderr, "[Thread 0x%p] released the mutex\n", (void *)self);
	return NULL;
}
void test_pthread_cond_signal()
{
    struct timespec completion_wait_ts = {0, 100000};
	int i, rc;
	struct sigaction act;
	if (pthread_mutex_init(&signal_td.mutex, NULL) != 0) {
		printf("Fail to initialize mutex\n");
		return;
	}
	if (pthread_cond_init(&signal_td.cond, NULL) != 0) {
		printf("Fail to initialize cond\n");
		return;
	}

	for (i = 0; i < THREAD_NUM; i++) {	/* create THREAD_NUM threads */
		if (pthread_create(&thread[i], NULL, signal_thr_func, NULL) != 0) {
			printf("Fail to create thread[%d]\n", i);
			return;
		}
	}
	while (signal_start_num < THREAD_NUM)	/* waiting for all threads started */
		nanosleep(&completion_wait_ts, NULL);

	/* Acquire the mutex to make sure that all waiters are currently
	   blocked on pthread_cond_wait */
	if (pthread_mutex_lock(&signal_td.mutex) != 0) {
		printf("Main: Fail to acquire mutex\n");
		return;
	}
	if (pthread_mutex_unlock(&signal_td.mutex) != 0) {
		printf("Main: Fail to release mutex\n");
		return;
	}

	/* signal once and check if at least one waiter is wakened */
	printf("[Main thread] signals a condition\n");
	rc = pthread_cond_signal(&signal_td.cond);
	if (rc != 0) {
		printf("[Main thread] failed to signal the condition\n");
		return;
	}
	rt_thread_mdelay(500);
	if (signal_waken_num <= 0) {
		printf("[Main thread] but no waiters were wakened\n");
		printf("Test FAILED\n");
		/* Cancel the threads */
		for (i = 0; i < THREAD_NUM; i++) {	/* cancel threads */
			pthread_cancel(thread[i]);
		}
		return;
	}
	printf("[Main thread] %d waiters were wakened\n", signal_waken_num);

	/* Setup alarm handler */
	act.sa_handler = alarm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	//alarm(5);

	/* loop to wake up the rest threads */
	for (i = 1; i < THREAD_NUM; i++) {
		printf("[Main thread] signals to wake up the next thread\n");
		if (pthread_cond_signal(&signal_td.cond) != 0) {
			printf("Main failed to signal the condition\n");
			return;
		}
		// nanosleep(&completion_wait_ts, NULL);
	}

	/* join all secondary threads */
	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			printf("Fail to join thread[%d]\n", i);
			return;
		}
	}
	printf("test_pthread_cond_signal PASSED\n");
}


void test_pthread_condattr_clock()
{
    pthread_condattr_t condattr;
	clockid_t clockid;
	int rc;

	/* Initialize a cond attributes object */
	if ((rc = pthread_condattr_init(&condattr)) != 0) {
		printf("Error at pthread_condattr_init(), rc=%d\n",
			rc);
		return;
	}
    rc = pthread_condattr_setclock(&condattr, CLOCK_REALTIME);
	if (rc != 0) {
		printf("Test FAILED: Could not set clock to CLOCK_REALTIME\n");
		return ;
	}

	rc = pthread_condattr_getclock(&condattr, &clockid);
	if (rc != 0) {
		printf("Test FAILED: Could not get the clock attribute\n");
		return;
	}

	printf("test_pthread_condattr_clock PASSED\n");
}


static void *thread_detach_func()
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* If the thread wasn't canceled in 10 seconds, time out */
	rt_thread_mdelay(10000);

	printf("Thread couldn't be canceled (at cleanup time), timing out\n");
	pthread_exit(0);
	return NULL;
}
void test_pthread_detach()
{
    pthread_attr_t new_attr;
	pthread_t new_th;
	int ret;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0) {
		printf("Cannot initialize attribute object\n");
		return;
	}

	/* Set the attribute object to be joinable */
	if (pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_JOINABLE) !=
	    0) {
		printf("Error in pthread_attr_setdetachstate()\n");
		return;
	}

	/* Create the thread */
	if (pthread_create(&new_th, &new_attr, thread_detach_func, NULL) != 0) {
		printf("Error creating thread\n");
		return;
	}

    if (pthread_equal(new_th, new_th) == 0) {
		printf("pthread_equal FAILED\n");
		return;
	}

	/* Detach the thread. */
	if (pthread_detach(new_th) != 0) {
		printf("Error detaching thread\n");
		return;
	}

	/* Now try and join it.  This should fail. */
	ret = pthread_join(new_th, NULL);

	/* Cleanup: Cancel the thread */
	pthread_cancel(new_th);

	if (ret == 0) {
		printf("Test FAILED\n");
		return;
	} else if (ret == EINVAL) {
		printf("Test PASSED\n");
		return;
	} else {
		printf("Error in pthread_join\n");
		return;
	}
}


void test_pthread_key()
{
#define NUM_OF_KEYS 10
#define KEY_VALUE 0
    pthread_key_t keys[NUM_OF_KEYS];
	int i;
	void *rc;

	for (i = 0; i < NUM_OF_KEYS; i++) {
		if (pthread_key_create(&keys[i], NULL) != 0) {
			printf("Error: pthread_key_create() failed\n");
			return;
		} else {
			if (pthread_setspecific
			    (keys[i], (void *)(long)(i + KEY_VALUE)) != 0) {
				printf("Error: pthread_setspecific() failed\n");
				return;
			}

		}
	}

	for (i = 0; i < NUM_OF_KEYS; ++i) {
		rc = pthread_getspecific(keys[i]);
		if (rc != (void *)(long)(i + KEY_VALUE)) {
			printf("Test FAILED: Did not return correct value of thread-specific key, expected %d, but got %ld\n",
			     (i + KEY_VALUE), (long)rc);
			return;
		} else {
			if (pthread_key_delete(keys[i]) != 0) {
				printf("Error: pthread_key_delete() failed\n");
				return;
			}
		}
	}
    printf("test_pthread_getspecific PASSED\n");
	printf("test_pthread_key PASSED\n");
}


void test_pthread_mutex()
{
    pthread_mutex_t mutex1, mutex2;
    pthread_mutexattr_t mta;
	int rc;

	/* Initialize a mutex attributes object */
	if ((rc = pthread_mutexattr_init(&mta)) != 0) {
		printf("Error at pthread_mutexattr_init(), rc=%d\n", rc);
		return;
	}

	/* Initialize mutex1 with the default mutex attributes */
	if ((rc = pthread_mutex_init(&mutex1, &mta)) != 0) {
		printf("Fail to initialize mutex1, rc=%d\n", rc);
		return;
	}

	/* Initialize mutex2 with NULL attributes */
	if ((rc = pthread_mutex_init(&mutex2, NULL)) != 0) {
		printf("Fail to initialize mutex2, rc=%d\n", rc);
		return;
	}

	/* Destroy the mutex attributes object */
	if ((rc = pthread_mutexattr_destroy(&mta)) != 0) {
		printf("Error at pthread_mutexattr_destroy(), rc=%d\n", rc);
		return;
	}

	/* Destroy mutex1 */
	if ((rc = pthread_mutex_destroy(&mutex1)) != 0) {
		printf("Fail to destroy mutex1, rc=%d\n", rc);
		printf("Test FAILED\n");
		return;
	}

	/* Destroy mutex2 */
	if ((rc = pthread_mutex_destroy(&mutex2)) != 0) {
		printf("Fail to destroy mutex2, rc=%d\n", rc);
		printf("Test FAILED\n");
		return;
	}

	printf("test_pthread_mutex PASSED\n");
}

static pthread_mutex_t trylock_mutex = PTHREAD_MUTEX_INITIALIZER;
static int trylock_start = 0;
static int trylock_pause = 1;
static void *mutex_trylock_func(void *parm)
{
	int rc;

	if ((rc = pthread_mutex_lock(&trylock_mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_lock(), rc=%d\n", rc);
		pthread_exit(NULL);
	}
	trylock_start = 1;

	while (trylock_pause){
        rt_thread_mdelay(100);
    }
		

	if ((rc = pthread_mutex_unlock(&trylock_mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_unlock(), rc=%d\n", rc);
		pthread_exit(NULL);
	}

	pthread_exit(0);
	return (void *)(0);
}
void test_pthread_mutex_trylock()
{
    int i, rc;
	pthread_t t1;

	/* Create a secondary thread and wait until it has locked the mutex */
	pthread_create(&t1, NULL, mutex_trylock_func, NULL);
    while (!trylock_start)
    {
        rt_thread_mdelay(1000);
    }
    

	/* Trylock the mutex and expect it returns EBUSY */
	rc = pthread_mutex_trylock(&trylock_mutex);
	if (rc != EBUSY) {
		printf("Expected %d(EBUSY), got %d\n", EBUSY, rc);
		printf("Test FAILED\n");
		return;
	}

	/* Allow the secondary thread to go ahead */
	trylock_pause = 0;

	/* Trylock the mutex for N times */
	for (i = 0; i < 5; i++) {
		rc = pthread_mutex_trylock(&trylock_mutex);
		if (rc == 0) {
			pthread_mutex_unlock(&trylock_mutex);
			break;
		} else if (rc == EBUSY) {
			rt_thread_mdelay(1000);
			continue;
		} else {
			printf("Unexpected error code(%d) for pthread_mutex_lock()\n",
				rc);
			return;
		}
	}

	/* Clean up */
	pthread_join(t1, NULL);
	pthread_mutex_destroy(&trylock_mutex);

	if (i >= 5) {
		printf("Have tried %d times but failed to get the mutex\n", i);
		return;
	}
	printf("test_pthread_mutex_trylock PASSED\n");
}



void test_pthread_mutexattr_protocol()
{
    pthread_mutexattr_t mta;
	int protocol, rc;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		printf("Error at pthread_mutexattr_init()\n");
		return;
	}

    if (pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_INHERIT) != 0) {
			printf("Error setting protocol to %d\n", PTHREAD_PRIO_INHERIT);
			return;
	}
	/* Get the protocol mutex attr. */
	if ((rc = pthread_mutexattr_getprotocol(&mta, &protocol)) != 0) {
		printf("Test FAILED: Error in pthread_mutexattr_getprotocol rc=%d\n",rc);
		return;
	}

	printf("pthread_mutexattr_protocol PASSED\n");
}



void test_pthread_mutexattr_type()
{
    pthread_mutexattr_t mta;
	int type;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		printf("Error at pthread_mutexattr_init()\n");
		return;
	}

    if (pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL) != 0) {
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return;
	}

	/* The default 'type' attribute should be PTHREAD_MUTEX_DEFAULT  */
	if (pthread_mutexattr_gettype(&mta, &type) != 0) {
		printf("pthread_mutexattr_gettype(): Error obtaining the attribute 'type'\n");
		return;
	}

	if (type != PTHREAD_MUTEX_DEFAULT) {
		printf("Test FAILED: Incorrect default mutexattr 'type' value: %d\n",
		     type);
		return;
	}

	printf("test_pthread_mutexattr_type PASSED\n");
}



static int init_flag;
static void once_init_func(void)
{
	init_flag++;
}
void test_pthread_once(void)
{
	int ret;

	pthread_once_t once_control = PTHREAD_ONCE_INIT;

	init_flag = 0;

	ret = pthread_once(&once_control, once_init_func);
	if (ret != 0) {
		printf("pthread_once failed\n");
		return;
	}

	ret = pthread_once(&once_control, once_init_func);
	if (ret != 0) {
		printf("pthread_once failed\n");
		return;
	}

	if (init_flag != 1) {
		printf("Test FAILED\n");
		return;
	}

	printf("Test PASSED\n");

}



static pthread_t self_th2;
static void *a_thread_func()
{
	self_th2 = pthread_self();
	pthread_exit(0);
	return NULL;
}
void test_pthread_self()
{
    pthread_t new_th1;

	/* Create a new thread. */
	if (pthread_create(&new_th1, NULL, a_thread_func, NULL) != 0) {
		printf("Error creating thread\n");
		return;
	}

	/* Wait for thread to return */
	if (pthread_join(new_th1, NULL) != 0) {
		printf("Error in pthread_join()\n");
		return;
	}

	/* Call pthread_equal() and pass to it the new thread ID in both
	 * parameters.  It should return a non-zero value, indicating that
	 * both thread IDs are equal, and therefore refer to the same
	 * thread. */
	if (pthread_equal(new_th1, self_th2) == 0) {
		printf("Test FAILED\n");
		return;
	}
	printf("test_pthread_self PASSED\n");
}



static int testcancel_sem1;			/* Manual semaphore */
static int testcancel_cleanup_flag;		/* Flag to indicate the thread's cleanup handler was called */
static pthread_mutex_t testcancel_mutex = PTHREAD_MUTEX_INITIALIZER;	/* Mutex */

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled. */
static void testcancel_cleanup_func()
{
	testcancel_cleanup_flag = -1;
	return;
}

/* Function that the thread executes upon its creation */
static void *testcancel_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	pthread_cleanup_push(testcancel_cleanup_func, NULL);

	/* Indicate to main() that the thread has been created. */
	testcancel_sem1 = INMAIN;

	/* Lock the mutex. It should have already been locked in main, so the thread
	 * should block. */
	if (pthread_mutex_lock(&testcancel_mutex) != 0) {
		printf("Error in pthread_mutex_lock()\n");
		pthread_exit(NULL);
		return NULL;
	}

	/* Should get here if the cancel request was deffered. */
	pthread_cleanup_pop(0);
	testcancel_cleanup_flag = 1;

	/* Cancelation point.  Cancel request should not be honored here. */
	pthread_testcancel();

	/* Should not get here if the cancel request was honored at the cancelation point
	 * pthread_testcancel(). */
	testcancel_cleanup_flag = -2;
	pthread_exit(0);
	return NULL;
}

void test_pthread_testcancel(void)
{
	pthread_t new_th;

	/* Initializing values */
	testcancel_sem1 = INTHREAD;
	testcancel_cleanup_flag = 0;

	/* Lock the mutex */
	if (pthread_mutex_lock(&testcancel_mutex) != 0) {
		printf("Error in pthread_mutex_lock()\n");
		return;
	}

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, testcancel_thread_func, NULL) != 0) {
		printf("Error creating thread\n");
		return;
	}

	/* Make sure thread is created before we cancel it. (wait for
	 * a_thread_func() to set sem1=INMAIN.) */
	while (testcancel_sem1 == INTHREAD)
		rt_thread_mdelay(500);

	/* Send cancel request to the thread.  */
	if (pthread_cancel(new_th) != 0) {
		printf("Test FAILED: Couldn't cancel thread\n");
		return;
	}

	/* Cancel request has been sent, unlock the mutex */
	if (pthread_mutex_unlock(&testcancel_mutex) != 0) {
		printf("Error in pthread_mutex_unlock()\n");
		return;
	}

	/* Wait 'till the thread has been canceled or has ended execution. */
	if (pthread_join(new_th, NULL) != 0) {
		printf("Error in pthread_join()\n");
		return;
	}

	/* This means that the cleanup function wasn't called, so the cancel
	 * request was not honord immediately like it should have been. */
	if (cleanup_flag == -1) {
		printf("Cancel request was not deferred.\n");
		return;
	}

	if (cleanup_flag == -2) {
		printf("Test FAILED:pthread_testcancel() not treated as a cancelation point.\n");
		return;
	}

	printf("test_pthread_testcancel PASSED\n");
	return;
}








#define HANDLER_NOTCALLED 0
#define HANDLER_CALLED 1

static int prep_val;
static int parent_val;
static int child_val;

static void prepare_handler()
{
	prep_val = HANDLER_CALLED;
	return;
}

static void parent_handler()
{
	parent_val = HANDLER_CALLED;
	return;
}

static void child_handler()
{
	child_val = HANDLER_CALLED;
	return;
}

void test_pthread_atfork(void)
{
	// pid_t pid;

	/* Initialize values */
	prep_val = HANDLER_NOTCALLED;
	parent_val = HANDLER_NOTCALLED;
	child_val = HANDLER_NOTCALLED;

	/* Set up the fork handlers */
	if (pthread_atfork(prepare_handler, parent_handler, child_handler) != 0) {
		printf("Error in pthread_atfork\n");
		return;
	}

	/* Now call fork() */
	// pid = fork();

	// if (pid < 0) {
	// 	printf("Error in fork()\n");
	// 	return;
	// }
	// if (pid == 0) {
	// 	/* Child process */
	// 	pthread_exit(0);
	// } else {
	// 	/* Parent process */
	// 	wait(NULL);
	// }

	/* Check if fork handlers were called */
	if (prep_val == 1) {
		if (parent_val == 1) {
			if (parent_val == 1) {
				printf("test_pthread_atfork PASSED\n");
				return;
			} else {
				printf("Test FAILED: child handler not called\n");
				return;
			}
		} else {
			printf("Test FAILED: parent handler not called\n");
			return;
		}
	} else {
		printf("Test FAILED: prepare handler not called\n");
		return;
	}

	/* Should not reach here */
	printf("Error: control should not reach here\n");
	return;
}





void test_pthread_condattr_pshared()
{
    
	/* Make sure there is process-shared capability. */
#ifndef PTHREAD_PROCESS_SHARED
	printf("process-shared attribute is not available for testing\n");
	return;
#endif

	pthread_condattr_t attr[10];
	int ret, i, pshared;

	for (i = 0; i < 10; i++) {
		/* Initialize a cond attributes object */
		if (pthread_condattr_init(&attr[i]) != 0) {
			printf("Error at pthread_condattr_init()\n");
			return;
		}

		/* Set 'pshared' to PTHREAD_PROCESS_PRIVATE. */
		ret =
		    pthread_condattr_setpshared(&attr[i],
						PTHREAD_PROCESS_PRIVATE);
		if (ret != 0) {
			printf("Error in pthread_condattr_setpshared(), error: %d\n", ret);
			return;
		}

		/* Get 'pshared'.  It should be PTHREAD_PROCESS_PRIVATE. */
		if (pthread_condattr_getpshared(&attr[i], &pshared) != 0) {
			printf("Error obtaining the attribute process-shared\n");
			return;
		}

		if (pshared != PTHREAD_PROCESS_PRIVATE) {
			printf("Test FAILED: Incorrect pshared value: %d\n",
			       pshared);
			return;
		}

		/* Destory the cond attributes object */
		if (pthread_condattr_destroy(&attr[i]) != 0) {
			printf("Error at pthread_condattr_destroy()\n");
			return;
		}
	}

	printf("test_pthread_condattr_pshared PASSED\n");
}




void test_pthread_mutexattr_pshared()
{
    pthread_mutexattr_t mta;
	int ret;
    int pshared;
	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		printf("Error at pthread_mutexattr_init()\n");
		return;
	}

	/* Set the 'pshared' attribute to PTHREAD_PROCESS_PRIVATE */
	if ((ret =
	     pthread_mutexattr_setpshared(&mta,PTHREAD_PROCESS_PRIVATE)) != 0) {
		printf("Test FAILED: Cannot set pshared attribute to PTHREAD_PROCESS_PRIVATE. Error: %d\n",
		     ret);
		return;
	}

	/* Set the 'pshared' attribute to PTHREAD_PROCESS_SHARED */
	if ((ret =
	     pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_SHARED)) != 0) {
		printf("Test FAILED: Cannot set pshared attribute to PTHREAD_PROCESS_SHARED. Error code: %d\n",
		     ret);
		return;
	}

    if (pthread_mutexattr_getpshared(&mta, &pshared) != 0) {
		printf("Error obtaining the attribute process-shared\n");
		return;
	}
	printf("test_pthread_mutexattr_pshared PASSED\n");
}





#define THREAD_NUM  3

static struct broadcast_testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} broadcast_td;

static int broadcast_start_num;
static int broadcast_waken_num;

static void *broadcast_thr_func(void *arg)
{
	int rc;
	pthread_t self = pthread_self();

	if (pthread_mutex_lock(&broadcast_td.mutex) != 0) {
		printf("[Thread 0x%p] failed to acquire the mutex\n",
			(void *)self);
		return NULL;
	}
	broadcast_start_num++;
	printf("[Thread 0x%p] started and locked the mutex\n",
		(void *)self);

	printf("[Thread 0x%p] is waiting for the cond\n",
		(void *)self);
	rc = pthread_cond_wait(&broadcast_td.cond, &broadcast_td.mutex);
	if (rc != 0) {
		printf("pthread_cond_wait return %d\n", rc);
		return NULL;
	}
	broadcast_waken_num++;
	printf("[Thread 0x%p] was wakened and acquired the mutex "
		"again\n", (void *)self);

	if (pthread_mutex_unlock(&broadcast_td.mutex) != 0) {
		printf("[Thread 0x%p] failed to release the mutex\n",
			(void *)self);
		return NULL;
	}
	printf("[Thread 0x%p] released the mutex\n", (void *)self);
	return NULL;
}

void test_pthread_cond_broadcast(void)
{
	// struct timespec completion_wait_ts = {0, 100000};
	int i, rc;
	pthread_t thread[THREAD_NUM];

	if (pthread_mutex_init(&broadcast_td.mutex, NULL) != 0) {
		printf("Fail to initialize mutex\n");
		return;
	}
	if (pthread_cond_init(&broadcast_td.cond, NULL) != 0) {
		printf("Fail to initialize cond\n");
		return;
	}

	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_create(&thread[i], NULL, broadcast_thr_func, NULL) != 0) {
			printf("Fail to create thread[%d]\n", i);
			return;
		}
	}

	while (broadcast_start_num < THREAD_NUM)
		rt_thread_mdelay(500);

	/*
	 * Acquire the mutex to make sure that all waiters are currently
	 * blocked on pthread_cond_wait
	 */
	if (pthread_mutex_lock(&broadcast_td.mutex) != 0) {
		printf("Main: Fail to acquire mutex\n");
		return;
	}
	if (pthread_mutex_unlock(&broadcast_td.mutex) != 0) {
		printf("Main: Fail to release mutex\n");
		return;
	}

	/* broadcast and check if all waiters are wakened */
	printf("[Main thread] broadcast the condition\n");
	rc = pthread_cond_broadcast(&broadcast_td.cond);
	if (rc != 0) {
		printf("[Main thread] failed to broadcast the "
			"condition\n");
		return;
	}
	rt_thread_mdelay(500);
	if (broadcast_waken_num < THREAD_NUM) {
		printf("[Main thread] Not all waiters were wakened\n");
		printf("Test FAILED\n");
		for (i = 0; i < THREAD_NUM; i++)
			pthread_cancel(thread[i]);

		return;
	}
	printf("[Main thread] all waiters were wakened\n");

	/* join all secondary threads */
	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			printf("Fail to join thread[%d]\n", i);
			return;
		}
	}
	printf("pthread_cond_broadcast PASSED\n");
	return;
}




static struct timedwait_testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} timedwait_td;

static int timedwait_th_start = 0;
static int timedwait_signaled = 0;

static void *timedwait_func(void *arg)
{
	int rc;
	struct timespec timeout;
	struct timeval curtime;

	(void) arg;

	if (pthread_mutex_lock(&timedwait_td.mutex) != 0) {
		printf("Thread1: Fail to acquire mutex\n");
		return NULL;
	}
	printf("Thread1 started\n");
	timedwait_th_start = 1;		/* let main thread continue */

	// if (gettimeofday(&curtime, NULL) != 0) {
	// 	printf("Fail to get current time\n");
	// 	return NULL;
	// }
    curtime.tv_sec = 0;
    curtime.tv_usec = 0;
	timeout.tv_sec = curtime.tv_sec;
	timeout.tv_nsec = curtime.tv_usec * 1000;
	timeout.tv_sec += 5;

	printf("Thread1 is waiting for the cond\n");
	rc = pthread_cond_timedwait(&timedwait_td.cond, &timedwait_td.mutex, &timeout);
	if (rc != 0) {
		if (rc == ETIMEDOUT) {
			printf("Thread1 stops waiting when time is out\n");
			return NULL;
		} else {
			printf("pthread_cond_timedwait return %d\n",
				rc);
			return NULL;
		}
	}

	printf("Thread1 wakened up\n");
	if (timedwait_signaled == 0) {
		printf("Thread1 did not block on the cond at all\n");
		printf("Test FAILED\n");
		return NULL;
	}
	pthread_mutex_unlock(&timedwait_td.mutex);
	return NULL;
}

void test_pthread_cond_timedwait(void)
{
	pthread_t thread1;
	struct timespec thread_start_ts = {0, 100000};

	if (pthread_mutex_init(&timedwait_td.mutex, NULL) != 0) {
		printf("Fail to initialize mutex\n");
		return;
	}
	if (pthread_cond_init(&timedwait_td.cond, NULL) != 0) {
		printf("Fail to initialize cond\n");
		return;
	}

	if (pthread_create(&thread1, NULL, timedwait_func, NULL) != 0) {
		printf("Fail to create thread 1\n");
		return;
	}
	while (!timedwait_th_start)	/* wait for thread1 started */
		nanosleep(&thread_start_ts, NULL);

	/* acquire the mutex released by pthread_cond_wait() within thread 1 */
	if (pthread_mutex_lock(&timedwait_td.mutex) != 0) {
		printf("Main: Fail to acquire mutex\n");
		return;
	}
	if (pthread_mutex_unlock(&timedwait_td.mutex) != 0) {
		printf("Main: Fail to release mutex\n");
		return;
	}
	rt_thread_mdelay(1000);

	printf("Time to wake up thread1 by signaling a condition\n");
	timedwait_signaled = 1;
	if (pthread_cond_signal(&timedwait_td.cond) != 0) {
		printf("Main: Fail to signal cond\n");
		return;
	}

	pthread_join(thread1, NULL);
	printf("pthread_cond_timedwait PASSED\n");
	return;
}


int main()
{
    int rc = 0;
	pthread_attr_t attr;
    int policy = SCHED_FIFO;
    int priority;
    struct sched_param param;

    printf("pthread testcases.\n");
    
    
    
    rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf("pthread_attr_init error rc = %d\n", rc);
		return 0;
	}
    
/*pthread_attr_getdetachstate*/
    printf(">>>test_pthread_attr_getdetachstate\n");
    int detach_state;
    if (pthread_attr_getdetachstate(&attr, &detach_state) != 0) {
		printf("pthread_attr_getdetachstate FAILED\n");
	}

	if (detach_state == PTHREAD_CREATE_JOINABLE) {
		printf("pthread_attr_getdetachstate PASSED\n");
	} else {
		printf("pthread_attr_getdetachstate FAILED\n");
	}

/*pthread_attr_getstackaddr*/

    //void * stackaddr;
    //rc = pthread_attr_getstackaddr(&attr,&stackaddr);

/*pthread_attr_setschedpolicy*/
/*pthread_attr_getschedpolicy*/
    printf(">>>test_pthread_attr_schedpolicy\n");
    rc = pthread_attr_setschedpolicy(&attr, policy);
    if (rc != 0) {
		printf("pthread_attr_setschedpolicy error rc = %d\n", rc);
	} 
    else 
    {
        int p;
        rc = pthread_attr_getschedpolicy(&attr, &p);
        if (rc != 0) {
            printf("pthread_attr_getschedpolicy error rc = %d\n", rc);
        }
        else
        {
            if (p != policy) {
			    printf("got wrong policy param\n");
		    }
        }
    }

    priority = sched_get_priority_max(policy);
    param.sched_priority = priority;
/*pthread_attr_setschedparam*/
/*pthread_attr_getschedparam*/
    printf(">>>test_pthread_attr_schedparam\n");
    rc = pthread_attr_setschedparam(&attr, &param);
    if (rc != 0) {
		printf("pthread_attr_setschedparam error rc = %d\n", rc);
	} 
    else 
    {
        rc = pthread_attr_getschedparam(&attr, &param);
        if (rc != 0) {
            printf("pthread_attr_getschedparam error rc = %d\n", rc);
        }
        else
        {
            if (priority != param.sched_priority) {
                printf("got wrong sched param\n");
            }
            else printf("test_pthread_attr_schedparam PASS\n");
        }
    }

    
/*pthread_attr_setscope*/
/*pthread_attr_getscope*/
    printf(">>>test_pthread_attr_setscope\n");
    rc = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	if (rc != 0) {
		printf("PTHREAD_SCOPE_SYSTEM is not supported rc = %d\n", rc);
	} else {
		int scope;
        rc = pthread_attr_getscope(&attr, &scope);
        if (rc != 0) {
            printf("pthread_attr_getscope rc = %d\n", rc);
        }
        else{
            if (scope != PTHREAD_SCOPE_SYSTEM) {
			    printf("got wrong scope param\n");
		    }
            else printf("setscope sucess\n");
        }
	}
    
    
    rc = pthread_attr_destroy(&attr);
    if (rc != 0) {
		printf("pthread_attr_destroy error rc = %d\n", rc);
	}
    printf(">>>test_pthread_attr_destroy\n");
    test_pthread_attr_destroy();
    printf(">>>test_pthread_attr_inheritsched\n");
    test_pthread_attr_inheritsched();
    printf(">>>test_pthread_attr_stacksize\n");
    test_pthread_attr_stacksize();
    printf(">>>test_pthread_attr_scope\n");
    test_pthread_attr_scope();
    printf(">>>test_pthread_cleanup\n");
    test_pthread_cleanup();
    printf(">>>test_pthread_cond_init_destroy\n");
    test_pthread_cond_init_destroy();
    printf(">>>test_pthread_cond_signal\n");
    test_pthread_cond_signal();
    printf(">>>test_pthread_condattr_clock\n");
    test_pthread_condattr_clock();
    printf(">>>test_pthread_key\n");
    test_pthread_key();
    printf(">>>test_pthread_mutex\n");
    test_pthread_mutex();
    printf(">>>test_pthread_mutexattr_type\n");
    test_pthread_mutexattr_type();
    printf(">>>test_pthread_once\n");
    test_pthread_once();
    printf(">>>test_pthread_self\n");
    test_pthread_self();
    printf(">>>test_pthread_condattr_pshared\n");
    test_pthread_condattr_pshared();
    printf(">>>test_pthread_mutexattr_pshared\n");
    test_pthread_mutexattr_pshared();

    printf("pthread test finished..\n");
    return 0;
}
