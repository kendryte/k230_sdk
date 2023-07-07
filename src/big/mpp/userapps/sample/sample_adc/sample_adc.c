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

#define REF_VOL         (1.8)
#define RESOLUTION      (4096)

#define ADC_CHN_ENABLE  (0)
#define ADC_CHN_DISABLE (1)

int main()
{
    int ret = 0;
    unsigned int channel = 0;
    unsigned int reg_value = 0;
    float vol = 0.0;

    rt_device_t adc_dev;
    printf("adc_driver test\n");

    adc_dev = rt_device_find("adc");
    if (adc_dev == RT_NULL)
    {
        printf("device find error\n");
        return -1;
    }
    printf("find device success\n");

    ret = rt_device_open(adc_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        printf("adc device open err\n");
        return -1;
    }

    uint32_t *p;
    for (channel = 0; channel < 5; channel++)
    {
        p = (uint32_t *)(intptr_t)channel;
        ret = rt_device_control(adc_dev, ADC_CHN_ENABLE, (void *)p);
        if (ret != RT_EOK)
        {
            printf("adc device control err\n");
            return -1;
        }
        printf("adc device control success. ");

        ret = rt_device_read(adc_dev, channel, (void *)&reg_value, sizeof(unsigned int));
        vol = REF_VOL * reg_value / RESOLUTION;
        printf("channel %d reg_value:0x%04x, voltage:%f\n", channel, reg_value, vol);

    }

    for (channel = 0; channel < 5; channel++)
    {
        p = (uint32_t *)(intptr_t)channel;
        ret = rt_device_control(adc_dev, ADC_CHN_DISABLE, (void *)p);
        if (ret != RT_EOK)
        {
            printf("adc device control err\n");
            return -1;
        }
        printf("adc device control success. ");

        ret = rt_device_read(adc_dev, channel, (void *)&reg_value, sizeof(unsigned int));
        vol = REF_VOL * reg_value / RESOLUTION;
        printf("channel %d reg_value:0x%04x, voltage:%f\n", channel, reg_value, vol);
    }


    return 0;
}