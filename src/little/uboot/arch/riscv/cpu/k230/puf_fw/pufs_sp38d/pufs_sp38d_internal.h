/**
 * @file      pufs_sp38d_internal.h
 * @brief     PUFsecurity SP38D internal interface
 * @copyright 2020 PUFsecurity
 */
/* THIS SOFTWARE IS SUPPLIED BY PUFSECURITY ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. TO THE FULLEST
 * EXTENT ALLOWED BY LAW, PUFSECURITY'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES,
 * IF ANY, THAT YOU HAVE PAID DIRECTLY TO PUFSECURITY FOR THIS SOFTWARE.
 */

#ifndef __PUFS_SP38D_INTERNAL_H__
#define __PUFS_SP38D_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_sp38d.h"
#include "pufs_sp38d_regs.h"
#include "pufs_crypto_internal.h"

/*****************************************************************************
 * Macros
 ****************************************************************************/

#define SP38D_VERSION 0x33384401

/*****************************************************************************
 * Variables
 ****************************************************************************/
extern struct pufs_sp38d_regs *sp38d_regs;

/*****************************************************************************
 * Enumerations
 ****************************************************************************/
/**
 * enum type for GCM input
 */
typedef enum {
    SP38D_NONE,
    SP38D_AAD,
    SP38D_TEXT,
} sp38d_stage;
/**
 * enum type for GCM context protection
 */
typedef enum {
    SP38D_AVAILABLE,
    SP38D_GHASH,
    SP38D_GCM,
    SP38D_GMAC,
} sp38d_op;

/*****************************************************************************
 * Structures
 ****************************************************************************/
/**
 * structure for context of block cipher GCM mode (128-bit block size)
 *
 * This structure keeps necessary information to trigger SP38D HW, including
 *  1. operation (AVAILABLE, GCM): op
 *  2. encryption or decryption: encrypt
 *  3. block cipher algotithm: cipher
 *  4. key information for AES: key, keybits, keyslot, keytype
 *  5. translated initial vector J_0: j0
 *  6. minimum byte length of the last input data: minlen
 *  7. buffer for incomplete-block input: buff, buflen
 *  8. intermediate ghash value: ghash
 *  9. input bit lengths: aadbits, inbits
 */
struct pufs_sp38d_context
{
    uint64_t aadbits;
    uint64_t inbits;
    uint8_t buff[BC_BLOCK_SIZE];
    uint8_t key[SW_KEY_MAXLEN];
    uint8_t j0[BC_BLOCK_SIZE];
    uint8_t ghash[BC_BLOCK_SIZE];
    uint32_t buflen;
    uint32_t keybits;
    uint32_t minlen;
    uint32_t keyslot;
    uint32_t incj0;
    pufs_key_type_t keytype;
    sp38d_op op;
    sp38d_stage stage;
    pufs_cipher_t cipher;
    bool encrypt;
    bool start;
};

extern pufs_sp38d_ctx *sp38d_ctx;

typedef pufs_status_t (* func_sp38d_get_config)(uint32_t *cfg, bool gctr, bool reg_in, bool reg_out);
extern func_sp38d_get_config cb_sp38d_get_config;   

typedef pufs_status_t (* func_sp38d_ctx_init)(sp38d_op op,
                                    bool encrypt,
                                    pufs_cipher_t cipher,
                                    pufs_key_type_t keytype,
                                    size_t keyaddr,
                                    uint32_t keybits,
                                    const uint8_t* j0);
extern func_sp38d_ctx_init cb_sp38d_ctx_init;   

typedef pufs_status_t (* func_sp38d_prepare)(const uint8_t* out,
                                   uint32_t inlen);
extern func_sp38d_prepare cb_sp38d_prepare;                           

typedef pufs_status_t (* func_sp38d_postproc)();
extern func_sp38d_postproc cb_sp38d_postproc;

typedef pufs_status_t (* func___sp38d_ctx_update)(uint8_t* out,
                                        uint32_t* outlen,
                                        const uint8_t* in,
                                        uint32_t inlen,
                                        bool last);
extern func___sp38d_ctx_update cb___sp38d_ctx_update;

typedef pufs_status_t (* func_sp38d_ctx_update)(sp38d_op op,
                                      bool encrypt,
                                      uint8_t* out,
                                      uint32_t* outlen,
                                      const uint8_t* in,
                                      uint32_t inlen);
extern func_sp38d_ctx_update cb_sp38d_ctx_update;

typedef pufs_status_t (* func_sp38d_tag)(uint8_t *tag, uint32_t taglen, bool from_reg);
extern func_sp38d_tag cb_sp38d_tag;

typedef pufs_status_t (* func_sp38d_ctx_final)(sp38d_op op,
                    bool encrypt,
                    uint8_t* out,
                    uint32_t* outlen,
                    uint8_t* tag,
                    uint32_t taglen,
                    bool from_reg);
extern func_sp38d_ctx_final cb_sp38d_ctx_final;

typedef pufs_status_t (* func_sp38d_ghash_init)(pufs_cipher_t cipher,
                                      pufs_key_type_t keytype,
                                      size_t keyaddr,
                                      uint32_t keybits);
extern func_sp38d_ghash_init cb_sp38d_ghash_init;

typedef pufs_status_t (* func_sp38d_ghash_update)(const uint8_t* in,
                                        uint32_t inlen);
extern func_sp38d_ghash_update cb_sp38d_ghash_update;

typedef pufs_status_t (* func_sp38d_ghash_final)(uint8_t* out,
                                       uint32_t outlen);
extern func_sp38d_ghash_final cb_sp38d_ghash_final;

typedef pufs_status_t (* func_sp38d_build_j0)(uint8_t* j0,
                                    pufs_cipher_t cipher,
                                    pufs_key_type_t keytype,
                                    size_t keyaddr,
                                    uint32_t keybits,
                                    const uint8_t* iv,
                                    uint32_t ivlen);
extern func_sp38d_build_j0 cb_sp38d_build_j0;

typedef pufs_status_t (* func_sp38d_step_stage)(sp38d_stage stage);
extern func_sp38d_step_stage cb_sp38d_step_stage;

typedef pufs_status_t (* func_pufs_gcm_init)(pufs_cipher_t cipher,
                                   pufs_key_type_t keytype,
                                   size_t keyaddr,
                                   uint32_t keybits,
                                   const uint8_t* iv,
                                   uint32_t ivlen,
                                   bool encrypt);
extern func_pufs_gcm_init cb_pufs_gcm_init;

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_SP38D_INTERNAL_H__ */
