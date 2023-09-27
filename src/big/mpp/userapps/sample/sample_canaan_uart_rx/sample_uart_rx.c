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
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <poll.h>

#define BUF_SIZE    (1024)
#define UART_DEVICE_NAME    "/dev/uart2"
static char buf[BUF_SIZE];
static char send[512];
#define	IOC_SET_BAUDRATE            _IOW('U', 0x40, int)

struct uart_configure
{
    rt_uint32_t baud_rate;

    rt_uint32_t data_bits               :4;
    rt_uint32_t stop_bits               :2;
    rt_uint32_t parity                  :2;
    rt_uint32_t fifo_lenth              :2;
    rt_uint32_t auto_flow               :1;
    rt_uint32_t reserved                :21;
};

typedef enum _uart_parity
{
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN
} uart_parity_t;

typedef enum _uart_receive_trigger
{
    UART_RECEIVE_FIFO_1,
    UART_RECEIVE_FIFO_8,
    UART_RECEIVE_FIFO_16,
    UART_RECEIVE_FIFO_30,
} uart_receive_trigger_t;


static pthread_t pthread_handle2 = RT_NULL;
static volatile char flag;
static pthread_cond_t cond;
static pthread_mutex_t mutex;



static void checkdata(char *buf1, char *buf2, int len)
{
    int i;
    int ret = 0;
    for (i = 0; i < len; i++)
    {
        if (buf1[i] != buf2[i])
        {
            printf("error, buf1[%d] != buf2[%d],  %d != %d\n", i, i, buf1[0], buf2[0]);
            ret = 1;
        }
    }
    if (ret == 0)
        printf("check success\n");
}



static void *thread2_entry(void *parameter)
{
    int fd;
    struct pollfd fds[1];
    int ret = 0, cnt = 0; 

    printf(" [app] open uart2.....\n");
    fd = open(UART_DEVICE_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("open dev uart2 failed!\n");
        return NULL;
    }

    struct uart_configure config = {
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = UART_PARITY_NONE,
        .fifo_lenth = UART_RECEIVE_FIFO_16,
        .auto_flow = 0,
    };


    if (ioctl(fd, IOC_SET_BAUDRATE, &config))
    {
        printf("uart2 ioctl failed!\n");
    }

    write(fd, send, 512);


    fds[0].fd = fd;
    fds[0].events = POLLIN;
    printf(" [app] ........... poll \n");

    while(1)
    {
        pthread_mutex_lock(&mutex);
        if (poll(fds, 1, -1) > 0 && fds[0].revents & POLLIN)
        {
            ret = read(fd, &buf[ret], BUF_SIZE);
            if (ret > 0 && ret <= BUF_SIZE)
            {
                cnt += ret;
            }
            printf("package size %d\n", ret);
            if (cnt == BUF_SIZE)
            {
                flag = 1;
                ret = 0;
                printf("[app] read %d byte\n", cnt);
                cnt = 0;
            }
            pthread_mutex_unlock(&mutex); 
            pthread_cond_signal(&cond);
        }
    }


    close(fd);
    return NULL;
}


int main()
{
    int i;
    for (i = 0; i < 512; i++)
    {
        send[i] = i % 256;
    }

    flag = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_create(&pthread_handle2, NULL, thread2_entry, NULL);


    while(1)
    {
        if (!pthread_cond_wait(&cond, &mutex))
        {
            if (flag)
            {

                checkdata(send, buf, 512);
                checkdata(send, &buf[512], 512);
                flag = 0;
            }

            pthread_mutex_unlock(&mutex); 
        }
    }


    pthread_join(pthread_handle2, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}