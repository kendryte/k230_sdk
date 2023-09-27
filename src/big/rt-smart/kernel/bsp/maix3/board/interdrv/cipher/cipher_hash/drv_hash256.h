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

#ifndef __DRV_HASH256__
#define __DRV_HASH256__
#include <stdint.h>

#define RT_HWHASH_CTRL_INIT             _IOWR('H', 0, int)
#define RT_HWHASH_CTRL_UPDATE           _IOWR('H', 1, int)
#define RT_HWHASH_CTRL_FINISH           _IOWR('H', 2, int)

#define K230_HASH_NAME                  "hwhash"
#define HMAC_SW_KEY_MAXLEN              64
#define HMAC_BLOCK_MAXLEN               128
#define DGST_INT_STATE_LEN              64
#define DLEN_MAX                        64

//----------------------DMA REGISTER OFFSET----------------------------------------//
#define DMA_STAT_0_OFFSET               0x10
#define DMA_STAT_1_OFFSET               0x14
#define DMA_START_OFFSET                0x20
#define DMA_CFG_0_OFFSET                0x24
#define DMA_DSC_CFG_0_OFFSET            0x34
#define DMA_DSC_CFG_1_OFFSET            0x38
#define DMA_DSC_CFG_2_OFFSET            0x3c
#define DMA_DSC_CFG_3_OFFSET            0x40
#define DMA_DSC_CFG_4_OFFSET            0x44
#define DMA_KEY_CFG_0_OFFSET            0x6c
//----------------------DMA REGISTER BIT----------------------------------------//
#define DMA_KEY_CFG_0_KEY_DST_BITS        4
#define DMA_KEY_CFG_0_KEY_SIZE_BITS       8
#define DMA_KEY_CFG_0_KEY_IDX_BITS        24

#define DMA_DSC_CFG_4_WRITE_PROT_BITS     0
#define DMA_DSC_CFG_4_READ_PROT_BITS      8
#define DMA_DSC_CFG_4_FIX_READ_BITS       16   
#define DMA_DSC_CFG_4_FIX_WRITE_BITS      17
#define DMA_DSC_CFG_4_NO_CRYP_BITS        23
#define DMA_DSC_CFG_4_OFFSET_BITS         24
#define DMA_DSC_CFG_4_DN_PAUSE_BITS       28
#define DMA_DSC_CFG_4_DN_INTRPT_BITS      29
#define DMA_DSC_CFG_4_TAIL_BITS           30
#define DMA_DSC_CFG_4_HEAD_BITS           31
//----------------------DMA REGISTER BIT MASKS----------------------------------//
#define DMA_STATUS_0_BUSY_MASK            0x00000001

//----------------------CRYPTO REGISTER OFFSET---------------------------------------//
#define CRYPTO_IV_OFFSET                0x130
#define CRYPTO_SW_KEY_OFFSET            0x140
#define CRYPTO_DGST_IN_OFFSET           0x180
#define CRYPTO_DGST_OUT_OFFSET          0x1c0


//----------------------HASH REGISTER OFFSET----------------------------------------//
#define HASH_STATUS_OFFSET              0x810
#define HASH_CONFIG_OFFSET              0x818
#define HASH_PLEN_OFFSET                0x820
#define HASH_ALEN_OFFSET                0x830
//----------------------HASH REGISTER BIT MASKS----------------------------------//
#define HMAC_HASH_INTRPT_INTRPT_ST_MASK 0x00000001
#define HMAC_HASH_INTRPT_INTRPT_EN_MASK 0x00010000
#define HMAC_HASH_FEATURE_HMAC_MASK     0x00000001
#define HMAC_HASH_FEATURE_SHA2_MASK     0x00000002
#define HMAC_HASH_STATUS_BUSY_MASK      0x00000001
#define HMAC_HASH_STATUS_RESP_MASK      0xfffffffe
#define HMAC_HASH_CFG_VARIANT_MASK      0x00000007
#define HMAC_HASH_CFG_FUNC_MASK         0x00000100
#define HMAC_HASH_PLEN_VALUE_MASK       0xffffffff
#define HMAC_HASH_ALEN_VALUE_MASK       0xffffffff

typedef enum {
    SWKEY = 0,
    OTPKEY = 1,
    PUFKEY = 2,
    RANDKEY = 3,
    SHARESEC = 4,
    SSKEY = 5,
    PRKEY,
} k_pufs_key_type_t;

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

typedef enum {
    ALGO_TYPE_HMAC = 1,
    ALGO_TYPE_NONE,
} k_pufs_algo_type_t;

typedef enum {
    HMAC_AVAILABLE,
    HMAC_HASH,
    HMAC_HMAC,
} hmac_op;

struct hash_context
{
    rt_uint8_t buff[HMAC_BLOCK_MAXLEN];
    rt_uint8_t key[HMAC_BLOCK_MAXLEN];
    rt_uint8_t state[DGST_INT_STATE_LEN];
    rt_uint32_t buflen;
    rt_uint32_t keybits;
    rt_uint32_t minlen;
    rt_uint32_t curlen;
    rt_uint32_t keyslot;
    rt_uint32_t blocklen;
    k_pufs_key_type_t keytype;
    hmac_op op;
    k_pufs_hash_t hash;
    rt_bool_t start;
};

/* Message digest structure. */
typedef struct
{
    rt_uint32_t dlen;
    rt_uint8_t dgst[DLEN_MAX];
} k_pufs_dgst_st;

typedef struct
{
    rt_bool_t process;
    const rt_uint8_t* addr;
    rt_uint32_t len;
} k_segstr;

typedef struct
{
    rt_uint32_t nsegs;
    k_segstr seg[3];
} k_blsegs;

struct rt_hash_config_args
{
    void *msg;
    void *dgst;
    rt_uint32_t msglen;
    rt_uint32_t dlen;
};

#endif  /*__DRV_HASH256__*/