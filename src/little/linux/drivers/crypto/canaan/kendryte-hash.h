/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __KENDRYTE_HASH_H__
#define __KENDRYTE_HASH_H__

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
#define CRYPTO_IV_OFFSET                0x30
#define CRYPTO_SW_KEY_OFFSET            0x40
#define CRYPTO_DGST_IN_OFFSET           0x80
#define CRYPTO_DGST_OUT_OFFSET          0xc0


//----------------------HASH REGISTER OFFSET----------------------------------------//
#define HASH_STATUS_OFFSET              0x10
#define HASH_CONFIG_OFFSET              0x18
#define HASH_PLEN_OFFSET                0x20
#define HASH_ALEN_OFFSET                0x30
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

/**
 * @brief Status code
 */
typedef enum {
    SUCCESS,     ///< Success.
    E_ALIGN,     ///< Address alignment mismatch.
    E_OVERFLOW,  ///< Space overflow.
    E_UNDERFLOW, ///< Size too small.
    E_INVALID,   ///< Invalid argument.
    E_BUSY,      ///< Resource is occupied.
    E_UNAVAIL,   ///< Resource is unavailable.
    E_FIRMWARE,  ///< Firmware error.
    E_VERFAIL,   ///< Invalid public key or digital signature.
    E_ECMPROG,   ///< Invalid ECC microprogram.
    E_DENY,      ///< Access denied.
    E_UNSUPPORT, ///< Not support.
    E_INFINITY,  ///< Point at infinity.
    E_ERROR,     ///< Unspecific error.
} kendryte_status_t;

/**
 * @brief Key types
 */
typedef enum {
    SWKEY = 0,
    OTPKEY = 1,
    PUFKEY = 2,
    RANDKEY = 3,
    SHARESEC = 4,
    SSKEY = 5,
    PRKEY,
} kendryte_key_type_t;

/**
 * @brief Algo types
 */
typedef enum {
    ALGO_TYPE_HKDF = 0,
    ALGO_TYPE_HMAC,
    ALGO_TYPE_CMAC,
    ALGO_TYPE_KLB = 4,
    ALGO_TYPE_SM2ENC = 7,
    ALGO_TYPE_SP38A,
    ALGO_TYPE_GCM,
    ALGO_TYPE_XTS,
    ALGO_TYPE_CCM,
    ALGO_TYPE_CHACHA,
    ALGO_TYPE_CYPT_REG_IO,
    ALGO_TYPE_KEY_EXPORT,
    ALGO_TYPE_NONE,
} kendryte_algo_type_t;

/**
 * structure for segment() return value
 */
typedef struct {
    bool process;
    const uint8_t *addr;
    uint32_t len;
} segstr;

typedef struct {
    uint32_t nsegs;
    segstr seg[3];
} blsegs;

typedef enum {
    SHA_224,
    SHA_256,
    SHA_384,
    SHA_512,
    SHA_512_224,
    SHA_512_256,
    SM3,
    N_HASH_T,
} kendryte_hash_t;

typedef enum {
    HMAC_AVAILABLE,
    HMAC_HASH,
    HMAC_HMAC,
} hmac_op;

struct kendryte_sha256_info {
    struct device *dev;
    struct clk *clk;
    struct reset_control *rst;
    void __iomem *base;
    int irq;
};

typedef struct {
    uint32_t dlen;
    uint8_t dgst[DLEN_MAX];
} kendryte_dgst_st;

struct kendryte_sha256_ctx {
    struct kendryte_sha256_info *dev;

    uint8_t buff[HMAC_BLOCK_MAXLEN];
    uint8_t key[HMAC_BLOCK_MAXLEN];
    uint8_t state[DGST_INT_STATE_LEN];
    uint32_t buflen;
    uint32_t keybits;
    uint32_t minlen;
    uint32_t curlen;
    uint32_t keyslot;
    uint32_t blocklen;
    kendryte_key_type_t keytype;
    hmac_op op;
    kendryte_hash_t hash;
    bool start;
};

#endif  /* __KENDRYTE_HASH_H__ */