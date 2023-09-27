/**
 * @file      pufs_hmac.c
 * @brief     PUFsecurity HMAC API implementation
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

#include "pufs_internal.h"
#include "pufs_hmac_internal.h"
#include "pufs_ka_internal.h"
#include "pufs_dma_internal.h"
#include <common.h>
#include "platform.h"
#include "pufs_ecp.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
struct pufs_hmac_regs *hmac_regs = (struct pufs_hmac_regs *)(PUFIOT_ADDR_START+HMAC_HASH_ADDR_OFFSET);

static pufs_hmac_ctx hmac_ctx_static = { .op = HMAC_AVAILABLE };
pufs_hmac_ctx *hmac_ctx = &hmac_ctx_static;

static pufs_status_t __hmac_ctx_final(pufs_dgst_st* md)
{
    cb_crypto_read_dgest(md->dgst, DGST_INT_STATE_LEN);
    md->dlen = 32;
    return SUCCESS;
}

/**
 * @brief Pass the input into the HMAC hardware
 *
 * @param[in]  hmac_ctx  HMAC context.
 * @param[out] md        The pointer to the space which the digest is written to.
 * @param[in]  msg       The message.
 * @param[in]  msglen    The length of the message in bytes.
 * @param[in]  last      True if the input for this operation ends.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t __hmac_ctx_update(pufs_dgst_st* md,
                                       const uint8_t* msg,
                                       uint32_t msglen,
                                       bool last)
{
    uint32_t val32;

    if (cb_dma_check_busy_status(NULL))
        return E_BUSY;

    cb_dma_write_config_0(false, false, false);
    cb_dma_write_data_block_config(hmac_ctx->start ? false : true, last, true, true, 0);

    flush_dcache_range((uint64_t *)msg, msg+msglen); // //csi_dcache_clean_range
    cb_dma_write_rwcfg(NULL, msg, msglen);

    cb_dma_write_key_config_0(hmac_ctx->keytype,
                           ALGO_TYPE_HMAC,
                           (hmac_ctx->keybits < 512) ? hmac_ctx->keybits : 512,
                           cb_get_key_slot_idx(hmac_ctx->keytype, hmac_ctx->keyslot));

    if (hmac_ctx->start)
        cb_crypto_write_dgst(hmac_ctx->state, DGST_INT_STATE_LEN);

    val32 = ((hmac_ctx->hash == SHA_256) ? 0x03 : 0x08);
    
    hmac_regs->cfg = val32;
    hmac_regs->plen = hmac_ctx->curlen;

    cb_dma_write_start();

    while(cb_dma_check_busy_status(&val32));

    if (val32 != 0)
    {
        return E_ERROR;
    }

    val32 = hmac_regs->status;
    if (val32 != 0)
    {
        return E_ERROR;
    }

    if (!last)
    {
        cb_crypto_read_dgest(hmac_ctx->state, DGST_INT_STATE_LEN);
        hmac_ctx->curlen = hmac_regs->alen;
    }

    return SUCCESS;
}

/**
 * pufs_hash_init()
 */
pufs_status_t pufs_hash_init(pufs_hash_t hash)
{
    hmac_ctx->blocklen = 64;

    // initialize for hash
    hmac_ctx->buflen = 0;
    hmac_ctx->keybits = 0;
    hmac_ctx->minlen = 1;
    hmac_ctx->keytype = 0;
    hmac_ctx->curlen = 0;
    hmac_ctx->op = HMAC_HASH;
    hmac_ctx->hash = hash;
    hmac_ctx->start = false;

    return SUCCESS;
}
/**
 * pufs_hash_update()
 */
pufs_status_t pufs_hash_update(const uint8_t* msg,
                               uint32_t msglen)
{
    pufs_status_t check;

    blsegs segs = cb_segment(hmac_ctx->buff, hmac_ctx->buflen, msg, msglen,
                          hmac_ctx->blocklen, hmac_ctx->minlen);
    hmac_ctx->buflen = 0;

    for (uint32_t i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if ((check = cb___hmac_ctx_update(NULL, segs.seg[i].addr,
                                           segs.seg[i].len, false)) != SUCCESS)
            {
                // release hmac context
                hmac_ctx->op = HMAC_AVAILABLE;
                return check;
            }
            if (hmac_ctx->start == false)
                hmac_ctx->start = true;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == hmac_ctx->buff) && (hmac_ctx->buflen == 0))
            { // skip copy what already in the right place
                hmac_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                memcpy(hmac_ctx->buff + hmac_ctx->buflen, segs.seg[i].addr,
                        segs.seg[i].len);
                hmac_ctx->buflen += segs.seg[i].len;
            }
        }
    }


    return SUCCESS;
}

/**
 * pufs_hash_final()
 */
pufs_status_t pufs_hash_final(pufs_dgst_st* md)
{
    pufs_status_t check = SUCCESS;

    check = cb___hmac_ctx_update(md, hmac_ctx->buff,
                                    hmac_ctx->buflen, true);
    if (check != SUCCESS)
        goto done;

    check = cb___hmac_ctx_final(md);

done:
    hmac_ctx->op = HMAC_AVAILABLE;
    return check;
}
/**
 * pufs_hash()
 */
pufs_status_t pufs_hash(pufs_dgst_st* md,
                        const uint8_t* msg,
                        uint32_t msglen,
                        pufs_hash_t hash)
{
    pufs_status_t check;

    // Call I-U-F model
    if ((check = cb_pufs_hash_init(hash)) != SUCCESS)
        return check;
    if ((check = cb_pufs_hash_update(msg, msglen)) != SUCCESS)
        return check;
    return cb_pufs_hash_final(md);
}

pufs_status_t pufs_hash_lock(pufs_dgst_st* md,
                        const uint8_t* msg,
                        uint32_t msglen,
                        pufs_hash_t hash)
{
    pufs_status_t ret =0;
    hardlock_config(lock);
    ret = pufs_hash(md, msg, msglen, hash);
    hardlock_config(unlock);
    return ret;
}

func___hmac_ctx_final cb___hmac_ctx_final = __hmac_ctx_final;
func___hmac_ctx_update cb___hmac_ctx_update = __hmac_ctx_update;
func_pufs_hash_init cb_pufs_hash_init = pufs_hash_init;
func_pufs_hash_update cb_pufs_hash_update = pufs_hash_update;
func_pufs_hash_final cb_pufs_hash_final = pufs_hash_final;
func_pufs_hash cb_pufs_hash = pufs_hash_lock;
#pragma GCC pop_options