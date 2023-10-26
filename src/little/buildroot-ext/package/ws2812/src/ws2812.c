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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/select.h>

#define WSIOC_RESET     _IOW('W', 0x10, int)
#define LED_NUM			24   /* 根据实际使用的数量修改 */

static void sleep_ms(unsigned int ms)
{
	struct timeval tval;

	tval.tv_sec = ms / 1000;
	tval.tv_usec = (ms * 1000) % 1000000;
	select(0, NULL, NULL, NULL, &tval);
}

/* ws2812	24bit GRB
-------------------------------------
|	8bit G	|	8bit R	|	8bit B	|
-------------------------------------
1 --> 1110
0 --> 1000

*/

/* 测试使用灯条共24个ws2812灯珠 */

static void w2812_reset_clear(int fd, int num)
{
	unsigned char *clear;
	sleep(1);

	clear = (unsigned char*)malloc(num * 3);
	memset(clear, 0, num * 3);

	write(fd, clear, num * 3);
	write(fd, clear, num * 3);
	sleep(1);
	free(clear);
}

static void waterfall_light(int fd, int num)
{
	int i;
	int cnt = 10;
	int len = num * 3;
	unsigned char *data;
	data = (unsigned char*)malloc(len);

	w2812_reset_clear(fd, LED_NUM);
	printf("waterfall light....\n");

	memset(data, 0, len);

	while (cnt--)
	{
		for(i = 0; i < num; i++)
		{
			data[i*3 + 0] = 0xa9;
			data[i*3 + 1] = 0xa9;
			data[i*3 + 2] = 0xa9;
			write(fd, data, len);
			sleep_ms(20);
		}

		memset(data, 0, len);
		write(fd, data, len);
	}
	free(data);
}

static void breath_light(int fd, int num)
{
	int len = num * 3;
	unsigned char *data;
	int i, n;
	data = (unsigned char*)malloc(len);

	w2812_reset_clear(fd, LED_NUM);
	printf("breathing light.....\n");

	memset(data, 0, len);

	while(1)
	{
		for (i = 0; i < 0xff; i++)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = 0x0;
				data[n * 3 + 1] = i;
				data[n * 3 + 2] = 0x0;
			}
			write(fd, data, len);
			sleep_ms(1);
		}

		for (i = 0xff; i >= 0; i--)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = 0x0;
				data[n * 3 + 1] = i;
				data[n * 3 + 2] = 0x0;
			}
			write(fd, data, len);
			sleep_ms(1);
		}

		for (i = 0; i < 0xff; i++)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = i;
				data[n * 3 + 1] = 0x0;
				data[n * 3 + 2] = 0x0;
			}
			write(fd, data, len);
			sleep_ms(1);
		}

		for (i = 0xff; i >= 0; i--)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = i;
				data[n * 3 + 1] = 0x0;
				data[n * 3 + 2] = 0x0;
			}
			write(fd, data, len);
			sleep_ms(1);
		}

		for (i = 0; i < 0xff; i++)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = 0x0;
				data[n * 3 + 1] = 0x0;
				data[n * 3 + 2] = i;
			}
			write(fd, data, len);
			sleep_ms(1);
		}

		for (i = 0xff; i >= 0; i--)
		{
			for (n = 0; n < num; n++)
			{
				data[n * 3] = 0x0;
				data[n * 3 + 1] = 0x0;
				data[n * 3 + 2] = i;
			}
			write(fd, data, len);
			sleep_ms(1);
		}
	}

	free(data);
}


int main(int argc, char *argv[])
{
	int fd;
	int n = LED_NUM;

    fd = open("/dev/ws2812", O_RDWR);
	if(fd<=0)
	{
        printf("open ws2812 device error!\n");
		return 0;
	}

	if (argv[1])
	{
		n = atoi(argv[1]);
	}

	// ioctl(fd, WSIOC_RESET, NULL);  /* 产生复位时序接口，按需使用 */

	waterfall_light(fd, n);
	breath_light(fd, n);
	close(fd);
	return 0;
}
