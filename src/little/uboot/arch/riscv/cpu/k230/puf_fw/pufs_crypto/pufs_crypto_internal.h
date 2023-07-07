/**
 * @file      pufs_crypto_internal.h
 * @brief     PUFsecurity Crypto Internal
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

#ifndef __PUFS_CRYPTO_INTERNAL_H__
#define __PUFS_CRYPTO_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pufs_crypto.h"
#include "pufs_common.h"
#include "pufs_crypto_regs.h"

/*****************************************************************************
 * Macros
 ****************************************************************************/

#define CRYPTO_VERSION 0x5046A0A1

#define CRYPTO_IO_CTX_SIZE  144
#define CRYPTO_IO_CTX_NUM     8

#define crypto_write_regs(member, index, value) \
    cb__crypto_write_regs(offsetof(struct pufs_crypto_regs, member), index, value);

typedef void (* func_write_data)(uint32_t *dst, uint8_t *src, size_t length, bool le);
extern func_write_data cb_write_data;

typedef void (* func_read_data)(uint8_t *dst, uint32_t *src, size_t length, bool le);
extern func_read_data cb_read_data;

typedef void (* func__crypto_write_regs)(uint32_t offset, uint32_t index, uint32_t value);
extern func__crypto_write_regs cb__crypto_write_regs;

extern volatile struct pufs_crypto_regs *crypto_regs;

/*****************************************************************************
 * Internal Functions
 ****************************************************************************/

typedef pufs_status_t (* func_crypto_write_sw_key)(uint8_t *key, size_t length);
extern func_crypto_write_sw_key cb_crypto_write_sw_key;

typedef pufs_status_t (* func_crypto_write_iv)(uint8_t *iv, size_t length);
extern func_crypto_write_iv cb_crypto_write_iv;

typedef pufs_status_t (* func_crypto_read_iv)(uint8_t *out, size_t length);
extern func_crypto_read_iv cb_crypto_read_iv;

typedef pufs_status_t (* func_crypto_write_dgst)(uint8_t *dgst, size_t length);
extern func_crypto_write_dgst cb_crypto_write_dgst;

typedef void (* func_crypto_read_dgest)(uint8_t *out, size_t length);
extern func_crypto_read_dgest cb_crypto_read_dgest;
#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_CRYPTO_INTERNAL_H__*/