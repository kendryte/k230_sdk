/**
 * @file      pufs_sp38d.h
 * @brief     PUFsecurity SP38D API interface
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

#ifndef __PUFS_SP38D_H__
#define __PUFS_SP38D_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_common.h"
#include "pufs_dma.h"
#include "pufs_ka.h"

/*****************************************************************************
 * Type definitions
 ****************************************************************************/
typedef struct pufs_sp38d_context pufs_sp38d_ctx;

typedef enum {
    GCM_AAD,
    GCM_PLAINTEXT,
} pufs_gcm_input_data_t;

typedef pufs_status_t (* func_pufs_dec_gcm_init)(pufs_cipher_t cipher,
                                 pufs_key_type_t keytype,
                                 size_t keyaddr,
                                 uint32_t keybits,
                                 const uint8_t* iv,
                                 uint32_t ivlen);
extern func_pufs_dec_gcm_init cb_pufs_dec_gcm_init;

typedef pufs_status_t (* func_pufs_dec_gcm_update)(uint8_t* out,
                                  uint32_t* outlen,
                                  const uint8_t* in,
                                  uint32_t inlen);
extern func_pufs_dec_gcm_update cb_pufs_dec_gcm_update;

typedef pufs_status_t (* func_pufs_dec_gcm_final_tag)(uint8_t* out,
                                     uint32_t* outlen,
                                     uint8_t* tag,
                                     uint32_t taglen);
extern func_pufs_dec_gcm_final_tag cb_pufs_dec_gcm_final_tag;

typedef pufs_status_t (* func_pufs_dec_gcm_final)(uint8_t* out,
                                 uint32_t* outlen,
                                 const uint8_t* tag,
                                 uint32_t taglen);
extern func_pufs_dec_gcm_final cb_pufs_dec_gcm_final;

typedef pufs_status_t (* func_pufs_dec_gcm)(uint8_t* out,
                            uint32_t* outlen,
                            const uint8_t* in,
                            uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            size_t keyaddr,
                            uint32_t keybits,
                            const uint8_t* iv,
                            int ivlen,
                            const uint8_t* aad,
                            int aadlen,
                            const uint8_t* tag,
                            int taglen);
extern func_pufs_dec_gcm cb_pufs_dec_gcm;
pufs_status_t pufs_dec_gcm(uint8_t* out,
                            uint32_t* outlen,
                            const uint8_t* in,
                            uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            size_t keyaddr,
                            uint32_t keybits,
                            const uint8_t* iv,
                            int ivlen,
                            const uint8_t* aad,
                            int aadlen,
                            const uint8_t* tag,
                            int taglen);

pufs_status_t pufs_dec_gcm_poc(uint8_t* out,
                            uint32_t* outlen,
                            const uint8_t* in,
                            uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            const uint8_t* keyaddr,
                            uint32_t keybits,
                            const uint8_t* iv,
                            int ivlen,
                            const uint8_t* aad,
                            int aadlen,
                            const uint8_t* tag,
                            int taglen);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_SP38D_H__ */
