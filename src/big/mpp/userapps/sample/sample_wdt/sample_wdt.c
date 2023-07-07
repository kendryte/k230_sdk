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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/ioctl.h"

#define WDT_DEVICE_NAME    "/dev/watchdog1"    /* 看门狗设备名称 */

#define CTRL_WDT_GET_TIMEOUT    _IOW('W', 1, int) /* get timeout(in seconds) */
#define CTRL_WDT_SET_TIMEOUT    _IOW('W', 2, int) /* set timeout(in seconds) */
#define CTRL_WDT_GET_TIMELEFT   _IOW('W', 3, int) /* get the left time before reboot(in seconds) */
#define CTRL_WDT_KEEPALIVE      _IOW('W', 4, int) /* refresh watchdog */
#define CTRL_WDT_START          _IOW('W', 5, int) /* start watchdog */
#define CTRL_WDT_STOP           _IOW('W', 6, int) /* stop watchdog */

static unsigned int keepalive = 1;     /* 喂狗时间 */

static void *feed_dog_handle(void *arg)
{
    int wdt_fd = *(int*)arg;

    /* watchdog 开始计时 */
    if (ioctl(wdt_fd, CTRL_WDT_START, NULL))
    {
        printf("start watchdog1 err\n");
        return NULL;
    }

    while(1)
    {
        if(ioctl(wdt_fd, CTRL_WDT_KEEPALIVE, NULL))
        {
            printf("feed watchdog1 err\n");
            if (ioctl(wdt_fd, CTRL_WDT_STOP, NULL))
            {
                printf("close watchdog1 err\n");
            }
        }
        sleep(keepalive);
    }

    close(wdt_fd);
    return NULL;
}



int main(int argc, char *argv[])
{
    int ret;
	pthread_attr_t attr;
	struct sched_param sp;
    pthread_t feeddog_thread_handle;
    unsigned int timeout = 2;    /* 溢出时间 s */
    int wdt_fd;

    /* 判断命令行参数是否指定了超时时间和喂狗时间 */
    if (argc == 2)
    {
        timeout = atoi(argv[1]);
    } else if (argc == 3)
    {
        timeout = atoi(argv[1]);
        keepalive = atoi(argv[2]);
        printf("keep alive time is %ds\n", keepalive);
    }

    wdt_fd = open(WDT_DEVICE_NAME, O_RDWR);
    if (wdt_fd < 0)
    {
        perror("open /dev/watchdog1 err\n");
        pthread_exit((void *) "thread exit!");
    }

    if (ioctl(wdt_fd, CTRL_WDT_SET_TIMEOUT, &timeout))
    {
        perror("set timeout to watchdog1 err\n");
        pthread_exit((void *) "thread exit!");
    }
    printf("set timeout to wdt is %ds.\n", timeout);

    timeout = 0;
    if (ioctl(wdt_fd, CTRL_WDT_GET_TIMEOUT, &timeout))
    {
        perror("set timeout to watchdog1 err\n");
        pthread_exit((void *) "thread exit!");
    }

    printf("get timeout from wdt is %ds.\n", timeout);

	ret = pthread_attr_init(&attr);
	if (ret != 0) {
		printf("pthread_attr_init error");
		return -1;
	}

    sp.sched_priority = 4;

	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret != 0)
		goto error;

	ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (ret != 0)
		goto error;

    ret = pthread_attr_setschedparam(&attr, &sp);
    if (ret != 0)
        goto error;

    pthread_create(&feeddog_thread_handle, &attr, feed_dog_handle, (void*)&wdt_fd);


    while(1);
    pthread_join(feeddog_thread_handle, NULL);
    return 0;
error:
	pthread_attr_destroy(&attr);

    return 0;
}
