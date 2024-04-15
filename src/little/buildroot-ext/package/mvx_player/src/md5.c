/*
 * Copyright:
 * ----------------------------------------------------------------------------
 * This confidential and proprietary software may be used only as authorized
 * by a licensing agreement from Arm Technology (China) Co., Ltd.
 *      (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 * The entire notice above must be reproduced on all authorized copies and
 * copies may only be made to the extent permitted by a licensing agreement
 * from Arm Technology (China) Co., Ltd.
 * ----------------------------------------------------------------------------
 */

#include <stdint.h>
#include <unistd.h>
#include "md5.h"
#include <string.h>

/* Table taken from RFC1321 */
uint32_t const sbox[] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

/* Table taken from RFC1321 */
uint32_t const K[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

uint8_t const Z[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void MD5_Init(MD5_CTX *ctx)
{
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;

    ctx->nValidData = 0;
    ctx->nLength = 0;
}

#define lr(x, c) (((x) << (c)) | ((x) >> (32 - (c))));

#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow", "signed-integer-overflow")))
#endif
void MD5_Update(MD5_CTX *ctx, const void *data, size_t len)
{
    uint8_t const *p = (uint8_t const *)data;
    uint32_t remain = len;
    uint8_t *d = &ctx->data[0];
    uint32_t s;

    while (0 != remain)
    {
        s = MIN(remain, MD5_CHUNK_LENGTH - ctx->nValidData);
        memcpy(d + ctx->nValidData, p, s);

        remain -= s;
        ctx->nValidData += s;
        ctx->nLength += s;
        p += s;

        if (ctx->nValidData == MD5_CHUNK_LENGTH)
        {
            /* Time to update the hash */
            uint32_t *w = (uint32_t *)d;
            uint32_t A = ctx->A;
            uint32_t B = ctx->B;
            uint32_t C = ctx->C;
            uint32_t D = ctx->D;
            uint32_t dT;
            uint32_t i, g;
            uint32_t F;
            uint32_t W;

            for (i = 0; i < 64; ++i)
            {
                if (i < 16)
                {
                    F = (B & C) | ((~B) & D);
                    g = i;
                }
                else if (i < 32)
                {
                    F = (D & B) | ((~D) & C);
                    g = (5 * i + 1) % 16;
                }
                else if (i < 48)
                {
                    F = B ^ C ^ D;
                    g = (3 * i + 5) % 16;
                }
                else
                {
                    F = C ^ (B | (~D));
                    g = (7 * i) % 16;
                }

                dT = D;
                D = C;
                C = B;
                W = w[g];
                B = B + lr((A + F + K[i] + W), sbox[i]);
                A = dT;
            }

            ctx->A = ctx->A + A;
            ctx->B = ctx->B + B;
            ctx->C = ctx->C + C;
            ctx->D = ctx->D + D;

            /* Consumed the bytes in the chunk. Reset */
            ctx->nValidData = 0;
        }
    }
}

void MD5_Finalize(MD5_CTX *ctx)
{
    uint64_t length;

    length = ctx->nLength * 8; /* 64bit value, original length in _bits_ */

    ctx->data[ctx->nValidData++] = 0x80;

    if ((MD5_CHUNK_LENGTH - ctx->nValidData) < 8)
    {
        MD5_Update(ctx, Z, MD5_CHUNK_LENGTH - ctx->nValidData);
    }

    if (MD5_CHUNK_LENGTH - ctx->nValidData > 8)
    {
        MD5_Update(ctx, Z, MD5_CHUNK_LENGTH - ctx->nValidData - 8);
    }

    MD5_Update(ctx, &length, sizeof(uint64_t));
}

void MD5_GetHash(MD5_CTX *ctx, uint8_t *digest)
{
    uint32_t *d = (uint32_t *)digest;
    d[0] = (ctx->A);
    d[1] = (ctx->B);
    d[2] = (ctx->C);
    d[3] = (ctx->D);
}

void MD5_Final(unsigned char *md, MD5_CTX *c)
{
    MD5_Finalize(c);
    MD5_GetHash(c, (uint8_t *)md);
}

