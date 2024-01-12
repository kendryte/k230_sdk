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

// #define _debug_write

#define OTP_DEV_NAME            "otp"


static int otp_read(rt_device_t otp_dev, uint32_t pos, void *buffer, uint32_t size)
{
    int reval = -1;
    uint32_t *buf = (uint32_t *)buffer;

    reval = rt_device_read(otp_dev, pos, (void*)buf, size);
    if(reval <= 0)
    {
        return reval;
    }
    else
    {
        printf("READ OTP SUCCESS!\n");
        for(int i=0; i<reval/4; i++)
        {
            printf("\n\r%08x    0x%08x", pos+i*4, buf[i]);
        }
        printf("\n");
    }

    return reval;
}

#ifdef _debug_write
static int otp_write(rt_device_t otp_dev, uint32_t pos, void *buffer, uint32_t size)
{
    int reval = -1;

    reval = rt_device_write(otp_dev, pos, buffer, size);
    if(reval <= 0)
    {
        return reval;
    }
    else
    {
        printf("WRITE OTP SUCCESS!\n");
    }

    return reval;
}
#endif

int main()
{
    int ret = 0;
    uint32_t pos;
    uint32_t size;
    uint32_t buffer[256] = {0};

    rt_device_t otp_dev;
    printf("otp_driver test\n");

    otp_dev = rt_device_find(OTP_DEV_NAME);
    if (otp_dev == RT_NULL)
    {
        printf("device find error\n");
        return -1;
    }
    printf("find device success\n");

    ret = rt_device_open(otp_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        printf("otp device open err\n");
        return -1;
    }

    // read test    initilize param
    pos = 0x0;
    size = 0x300;

    ret = otp_read(otp_dev, pos, (void *)buffer, size);
    if(ret < 0)
    {
        printf("read test failed!\n");
        return -1;
    }

#ifdef _debug_write
    // write test   initilize param
    pos = 0x10;
    size = 0x4;
    buffer[0] = 0xff11ff11;

    ret = otp_write(otp_dev, pos, (void *)buffer, size);
    if(ret < 0)
    {
        rt_kprintf("write test failed!\n");
        return -1;
    }

    // read test
    pos = 0x0;
    size = 0x300;
    ret = otp_read(otp_dev, pos, (void *)buffer, size);
    if(ret < 0)
    {
        printf("read test failed!\n");
        return -1;
    }
#endif

    rt_device_close(otp_dev);


    return 0;
}