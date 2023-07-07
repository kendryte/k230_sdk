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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define BUFFER_SIZE 16

// #define _debug


double pow(double base, double exponent) {
   // 实现自己的pow()函数代码
   double result = 1.0;
   for(int i=0; i<exponent; i++) {
      result *= base;
   }
   return result;
}


int main(int argc, char* argv[])
{
   int fd;
   unsigned char buffer[BUFFER_SIZE];
   int cnt;
   int ts_val = 0;
   double code = 0;
   double temp = 0;

   if (argc != 2) {
      printf("Input Error! [./ts_test_demo count], count>=1\n");
      exit(-1);
   }

   cnt = atoi(argv[1]);

   while(cnt--)
   {
      // 打开 /sys/class/thermal/thermal_zone0/temp 文件
      fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
      if (fd == -1) {
         perror("Failed to open device file");
         return 1;
      }

      // 从文件中读取温度数据
      ssize_t ret = read(fd, &buffer, BUFFER_SIZE);
      if (ret == -1) {
         perror("Failed to read from device file");
         close(fd);
         return 2;
      }

      // 关闭文件
      close(fd);

   #ifdef _debug
      printf("[TS]: 0x%x\n", *buffer);
   #endif
      ts_val = atoi(buffer);
   #ifdef _debug
      printf("[TS]: 0x%x\n", ts_val);
   #endif
      code = (double)(ts_val & 0xfff);
      
      temp = (1e-10 * pow(code, 4) * 1.01472 - 1e-6 * pow(code, 3) * 1.10063 + 4.36150 * 1e-3 * pow(code, 2) - 7.10128 * code + 3565.87);
      printf("ts_val: 0x%x, TS = %lf C\n", ts_val, temp);
   }

   return 0;
}

