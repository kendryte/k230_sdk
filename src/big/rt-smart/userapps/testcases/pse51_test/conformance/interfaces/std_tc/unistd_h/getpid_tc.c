#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

static int pid;

static void *a_thread_func()
{
	pid = getpid();
    if(pid <= 0)
    {
        printf("get pid error %d\n",pid);
        exit(0);
    }
    printf("a_thread_func pid %d\n",pid);
	return 0;
}

int main(void)
{
	pthread_t main_th, new_th;
	int ret, ppid;

    pid = getpid();
    if(pid <= 0)
    {
        printf("get pid error %d\n",pid);
        return 0;
    }
    printf("main_func pid %d\n",pid);

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return 0;
	}

	ret = pthread_join(new_th, NULL);
	if (ret) {
		fprintf(stderr, "pthread_join(): %s\n", strerror(ret));
		return 0;
	}

	printf("{Test PASSED}\n");
	return 0;
}
