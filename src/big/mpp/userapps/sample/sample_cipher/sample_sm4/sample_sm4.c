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

#define AES_DEVICE_NAME                 "/dev/sm4"
#define RT_SM4_ENC                  _IOWR('S', 0, int)
#define RT_SM4_DEC                  _IOWR('S', 1, int)

static const struct sm4_test_pattern
{
    uint32_t keybits;
    const void* key;
    uint32_t ptlen;
    const void* pt;
    uint32_t ivlen;
    const void* iv;
    uint32_t ctlen;
    const void* ct;
} sm4_tp[] = 
{
    // ecb
    { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
      32, "\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb",
      0, NULL,
      32, "\x5e\xc8\x14\x3d\xe5\x09\xcf\xf7\xb5\x17\x9f\x8f\x47\x4b\x86\x19\x2f\x1d\x30\x5a\x7f\xb1\x7d\xf9\x85\xf8\x1c\x84\x82\x19\x23\x04",
    },

    // { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
    //   16, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
    //   0, NULL,
    //   16, "\x68\x1e\xdf\x34\xd2\x06\x96\x5e\x86\xb3\xe9\x4f\x53\x6e\x42\x46",
    // },

    // cfb
    { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
      32, "\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb",
      16, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
      32, "\xac\x32\x36\xcb\x86\x1d\xd3\x16\xe6\x41\x3b\x4e\x3c\x75\x24\xb7\x69\xd4\xc5\x4e\xd4\x33\xb9\xa0\x34\x60\x09\xbe\xb3\x7b\x2b\x3f",
    },

    // ofb
    { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
      32, "\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb",
      16, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
      32, "\xac\x32\x36\xcb\x86\x1d\xd3\x16\xe6\x41\x3b\x4e\x3c\x75\x24\xb7\x1d\x01\xac\xa2\x48\x7c\xa5\x82\xcb\xf5\x46\x3e\x66\x98\x53\x9b",
    },

    // cbc
    { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
      32, "\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb",
      16, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
      32, "\x78\xeb\xb1\x1c\xc4\x0b\x0a\x48\x31\x2a\xae\xb2\x04\x02\x44\xcb\x4c\xb7\x01\x69\x51\x90\x92\x26\x97\x9b\x0d\x15\xdc\x6a\x8f\x6d",
    },

    // ctr
    { 128, "\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",
      64, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xee\xee\xee\xee\xff\xff\xff\xff\xff\xff\xff\xff\xee\xee\xee\xee\xee\xee\xee\xee\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
      16, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
      64, "\xac\x32\x36\xcb\x97\x0c\xc2\x07\x91\x36\x4c\x39\x5a\x13\x42\xd1\xa3\xcb\xc1\x87\x8c\x6f\x30\xcd\x07\x4c\xce\x38\x5c\xdd\x70\xc7\xf2\x34\xbc\x0e\x24\xc1\x19\x80\xfd\x12\x86\x31\x0c\xe3\x7b\x92\x2a\x46\xb8\x94\xbe\xe4\xfe\xb7\x9a\x38\x22\x94\x0c\x93\x54\x05",
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
    uint32_t outlen;
    char *mode;
} aes_config_t;

int do_sm4(char *crypto_type)
{
    int fd;
    int i, j;
    void *in;
    void *out;
    int ntests = sizeof(sm4_tp) / sizeof(struct sm4_test_pattern);
    aes_config_t config = { .mode = crypto_type };
    int ret = 0;

    printf("##########################################################\n");
    printf("SM4-%s TEST DEMO\n", crypto_type);
    printf("##########################################################\n");

    fd = open(AES_DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        printf("open /dev/aes err!\n");
        return -1;
    }

    printf("************** SM4-%s ENCRYPT CASE **********************\n\t", crypto_type);
    for(i=0; i<ntests; i++)
    {
    #if 1
        if(strcmp(crypto_type, "ecb-sm4") == 0)
            i = 0;
        else if(strcmp(crypto_type, "cfb-sm4") == 0)
            i = 1;
        else if(strcmp(crypto_type, "ofb-sm4") == 0)
            i = 2;
        else if(strcmp(crypto_type, "cbc-sm4") == 0)
            i = 3;
        else if(strcmp(crypto_type, "ctr-sm4") == 0)
            i = 4;
        else
            return -1;
    #endif

        printf("case %d, pclen = %d\n", i, sm4_tp[i].ptlen);
        in = malloc(sm4_tp[i].ptlen);
        out = malloc(sm4_tp[i].ctlen);
        if(!in || !out) {
            perror("malloc");
            ret = -1;
            goto ret;
        }
        memcpy(in, sm4_tp[i].pt, sm4_tp[i].ptlen);
        memset(out, 0, sm4_tp[i].ctlen);

        config.key = (void *)sm4_tp[i].key;
        config.keybits = sm4_tp[i].keybits;
        config.iv = (void *)sm4_tp[i].iv;
        config.ivlen = sm4_tp[i].ivlen;
        config.pclen = sm4_tp[i].ptlen;
        config.outlen = 0;
        config.in = in;
        config.out = out;
        if(ioctl(fd, RT_SM4_ENC, &config))
        {
            perror("ioctl");
            goto enc_err;
        }

        // for(j=0; j<config.outlen; j++)
        //         printf("0x%x\n", *(uint8_t *)(config.out + j));

        if(memcmp(config.out, (void *)sm4_tp[i].ct, config.outlen) == 0)
        {
            printf("Success!\n");
            for(j=0; j<config.outlen; j++)
                printf("0x%x\n", *(uint8_t *)(config.out + j));
        }
        else
        {
            printf("Fail!\n");
            printf("golden message digest       computed message digest\n\r");
            for(j=0; j<config.outlen; j++)
                printf("0x%x 0x%x\n", *(uint8_t *)(sm4_tp[i].ct + j), *(uint8_t *)(config.out + j));
        }
        free(in);
        free(out);
        break;
    }

    // decrypt
    printf("************** SM4-%s DECRYPT CASE **********************\n\t", crypto_type);
    for(i=0; i<ntests; i++)
    {
    #if 1
        if(strcmp(crypto_type, "ecb-sm4") == 0)
            i = 0;
        else if(strcmp(crypto_type, "cfb-sm4") == 0)
            i = 1;
        else if(strcmp(crypto_type, "ofb-sm4") == 0)
            i = 2;
        else if(strcmp(crypto_type, "cbc-sm4") == 0)
            i = 3;
        else if(strcmp(crypto_type, "ctr-sm4") == 0)
            i = 4;
        else
            return -1;
    #endif

        printf("case %d, ctlen = %d\n", i, sm4_tp[i].ctlen);
        in = malloc(sm4_tp[i].ctlen);
        out = malloc(sm4_tp[i].ptlen);
        if(!in || !out) {
            perror("malloc");
            ret = -1;
            goto ret;
        }
        memcpy(in, sm4_tp[i].ct, sm4_tp[i].ctlen);
        memset(out, 0, sm4_tp[i].ptlen);

        config.key = (void *)sm4_tp[i].key;
        config.keybits = sm4_tp[i].keybits;
        config.iv = (void *)sm4_tp[i].iv;
        config.ivlen = sm4_tp[i].ivlen;
        config.pclen = sm4_tp[i].ctlen;
        config.outlen = 0;
        config.in = in;
        config.out = out;
        if(ioctl(fd, RT_SM4_DEC, &config))
        {
            perror("ioctl");
            goto enc_err;
        }

        // for(j=0; j<config.outlen; j++)
        //         printf("0x%x\n", *(uint8_t *)(config.out + j));

        if(memcmp(config.out, (void *)sm4_tp[i].pt, config.outlen) == 0)
        {
            printf("Success!\n");
            for(j=0; j<config.outlen; j++)
                printf("0x%x\n", *(uint8_t *)(config.out + j));
        }
        else
        {
            printf("Fail!\n");
            printf("golden message digest       computed message digest\n\r");
            for(j=0; j<config.outlen; j++)
                printf("0x%x 0x%x\n", *(uint8_t *)(sm4_tp[i].pt + j), *(uint8_t *)(config.out + j));
        }
        free(in);
        free(out);
        break;
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
    -t  crypto type: ecb-sm4/cfb-sm4/ofb-sm4/cbc-sm4/ctr-sm4\n");
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

    if((strcmp(crypto_type, "ecb-sm4") == 0) || \
        (strcmp(crypto_type, "cfb-sm4") == 0) || \
        (strcmp(crypto_type, "ofb-sm4") == 0) || \
        (strcmp(crypto_type, "cbc-sm4") == 0) || \
        (strcmp(crypto_type, "ctr-sm4") == 0))
        ret = do_sm4(crypto_type);
	else
		printf("Crypto type input error!\n");

	if(ret<0) {
		printf("Crypto computing error!\n");
		return ret;
	}

    return 0;
}
