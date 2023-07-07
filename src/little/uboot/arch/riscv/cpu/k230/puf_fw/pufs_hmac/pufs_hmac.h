/**
 * @file      pufs_hmac.h
 * @brief     PUFsecurity HMAC API interface
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

#ifndef __PUFS_HMAC_H__
#define __PUFS_HMAC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pufs_common.h"
#include "pufs_ka.h"
#include "pufs_dma.h"

/*****************************************************************************
 * Type definitions
 ****************************************************************************/
typedef struct pufs_hmac_context pufs_hmac_ctx;
typedef pufs_hmac_ctx pufs_hash_ctx;

/*****************************************************************************
 * Enumerations
 ****************************************************************************/
/**
 * @brief Cryptographic hash algorithms
 */
typedef enum {
    SHA_224,     ///< SHA224
    SHA_256,     ///< SHA256
    SHA_384,     ///< SHA384
    SHA_512,     ///< SHA512
    SHA_512_224, ///< SHA512/224
    SHA_512_256, ///< SHA512/256
    SM3,         ///< SM3
    N_HASH_T,    // keep in the last one
} pufs_hash_t;

/*****************************************************************************
 * API functions
 ****************************************************************************/
typedef pufs_status_t (* func___hmac_ctx_final)(pufs_dgst_st* md);
extern func___hmac_ctx_final cb___hmac_ctx_final;

typedef pufs_status_t (* func___hmac_ctx_update)(pufs_dgst_st* md,
                                       const uint8_t* msg,
                                       uint32_t msglen,
                                       bool last);
extern func___hmac_ctx_update cb___hmac_ctx_update;
/**
 * @brief Initialize hash calculator
 *
 * @param[in] hmac_ctx  HMAC context.
 * @param[in] hash      Hash algorithm.
 * @return              SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_hash_init)(pufs_hash_t hash);
extern func_pufs_hash_init cb_pufs_hash_init;

/**
 * @brief Input data into hash calculator
 *
 * @param[in] hmac_ctx  HMAC context.
 * @param[in] msg       Message.
 * @param[in] msglen    Message length in bytes.
 * @return              SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_hash_update)(const uint8_t* msg,
                               uint32_t msglen);
extern func_pufs_hash_update cb_pufs_hash_update;

/**
 * @brief Extract message digest from hash calculator
 *
 * @param[in]  hmac_ctx  HMAC context.
 * @param[out] md        Message digest.
 * @return               SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_hash_final)(pufs_dgst_st* md);
extern func_pufs_hash_final cb_pufs_hash_final;

/**
 * @brief Calculate hash value of a message.
 *
 * @param[out] md      Message digest.
 * @param[in]  msg     Message.
 * @param[in]  msglen  Message length in bytes.
 * @param[in]  hash    Hash algorithm.
 * @return             SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_hash)(pufs_dgst_st* md,
                        const uint8_t* msg,
                        uint32_t msglen,
                        pufs_hash_t hash);
extern func_pufs_hash cb_pufs_hash;

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_HMAC_H__ */
