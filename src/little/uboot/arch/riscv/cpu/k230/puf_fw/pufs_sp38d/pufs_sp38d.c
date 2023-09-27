/**
 * @file      pufs_sp38d.c
 * @brief     PUFsecurity SP38D API implementation
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
#include "pufs_crypto_internal.h"
#include "pufs_sp38d_internal.h"
#include "pufs_ka_internal.h"
#include "pufs_dma_internal.h"
#include "platform.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

struct pufs_sp38d_regs *sp38d_regs = (struct pufs_sp38d_regs *)(PUFIOT_ADDR_START+SP38D_ADDR_OFFSET);


static pufs_sp38d_ctx sp38d_ctx_static = { .op = SP38D_AVAILABLE };
pufs_sp38d_ctx *sp38d_ctx = &sp38d_ctx_static;

/*****************************************************************************
 * Static functions
 ****************************************************************************/
static pufs_status_t sp38d_get_config(uint32_t *cfg, bool gctr, bool reg_in, bool reg_out)
{
    uint32_t val32;
    switch (sp38d_ctx->cipher)
    {
    case AES:
        val32 = 0x2;
        // switch (sp38d_ctx->keybits)
        // {
        // case 128:
        //     val32 = 0x0;
        //     break;
        // case 192:
        //     val32 = 0x1;
        //     break;
        // case 256:
        //     val32 = 0x2;
        //     break;
        // default:
        //     return E_FIRMWARE;
        // }
        break;
    case SM4:
        val32 = 0x3;
        // switch (sp38d_ctx->keybits)
        // {
        // case 128:
        //     val32 = 0x3;
        //     break;
        // default:
        //     return E_FIRMWARE;
        // }
        break;
    default:
        return E_FIRMWARE;
    }
    if (sp38d_ctx->inbits != ULLONG_MAX)
        val32 |= 0x1 << SP38C_CFG_GHASH_BITS;

    if (gctr)
        val32 |= 0x1 << SP38C_CFG_GCTR_BITS;

    val32 |= (sp38d_ctx->encrypt ? 0x1 : 0x0 ) << SP38C_CFG_ENCRYPT_BITS;

    if (reg_in)
        val32 |= 0x1 << SP38C_CFG_REG_IN_BITS;

    if (reg_out)
        val32 |= 0x1 << SP38C_CFG_REG_OUT_BITS;
    
    *cfg = val32;
    return SUCCESS;
}

/**
 * @brief Initialize the internal context for block cipher GCM mode
 *
 * @param[in] op         GCM operation mode.
 * @param[in] sp38d_ctx  SP38D context to be initialized.
 * @param[in] encrypt    True/false for encryption/decryption
 * @param[in] cipher     Block cipher algorithm.
 * @param[in] keytype    Key type.
 * @param[in] keyaddr    Key address.
 * @param[in] keybits    Key length in bits.
 * @param[in] j0         J_0
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ctx_init(sp38d_op op,
                                    bool encrypt,
                                    pufs_cipher_t cipher,
                                    pufs_key_type_t keytype,
                                    size_t keyaddr,
                                    uint32_t keybits,
                                    const uint8_t* j0)
{
    // check and set J_0 if needed
    if (op != SP38D_GHASH)
    {
        if (j0 == NULL)
            return E_INVALID;
        memcpy(sp38d_ctx->j0, j0, BC_BLOCK_SIZE);
    }

    // initialize for block-cipher GCM mode
    sp38d_ctx->aadbits = 0;
    sp38d_ctx->inbits = 0;
    sp38d_ctx->buflen = 0;
    sp38d_ctx->cipher = cipher;
    sp38d_ctx->encrypt = encrypt;
    sp38d_ctx->op = op;
    sp38d_ctx->minlen = 1;
    sp38d_ctx->incj0 = 1;
    sp38d_ctx->stage = SP38D_NONE;

    memset(sp38d_ctx->ghash, 0, BC_BLOCK_SIZE);

    // set key
    sp38d_ctx->keybits = keybits;
    sp38d_ctx->keytype = keytype;
    // if (keytype != SWKEY)
        sp38d_ctx->keyslot = (uint32_t)keyaddr;
    // else
    //     memcpy(sp38d_ctx->key, (const void*)keyaddr, b2B(keybits));

    return SUCCESS;
}
/**
 * @brief Prepare registers for SP38D operation
 *
 * @param[in] sp38d_ctx  SP38D context.
 * @param[in] inlen      Input length for the preparing operation.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_prepare(const uint8_t* out,
                                   uint32_t inlen)
{
    pufs_status_t check;
    uint32_t val32;

    // if (sp38d_ctx->keytype == SWKEY)
    //     cb_crypto_write_sw_key(sp38d_ctx->key, SW_KEY_MAXLEN);

    cb_dma_write_key_config_0(sp38d_ctx->keytype,
                           ALGO_TYPE_GCM,
                           sp38d_ctx->keybits,
                           cb_get_key_slot_idx(sp38d_ctx->keytype, sp38d_ctx->keyslot));

    if ((check = cb_sp38d_get_config(&val32, out != NULL, false, false)) != SUCCESS)
        return check;

    sp38d_regs->cfg = val32;
    
    
    if (sp38d_ctx->inbits != ULLONG_MAX)
        sp38d_regs->block_num = sp38d_ctx->incj0;
    else
        sp38d_regs->block_num = 0;

    // J_0
    if (out != NULL)
    {
        cb_crypto_write_iv(sp38d_ctx->j0, BC_BLOCK_SIZE);
        if (sp38d_ctx->inbits != ULLONG_MAX && inlen > 0)
            sp38d_ctx->incj0 += ((inlen - 1 + BC_BLOCK_SIZE) / BC_BLOCK_SIZE);
    }

    // Restore GHASH
    cb_crypto_write_dgst(sp38d_ctx->ghash, BC_BLOCK_SIZE);

    return SUCCESS;
}
/**
 * @brief Post-processing for SP38D operation
 *
 * @param[in] sp38d_ctx  SP38D context.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_postproc()
{
    cb_crypto_read_dgest(sp38d_ctx->ghash, BC_BLOCK_SIZE);
    return SUCCESS;
}
/**
 * @brief Pass the input into SP38D hardware
 *
 * @param[in]  sp38d_ctx  SP38D context.
 * @param[out] out        The pointer to the space where the output is written.
 * @param[out] outlen     The length of the output in bytes.
 * @param[in]  in         The input.
 * @param[in]  inlen      The length of the input in bytes.
 * @param[in]  last       True if the input for this operation ends
 * @return                SUCCESS on success, otherwise an error code.
 */
static pufs_status_t __sp38d_ctx_update(uint8_t* out,
                                        uint32_t* outlen,
                                        const uint8_t* in,
                                        uint32_t inlen,
                                        bool last)
{
    uint32_t val32;
    pufs_status_t check;
    // Register manipulation
    if (cb_dma_check_busy_status(NULL))
        return E_BUSY;
    
    cb_dma_write_config_0(false, false, false);

    cb_dma_write_data_block_config(sp38d_ctx->start ? false : true, last, true, true, 0);

    cb_dma_write_rwcfg(out, in, inlen);
    flush_dcache_range((uint64_t *)in, in+inlen);  //csi_dcache_clean_range
    
    if ((check = cb_sp38d_prepare(out, inlen)) != SUCCESS)
        return check;

    if (out != NULL)
    {
        invalidate_dcache_range((uint64_t *)out, out+inlen);//csi_dcache_clean_invalid_range
    }
    cb_dma_write_start();
    while (cb_dma_check_busy_status(&val32));
 
    if (val32 != 0)
    {
        LOG_ERROR("DMA status 0: 0x%x\n", val32);
        return E_ERROR;
    }

    val32 = sp38d_regs->status;
    if ((val32 & SP38D_STATUS_RESP_MASK) != 0)
    {
        LOG_ERROR("SP38D status: 0x%x\n", val32);
        return E_ERROR;
    }

    if ((check = cb_sp38d_postproc()) != SUCCESS)
        return check;

    if (out != NULL) // output
    {
        dma_read_output(out, inlen);
        *outlen = inlen;
    }

    return SUCCESS;
}
/**
 * @brief Input data into SP38D
 *
 * @param[in]  op         GCM operation mode.
 * @param[in]  sp38d_ctx  SP38D context.
 * @param[in]  encrypt    True/false for encryption/decryption
 * @param[out] out        Output data.
 * @param[out] outlen     Output data length in bytes.
 * @param[in]  in         Input data.
 * @param[in]  inlen      Input data length in bytes.
 * @return                SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ctx_update(sp38d_op op,
                                      bool encrypt,
                                      uint8_t* out,
                                      uint32_t* outlen,
                                      const uint8_t* in,
                                      uint32_t inlen)
{
    // // check sp38d_ctx is owned by this operation (GCM mode)
    // if ((sp38d_ctx->op != op) || (sp38d_ctx->encrypt != encrypt))
    //     return E_UNAVAIL;
    // continue if msg is NULL or msglen is zero
    if ((in == NULL) || (inlen == 0))
    {
        if (outlen != NULL)
            *outlen = 0;
        return SUCCESS;
    }

    switch (sp38d_ctx->stage)
    {
    case SP38D_NONE:
        break;
    case SP38D_AAD:
        sp38d_ctx->aadbits += (((uint64_t)inlen) << 3);
        break;
    case SP38D_TEXT:
        sp38d_ctx->inbits += (((uint64_t)inlen) << 3);
        break;
    default:
        return E_FIRMWARE;
    }

    blsegs segs = cb_segment(sp38d_ctx->buff, sp38d_ctx->buflen, in, inlen,
                          BC_BLOCK_SIZE, sp38d_ctx->minlen);
    sp38d_ctx->buflen = 0;

    uint32_t seglen = 0;
    pufs_status_t check = SUCCESS;
    if (sp38d_ctx->stage == SP38D_TEXT)
        *outlen = 0;
    for (uint32_t i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if (sp38d_ctx->stage == SP38D_TEXT)
                check = cb___sp38d_ctx_update(out + *outlen, &seglen,
                                           segs.seg[i].addr, segs.seg[i].len,
                                           false);
            else
                check = cb___sp38d_ctx_update(NULL, NULL, segs.seg[i].addr,
                                           segs.seg[i].len, false);
            if (check != SUCCESS)
            {
                // release sp38d context
                sp38d_ctx->op = SP38D_AVAILABLE;
                return check;
            }
            if (sp38d_ctx->stage == SP38D_TEXT)
                *outlen += seglen;
            if (sp38d_ctx->start == false)
                sp38d_ctx->start = true;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == sp38d_ctx->buff) &&
                (sp38d_ctx->buflen == 0))
            { // skip copy what already in the right place
                sp38d_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                memcpy(sp38d_ctx->buff + sp38d_ctx->buflen, segs.seg[i].addr,
                        segs.seg[i].len);
                sp38d_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return SUCCESS;
}

static pufs_status_t sp38d_tag(uint8_t *tag, uint32_t taglen, bool from_reg)
{
    uint32_t val32, tmplen = 0;
    pufs_status_t check;
    union {
        uint8_t uc[BC_BLOCK_SIZE*4];
        uint32_t u32[BC_BLOCK_SIZE*4 / 4];
    } tmp;

    if (sp38d_ctx->op == SP38D_GHASH)
    {
        memcpy(tag, sp38d_ctx->ghash, taglen);
        return SUCCESS;
    }

    // len(A) || len(C)
    tmp.u32[0] = cb_be2le((uint32_t)(sp38d_ctx->aadbits >> 32));
    tmp.u32[1] = cb_be2le((uint32_t)(sp38d_ctx->aadbits));
    tmp.u32[2] = cb_be2le((uint32_t)(sp38d_ctx->inbits >> 32));
    tmp.u32[3] = cb_be2le((uint32_t)sp38d_ctx->inbits);
    if ((check = cb___sp38d_ctx_update(NULL, NULL, tmp.uc,
                                    BC_BLOCK_SIZE, true)) != SUCCESS)
        return check;

    // last GCTR
    sp38d_ctx->inbits = ULLONG_MAX;

    if (!from_reg)
    {
        if (((check = cb___sp38d_ctx_update(tmp.uc, &tmplen, sp38d_ctx->ghash,
                                         BC_BLOCK_SIZE, true)) != SUCCESS) ||
        (tmplen != BC_BLOCK_SIZE))
            return E_FIRMWARE;
    
        memcpy(tag, tmp.uc, taglen);
        return SUCCESS;
    }

    cb_crypto_write_iv(sp38d_ctx->j0, BC_BLOCK_SIZE);

    if (sp38d_ctx->keytype == SWKEY)
        cb_crypto_write_sw_key(sp38d_ctx->key, SW_KEY_MAXLEN);

    if ((check = cb_sp38d_get_config(&val32, true, true, true)) != SUCCESS)
        return check;

    sp38d_regs->cfg = val32;
    sp38d_regs->block_num = 0;

    cb_dma_write_data_block_config(true, true, true, true, 0);
    cb_dma_write_rwcfg(NULL, NULL, 0);
    cb_dma_write_config_0(false, false, false);
    cb_dma_write_key_config_0(sp38d_ctx->keytype,
                           ALGO_TYPE_GCM,
                           sp38d_ctx->keybits,
                           cb_get_key_slot_idx(sp38d_ctx->keytype, sp38d_ctx->keyslot));
    
    cb_dma_write_start();

    while (cb_dma_check_busy_status(&val32));
 
    if (val32 != 0)
    {
        LOG_ERROR("DMA status 0: 0x%x\n", val32);
        return E_ERROR;
    }

    val32 = sp38d_regs->status;
    if ((val32 & SP38D_STATUS_RESP_MASK) != 0)
    {
        LOG_ERROR("SP38D status: 0x%x\n", val32);
        return E_ERROR;
    }
    cb_crypto_read_dgest(tag, taglen);

    return SUCCESS;
}

/**
 * @brief Finalize current GCM operation mode
 *
 * @param[in]  op         GCM operation mode.
 * @param[in]  sp38d_ctx  SP38D context.
 * @param[in]  encrypt    True/false for encryption/decryption
 * @param[out] out        Output data.
 * @param[out] outlen     Output data length in bytes.
 * @param[out] tag        Output tag.
 * @param[in]  taglen     Specified output tag length in bytes.
 * @return                SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ctx_final(sp38d_op op,
                                     bool encrypt,
                                     uint8_t* out,
                                     uint32_t* outlen,
                                     uint8_t* tag,
                                     uint32_t taglen,
                                     bool from_reg)
{
    pufs_status_t check = SUCCESS;

    if (outlen != NULL)
        *outlen = 0;

    if (sp38d_ctx->buflen != 0)
    {
        if ((check = cb___sp38d_ctx_update(out, outlen, sp38d_ctx->buff,
                                        sp38d_ctx->buflen, true)) != SUCCESS)
            goto release_sp38d;
    }

    check = cb_sp38d_tag(tag, taglen, from_reg);

release_sp38d:
    // release sp38d context
    sp38d_ctx->op = SP38D_AVAILABLE;

    return check;
}
/**
 * @brief Initialize GCM context for GHASH operation
 *
 * @param[in] sp38d_ctx  SP38D context to be initialized.
 * @param[in] cipher     Block cipher algorithm.
 * @param[in] keytype    Key type.
 * @param[in] keyaddr    Key address.
 * @param[in] keybits    Key length in bits.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ghash_init(pufs_cipher_t cipher,
                                      pufs_key_type_t keytype,
                                      size_t keyaddr,
                                      uint32_t keybits)
{
    return cb_sp38d_ctx_init(SP38D_GHASH, false, cipher,
                          keytype, keyaddr, keybits, NULL);
}
/**
 * @brief Input data into GHASH
 *
 * @param[in] sp38d_ctx  SP38D context.
 * @param[in] in         Input data.
 * @param[in] inlen      Input data length in bytes.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ghash_update(const uint8_t* in,
                                        uint32_t inlen)
{
    return cb_sp38d_ctx_update(SP38D_GHASH, false,
                            NULL, NULL, in, inlen);
}
/**
 * @brief Extract GHASH
 *
 * @param[in]  sp38d_ctx  SP38D context.
 * @param[out] out        GHASH value.
 * @param[in]  outlen     GHASH value length in bytes.
 * @return                SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_ghash_final(uint8_t* out,
                                       uint32_t outlen)
{
    return cb_sp38d_ctx_final(SP38D_GHASH, false,
                           NULL, NULL, out, outlen, false);
}
/**
 * @brief Build J_0 for GCM and GMAC operation
 *
 * @param[out] j0       J_0.
 * @param[in]  cipher   Block cipher algorithm.
 * @param[in]  keytype  Key type.
 * @param[in]  keyaddr  Key address.
 * @param[in]  keybits  Key length in bits.
 * @param[in]  iv       Initial vector.
 * @param[in]  ivlen    Initial vector length in bytes.
 * @return              SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_build_j0(uint8_t* j0,
                                    pufs_cipher_t cipher,
                                    pufs_key_type_t keytype,
                                    size_t keyaddr,
                                    uint32_t keybits,
                                    const uint8_t* iv,
                                    uint32_t ivlen)
{
    uint8_t tmp[BC_BLOCK_SIZE];
    uint64_t ivbits = ivlen << 3;
    pufs_status_t check;

    // if ((iv == NULL) || (ivlen == 0))
    //     return E_INVALID;

    if (ivlen == 12)
    {
        memcpy(j0, iv, ivlen);
        *(j0 + 12) = 0;
        *(j0 + 13) = 0;
        *(j0 + 14) = 0;
        *(j0 + 15) = 1;
        return SUCCESS;
    }

    if ((check = cb_sp38d_ghash_init(cipher, keytype,
                                  keyaddr, keybits)) != SUCCESS)
        return check;
    if ((check = cb_sp38d_ghash_update(iv, ivlen)) != SUCCESS)
        return check;
    memset(tmp, 0, BC_BLOCK_SIZE);
    if ((ivlen % BC_BLOCK_SIZE) != 0)
    {
        uint32_t padlen = BC_BLOCK_SIZE - (ivlen % BC_BLOCK_SIZE);
        if ((check = cb_sp38d_ghash_update(tmp, padlen)) != SUCCESS)
            return check;
    }
    *((uint32_t*)(tmp + 8)) = cb_be2le((uint32_t)(ivbits >> 32));
    *((uint32_t*)(tmp + 12)) = cb_be2le((uint32_t)ivbits);
    if ((check = cb_sp38d_ghash_update(tmp, BC_BLOCK_SIZE)) != SUCCESS)
        return check;

    return cb_sp38d_ghash_final(j0, BC_BLOCK_SIZE);
}
/**
 * @brief Step forward SP38D stage
 *
 * @param[in] sp38d_ctx  SP38D context.
 * @param[in] stage      Next SP38D stage.
 * @return             SUCCESS on success, otherwise an error code.
 */
static pufs_status_t sp38d_step_stage(sp38d_stage stage)
{
    switch (sp38d_ctx->stage)
    {
    case SP38D_NONE:
        break;
    case SP38D_AAD:
        switch (stage)
        {
        case SP38D_AAD:
            return SUCCESS;
        case SP38D_TEXT:
            if (sp38d_ctx->buflen != 0)
            { // clear AAD and start input
                pufs_status_t check;
                check = cb___sp38d_ctx_update(NULL, NULL, sp38d_ctx->buff,
                                           sp38d_ctx->buflen, true);
                if (check != SUCCESS)
                    return check;
                sp38d_ctx->buflen = 0;
            }
            break;
        default:
            return E_FIRMWARE;
        }
        break;
    case SP38D_TEXT:
        if (stage != SP38D_TEXT)
            return E_INVALID;
        break;
    default:
        return E_FIRMWARE;
    }

    if (sp38d_ctx->stage != stage)
    {
        sp38d_ctx->start = false;
        sp38d_ctx->stage = stage;
    }
    return SUCCESS;
}
/**
 * @brief Initialize GCM context for GCM operation
 *
 * @param[in] sp38d_ctx  SP38D context.
 * @param[in] cipher     Block cipher algorithm.
 * @param[in] keytype    Key type.
 * @param[in] keyaddr    Key address.
 * @param[in] keybits    Key length in bits.
 * @param[in] iv         Initial vector.
 * @param[in] ivlen      Initial vector length in bytes.
 * @param[in] encrypt    True/false for encryption/decryption.
 * @return               SUCCESS on success, otherwise an error code.
 */
static pufs_status_t pufs_gcm_init(pufs_cipher_t cipher,
                                   pufs_key_type_t keytype,
                                   size_t keyaddr,
                                   uint32_t keybits,
                                   const uint8_t* iv,
                                   uint32_t ivlen,
                                   bool encrypt)
{
    uint8_t j0[BC_BLOCK_SIZE];
    pufs_status_t check;
    if ((check = cb_sp38d_build_j0(j0, cipher, keytype, keyaddr,
                                keybits, iv, ivlen)) != SUCCESS)
        return check;
    return cb_sp38d_ctx_init(SP38D_GCM, encrypt, cipher,
                          keytype, keyaddr, keybits, j0);
}

/*****************************************************************************
 * Internal functions
 ****************************************************************************/

/**
 * pufs_dec_gcm()
 */
pufs_status_t pufs_dec_gcm_init(pufs_cipher_t cipher,
                                 pufs_key_type_t keytype,
                                 size_t keyaddr,
                                 uint32_t keybits,
                                 const uint8_t* iv,
                                 uint32_t ivlen)
{
    return cb_pufs_gcm_init(cipher, keytype, keyaddr, keybits,
                         iv, ivlen, false);
}
pufs_status_t pufs_dec_gcm_update(uint8_t* out,
                                  uint32_t* outlen,
                                  const uint8_t* in,
                                  uint32_t inlen)
{
    pufs_status_t check = cb_sp38d_step_stage((out == NULL) ?
                                           SP38D_AAD : SP38D_TEXT);
    if (check != SUCCESS)
        return check;
    return cb_sp38d_ctx_update(SP38D_GCM, false,
                            out, outlen, in, inlen);
}

pufs_status_t pufs_dec_gcm_final_tag(uint8_t* out,
                                     uint32_t* outlen,
                                     uint8_t* tag,
                                     uint32_t taglen)
{
    return cb_sp38d_ctx_final(SP38D_GCM, false,
                           out, outlen, tag, taglen, false);
}

pufs_status_t pufs_dec_gcm_final(uint8_t* out,
                                 uint32_t* outlen,
                                 const uint8_t* tag,
                                 uint32_t taglen)
{
    uint8_t newtag[BC_BLOCK_SIZE];
    pufs_status_t check;
    if ((check = cb_pufs_dec_gcm_final_tag(out, outlen,
                                        newtag, taglen)) != SUCCESS)
        return check;
    return ((memcmp(tag, newtag, taglen) == 0) ? SUCCESS : E_VERFAIL);
}
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
                            int taglen)
{
    pufs_status_t check;
    uint32_t toutlen;
    *outlen = 0;
        
    // Call I-U-F model
    if ((check = cb_pufs_dec_gcm_init(cipher, keytype, keyaddr,
                                    keybits, iv, ivlen)) != SUCCESS)
        return check;
    if ((check = cb_pufs_dec_gcm_update(NULL, NULL,
                                     aad, aadlen)) != SUCCESS)
        return check;
    if ((check = cb_pufs_dec_gcm_update(out, &toutlen,
                                     in, inlen)) != SUCCESS)
        return check;
    *outlen += toutlen;

    if ((check = cb_pufs_dec_gcm_final(out + *outlen, &toutlen,
                                    tag, taglen)) != SUCCESS)
        return check;
    *outlen += toutlen;
    return check;
}

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
                            int taglen)
{
    pufs_status_t check;

    pufs_ka_slot_t keyslot = (keybits > 128) ? SK256_0 : SK128_0;
    if((check = pufs_import_plaintext_key(keytype, keyslot, keyaddr, keybits)) != SUCCESS)
        return check;

    if((check = pufs_dec_gcm(out, outlen, in, inlen, cipher, keytype, keyslot, keybits, iv, ivlen, aad, aadlen, tag, taglen)) != SUCCESS)
        return check;

    return check;
}


func_sp38d_get_config cb_sp38d_get_config = sp38d_get_config;
func_sp38d_ctx_init cb_sp38d_ctx_init = sp38d_ctx_init;
func_sp38d_prepare cb_sp38d_prepare = sp38d_prepare;
func_sp38d_postproc cb_sp38d_postproc = sp38d_postproc;
func___sp38d_ctx_update cb___sp38d_ctx_update = __sp38d_ctx_update;
func_sp38d_ctx_update cb_sp38d_ctx_update = sp38d_ctx_update;
func_sp38d_tag cb_sp38d_tag = sp38d_tag;
func_sp38d_ctx_final cb_sp38d_ctx_final = sp38d_ctx_final;
func_sp38d_ghash_init cb_sp38d_ghash_init = sp38d_ghash_init;
func_sp38d_ghash_update cb_sp38d_ghash_update = sp38d_ghash_update;
func_sp38d_ghash_final cb_sp38d_ghash_final = sp38d_ghash_final;
func_sp38d_build_j0 cb_sp38d_build_j0 = sp38d_build_j0;
func_sp38d_step_stage cb_sp38d_step_stage = sp38d_step_stage;
func_pufs_gcm_init cb_pufs_gcm_init = pufs_gcm_init;
func_pufs_dec_gcm_init cb_pufs_dec_gcm_init = pufs_dec_gcm_init;
func_pufs_dec_gcm_update cb_pufs_dec_gcm_update = pufs_dec_gcm_update;
func_pufs_dec_gcm_final_tag cb_pufs_dec_gcm_final_tag = pufs_dec_gcm_final_tag;
func_pufs_dec_gcm_final cb_pufs_dec_gcm_final = pufs_dec_gcm_final;
func_pufs_dec_gcm cb_pufs_dec_gcm = pufs_dec_gcm;
#pragma GCC pop_options