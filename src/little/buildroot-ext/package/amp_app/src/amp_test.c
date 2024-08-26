/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "image.h"
#include "gunzip.h"

#define AMP_DEV             "/dev/k230-amp"
#define AMP_CMD_BOOT         _IOWR('q', 1, unsigned long)
#define UNCOMPRLEN          0x800000

// 存储介质里面的固件的结构：
// | firmware_head_s | 4B version | image_header_t | data |
static int amp_load_firmware(char *dev_name, char *buff)
{
    int ret = 0;
    int mtd0_fd;
    image_header_t * pUh = NULL;
    firmware_head_s *pfh = NULL;
    size_t len = 0;

    if ((mtd0_fd = open(dev_name, O_RDONLY)) < 0) {
        printf("Error opening dev %s \n", dev_name);
        return -1;
    }
    
    pfh = (firmware_head_s *)malloc(sizeof(firmware_head_s));
    len = sizeof(*pfh);
    
    // 先读firmware_head_s，并解析。
    if(read(mtd0_fd, pfh, len) != len)
    {
        printf("ERR: read %d err\n", len);
        ret = -2;
        goto exit0;
    }

    if(pfh->magic != MAGIC_NUM){
        printf("pfh->magic 0x%x != 0x%x \n", pfh->magic, MAGIC_NUM);
        ret = -3;
        goto exit0;
    }

    unsigned char *firmware = (unsigned char *)malloc(pfh->length);

    if(read(mtd0_fd, firmware, pfh->length) != pfh->length)
    {
        printf("ERR: read %d err\n", pfh->length);
        ret = -4;
        goto exit1;
    }

    // 跳过4B的version，解析image_header_t
    pUh = firmware + 4;
    if (!image_check_magic(pUh)){
        printf("bad magic \n");
        ret = -5;
        goto exit1;
    }
    if (image_get_os(pUh) != IH_OS_OPENSBI)
    {
        printf("not rtt image %d\n", image_get_os(pUh));
        ret = -6;
        goto exit1;
    }
    // 参考uboot 里面的解析固件方式
    char *data;

    image_multi_getimg(pUh, 0, &data, &len);
    if(data==NULL || len == 0)
    {
        printf("no image \n");
        ret = -7;
        goto exit1;
    }

    // uboot里面采用硬件解压缩的方式，使用软件解压缩方式效果一样。
    data[2] = 0x8; //uboot通过data[2]决定硬解压缩还是软解压缩
    ret = stand_gunzip((void *)(ulong)buff, UNCOMPRLEN, (void *)data, &len);
    if(ret){
        printf("unzip fialed ret =%x\n", ret);
        ret = -8;
        goto exit1;
    }

exit1:
    free(firmware);
exit0:
    free(pfh);
    close(mtd0_fd);

    return ret;
}

int main(int argc, char *argv[])
{
    int amp_fd;
    char *buffer;

    if (argc != 2)
      return -1;

    char *dev_name = argv[1];
   
    if((amp_fd = open(AMP_DEV, O_RDWR)) < 0)
    {
        printf("ERR: Open %s err\n", AMP_DEV);
        return EXIT_FAILURE;
    }

    // UNCOMPRLEN rtthread解压之后占用的内存大小，默认8MB。
    // 关于mmap，linux/drivers/misc/canaan/k230-amp.c 驱动实现了.mmap。会映射rtthread使用的物理内存，用户态通过虚拟地址访问。
    // rtthread使用的物理内存 是由linux设备树指定的。
    // amp-reset-vec = <0x200000>;
    // 与顶层CONFIG_MEM_RTT_SYS_BASE=0x00200000 保持一致。
    buffer = mmap(NULL, UNCOMPRLEN, PROT_READ | PROT_WRITE, MAP_SHARED, amp_fd, NULL);

    // 把rtthread的固件写到rtthread使用的物理内存上。buffer为映射之后用户态可以访问的虚拟地址。
    if(amp_load_firmware(dev_name, buffer) < 0)
    {
        printf("ERR: load %s err\n", dev_name);
        return EXIT_FAILURE;
    }
    // reset 大核，也就是让大核重新执行程序。传递虚拟地址，内核态需要执行刷cache的操作。
    if (ioctl(amp_fd, AMP_CMD_BOOT, buffer) < 0)
    {
        printf("Call cmd AMP_CMD_BOOT fail\n");
        return EXIT_FAILURE;
    }

    close(amp_fd);
    return EXIT_SUCCESS;
}
