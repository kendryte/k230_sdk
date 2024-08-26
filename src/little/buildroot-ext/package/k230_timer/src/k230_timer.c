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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define TMIOC_SET_TIMEOUT     _IOW('T', 0x20, int)

int main(int argc, char* argv[])
{
   int fd;
   unsigned long value;
   float rate;
   ssize_t ret;
   unsigned int timeout;
   int cnt = 0;

   if (argc < 3)
      return -1;

   char *dev = argv[1];
   timeout = atoi(argv[2]);

   fd = open(dev, O_RDONLY);
   if (fd == -1) {
      perror("Failed to open device file");
      return 1;      
   }
   printf("open OK!\n");

   ret = ioctl(fd, TMIOC_SET_TIMEOUT, &timeout);
   if (ret < 0)
   {
      perror("ioctl failed\n");
      return 1;      
   }


   while(1)
   {
      ret = read(fd, &value, sizeof(value));
      if (ret == -1) {
         perror("Failed to read from device file");
         close(fd);
         return 2;
      }

      rate = (float)value / (float)1000000;
      printf("app: %ld Hz,  %f MHz\n", value, rate);
      sleep(1);

   }

   close(fd);
   return 0;
}

