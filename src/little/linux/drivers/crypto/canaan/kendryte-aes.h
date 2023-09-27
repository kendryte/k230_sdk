/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __KENDRYTE_AES_H__
#define __KENDRYTE_AES_H__


/* define single block cipher and mode*/
#define KENDRYTE_CIPHER_AES             0
#define KENDRYTE_CIPHER_SM4             1
#define KENDRYTE_CIPHER_SHASH           2
#define KENDRYTE_CIPHER_SM3             3
#define KENDRYTE_CIPHER_RSA             4
#define KENDRYTE_CIPHER_SM2             5

#define KENDRYTE_MODE_CBC               0
#define KENDRYTE_MODE_ECB               1
#define KENDRYTE_MODE_CFB               2
#define KENDRYTE_MODE_OFB               3
#define KENDRYTE_MODE_CTR               4
#define KENDRYTE_MODE_CCM               5
#define KENDRYTE_MODE_GCM               6
#define KENDRYTE_MODE_SM4               7
#define KENDRYTE_MODE_SHA256            8
#define KENDRYTE_MODE_SM3               9
#define KENDRYTE_MODE_RSA               10
#define KENDRYTE_MODE_SM2               11

/* define the cipher name str */
#define STR_CBC_AES "cbc(aes)"
#define STR_ECB_AES "ecb(aes)"
#define STR_CFB_AES "cfb(aes)"
#define STR_OFB_AES "ofb(aes)"
#define STR_CTR_AES "ctr(aes)"
#define STR_CCM_AES "ccm(aes)"
#define STR_GCM_AES "gcm(aes)"

#define AES_BLOCK_SIZE                  16
#define ULLONG_MAX                      (~0ULL)
#define SW_KEY_MAXLEN                   64
#define DGST_INT_STATE_LEN              64
#define IV_MAXLEN                       16

//----------------------CRYPTO REGISTER OFFSET---------------------------------------//
#define CRYPTO_IV_OFFSET                0x30
#define CRYPTO_SW_KEY_OFFSET            0x40
#define CRYPTO_DGST_IN_OFFSET           0x80
#define CRYPTO_DGST_OUT_OFFSET          0xc0


//----------------------GCM REGISTER BITS---------------------------------------//
#define GCM_CFG_BC_TYPE_BITS          0
#define GCM_CFG_GHASH_BITS            2
#define GCM_CFG_GCTR_BITS             3
#define GCM_CFG_ENCRYPT_BITS          4
#define GCM_CFG_REG_IN_BITS           5
#define GCM_CFG_REG_OUT_BITS          6
//----------------------GCM REGISTER OFFSET----------------------------------------//
#define GCM_STAT_OFFSET                   0x70
#define GCM_CFG_1_OFFSET                  0x78
#define GCM_CFG_2_OFFSET                  0x7c
//----------------------REGISTER BIT MASKS----------------------------------//
#define GCM_STATUS_RESP_MASK          0xfffffffe


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

//----------------------KWP REGISTER OFFSET----------------------------------//
#define KWP_STATUS_OFFSET               0x10
#define KWP_START_OFFSET                0x14
#define KWP_CONFIG_OFFSET               0x18
#define KWP_KEY_IN_OUT_OFFSET           0x40
#define KWP_KEY_MAXLEN                  80
#define BUFFER_SIZE                     512
//----------------------KWP REGISTER BIT MASKS----------------------------------//
#define KWP_STATUS_BUSY_MASK            0x00000001


//----------------------KA REGISTER OFFSET----------------------------------//
#define KA_SK_FREE_OFFSET               0x10
#define KA_SK_0_OFFSET                  0x20
//----------------------KA REGISTER BIT MASKS----------------------------------//
#define SK_KEY_VAILD_MASK               0x00000001
#define SK_KEY_ORIGIN_MASK              0x0000000e
#define SK_KEY_SIZE_MASK                0x00007ff0
#define SK_KEY_TAG_MASK                 0x00ff0000

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
 * @brief Block cipher algorithm.
 */
typedef enum {
    AES,
    SM4,
    N_CIPHER_T,
} kendryte_cipher_t;

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
 * @brief Key array (KA) slots
 */
typedef enum {
    // session key slots
    SK128_0,     ///< 128-bit session key slot 0
    SK128_1,     ///< 128-bit session key slot 1
    SK128_2,     ///< 128-bit session key slot 2
    SK128_3,     ///< 128-bit session key slot 3
    SK128_4,     ///< 128-bit session key slot 4
    SK128_5,     ///< 128-bit session key slot 5
    SK128_6,     ///< 128-bit session key slot 6
    SK128_7,     ///< 128-bit session key slot 7
    SK256_0,     ///< 256-bit session key slot 0
    SK256_1,     ///< 256-bit session key slot 1
    SK256_2,     ///< 256-bit session key slot 2
    SK256_3,     ///< 256-bit session key slot 3
    SK512_0,     ///< 512-bit session key slot 0
    SK512_1,     ///< 512-bit session key slot 1
    // private key slots
    PRK_0,       ///< private key slot 0
    PRK_1,       ///< private key slot 1
    PRK_2,       ///< private key slot 2
    /// shared secret slot 0.
    SHARESEC_0,  ///< shared secret slot 0
    N_KA_SLOT_T, ///< keep in the last one
} kendryte_ka_slot_t;


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


/**
 * enum type for GCM input
 */
typedef enum {
    GCM_NONE_S,
    GCM_AAD_S,
    GCM_TEXT_S,
} gcm_stage;

/**
 * enum type for GCM context protection
 */
typedef enum {
    GCM_AVAILABLE_OP,
    GCM_GHASH_OP,
    AES_GCM_OP,
    GCM_GMAC_OP,
} gcm_op;

typedef enum {
    GCM_AAD_I,
    GCM_PLAINTEXT_I,
} gcm_input_data_t;



struct kendryte_crypto_info {
    struct device *dev;
    struct clk *clk;
    struct reset_control *rst;
    void __iomem *base;
    int irq;
    spinlock_t lock;

    // spinlock *crypto_spinlock;
};

struct kendryte_aes_ctx {
    struct kendryte_crypto_info  *dev;
    uint32_t mode;

    uint8_t key[32];
    uint64_t keylen;
    uint64_t ivlen;
    uint64_t aadlen;
    uint8_t tag[AES_BLOCK_SIZE];
    uint64_t taglen;
    uint64_t pclen;
    uint64_t inlen;
    uint32_t buflen;
    uint32_t minlen;
    uint32_t incj0;
    bool start;
    bool encrypt;
    uint8_t j0[AES_BLOCK_SIZE];
    uint8_t ghash[AES_BLOCK_SIZE];
    uint8_t buffer[AES_BLOCK_SIZE];
    gcm_op op;
    gcm_stage stage;
    kendryte_key_type_t keytype;
    kendryte_cipher_t cipher;
    kendryte_ka_slot_t keyslot;
};

#endif /* __KENDRYTE_AES_H__ */