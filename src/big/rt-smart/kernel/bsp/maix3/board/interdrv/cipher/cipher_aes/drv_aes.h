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

#ifndef __DRV_AES__
#define __DRV_AES__
#include <stdint.h>

#define K230_AES_NAME               "aes"
#define RT_AES_INIT                 _IOWR('G', 0, int)
#define RT_AES_UPDATE               _IOWR('G', 1, int)
#define RT_AES_FINAL                _IOWR('G', 2, int)
#define AES_BLOCK_SIZE              16
#define IV_MAXLEN                   16
#define SW_KEY_MAXLEN               64
#define DGST_INT_STATE_LEN          64

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
#define SW_KEY_MAXLEN                   64

//----------------------GCM REGISTER OFFSET----------------------------------------//
#define GCM_STAT_OFFSET                   0x270
#define GCM_CFG_1_OFFSET                  0x278
#define GCM_CFG_2_OFFSET                  0x27c
//----------------------GCM REGISTER BIT MASKS----------------------------------//
#define GCM_STATUS_RESP_MASK          0xfffffffe
//----------------------GCM REGISTER BITS---------------------------------------//
#define GCM_CFG_BC_TYPE_BITS            0
#define GCM_CFG_GHASH_BITS              2
#define GCM_CFG_GCTR_BITS               3
#define GCM_CFG_ENCRYPT_BITS            4
#define GCM_CFG_REG_IN_BITS             5
#define GCM_CFG_REG_OUT_BITS            6

//----------------------KWP REGISTER OFFSET----------------------------------//
#define KWP_STATUS_OFFSET               0x310
#define KWP_START_OFFSET                0x314
#define KWP_CONFIG_OFFSET               0x318
#define KWP_KEY_IN_OUT_OFFSET           0x340
#define KWP_KEY_MAXLEN                  80
#define BUFFER_SIZE                     512
//----------------------KWP REGISTER BIT MASKS----------------------------------//
#define KWP_STATUS_BUSY_MASK            0x00000001


//----------------------KA REGISTER OFFSET----------------------------------//
#define KA_SK_FREE_OFFSET               0xc10
#define KA_SK_0_OFFSET                  0xc20
//----------------------KA REGISTER BIT MASKS----------------------------------//
#define SK_KEY_VAILD_MASK               0x00000001
#define SK_KEY_ORIGIN_MASK              0x0000000e
#define SK_KEY_SIZE_MASK                0x00007ff0
#define SK_KEY_TAG_MASK                 0x00ff0000

/*****************************************************************************
 * Macros
 ****************************************************************************/
/**
 * @brief Convert number of bits to number of bytes
 *
 * @param[in] bits  Number of bits.
 * @return          Number of bytes.
 */
#define b2B(bits) (((bits) + 7) / 8)
/**
 * @brief Convert number of bytes to number of bits
 *
 * @param[in] len  Number of bytes.
 * @return         Number of bits.
 */
#define B2b(len) (8 * (len))

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
} pufs_status_t;

union rt_aes_control_args {
    struct {
        uint8_t mode;
        uint8_t encrypt;
        uint8_t keytype;
        uint8_t keyslot;
        uint8_t *key;
        uint8_t *iv;
        uint32_t keylen;
        uint32_t ivlen;
    } init;
    struct {
        uint8_t *out;
        uint32_t *outlen;
        uint8_t *in;
        uint32_t inlen;
    } update;
    struct {
        uint8_t *out;
        uint32_t *outlen;
        uint8_t *tag;
        uint32_t taglen;
    } final;
};

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
} pufs_algo_type_t;

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
} pufs_ka_slot_t;

typedef enum {
    // PUF slots
    PUFSLOT_0,  ///< PUF slot 0, 256 bits
    PUFSLOT_1,  ///< PUF slot 1, 256 bits
    PUFSLOT_2,  ///< PUF slot 2, 256 bits
    PUFSLOT_3,  ///< PUF slot 3, 256 bits
    // OTP key slots
    OTPKEY_0,   ///< OTP key slot 0, 256 bits
    OTPKEY_1,   ///< OTP key slot 1, 256 bits
    OTPKEY_2,   ///< OTP key slot 2, 256 bits
    OTPKEY_3,   ///< OTP key slot 3, 256 bits
    OTPKEY_4,   ///< OTP key slot 4, 256 bits
    OTPKEY_5,   ///< OTP key slot 5, 256 bits
    OTPKEY_6,   ///< OTP key slot 6, 256 bits
    OTPKEY_7,   ///< OTP key slot 7, 256 bits
    OTPKEY_8,   ///< OTP key slot 8, 256 bits
    OTPKEY_9,   ///< OTP key slot 9, 256 bits
    OTPKEY_10,  ///< OTP key slot 10, 256 bits
    OTPKEY_11,  ///< OTP key slot 11, 256 bits
    OTPKEY_12,  ///< OTP key slot 12, 256 bits
    OTPKEY_13,  ///< OTP key slot 13, 256 bits
    OTPKEY_14,  ///< OTP key slot 14, 256 bits
    OTPKEY_15,  ///< OTP key slot 15, 256 bits
    OTPKEY_16,  ///< OTP key slot 16, 256 bits
    OTPKEY_17,  ///< OTP key slot 17, 256 bits
    OTPKEY_18,  ///< OTP key slot 18, 256 bits
    OTPKEY_19,  ///< OTP key slot 19, 256 bits
    OTPKEY_20,  ///< OTP key slot 20, 256 bits
    OTPKEY_21,  ///< OTP key slot 21, 256 bits
    OTPKEY_22,  ///< OTP key slot 22, 256 bits
    OTPKEY_23,  ///< OTP key slot 23, 256 bits
    OTPKEY_24,  ///< OTP key slot 24, 256 bits
    OTPKEY_25,  ///< OTP key slot 25, 256 bits
    OTPKEY_26,  ///< OTP key slot 26, 256 bits
    OTPKEY_27,  ///< OTP key slot 27, 256 bits
    OTPKEY_28,  ///< OTP key slot 28, 256 bits
    OTPKEY_29,  ///< OTP key slot 29, 256 bits
    OTPKEY_30,  ///< OTP key slot 30, 256 bits
    OTPKEY_31,  ///< OTP key slot 31, 256 bits
} pufs_rt_slot_t;

typedef struct {
    rt_bool_t process;
    const rt_uint8_t* addr;
    rt_uint32_t len;
} k_segstr;

typedef struct {
    rt_uint32_t nsegs;
    k_segstr seg[3];
} k_blsegs;

typedef enum {
    AES,
    SM4,
    N_CIPHER_T,
} pufs_cipher_t;

typedef enum {
    SWKEY = 0,
    OTPKEY = 1,
    PUFKEY = 2,
    RANDKEY = 3,
    SHARESEC = 4,
    SSKEY = 5,
    PRKEY,
} pufs_key_type_t;

typedef enum {
    GCM_NONE_S,
    GCM_AAD_S,
    GCM_TEXT_S,
} gcm_stage;

typedef enum {
    GCM_AVAILABLE_OP,
    GCM_GHASH_OP,
    AES_GCM_OP,
    GCM_GMAC_OP,
} gcm_op;

struct aes_context {
    rt_uint64_t aadlen;
    rt_uint64_t inlen;
    rt_uint8_t buff[AES_BLOCK_SIZE];
    rt_uint8_t key[SW_KEY_MAXLEN];
    rt_uint8_t j0[AES_BLOCK_SIZE];
    rt_uint8_t ghash[AES_BLOCK_SIZE];
    rt_uint32_t buflen;
    rt_uint32_t keylen;
    rt_uint32_t minlen;
    rt_uint32_t keyslot;
    rt_uint32_t incj0;
    pufs_key_type_t keytype;
    gcm_op op;
    gcm_stage stage;
    pufs_cipher_t cipher;
    rt_bool_t encrypt;
    rt_bool_t start;
    rt_bool_t busy;
};

#endif /*__DRV_AES__*/
