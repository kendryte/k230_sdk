/**
 * @file      pufs_sm2.h
 * @brief     PUFsecurity SM2 API interface
 * @copyright 2021 PUFsecurity
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

#ifndef __PUFS_SM2_H__
#define __PUFS_SM2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_common.h"
#include "pufs_ka.h"
#include "pufs_ecc.h"

typedef pufs_status_t (* func_pufs_sm2_gen_z)(pufs_dgst_st *md,
                                    const uint8_t* id,
                                    uint32_t idlen,
                                    pufs_ec_point_st *puk);
extern func_pufs_sm2_gen_z cb_pufs_sm2_gen_z;

typedef pufs_status_t (* func_pufs_sm2_sign_m_hash)(pufs_dgst_st *md,
                                         pufs_dgst_st *za,
                                         const uint8_t *msg,
                                         uint32_t msg_len);
extern func_pufs_sm2_sign_m_hash cb_pufs_sm2_sign_m_hash;

/**
 * @brief SM2 signature verification
 *
 * @param[in]  sig     SM2 signature.
 * @param[in]  msg     Message.
 * @param[in]  msglen  Message length in bytes.
 * @param[in]  id      Identity.
 * @param[in]  idlen   Identity length in bytes.
 * @param[in]  puk     Public key.
 * @return             SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_sm2_verify)(pufs_ecdsa_sig_st sig,
                              const uint8_t* msg,
                              uint32_t msglen,
                              const uint8_t* id,
                              uint32_t idlen,
                              pufs_ec_point_st puk);
extern func_pufs_sm2_verify cb_pufs_sm2_verify;
#ifndef BOOTROM
/**
 * @brief Wrapper function of _pufs_sm2_sign() to set NULL as the default value
 *        of the last parameter if not provided.
 */
#define pufs_sm2_sign(sig, msg, msglen, id, idlen, ...) \
    _pufs_sm2_sign(sig, msg, msglen, id, idlen, DEF_ARG(__VA_ARGS__, NULL))
/**
 * @brief SM2 signature signing
 *
 * @param[in]  sig      SM2 signature.
 * @param[in]  msg      Message.
 * @param[in]  msglen   Message length in bytes.
 * @param[in]  id       Identity.
 * @param[in]  idlen    Identity length in bytes.
 * @param[in]  prkslot  Private key slot.
 * @param[in]  k        Ephemeral private key.
 * @return              SUCCESS on success, otherwise an error code.
 *
 * @note Currently input \em k is not supported.
 */
pufs_status_t _pufs_sm2_sign(pufs_ecdsa_sig_st* sig,
                             const uint8_t* msg,
                             uint32_t msglen,
                             const uint8_t* id,
                             uint32_t idlen,
                             pufs_ka_slot_t prk,
                             const uint8_t* k);

#endif
#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_SM2_H__ */
