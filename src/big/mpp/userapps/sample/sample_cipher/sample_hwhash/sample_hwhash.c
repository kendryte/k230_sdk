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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "sys/ioctl.h"

#define HASH_DEVICE_NAME                "/dev/hash"
#define RT_HASH_INIT                    _IOWR('H', 0, int)
#define RT_HASH_UPDATE                  _IOWR('H', 1, int)
#define RT_HASH_FINAL                   _IOWR('H', 2, int)
#define RT_HASH_FAST_DOUBLE             _IOWR('H', 4, int)

typedef enum {
    SHA_224,
    SHA_256,
    SHA_384,
    SHA_512,
    SHA_512_224,
    SHA_512_256,
    SM3,
    N_HASH_T,
} k_pufs_hash_t;

union rt_hash_control_args
{
    struct {
        uint8_t mode;
    } init;
    struct {
        uint8_t *msg;
        uint32_t msglen;
    } update;
    struct {
        uint8_t *dgst;
        uint32_t dlen;
    } final;
    struct {
        uint8_t *msg;
        uint32_t msglen;
        uint8_t *dgst;
        uint32_t dlen;
    } fast;
};

static const struct hash_test_pattern
{
    uint32_t msglen;
    const void *msg;
    const void *md;
} hash_tp[] =
{
    {
        0,
        NULL,
        "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55",
    },
    {
        163,
        "\x45\x11\x01\x25\x0e\xc6\xf2\x66\x52\x24\x9d\x59\xdc\x97\x4b\x73\x61\xd5\x71\xa8\x10\x1c\xdf\xd3\x6a\xba\x3b\x58\x54\xd3\xae\x08\x6b\x5f\xdd\x45\x97\x72\x1b\x66\xe3\xc0\xdc\x5d\x8c\x60\x6d\x96\x57\xd0\xe3\x23\x28\x3a\x52\x17\xd1\xf5\x3f\x2f\x28\x4f\x57\xb8\x5c\x8a\x61\xac\x89\x24\x71\x1f\x89\x5c\x5e\xd9\x0e\xf1\x77\x45\xed\x2d\x72\x8a\xbd\x22\xa5\xf7\xa1\x34\x79\xa4\x62\xd7\x1b\x56\xc1\x9a\x74\xa4\x0b\x65\x5c\x58\xed\xfe\x0a\x18\x8a\xd2\xcf\x46\xcb\xf3\x05\x24\xf6\x5d\x42\x3c\x83\x7d\xd1\xff\x2b\xf4\x62\xac\x41\x98\x00\x73\x45\xbb\x44\xdb\xb7\xb1\xc8\x61\x29\x8c\xdf\x61\x98\x2a\x83\x3a\xfc\x72\x8f\xae\x1e\xda\x2f\x87\xaa\x2c\x94\x80\x85\x8b\xec",
        "\x3c\x59\x3a\xa5\x39\xfd\xcd\xae\x51\x6c\xdf\x2f\x15\x00\x0f\x66\x34\x18\x5c\x88\xf5\x05\xb3\x97\x75\xfb\x9a\xb1\x37\xa1\x0a\xa2",
    },
    {
        64,
        "\x45\x11\x01\x25\x0e\xc6\xf2\x66\x52\x24\x9d\x59\xdc\x97\x4b\x73\x61\xd5\x71\xa8\x10\x1c\xdf\xd3\x6a\xba\x3b\x58\x54\xd3\xae\x08\x6b\x5f\xdd\x45\x97\x72\x1b\x66\xe3\xc0\xdc\x5d\x8c\x60\x6d\x96\x57\xd0\xe3\x23\x28\x3a\x52\x17\xd1\xf5\x3f\x2f\x28\x4f\x57\xb8",
        "\x1a\xaa\xf9\x28\x5a\xf9\x45\xb8\xa9\x7c\xf1\x4f\x86\x9b\x18\x90\x14\xc3\x84\xf3\xc7\xc2\xb7\xd2\xdf\x8a\x97\x13\xbf\xfe\x0b\xf1",
    },
    {
        32,
        "\x45\x11\x01\x25\x0e\xc6\xf2\x66\x52\x24\x9d\x59\xdc\x97\x4b\x73\x61\xd5\x71\xa8\x10\x1c\xdf\xd3\x6a\xba\x3b\x58\x54\xd3\xae\x08",
        "\xb3\xad\xf1\x28\xb3\xb8\x83\x99\x92\x4a\x6b\x38\x88\x6a\x5b\xf1\x22\x65\xaa\xa8\x14\xce\xa9\xc1\x9b\x7f\x55\x8e\x4a\x74\xc8\x14",
    },
    {
        128,
        "\x45\x11\x01\x25\x0e\xc6\xf2\x66\x52\x24\x9d\x59\xdc\x97\x4b\x73\x61\xd5\x71\xa8\x10\x1c\xdf\xd3\x6a\xba\x3b\x58\x54\xd3\xae\x08\x6b\x5f\xdd\x45\x97\x72\x1b\x66\xe3\xc0\xdc\x5d\x8c\x60\x6d\x96\x57\xd0\xe3\x23\x28\x3a\x52\x17\xd1\xf5\x3f\x2f\x28\x4f\x57\xb8\x45\x11\x01\x25\x0e\xc6\xf2\x66\x52\x24\x9d\x59\xdc\x97\x4b\x73\x61\xd5\x71\xa8\x10\x1c\xdf\xd3\x6a\xba\x3b\x58\x54\xd3\xae\x08\x6b\x5f\xdd\x45\x97\x72\x1b\x66\xe3\xc0\xdc\x5d\x8c\x60\x6d\x96\x57\xd0\xe3\x23\x28\x3a\x52\x17\xd1\xf5\x3f\x2f\x28\x4f\x57\xb8",
        "\xa9\x9d\xb8\xc7\x02\x2f\xa9\x9e\x97\x62\x19\xae\x69\x96\x7a\x61\x6d\x23\xa5\x15\x4c\x1c\xa4\xca\xcd\x59\x85\xd9\x6b\x77\xe8\x82",
    },
};

int main(int argc, char *argv[])
{
    int fd;
    int ntests = sizeof(hash_tp) / sizeof(struct hash_test_pattern);
    int i, j;
    uint8_t out[32] = {0};
    union rt_hash_control_args ctl;

    fd = open(HASH_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        printf("open /dev/HWhash err!\n");
        return -1;
    }

    for(i=0; i<ntests; i++)
    {
        printf("case %d, msglen = %d\n", i, hash_tp[i].msglen);
        ctl.init.mode = SHA_256;
        // init
        if(ioctl(fd, RT_HASH_INIT, &ctl))
        {
            perror("ioctl");
            close(fd);
            return -1;
        }
        ctl.update.msg = (void *)hash_tp[i].msg;
        ctl.update.msglen = hash_tp[i].msglen;
        // update
        if(ioctl(fd, RT_HASH_UPDATE, &ctl))
        {
            perror("ioctl");
            close(fd);
            return -1;
        }
        ctl.final.dgst = (void *)out;
        ctl.final.dlen = 32;
        // finish
        if(ioctl(fd, RT_HASH_FINAL, &ctl))
        {
            perror("ioctl");
            close(fd);
            return -1;
        }

        if(memcmp(out, (void *)hash_tp[i].md, ctl.final.dlen) == 0)
        {
            printf("Success!\n");
        }
        else
        {
            printf("Fail!\n");
            printf("golden message digest       computed message digest\n\r");
            for(j=0; j<ctl.final.dlen; j++)
                printf("0x%x 0x%x\n", *(uint8_t *)(hash_tp[i].md + j), *(uint8_t *)(out + j));
        }

        memset(out, 0, 32);
    }

    close(fd);

    return 0;
}
