/**
 * @file      pufs_sp38a.c
 * @brief     PUFsecurity SP38A API implementation
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
#include "pufs_sp38a_internal.h"
#include "pufs_ka_internal.h"
#include "pufs_dma_internal.h"
#include "cpu_func.h"
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
struct pufs_sp38a_regs *sp38a_regs = (struct pufs_sp38a_regs *)(PUFIOT_ADDR_START+SP38A_ADDR_OFFSET);

static pufs_sp38a_ctx sp38a_ctx_static = { .op = SP38A_AVAILABLE };
pufs_sp38a_ctx *sp38a_ctx = &sp38a_ctx_static;

/*****************************************************************************
 * Static functions
 ****************************************************************************/

static pufs_status_t sp38a_get_cfg(uint32_t *cfg)
{
    if(sp38a_ctx->cipher == AES) *cfg = 0x2;
    if(sp38a_ctx->cipher == SM4) *cfg = 0x3;

    (*cfg) |= 0x3<<4;

    // (*cfg) |= (sp38a_ctx->encrypt ? 0x1 : 0x0 )<<8;
    return SUCCESS;
}

/**
 * @brief Initialize the internal context for block cipher mode of operation
 *
 * @param[in] op         The mode of operation.
 * @param[in] sp38a_ctx  SP38A context to be initialized.
 * @param[in] cipher     The block cipher algorithm.
 * @param[in] encrypt    True/false for encryption/decryption
 * @param[in] keytype    The type of source which the key is from.
 * @param[in] keyaddr    The pointer to the space in SWKEY or the slot of the
 *                        source which the key is stored in.
 * @param[in] keybits    The key length in bits.
 * @param[in] iv         The initial vector used for CBC/CTR modes.
 * @param[in] option     The additional control of mode for CBC/CTR modes.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38a_ctx_init(sp38a_op op,
                                    pufs_cipher_t cipher,
                                    bool encrypt,
                                    pufs_key_type_t keytype,
                                    size_t keyaddr,
                                    uint32_t keybits,
                                    const uint8_t* iv,
                                    int option)
{
    // // check and set iv if needed
    // if (op != SP38A_ECB_CLR)
    // {
    //     if (iv == NULL)
    //         return E_INVALID;
    //     else
            memcpy(sp38a_ctx->iv, iv, BC_BLOCK_SIZE);
    // }

    // initialize for block-cipher mode of operation
    sp38a_ctx->buflen = 0;
    sp38a_ctx->cipher = cipher;
    sp38a_ctx->encrypt = encrypt;
    sp38a_ctx->start = false;

    // set key
    sp38a_ctx->keybits = keybits;
    sp38a_ctx->keytype = keytype;
    // if (keytype != SWKEY)
        sp38a_ctx->keyslot = (uint32_t)keyaddr;
    // else
    //     memcpy(sp38a_ctx->key, (const void*)keyaddr, b2B(keybits));

    if (op == SP38A_CBC_CLR) {
        sp38a_ctx->op = SP38A_CBC_CLR;
        sp38a_ctx->minlen = 1;
    }

    return SUCCESS;
}
/**
 * @brief Prepare registers for SP38A operation
 *
 * @param[in] sp38a_ctx  SP38A context.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38a_prepare()
{
    uint32_t val32;
    pufs_status_t check;

    // if (sp38a_ctx->keytype == SWKEY)
    //     cb_crypto_write_sw_key(sp38a_ctx->key, SW_KEY_MAXLEN);

    cb_dma_write_key_config_0(sp38a_ctx->keytype,
                           ALGO_TYPE_SP38A,
                           sp38a_ctx->keybits,
                           cb_get_key_slot_idx(sp38a_ctx->keytype, sp38a_ctx->keyslot));

    cb_crypto_write_iv(sp38a_ctx->iv, BC_BLOCK_SIZE);

    if ((check = cb_sp38a_get_cfg(&val32)) != SUCCESS)
        return check;

    sp38a_regs->cfg = val32;

    return SUCCESS;
}
/**
 * @brief Post-processing for SP38A operation
 *
 * @param[in] sp38a_ctx  SP38A context.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38a_postproc()
{
    cb_crypto_read_iv(sp38a_ctx->iv, BC_BLOCK_SIZE);
    return SUCCESS;
}
/**
 * @brief Pass the input into the mode of operation hardware
 *
 * @param[in]  sp38a_ctx  SP38A context.
 * @param[out] outbuf     The pointer to the space where the output is written.
 * @param[out] outlen     The length of the output in bytes.
 * @param[in]  inbuf      The input.
 * @param[in]  inlen      The length of the input in bytes.
 * @param[in]  descs      Input arrays of SGDMA descriptors.
 * @param[in]  descs_len  The length of SGDMA descriptor array.
 * @param[in]  last       True if the input for this operation ends
 * @return                SUCCESS on success, otherwise an error code.
 */
static pufs_status_t __sp38a_ctx_update(uint8_t* outbuf,
                                        uint32_t* outlen,
                                        const uint8_t* inbuf,
                                        uint32_t inlen,
                                        bool last)
{
    uint32_t val32;
    pufs_status_t check;

    // Register manipulation
    if (cb_dma_check_busy_status(NULL))
        return E_BUSY;

    cb_dma_write_config_0(false, false, false);
    cb_dma_write_data_block_config(sp38a_ctx->start ? false : true, last, true, true, 0);

    flush_dcache_range((uint64_t *)inbuf, inbuf+inlen);// //csi_dcache_clean_range
    invalidate_dcache_range((uint64_t *)outbuf, outbuf+inlen);
    //csi_dcache_clean_invalid_range((uint64_t *)outbuf, inlen);
    cb_dma_write_rwcfg(outbuf, inbuf, inlen);

    if ((check = cb_sp38a_prepare(sp38a_ctx)) != SUCCESS)
        return check;

    cb_dma_write_start();

    while(cb_dma_check_busy_status(&val32));

    if (val32 != 0)
    {
        LOG_ERROR("[ERROR] DMA status 0: 0x%x\n", val32);
        return E_ERROR;
    }

    val32 = sp38a_regs->status;
    if ((val32 & SP38A_STATUS_ERROR_MASK) != 0)
    {
        LOG_ERROR("[ERROR] SP38A status: 0x%x\n", val32);
        return E_ERROR;
    }

    // post-processing
    if (last == false)
        cb_sp38a_postproc(sp38a_ctx);

    dma_read_output(outbuf, inlen);
    *outlen = inlen;

    return SUCCESS;
}

/**
 * @brief Handle input and update the buffer for block cipher mode of operation
 *
 * @see sp38a_ctx_init().
 * @see __sp38a_ctx_update().
 * @return SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38a_ctx_update(sp38a_op op,
                                      bool encrypt,
                                      uint8_t* outbuf,
                                      uint32_t* outlen,
                                      const uint8_t* inbuf,
                                      uint32_t inlen)
{
    // continue if msg is NULL or msglen is zero
    *outlen = 0;
    if ((inbuf == NULL) || (inlen == 0))
        return SUCCESS;

    pufs_status_t check = SUCCESS;
    uint32_t seglen = 0;
    blsegs segs = cb_segment(sp38a_ctx->buff, sp38a_ctx->buflen, inbuf, inlen,
                          BC_BLOCK_SIZE, sp38a_ctx->minlen);
    sp38a_ctx->buflen = 0;

    for (uint32_t i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if ((check = cb___sp38a_ctx_update(outbuf + *outlen, &seglen,
                                             segs.seg[i].addr, segs.seg[i].len,
                                             false)) != SUCCESS)
            {
                // release sp38a context
                sp38a_ctx->op = SP38A_AVAILABLE;
                return check;
            }
            *outlen += seglen;
            if (sp38a_ctx->start == false)
                sp38a_ctx->start = true;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == sp38a_ctx->buff) &&
                (sp38a_ctx->buflen == 0))
            { // skip copy what already in the right place
                sp38a_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                memcpy(sp38a_ctx->buff + sp38a_ctx->buflen, segs.seg[i].addr,
                        segs.seg[i].len);
                sp38a_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return SUCCESS;
}

/**
 * _pufs_dec_cbc()
 */
pufs_status_t _pufs_dec_cbc_init(pufs_cipher_t cipher,
                                 pufs_key_type_t keytype,
                                 size_t keyaddr,
                                 uint32_t keybits,
                                 const uint8_t* iv,
                                 int csmode)
{
    return cb_sp38a_ctx_init(SP38A_CBC_CLR, cipher, false, keytype,
                          keyaddr, keybits, iv, csmode);
}
pufs_status_t pufs_dec_cbc_update(uint8_t* out,
                                  uint32_t* outlen,
                                  const uint8_t* in,
                                  uint32_t inlen)
{
    return cb_sp38a_ctx_update(SP38A_CBC_CLR, false,
                            out, outlen, in, inlen);
}

pufs_status_t pufs_dec_cbc_final(uint8_t* out,
                                 uint32_t* outlen)
{
    pufs_status_t check = SUCCESS;

    // in final call, it must be minimum-length bytes depending on modes to
    //  pass into the modes of operation module
    // if (sp38a_ctx->buflen < sp38a_ctx->minlen)
    //     check = E_INVALID;
    // else
        check = cb___sp38a_ctx_update(out, outlen, sp38a_ctx->buff,
                                    sp38a_ctx->buflen, true);

    // release sp38a context
    sp38a_ctx->op = SP38A_AVAILABLE;
    return check;
}

pufs_status_t pufs_dec_cbc(uint8_t* out,
                            uint32_t* outlen,
                            const uint8_t* in,
                            uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            size_t keyaddr,
                            uint32_t keybits,
                            const uint8_t* iv,
                            int csmode)
{
    pufs_status_t check;
    uint32_t toutlen;
    *outlen = 0;

    // Call I-U-F model
    if ((check = cb__pufs_dec_cbc_init(cipher, keytype, keyaddr,
                                    keybits, iv, csmode)) != SUCCESS)
        return check;
    if ((check = cb_pufs_dec_cbc_update(out, &toutlen, in, inlen)) != SUCCESS)
        return check;
    *outlen += toutlen;
    if ((check = cb_pufs_dec_cbc_final(out + *outlen, &toutlen)) != SUCCESS)
        return check;
    *outlen += toutlen;
    return check;
}

func_sp38a_get_cfg cb_sp38a_get_cfg = sp38a_get_cfg;
func_sp38a_ctx_init cb_sp38a_ctx_init = sp38a_ctx_init;
func_sp38a_prepare cb_sp38a_prepare = sp38a_prepare;
func_sp38a_postproc cb_sp38a_postproc = sp38a_postproc;
func___sp38a_ctx_update cb___sp38a_ctx_update = __sp38a_ctx_update;
func_sp38a_ctx_update cb_sp38a_ctx_update = sp38a_ctx_update;
func__pufs_dec_cbc_init cb__pufs_dec_cbc_init = _pufs_dec_cbc_init;
func_pufs_dec_cbc_update cb_pufs_dec_cbc_update = pufs_dec_cbc_update;
func_pufs_dec_cbc_final cb_pufs_dec_cbc_final = pufs_dec_cbc_final;
func_pufs_dec_cbc cb_pufs_dec_cbc = pufs_dec_cbc;
#pragma GCC pop_options