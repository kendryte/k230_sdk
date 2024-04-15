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

#ifndef __MD5_H__
#define __MD5_H__

#ifdef __cplusplus
extern "C" {
#endif


/* 512 bit chunks. Divide by eight for bytes. */
#define MD5_CHUNK_LENGTH (512 >> 3)


#ifndef MIN
#define MIN( x, y )            ((x) < (y) ? (x) : (y))
#endif


typedef struct MD5_CTX
{
    uint32_t A, B, C, D;
    uint8_t data[MD5_CHUNK_LENGTH];
    uint32_t nValidData;
    uint32_t nLength;
} MD5_CTX;

void MD5_Init(MD5_CTX *ctx);
void MD5_Update(MD5_CTX *ctx, const void *data, size_t len);
void MD5_Finalize(MD5_CTX *ctx);
void MD5_GetHash(MD5_CTX *ctx, uint8_t *digest);
void MD5_Final(unsigned char *md, MD5_CTX *c);


#ifdef __cplusplus
}
#endif

#endif /* __MD5_H__ */
