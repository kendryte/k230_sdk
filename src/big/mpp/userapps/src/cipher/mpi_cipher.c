/**
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#include <string.h>

#include "k_cipher_comm.h"

/*  Internal Funciton.
    see Section 4.1.2 in NIST.FIPS.180-4

    ROTR(x, n)=((x >> n) | (x << (32 - n)))             ----Rotate right (circular right shift) operation
    SHR(x, n)=(x >> n)                                  ----Right shift operation

    Ch(x,y,z) = (x ∧ y) ⊕ (¬x ∧ z)                     ----choose function
    Maj(x,y,z) = (x ∧ y) ⊕ (x ∧ z) ⊕ (y ∧ z)          ----majority choose
    Σ0(x) = ROTR^2(x) ⊕ ROTR^13(x) ⊕ ROTR^22(x)       ----bitwise right shift operation
    Σ1(x) = ROTR^6(x) ⊕ ROTR^11(x) ⊕ ROTR^25(x)
    σ0(x) = ROTR^7(x) ⊕ ROTR^18(x) ⊕ SHR^3(x)
    σ1(x) = ROTR^17(x) ⊕ ROTR^19(x) ⊕ SHR^10(x)
*/
#define ROTR(x, n)              ((x >> n) | (x << (32 - n)))
#define SHR(x, n)               (x >> n)
#define CH(x, y, z)             ((x & y) ^ (~x & z))
#define MAJ(x, y, z)            ((x & y) ^ (x & z) ^ (y & z))
#define EP0(x)                  (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x)                  (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIGMA0(x)               (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define SIGMA1(x)               (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))


// define constant
static const k_u32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void addbits(k_sha256_ctx *ctx, k_u32 n)
{
    if (ctx->bits[0] > (0xFFFFFFFF - n))
    {
        ctx->bits[1] = (ctx->bits[1] + 1) & 0xFFFFFFFF;
    }
    ctx->bits[0] = (ctx->bits[0] + n) & 0xFFFFFFFF;
}

// logic operation
static void sha256_transform(k_sha256_ctx *ctx)
{
    // eight working variables, message schedule, Temp variables
    k_u32 a, b, c, d, e, f, g, h;
    k_u32 T1, T2;
    k_u32 W[64];
    k_u32 i, j;

    a = ctx->hash[0];
    b = ctx->hash[1];
    c = ctx->hash[2];
    d = ctx->hash[3];
    e = ctx->hash[4];
    f = ctx->hash[5];
    g = ctx->hash[6];
    h = ctx->hash[7];

    for(i=0,j=0; i<16; i++, j+=4)
    {
        W[i] = (ctx->data[j] << 24) | (ctx->data[j + 1] << 16) | (ctx->data[j + 2] << 8) | (ctx->data[j + 3] << 0);
    }
    for(; i<64; i++)
    {
        W[i] = SIGMA1(W[i - 2])  + W[i - 7] + SIGMA0(W[i - 15]) + W[i - 16];
    }

    for(i=0; i<64; i++)
    {
        T1 = h + EP1(e) + CH(e, f, g) + K[i] + W[i];
        T2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    // update hash value
    ctx->hash[0] += a;
    ctx->hash[1] += b;
    ctx->hash[2] += c;
    ctx->hash[3] += d;
    ctx->hash[4] += e;
    ctx->hash[5] += f;
    ctx->hash[6] += g;
    ctx->hash[7] += h;
}


// context init
void sha256_init(k_sha256_ctx *ctx)
{
    if (NULL != ctx) {
        ctx->bits[0] = ctx->bits[1] = ctx->datalen = 0;
        ctx->hash[0] = 0x6a09e667;
        ctx->hash[1] = 0xbb67ae85;
        ctx->hash[2] = 0x3c6ef372;
        ctx->hash[3] = 0xa54ff53a;
        ctx->hash[4] = 0x510e527f;
        ctx->hash[5] = 0x9b05688c;
        ctx->hash[6] = 0x1f83d9ab;
        ctx->hash[7] = 0x5be0cd19;
    }
}

// update the message, kdivide the message into chunks
void sha256_update(k_sha256_ctx *ctx, const void *data, size_t len)
{
    k_u32 i;
    const k_u8 *val = (const k_u8 *)data;

    if ((NULL != ctx) && (NULL != ctx) && (ctx->datalen < sizeof(ctx->data)))
    {
        for (i=0; i<len; i++)
        {
            ctx->data[ctx->datalen++] = val[i];
            if (sizeof(ctx->data) == ctx->datalen)
            {
                sha256_transform(ctx);
                addbits(ctx, sizeof(ctx->data) * 8);
                ctx->datalen = 0;
            }
        }
    }
}

// process the final incomplete block
void sha256_final(k_sha256_ctx *ctx, k_u8 *hash)
{
    k_u32 i, j;

    if (NULL != ctx)
    {
        i = ctx->datalen;
        
        // padding the '1' and '0' in data[]
        if(ctx->datalen < 56)
        {
            ctx->data[i++] = 0x80;
            while(i < 56)
            {
                ctx->data[i++] = 0x00;
            }
        }
        else
        {
            ctx->data[i++] = 0x80;
            while(i < 64)
            {
                ctx->data[i++] = 0x00;
            }

            sha256_transform(ctx);
            memset(ctx->data, 0, 56);
        }

        // Append padding.
        addbits(ctx, ctx->datalen * 8);
        ctx->data[63] = SHR(ctx->bits[0],  0);
        ctx->data[62] = SHR(ctx->bits[0],  8);
        ctx->data[61] = SHR(ctx->bits[0], 16);
        ctx->data[60] = SHR(ctx->bits[0], 24);
        ctx->data[59] = SHR(ctx->bits[1],  0);
        ctx->data[58] = SHR(ctx->bits[1],  8);
        ctx->data[57] = SHR(ctx->bits[1], 16);
        ctx->data[56] = SHR(ctx->bits[1], 24);
        sha256_transform(ctx);

        // our CPU is little endian byte ordering and SHA256 use big endian
        if(NULL != hash)
        {
            for(i=0; i<4; i++)
            {
                hash[i + 0] = (SHR(ctx->hash[0], (24 - i * 8)) & 255);
                hash[i + 4] = (SHR(ctx->hash[1], (24 - i * 8)) & 255);
                hash[i + 8] = (SHR(ctx->hash[2], (24 - i * 8)) & 255);
                hash[i + 12] = (SHR(ctx->hash[3], (24 - i * 8)) & 255);
                hash[i + 16] = (SHR(ctx->hash[4], (24 - i * 8)) & 255);
                hash[i + 20] = (SHR(ctx->hash[5], (24 - i * 8)) & 255);
                hash[i + 24] = (SHR(ctx->hash[6], (24 - i * 8)) & 255);
                hash[i + 28] = (SHR(ctx->hash[7], (24 - i * 8)) & 255);
            }
        }
    }
}


k_s32 kd_mpi_cipher_sha256(const void *data, k_u32 len, k_u8 *hash)
{
    k_sha256_ctx ctx;
    k_s32 ret = K_ERR_OK;

    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, hash);

    return ret;
}