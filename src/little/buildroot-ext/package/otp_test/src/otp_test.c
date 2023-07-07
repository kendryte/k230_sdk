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
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#define SYSFS_NVMEM_DIR "/sys/bus/nvmem/devices/kendryte_otp0/nvmem"

int main(int argc, char* argv[])
{
   int fd;
//    char filename[sizeof(SYSFS_NVMEM_DIR) + 32];

//    sprintf(filename, "%s%s/nvmem", SYSFS_NVMEM_DIR, argv[1]);

//    fd = open(filename, O_RDONLY);

	fd = open(SYSFS_NVMEM_DIR, O_RDONLY);
	if (fd < 0) {
		printf("Error opening nvmem device!\n");
		exit(-1);
	}

	if (argc != 2) {
		printf("Input Error! [./otp_test_demo len], len is 1~768\n");
		close(fd);
		exit(-1);
	} else {
		uint32_t buf[atoi(argv[1])];
		if(0xc00 < sizeof(buf))
		{
			printf("Array length exceeds OTP readable range!\n");
			exit(-1);
		}
		int len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			printf("Error reading nvmem device!\n");
			close(fd);
			exit(-1);
      	}

		int cnt;
		printf("\n\rAddr			Value\n");
		for(cnt=0; cnt<atoi(argv[1]); cnt++)
		{
			printf("%08x		0x%08x\n", cnt*4, buf[cnt]);
		}
	}

   close(fd);
   return 0;
}