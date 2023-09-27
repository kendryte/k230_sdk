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

#define AES_DEVICE_NAME                 "/dev/aes"
#define RT_AES_GCM_ENC              _IOWR('G', 0, int)
#define RT_AES_GCM_DEC              _IOWR('G', 1, int)
#define RT_AES_GCM_UPDATE           _IOWR('G', 2, int)
#define RT_AES_GCM_FINISH           _IOWR('G', 3, int)

static const struct gcm_test_pattern
{
    uint32_t keybits;
	const void *key;
	uint32_t ptlen;
	const void *pt;
	uint32_t ivlen;
	const void *iv;
	uint32_t aadlen;
	const void *aad;
	uint32_t ctlen;
	const void *ct;
	uint32_t taglen;
	const void *tag;
} aes_gcm_tp[] =
{
    { 256, "\xb5\x2c\x50\x5a\x37\xd7\x8e\xda\x5d\xd3\x4f\x20\xc2\x25\x40\xea\x1b\x58\x96\x3c\xf8\xe5\xbf\x8f\xfa\x85\xf9\xf2\x49\x25\x05\xb4",
      0, "",
      12, "\x51\x6c\x33\x92\x9d\xf5\xa3\x28\x4f\xf4\x63\xd7",
      0, "",
      0, "",
      16, "\xbd\xc1\xac\x88\x4d\x33\x24\x57\xa1\xd2\x66\x4f\x16\x8c\x76\xf0",
    },

    { 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      1, "\x27",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      1, "\xeb",
      16, "\x63\x35\xe1\xd4\x9e\x89\x88\xea\xc4\x8e\x42\x19\x4e\x5f\x56\xdb",
    },

    { 256, "\x1f\xde\xd3\x2d\x59\x99\xde\x4a\x76\xe0\xf8\x08\x21\x08\x82\x3a\xef\x60\x41\x7e\x18\x96\xcf\x42\x18\xa2\xfa\x90\xf6\x32\xec\x8a",
      51, "\x06\xb2\xc7\x58\x53\xdf\x9a\xeb\x17\xbe\xfd\x33\xce\xa8\x1c\x63\x0b\x0f\xc5\x36\x67\xff\x45\x19\x9c\x62\x9c\x8e\x15\xdc\xe4\x1e\x53\x0a\xa7\x92\xf7\x96\xb8\x13\x8e\xea\xb2\xe8\x6c\x7b\x7b\xee\x1d\x40\xb0",
      12, "\x1f\x3a\xfa\x47\x11\xe9\x47\x4f\x32\xe7\x04\x62",
      0, "",
      51, "\x91\xfb\xd0\x61\xdd\xc5\xa7\xfc\xc9\x51\x3f\xcd\xfd\xc9\xc3\xa7\xc5\xd4\xd6\x4c\xed\xf6\xa9\xc2\x4a\xb8\xa7\x7c\x36\xee\xfb\xf1\xc5\xdc\x00\xbc\x50\x12\x1b\x96\x45\x6c\x8c\xd8\xb6\xff\x1f\x8b\x3e\x48\x0f",
      16, "\x30\x09\x6d\x34\x0f\x3d\x5c\x42\xd8\x2a\x6f\x47\x5d\xef\x23\xeb",
    },

    { 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      51, "\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20\xff\x22\x19\xd1\x4e\x8d\x63\x1b\x38\x72\x26\x5c\xf1\x17\xee\x86\x75\x7a\xcc\xb1\x58\xbd\x9a\xbb\x38\x68\xfd\xc0\xd0\xb0\x74\xb5\xf0\x1b\x2c",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      51, "\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xb8\x74\xff\xbf\x1d\x65\xc6\xf6\x4a\x69\x8d\x83\x9b\x0b\x06\x14\x5d\xae\x82\x05\x7a\xd5\x59\x94\xcf\x59\xad\x7f\x67\xc0\xfa\x5e\x85\xfa\xb8",
      16, "\xbc\x95\xc5\x32\xfe\xcc\x59\x4c\x36\xd1\x55\x02\x86\xa7\xa3\xf0",
    },
};

typedef struct
{
    void *key;
    void *iv;
    void *in;
    void *out;
    uint32_t keybits;
    uint32_t ivlen;
    uint32_t pclen;
    uint32_t aadlen;
    uint32_t taglen;
    uint32_t outlen;
} aes_config_t;

int do_aes_gcm(char *crypto_type)
{
    int fd;
    int i, j;
    void *in;
    void *out;
    int ntests = sizeof(aes_gcm_tp) / sizeof(struct gcm_test_pattern);
    aes_config_t config;
    int ret = 0;

    printf("##########################################################\n");
    printf("AES-GCM TEST DEMO\n");
    printf("##########################################################\n");

    fd = open(AES_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        printf("open /dev/aes err!\n");
        return -1;
    }

    printf("************** GCM ENCRYPT CASE **********************\n\t");
    for(i=0; i<ntests; i++)
    {
        printf("case %d, pclen = %d\n", i, aes_gcm_tp[i].ptlen);
        in = malloc(aes_gcm_tp[i].aadlen + aes_gcm_tp[i].ptlen);
        out = malloc(aes_gcm_tp[i].aadlen + aes_gcm_tp[i].ctlen + aes_gcm_tp[i].taglen);
        if(!in || !out) {
            perror("malloc");
            ret = -1;
            goto ret;
        }
        memcpy(in, aes_gcm_tp[i].aad, aes_gcm_tp[i].aadlen);
        memcpy((in + aes_gcm_tp[i].aadlen), aes_gcm_tp[i].pt, aes_gcm_tp[i].ptlen);

        config.key = (void *)aes_gcm_tp[i].key;
        config.keybits = aes_gcm_tp[i].keybits;
        config.iv = (void *)aes_gcm_tp[i].iv;
        config.ivlen = aes_gcm_tp[i].ivlen;
        config.aadlen = aes_gcm_tp[i].aadlen;
        config.pclen = aes_gcm_tp[i].ptlen;
        config.taglen = 0;
        config.outlen = 0;
        config.in = in;
        config.out = out;
        // encrypt
        if(ioctl(fd, RT_AES_GCM_ENC, &config))
        {
            perror("ioctl");
            goto enc_err;
        }

        // for(j=0; j<config.outlen; j++)
        //         printf("0x%x\n", *(uint8_t *)(config.out + j));

        if((memcmp((config.out + config.aadlen), (void *)aes_gcm_tp[i].ct, config.pclen) == 0) && (memcmp((config.out + config.aadlen + config.pclen), (void *)aes_gcm_tp[i].tag, config.taglen) == 0))
        {
            printf("Success!\n");
            for(j=0; j<(config.outlen - config.aadlen - config.taglen); j++)
                printf("0x%x\n", *(uint8_t *)(config.out + config.aadlen + j));
        }
        else
        {
            printf("Fail!\n");
            printf("golden message digest       computed message digest\n\r");
            for(j=0; j<(config.outlen - config.aadlen - config.taglen); j++)
                printf("0x%x 0x%x\n", *(uint8_t *)(aes_gcm_tp[i].ct + j), *(uint8_t *)(config.out + config.aadlen + j));
        }
        free(in);
        free(out);
    }

    // decrypt
    printf("************** GCM DECRYPT CASE **********************\n\t");
    for(i=0; i<ntests; i++)
    {
        printf("case %d, ctlen = %d\n", i, aes_gcm_tp[i].ctlen);
        in = malloc(aes_gcm_tp[i].aadlen + aes_gcm_tp[i].ctlen + aes_gcm_tp[i].taglen);
        out = malloc(aes_gcm_tp[i].aadlen + aes_gcm_tp[i].ptlen);
        if(!in || !out) {
            perror("malloc");
            ret = -1;
            goto ret;
        }
        memcpy(in, aes_gcm_tp[i].aad, aes_gcm_tp[i].aadlen);
        memcpy((in + aes_gcm_tp[i].aadlen), aes_gcm_tp[i].ct, aes_gcm_tp[i].ctlen);
        memcpy((in + aes_gcm_tp[i].aadlen + aes_gcm_tp[i].ctlen), aes_gcm_tp[i].tag, aes_gcm_tp[i].taglen);

        config.key = (void *)aes_gcm_tp[i].key;
        config.keybits = aes_gcm_tp[i].keybits;
        config.iv = (void *)aes_gcm_tp[i].iv;
        config.ivlen = aes_gcm_tp[i].ivlen;
        config.aadlen = aes_gcm_tp[i].aadlen;
        config.pclen = aes_gcm_tp[i].ctlen;
        config.taglen = aes_gcm_tp[i].taglen;
        config.outlen = 0;
        config.in = in;
        config.out = out;
        if(ioctl(fd, RT_AES_GCM_DEC, &config))
        {
            perror("ioctl");
            goto enc_err;
        }

        // for(j=0; j<config.outlen; j++)
        //         printf("0x%x\n", *(uint8_t *)(config.out + j));

        if(memcmp((config.out + config.aadlen), (void *)aes_gcm_tp[i].pt, config.pclen) == 0)
        {
            printf("Success!\n");
            for(j=0; j<(config.outlen - config.aadlen); j++)
                printf("0x%x\n", *(uint8_t *)(config.out + config.aadlen + j));
        }
        else
        {
            printf("Fail!\n");
            printf("golden message digest       computed message digest\n\r");
            for(j=0; j<(config.outlen - config.aadlen); j++)
                printf("0x%x 0x%x\n", *(uint8_t *)(aes_gcm_tp[i].pt + j), *(uint8_t *)(config.out + config.aadlen + j));
        }

        free(in);
        free(out);
    }

ret:
    close(fd);
    return ret;

enc_err:
    free(in);
    free(out);
    ret = -1;
    goto ret;
}

static void show_help(void)
{
	printf("\
Usage: ./crypto_demo [-ht] ...\n\
    -h  display help\n\
    -t  crypto type: gcm-aes\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int opt;
    char *crypto_type;

    if(argc <= 1) {
        printf("Input param error!\n");
		show_help();
		exit(-1);
    }

    while((opt = getopt(argc, argv, "ht:")) != -1)
	{
		switch(opt)
		{
			case 't':
				crypto_type = optarg;
				break;
			case 'h':
				show_help();
				exit(0);
			case '?':
				break;
		}
	}

    if(strcmp(crypto_type, "gcm-aes") == 0)
		ret = do_aes_gcm(crypto_type);
	else
		printf("Crypto type input error!\n");

	if(ret<0) {
		printf("Crypto computing error!\n");
		return ret;
	}

    return 0;
}
