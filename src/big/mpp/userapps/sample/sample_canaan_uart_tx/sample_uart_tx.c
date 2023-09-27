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
#include "stdint.h"

#define TIME_RATE (1600*1000)
#define UART_DEVICE_NAME1    "/dev/uart4"
#define UART_DEVICE_NAME2    "/dev/uart2"
#define BUF_SIZE    (512)

int main(int argc, char *argv[])
{
    int fd1;
    int fd2;
    int i;
    char send[BUF_SIZE];
    char buf[BUF_SIZE] = {0};

    fd1 = open(UART_DEVICE_NAME1, O_RDWR);
    if (fd1 < 0)
    {
        printf("open dev uart4 failed!\n");
        return -1;
    }

    fd2 = open(UART_DEVICE_NAME2, O_RDWR);
    if (fd2 < 0)
    {
        printf("open dev uart2 failed!\n");
        return -1;
    }

    for (i = 0; i < BUF_SIZE; i++)
    {
        send[i] = i % 256;
    }



    write(fd1, send, BUF_SIZE);
    read(fd2, buf, BUF_SIZE);

    for (i = 0; i < BUF_SIZE; i++)
    {
        printf("%d ", buf[i]);
    }
    printf("\n");

    close(fd1);
    close(fd2);
    return 0;
}
