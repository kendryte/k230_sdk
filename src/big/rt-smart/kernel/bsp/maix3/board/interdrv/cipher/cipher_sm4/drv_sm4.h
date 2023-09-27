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

#ifndef __DRV_SM4__
#define __DRV_SM4__
#include <stdint.h>

#define K230_SM4_NAME               "sm4"
#define RT_SM4_ENC                  _IOWR('S', 0, int)
#define RT_SM4_DEC                  _IOWR('S', 1, int)
#define AES_BLOCK_SIZE              16
// #define ULLONG_MAX                  (~0ULL)
#define IV_MAXLEN                   16
#define SW_KEY_MAXLEN               64

//----------------------DMA REGISTER OFFSET----------------------------------------//
#define DMA_STAT_0_OFFSET               0x10
#define DMA_START_OFFSET                0x20
#define DMA_CFG_0_OFFSET                0x24
#define DMA_DSC_CFG_0_OFFSET            0x34
#define DMA_DSC_CFG_1_OFFSET            0x38
#define DMA_DSC_CFG_2_OFFSET            0x3c
#define DMA_DSC_CFG_4_OFFSET            0x44
#define DMA_KEY_CFG_0_OFFSET            0x6c
//----------------------DMA REGISTER BIT----------------------------------------//
#define DMA_KEY_CFG_0_KEY_DST_BITS        4
#define DMA_KEY_CFG_0_KEY_SIZE_BITS       8
#define DMA_KEY_CFG_0_KEY_IDX_BITS        24

#define DMA_DSC_CFG_4_OFFSET_BITS         24
#define DMA_DSC_CFG_4_DN_PAUSE_BITS       28
#define DMA_DSC_CFG_4_DN_INTRPT_BITS      29
#define DMA_DSC_CFG_4_TAIL_BITS           30
#define DMA_DSC_CFG_4_HEAD_BITS           31
//----------------------DMA REGISTER BIT MASKS----------------------------------//
#define DMA_STATUS_0_BUSY_MASK            0x00000001

//----------------------CRYPTO REGISTER OFFSET---------------------------------------//
#define CRYPTO_IV_OUT_OFFSET            0x120
#define CRYPTO_IV_OFFSET                0x130
#define CRYPTO_SW_KEY_OFFSET            0x140
#define SW_KEY_MAXLEN                   64

//----------------------SP38A REGISTER OFFSET----------------------------------------//
#define SP38A_FEATURE_OFFSET            0x208
#define SP38A_STAT_OFFSET               0x210
#define SP38A_CFG_OFFSET                0x218
//----------------------SP38A REGISTER BIT MASKS----------------------------------//
#define SP38A_FEATURE_ECB_CLR_MASK      0x00000001
#define SP38A_FEATURE_CFB_MASK          0x00000002
#define SP38A_FEATURE_OFB_MASK          0x00000004
#define SP38A_FEATURE_CBC_CLR_MASK      0x00000008
#define SP38A_FEATURE_CBC_CS1_MASK      0x00000010
#define SP38A_FEATURE_CBC_CS2_MASK      0x00000020
#define SP38A_FEATURE_CBC_CS3_MASK      0x00000040
#define SP38A_FEATURE_CTR_MASK          0x00000080
#define SP38A_STATUS_ERROR_MASK         0xfffff0c0
#define SP38A_FEATURE_ENC_MASK          0x00000100
#define SP38A_FEATURE_DEC_MASK          0x00000200

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

/*
* Encryption case:
*  INPUT  =   Assocdata  ||   Plaintext
*           <-  aadlen ->   <-  pclen  -> <- taglen=0 ->
*
*  OUTPUT =   Assocdata  ||   Ciphertext  ||   Tag
*           <-  aadlen ->    <- outlen ->    <- taglen ->
*
* Decryption case:
*  INPUT  =   Assocdata  ||  Ciphertext  ||     Tag
*          <- aadlen ->    <------- pclen ------------>
*                                          <- taglen ->
*
*  OUTPUT =   Assocdata  ||  Plaintext
*            <- aadlen -> <- outlen -> <- taglen=0 ->
*/
struct rt_sm4_config_args
{
    void *key;
    void *iv;
    void *in;
    void *out;
    rt_uint32_t keybits;
    rt_uint32_t ivlen;
    rt_uint32_t pclen;
    rt_uint32_t outlen;
    char *mode;
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
    SP38A_AVAILABLE,
    SP38A_ECB_CLR,
    SP38A_CFB_CLR,
    SP38A_OFB,
    SP38A_CBC_CLR,
    SP38A_CBC_CS1,
    SP38A_CBC_CS2,
    SP38A_CBC_CS3,
    SP38A_CTR_32,
    SP38A_CTR_64,
    SP38A_CTR_128,
} sp38a_op;

struct sm4_context
{
    rt_uint8_t buff[2 * AES_BLOCK_SIZE];
    rt_uint8_t key[SW_KEY_MAXLEN];
    rt_uint8_t iv[AES_BLOCK_SIZE];
    rt_uint32_t buflen;
    rt_uint32_t keylen;
    rt_uint32_t minlen;
    rt_uint32_t keyslot;
    pufs_key_type_t keytype;
    sp38a_op op;
    pufs_cipher_t cipher;
    rt_bool_t encrypt;
    rt_bool_t start;
};

#endif  /*__DRV_SM4__*/