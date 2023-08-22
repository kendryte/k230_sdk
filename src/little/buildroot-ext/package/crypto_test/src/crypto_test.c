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
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_alg.h>

#define AES_TEST_LEN 256

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
      51, "\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20\xff\x22\x19\xd1\x4e\x8d\x63\x1b\x38\x72\x26\x5c\xf1\x17\xee\x86\x75\x7a\xcc\xb1\x58\xbd\x9a\xbb\x38\x68\xfd\xc0\xd0\xb0\x74\xb5\xf0\x1b\x2c",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      51, "\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xb8\x74\xff\xbf\x1d\x65\xc6\xf6\x4a\x69\x8d\x83\x9b\x0b\x06\x14\x5d\xae\x82\x05\x7a\xd5\x59\x94\xcf\x59\xad\x7f\x67\xc0\xfa\x5e\x85\xfa\xb8",
      16, "\xbc\x95\xc5\x32\xfe\xcc\x59\x4c\x36\xd1\x55\x02\x86\xa7\xa3\xf0",
    },
	
	{ 256, "\xb5\x2c\x50\x5a\x37\xd7\x8e\xda\x5d\xd3\x4f\x20\xc2\x25\x40\xea\x1b\x58\x96\x3c\xf8\xe5\xbf\x8f\xfa\x85\xf9\xf2\x49\x25\x05\xb4",
      0, "",
      12, "\x51\x6c\x33\x92\x9d\xf5\xa3\x28\x4f\xf4\x63\xd7",
      0, "",
      16, "\xbd\xc1\xac\x88\x4d\x33\x24\x57\xa1\xd2\x66\x4f\x16\x8c\x76\xf0",
      16, "\xbd\xc1\xac\x88\x4d\x33\x24\x57\xa1\xd2\x66\x4f\x16\x8c\x76\xf0",
    },

	{ 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      1, "\x27",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      17, "\xeb\x63\x35\xe1\xd4\x9e\x89\x88\xea\xc4\x8e\x42\x19\x4e\x5f\x56\xdb",
      16, "\x63\x35\xe1\xd4\x9e\x89\x88\xea\xc4\x8e\x42\x19\x4e\x5f\x56\xdb",
    },

	{ 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      16, "\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      32, "\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xd3\xaa\x97\x58\x15\xa0\x51\xd2\x5b\x7f\x29\xf1\x69\x6f\x6f\x21",
      16, "\xd3\xaa\x97\x58\x15\xa0\x51\xd2\x5b\x7f\x29\xf1\x69\x6f\x6f\x21",
    },

	{ 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      17, "\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20\xff",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      33, "\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xb8\x9a\x5d\x58\xd0\xc9\xf8\x48\x96\xb3\xe4\x67\x4a\xa9\x04\xad\x97",
      16, "\x9a\x5d\x58\xd0\xc9\xf8\x48\x96\xb3\xe4\x67\x4a\xa9\x04\xad\x97",
    },

	{ 256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
      52, "\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20\xff\x22\x19\xd1\x4e\x8d\x63\x1b\x38\x72\x26\x5c\xf1\x17\xee\x86\x75\x7a\xcc\xb1\x58\xbd\x9a\xbb\x38\x68\xfd\xc0\xd0\xb0\x74\xb5\xf0\x1b\x2c\x99",
      12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
      20, "\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef",
      68, "\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xb8\x74\xff\xbf\x1d\x65\xc6\xf6\x4a\x69\x8d\x83\x9b\x0b\x06\x14\x5d\xae\x82\x05\x7a\xd5\x59\x94\xcf\x59\xad\x7f\x67\xc0\xfa\x5e\x85\xfa\xb8\x46\x54\x5d\xc1\xfa\x62\x39\x58\x88\xc6\xc7\x91\xdd\x94\x15\x93\xda",
      16, "\x54\x5d\xc1\xfa\x62\x39\x58\x88\xc6\xc7\x91\xdd\x94\x15\x93\xda",
    },
	
};

int socket_aes_gcm_cipher(const uint8_t *key,
						uint32_t keybits,
						unsigned int op,
						const uint8_t *in,
						uint32_t inlen,
						const uint8_t *aad,
						uint32_t aadlen,
						const uint8_t *iv,
						uint32_t ivlen,
						const uint8_t *tag,
						uint32_t taglen,
						uint8_t *out)
{
	int tfmfd;
	int opfd;
	int cmsg_size;
	int out_msg_len = (op == ALG_OP_DECRYPT) ? inlen : (inlen + taglen);
	unsigned char *buf = (unsigned char *)malloc(out_msg_len + 256);

	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type = "aead",
		.salg_name = "gcm(aes)",
	};

	// create and bind socket
	tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	// printf("open socket\n");
	bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

	//set socket options: key, AEAD Authentication size
	setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY, key, (keybits >> 3));
	setsockopt(tfmfd, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL, 16);
	//accept connection
	opfd = accept(tfmfd, NULL, 0);

	//Prepare Message
	struct msghdr msg = {};
	struct cmsghdr *cmsg;
	struct iovec iov[2];

	cmsg_size = CMSG_SPACE(4);
	cmsg_size += aadlen ? CMSG_SPACE(4) : 0;
	cmsg_size += ivlen ? CMSG_SPACE(sizeof(struct af_alg_iv) + ivlen) : 0;
    char *cbuf = (char *)malloc(cmsg_size);
    memset(cbuf, 0, cmsg_size);

	msg.msg_control = cbuf;
	msg.msg_controllen = cmsg_size;
	msg.msg_iov = iov;

	//set the Headervalues for the Operation
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(4);
	*(__u32 *)CMSG_DATA(cmsg) = op;

	//set headervalues for aad
	if(aadlen)
	{
		cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = SOL_ALG;
        cmsg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
        cmsg->cmsg_len = CMSG_LEN(4);
		unsigned int ad_data = (void *)CMSG_DATA(cmsg);
        *(unsigned int *)CMSG_DATA(cmsg) = aadlen;

		iov[0].iov_base = (void *)aad;
        iov[0].iov_len = aadlen;
        iov[1].iov_base = (void *)in;
        iov[1].iov_len = inlen;
        msg.msg_iovlen = 2;
	}
	else
	{
		iov[0].iov_base = (void *)in;
		iov[0].iov_len = inlen;
		msg.msg_iovlen = 1;
	}

	//set headervalues for iv
	if(ivlen)
	{
		struct af_alg_iv *algiv;

		cmsg = CMSG_NXTHDR(&msg, cmsg);
		cmsg->cmsg_level = SOL_ALG;
		cmsg->cmsg_type = ALG_SET_IV;
		cmsg->cmsg_len = CMSG_LEN(sizeof(*algiv) + ivlen);

		algiv = (void *)CMSG_DATA(cmsg);
		algiv->ivlen = ivlen;
		memcpy(algiv->iv, iv, ivlen);
	}

	// send message
	// sendmsg(opfd, &msg, 0);
	sendmsg(opfd, &msg, MSG_DONTWAIT);

	if (aadlen)
	{
		iov[0].iov_base = malloc(aadlen);
		iov[0].iov_len = aadlen;
		iov[1].iov_base = (void *)buf;
		iov[1].iov_len = out_msg_len;
		msg.msg_iovlen = 2;

		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		// receive message
		// recvmsg(opfd, &msg, 0);
		recvmsg(opfd, &msg, MSG_DONTWAIT);

		free(iov[0].iov_base);
	}
	else
	{
		// read result into buf
	    read(opfd, buf, out_msg_len);
    }

	memcpy(out, buf, (out_msg_len) * sizeof(unsigned char));

	free(buf);
    free(cbuf);
	close(opfd);
	close(tfmfd);
	
	return 0;
}

int main(void)
{
	int i, j;
	int ntests = sizeof(aes_gcm_tp) / sizeof(struct gcm_test_pattern);
	uint8_t out[AES_TEST_LEN];

#if 1
	printf("GCM_ENCRYPT......\n");

	for(i=0; i<ntests-5; i++)
	{
		/* aes gcm test */
		socket_aes_gcm_cipher(aes_gcm_tp[i].key, aes_gcm_tp[i].keybits,
							ALG_OP_ENCRYPT,
							aes_gcm_tp[i].pt, aes_gcm_tp[i].ptlen,
							aes_gcm_tp[i].aad, aes_gcm_tp[i].aadlen, 
							aes_gcm_tp[i].iv, aes_gcm_tp[i].ivlen,
							aes_gcm_tp[i].tag, aes_gcm_tp[i].taglen,
							out);

		if((memcmp(out, aes_gcm_tp[i].ct, aes_gcm_tp[i].ctlen) == 0) && (memcmp(out+aes_gcm_tp[i].ctlen, aes_gcm_tp[i].tag, aes_gcm_tp[i].taglen) == 0))
		{
			printf("gcm encrypt ok\n");
		#if 0
			printf("golden ciphertext	computed ciphertext\n\r");
			for(j=0; j<aes_gcm_tp[i].ctlen; j++)
			{
				printf("0x%x	0x%x\n", *(uint8_t *)(aes_gcm_tp[i].ct + j), out[j]);
			}

			printf("golden tag	computed tag\n\r");
			for(j=0; j<aes_gcm_tp[i].taglen; j++)
			{
				printf("0x%x	0x%x\n", *(uint8_t *)(aes_gcm_tp[i].tag + j), *(uint8_t *)(out+aes_gcm_tp[i].ctlen + j));
			}
		#endif
		}
		else
		{
			printf("gcm encrypt fail\n");
			printf("golden ciphertext	computed ciphertext:\n\r");
			for(j=0; j<aes_gcm_tp[i].ctlen; j++)
			{
				printf("0x%x 0x%x\n", *(uint8_t *)(aes_gcm_tp[i].ct + j), out[j]);
			}

			printf("golden tag	computed tag\n\r");
			for(j=0; j<aes_gcm_tp[i].taglen; j++)
			{
				printf("0x%x	0x%x\n", *(uint8_t *)(aes_gcm_tp[i].tag + j), *(uint8_t *)(out+aes_gcm_tp[i].ctlen + j));
			}
		}
	}
#endif

#if 1
	printf("GCM_DECRYPT......\n");

	for(i=2; i<ntests; i++)
	{
		/* aes gcm test */
		socket_aes_gcm_cipher(aes_gcm_tp[i].key, aes_gcm_tp[i].keybits,
							ALG_OP_DECRYPT,
							aes_gcm_tp[i].ct, aes_gcm_tp[i].ctlen,
							aes_gcm_tp[i].aad, aes_gcm_tp[i].aadlen, 
							aes_gcm_tp[i].iv, aes_gcm_tp[i].ivlen,
							aes_gcm_tp[i].tag, aes_gcm_tp[i].taglen,
							out);

		printf("%s-%d\n", __func__, __LINE__);

		if(memcmp(out, aes_gcm_tp[i].pt, aes_gcm_tp[i].ptlen) == 0)
		{
			printf("gcm decrypt ok\n");
		#if 0
			printf("golden plaintext	computed plaintext\n\r");
			for(j=0; j<aes_gcm_tp[i].ptlen; j++)
			{
				printf("0x%x	0x%x\n", *(uint8_t *)(aes_gcm_tp[i].pt + j), out[j]);
			}
		#endif
		}
		else
		{
			printf("gcm decrypt fail\n");
			printf("golden plaintext	computed plaintext\n\r");
			for(j=0; j<aes_gcm_tp[i].ptlen; j++)
			{
				printf("0x%x 0x%x\n", *(uint8_t *)(aes_gcm_tp[i].pt + j), out[j]);
			}
		}
	}
#endif

	return 0;
}