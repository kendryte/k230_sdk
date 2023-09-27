#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <pthread.h>

void __attribute__((noinline, noclone, optimize(3)))
vadd (int *dst, int *op1, int *op2, int count)
{
  for (int i = 0; i < count; ++i)
    dst[i] = op1[i] + op2[i];
}

#define ELEMS 10

int vadd_test(void)
{
    int in1[ELEMS] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int in2[ELEMS] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    int out[ELEMS];
    int check[ELEMS] = { 3, 5, 7, 9, 11, 13, 15, 17, 19, 21 };

    vadd (out, in1, in2, ELEMS);

    for (int i = 0; i < ELEMS; ++i)
    {
        if (out[i] != check[i]) {
            printf("check error\n");
            __builtin_abort();
        }
    }
    return 0;
}

void* thread_entry(void* arg)
{
    int rand_num = (rand() % 100) + 1;
    printf("enter thread run [%d] times\n", rand_num);
    for (int i = 0; i < rand_num; ++i)
    {
        vadd_test();
        rt_thread_mdelay(1);
    }
    return NULL;
}

#define THREAD_NUM 10
#define THREAD_STACK_SIZE 2048
#define THREAD_TIMESLICE 20

int main(void)
{
    pthread_t thread[THREAD_NUM];
    for(int i = 0; i < THREAD_NUM; i++)
    {
        if (pthread_create(&thread[i], NULL, thread_entry, NULL) != 0) {
			printf("Fail to create thread[%d]\n", i);
			return 0;
		}
    }

    for (int i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			printf("Fail to join thread[%d]\n", i);
			return 0;
		}
	}

    printf("vadd_test check passed\n");
    return 0;
}

