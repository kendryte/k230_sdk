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
#include "k_type.h"

#define DEV_NAME            "eeprom"

#define ID_OFFSET 	128  // id address offset
#define ID_SIZE	 	8
#define EEPROM_SIZE 128

uint8_t wdata[3][EEPROM_SIZE] = {
	{
		12, 186, 120, 141, 3, 171, 164, 25, 157, 29, 
		14, 38, 19, 29, 238, 159, 89, 234, 183, 30, 
		65, 85, 4, 80, 63, 183, 28, 114, 105, 89, 
		129, 130, 8, 153, 93, 228, 50, 211, 184, 78, 
		102, 111, 60, 184, 239, 84, 164, 68, 104, 198, 
		75, 173, 108, 36, 73, 22, 126, 18, 188, 227, 
		84, 29, 24, 94,
		12, 186, 120, 141, 3, 171, 164, 25, 157, 29, 
		14, 38, 19, 29, 238, 159, 89, 234, 183, 30, 
		65, 85, 4, 80, 63, 183, 28, 114, 105, 89, 
		129, 130, 8, 153, 93, 228, 50, 211, 184, 78, 
		102, 111, 60, 184, 239, 84, 164, 68, 104, 198, 
		75, 173, 108, 36, 73, 22, 126, 18, 188, 227, 
		84, 29, 24, 94,
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	},
	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	}
};


k_bool test_read_romid(rt_device_t dev, uint8_t *buf, int *len)
{
	int ret = 0;
    ret = rt_device_read(dev, ID_OFFSET, (void*)buf, ID_SIZE);
    if (ret != ID_SIZE)
    {
        printf("eeprom device(s) read id failed !\n");
    }

	*len = ret;
	return (ret == 8)? K_TRUE: K_FALSE;
}

k_bool test_write_read_more(rt_device_t dev, uint8_t *wbuf, int len, uint8_t offset, unsigned int read_times)
{
	int flag = 0;
	int i = 0, count = 0;
	uint8_t buf[EEPROM_SIZE];

	if(rt_device_write(dev, offset, wbuf, len) != len)
    {
        printf("eeprom device(s) write failed !\n");
        return -1;
    }
	printf("#### write %d bytes, test starting...\n", len);

	while(count < read_times){
		usleep(10000);
		memset(buf, 0, sizeof(buf));
		if(rt_device_read(dev, offset, buf, len) != len)
        {
            printf("#### loop %d: file read error, this loop over!!!\n", count);
            flag = 1;
			continue;
        }

		for(i = 0; i < len; i++){
			if(buf[i] != wbuf[i]){
				printf("####\t loop %d index %d diff: write 0x%x, read 0x%x!\n", 
					count, i, wbuf[i], buf[i]);
				flag = 1;
			}
		}
		if(flag){
//			break;
		}

		count++;
	}

	return (flag == 0)? K_TRUE: K_FALSE;
}

int main(int argc, char *argv[])
{
    int ret;
    int len = 0, i = 0;
    uint8_t buf[EEPROM_SIZE];
    rt_device_t dev;
    unsigned int read_times = 1;
    int loop_times=0;
    if(argc > 1) {
        read_times = (unsigned int)atoi(argv[1]);
    }
    if(argc > 2) {
        loop_times = (unsigned int)atoi(argv[2]);
    }
    printf("read_times = %d \n", read_times);
    dev = rt_device_find("eeprom1");
    if (dev == RT_NULL) {
        rt_kprintf("Can't find %s device\n", "eeprom");
        return -1;
    }
    for(int loop=0; loop<loop_times; loop++){
        ret = rt_device_open(dev, RT_DEVICE_OFLAG_RDWR);
        if (ret != RT_EOK )
        {
            rt_kprintf("[%d]eeprom device(s) open failed !\n", loop);
            return -1;
        }

        if(test_read_romid(dev, buf, &len)){
            printf("#### file device id: %X %X %X %X %X %X %X %X\n",
                buf[0]&0xFF, buf[1]&0xFF, buf[2]&0xFF, buf[3]&0xFF, 
                buf[4]&0xFF, buf[5]&0xFF, buf[6]&0xFF, buf[7]&0xFF);
        }

        for(i = 0; i < sizeof(wdata)/sizeof(wdata[0]); i++){
            if(test_write_read_more(dev, wdata[i], EEPROM_SIZE, 0, read_times)){
                printf("[%d]#### file test write-read(%d) OK.\n", loop, i);
            } else {
                printf("####[%d] file test write-read(%d) failed!!! first byte is 0x%x\n", 
                    loop, i, wdata[i][0] & 0xFF);
                while(1);
    //			break;
            }
        }
        rt_device_close(dev);
    }
    rt_kprintf("over1.......");
    return 0;
}