/**
 * @file      pufs_hmac_internal.h
 * @brief     PUFsecurity HMAC internal interface
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

#ifndef __PUFS_HMAC_INTERNAL_H__
#define __PUFS_HMAC_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "pufs_hmac.h"
#include "pufs_crypto_regs.h"
#include "pufs_hmac_regs.h"
#include "pufs_hmac_regs.h"
#include "pufs_crypto_internal.h"

/*****************************************************************************
 * Macros
 ****************************************************************************/

#define HMAC_VERSION 0x31393800

/*****************************************************************************
 * Variables
 ****************************************************************************/

extern struct pufs_hmac_regs *hmac_regs;
extern pufs_hmac_ctx *hmac_ctx;

/*****************************************************************************
 * Enumerations
 ****************************************************************************/
/**
 * enum type for hash/HMAC context protection
 */
typedef enum {
    HMAC_AVAILABLE,
    HMAC_HASH,
    HMAC_HMAC,
} hmac_op;

/*****************************************************************************
 * Structures
 ****************************************************************************/
/**
 * structure for hash/HMAC context (128-bit block size)
 *
 * This structure keeps necessary information to trigger HMAC HW, including
 *  1. operation (Hash or HMAC): op
 *  2. hash algotithm: hash
 *  3. key information for HMAC: key, keybits, keyslot, keytype, swkey
 *  4. internal state: state
 *  5. whether the first block is sent to HW: start
 *  6. minimum byte length of the last input data: minlen
 *  7. buffer for incomplete-block input: buff, buflen
 */
#define HMAC_BLOCK_MAXLEN 128
struct pufs_hmac_context
{
    uint8_t buff[HMAC_BLOCK_MAXLEN];
    uint8_t key[HMAC_BLOCK_MAXLEN];
    uint8_t state[DGST_INT_STATE_LEN];
    uint32_t buflen;
    uint32_t keybits;
    uint32_t minlen;
    uint32_t curlen;
    uint32_t keyslot;
    uint32_t blocklen;
    pufs_key_type_t keytype;
    hmac_op op;
    pufs_hash_t hash;
    bool start;
};

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_HMAC_INTERNAL_H__ */
