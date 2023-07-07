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
#include <math.h>

#define TS_DEV_NAME            "ts"


int main()
{
    int ret = -1;
    int i = 20;
    uint32_t pos = 0;
    uint32_t size = 0;
    uint32_t buffer[1] = {0};
    uint32_t ts_val = 0;
    double code = 0, temp = 0;

    rt_device_t ts_dev;
    printf("ts_driver test\n");

    ts_dev = rt_device_find(TS_DEV_NAME);
    if (ts_dev == RT_NULL)
    {
        printf("device find error\n");
        return -1;
    }
    printf("find device success\n");

    ret = rt_device_open(ts_dev, RT_DEVICE_OFLAG_RDONLY);
    if (ret != RT_EOK)
    {
        printf("ts device open err\n");
        return -1;
    }

    while(i--)
    {
        ret = rt_device_read(ts_dev, pos, (void *)buffer, size);
        if(ret <= 0)
        {
            printf("read device fail\n");
            return -1;
        }

        ts_val = *(uint32_t *)buffer;
        code = (double)(ts_val & 0xfff);

        temp = (1e-10 * pow(code, 4) * 1.01472 - 1e-6 * pow(code, 3) * 1.10063 + 4.36150 * 1e-3 * pow(code, 2) - 7.10128 * code + 3565.87);
        printf("ts_val: 0x%x, TS = %lf C\n", ts_val, temp);
    }

    rt_device_close(ts_dev);

    return 0;
}