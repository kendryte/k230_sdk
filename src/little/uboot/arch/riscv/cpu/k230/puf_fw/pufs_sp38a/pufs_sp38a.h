/**
 * @file      pufs_sp38a.h
 * @brief     PUFsecurity SP38A API interface
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

#ifndef __PUFS_SP38A_H__
#define __PUFS_SP38A_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_common.h"
#include "pufs_dma.h"
#include "pufs_ka.h"

/*****************************************************************************
 * Type definitions
 ****************************************************************************/
typedef struct pufs_sp38a_context pufs_sp38a_ctx;
extern pufs_sp38a_ctx *sp38a_ctx;

/**
 * @brief Decryption using CBC mode with keyaddr type casting.
 *
 * @warning DO NOT call this function directly. Use pufs_dec_cbc() instead.
 */
typedef pufs_status_t (* func_pufs_dec_cbc)(uint8_t* out,
                            uint32_t* outlen,
                            const uint8_t* in,
                            uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            size_t keyaddr,
                            uint32_t keybits,
                            const uint8_t* iv,
                            int csmode); 
extern  func_pufs_dec_cbc cb_pufs_dec_cbc;

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_SP38A_H__ */
