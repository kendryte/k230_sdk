/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <ioremap.h>
#include <lwp_user_mm.h>
#include <mmu.h>
#include <cache.h>
#include "riscv_io.h"
#include "riscv_mmu.h"
#include "drv_hardlock.h"
#include "drv_aes.h"

// #define aes_debug

static struct rt_device g_aes_device = {0};
static void *sec_base_addr = RT_NULL;
static int aes_hardlock;
rt_uint8_t pufs_buffer[BUFFER_SIZE];
pufs_ka_slot_t g_keyslot;
struct aes_context aes_ctx = { .op = GCM_AVAILABLE_OP };

//----------------------BASE FUNCTION ----------------------------------------//
#ifdef aes_debug
static void debug_func(char *str, void *p, rt_uint32_t len)
{
    int i;

    if(len > 0) {
        rt_kprintf("the length is: 0x%x\n", len);
        for(i=0; i<len; i++)
            rt_kprintf("%s[%d] = 0x%x\n", str, i, *(rt_uint8_t*)(p + i));
    }
}
#else
static void debug_func(char *str, void *p, rt_uint32_t len)
{
    return;
}
#endif

static rt_uint64_t virt_to_phys(void *virt)
{
    return (rt_uint64_t)virt + PV_OFFSET;
}

static rt_uint32_t be2le(rt_uint32_t var)
{
    return (((0xff000000 & var) >> 24) |
            ((0x00ff0000 & var) >> 8) |
            ((0x0000ff00 & var) << 8) |
            ((0x000000ff & var) << 24));
}

static void write_data(rt_uint32_t *dst, rt_uint8_t *src, rt_size_t length, rt_bool_t le)
{
    rt_uint32_t *src32 = (rt_uint32_t *)src;
    length = length / 4;
        rt_size_t i;

    for (i = 0; i < length; ++i)
    {
        if (le)
            *(dst + i) = be2le(*(src32 + i));
        else
            *(dst + i) = *(src32 + i);
    }
}

static void read_data(rt_uint8_t *dst, rt_uint32_t *src, rt_size_t length, rt_bool_t le)
{
    rt_uint32_t *dst32 = (rt_uint32_t *)dst;
    rt_uint32_t word_nums = 0, rest = 0, last_word;
    word_nums = length / 4;
    rest = length % 4;
    rt_uint32_t i;

    for (i = 0; i < word_nums; i++)
    {
        if (le)
            dst32[i] = be2le(src[i]);
        else
            dst32[i] = src[i];
    }

    if (rest != 0)
    {
        if (le)
            last_word = be2le(src[word_nums]);
        else
            last_word = src[word_nums];

        rt_memcpy(dst + (word_nums * 4), &last_word, rest);
    }
}

static pufs_status_t get_config(rt_uint32_t *cfg, struct aes_context *aes_ctx, rt_bool_t gctr, rt_bool_t reg_in, rt_bool_t reg_out)
{
    rt_uint32_t val32;
    switch (aes_ctx->cipher)
    {
        case AES:
            switch (aes_ctx->keylen << 3)
            {
                case 128:
                    val32 = 0x0;
                    break;
                case 192:
                    val32 = 0x1;
                    break;
                case 256:
                    val32 = 0x2;
                    break;
                default:
                    return E_FIRMWARE;
            }
            break;
        case SM4:
            switch (aes_ctx->keylen << 3)
            {
                case 128:
                    val32 = 0x3;
                    break;
                default:
                    return E_FIRMWARE;
            }
            break;
        default:
            return E_FIRMWARE;
    }
    if (aes_ctx->inlen != ULLONG_MAX)
        val32 |= 0x1 << GCM_CFG_GHASH_BITS;

    if (gctr)
        val32 |= 0x1 << GCM_CFG_GCTR_BITS;

    val32 |= (aes_ctx->encrypt ? 0x1 : 0x0 ) << GCM_CFG_ENCRYPT_BITS;

    if (reg_in)
        val32 |= 0x1 << GCM_CFG_REG_IN_BITS;

    if (reg_out)
        val32 |= 0x1 << GCM_CFG_REG_OUT_BITS;
    
    *cfg = val32;
    return SUCCESS;
}

static k_blsegs segment(rt_uint8_t * buf,
                    rt_uint32_t buflen,
                    const rt_uint8_t* in,
                    rt_uint32_t inlen,
                    rt_uint32_t blocksize,
                    rt_uint32_t minlen)
{
    k_blsegs ret = { .nsegs = 0 };

    // calculate total number of blocks to be processed
    rt_uint32_t nprocblocks = 0;
    if ((buflen + inlen) >= (minlen + blocksize))
        nprocblocks = (buflen + inlen - minlen) / blocksize;

    // no available block for processing, keep input in the internal buffer.
    if (nprocblocks == 0)
    {
        ret.seg[ret.nsegs++] = (k_segstr){ RT_FALSE, buf, buflen };
        ret.seg[ret.nsegs++] = (k_segstr){ RT_FALSE, in, inlen };
        return ret;
    }

    const rt_uint8_t* start = in;
    // some blocks are ready for processing,
    // using bytes in the internal buffer first
    if (buflen != 0)
    {
        // if all data in the internal buffer will be processed
        if (nprocblocks * blocksize >= buflen)
        {
            // fill buffer if not a complete block
            rt_uint32_t proclen = blocksize;
            nprocblocks--;
            while (proclen < buflen)
            {
                proclen += blocksize;
                nprocblocks--;
            }
            rt_memcpy(buf + buflen, start, proclen - buflen);
            ret.seg[ret.nsegs++] = (k_segstr){ RT_TRUE, buf, proclen };
            start += (proclen - buflen);
            inlen -= (proclen - buflen);
        }
        else // some data will be remained in the internal buffer
        {
            ret.seg[ret.nsegs++] = (k_segstr){ RT_TRUE, buf, nprocblocks * blocksize };
            ret.seg[ret.nsegs++] = (k_segstr){ RT_FALSE, buf + nprocblocks * blocksize, buflen - nprocblocks * blocksize };
            nprocblocks = 0;
        }
    }
    // deal with input data
    if (nprocblocks > 0)
    {
        ret.seg[ret.nsegs++] = (k_segstr){ RT_TRUE, start, nprocblocks * blocksize };
    }
    ret.seg[ret.nsegs++] = (k_segstr){ RT_FALSE, start + nprocblocks * blocksize, inlen - nprocblocks * blocksize };
    return ret;
}

//---------------------------------CRYTPO FUNCTION----------------------------------------//
static pufs_status_t crypto_write_sw_key(rt_uint8_t *key, rt_size_t length)
{
    if (length > SW_KEY_MAXLEN)
        return E_INVALID;
    
    write_data((rt_uint32_t *)(sec_base_addr + CRYPTO_SW_KEY_OFFSET), key, length, RT_TRUE);
    return SUCCESS;
}

static pufs_status_t crypto_write_iv(rt_uint8_t *iv, rt_size_t length)
{
    if (length > IV_MAXLEN)
        return E_INVALID;
    
    write_data((rt_uint32_t *)(sec_base_addr + CRYPTO_IV_OFFSET), iv, length, RT_TRUE);
    return SUCCESS;
}

static pufs_status_t crypto_write_dgst(rt_uint8_t *dgst, rt_size_t length)
{
    if (length > DGST_INT_STATE_LEN)
        return E_INVALID;
    
    write_data((rt_uint32_t *)(sec_base_addr + CRYPTO_DGST_IN_OFFSET), dgst, length, RT_TRUE);
    return SUCCESS;
}

static void crypto_read_dgest(rt_uint8_t *out, rt_size_t length)
{
    length = length < DGST_INT_STATE_LEN ? length : DGST_INT_STATE_LEN;
    read_data(out, (void *)(sec_base_addr + CRYPTO_DGST_OUT_OFFSET), length, RT_TRUE);
}

//----------------------------------------DMA FUNCTION----------------------------------------//
static void dma_write_start(void)
{
    writel(0x1, sec_base_addr + DMA_START_OFFSET);
}

static rt_bool_t dma_check_busy_status(rt_uint32_t *status)
{
    rt_uint32_t stat = readl(sec_base_addr + DMA_STAT_0_OFFSET);
    rt_bool_t busy = (stat & DMA_STATUS_0_BUSY_MASK) != 0;
    
    if (status != RT_NULL)
        *status = stat;

    return busy;
}

static void dma_write_config_0(rt_bool_t rng_enable, rt_bool_t sgdma_enable, rt_bool_t no_cypt)
{
    rt_uint32_t value = 0;

    if (rng_enable)
        value |= 0x1;
    if (sgdma_enable)
        value |= 0x1 << 1;
    if (no_cypt)
        value |= 0x1 << 2;

    writel(value, sec_base_addr + DMA_CFG_0_OFFSET);
}

static void dma_write_data_block_config(rt_bool_t head, rt_bool_t tail, rt_bool_t dn_intrpt, rt_bool_t dn_pause, rt_uint32_t offset)
{
    rt_uint32_t value = 0;

    if (head)
        value |= 0x1 << DMA_DSC_CFG_4_HEAD_BITS;
    if (tail)
        value |= 0x1 << DMA_DSC_CFG_4_TAIL_BITS;
    if (dn_intrpt)
        value |= 0x1 << DMA_DSC_CFG_4_DN_INTRPT_BITS;
    if (dn_pause);
        value |= 0x1 << DMA_DSC_CFG_4_DN_PAUSE_BITS;
    value |= offset << DMA_DSC_CFG_4_OFFSET_BITS;

    writel(value, sec_base_addr + DMA_DSC_CFG_4_OFFSET);
}

static void dma_write_rwcfg(const rt_uint8_t *out, const rt_uint8_t *in, rt_uint32_t len, void *in_kvirt, void *out_kvirt)
{
    rt_uint64_t in_pa, out_pa;

    writel(len, sec_base_addr + DMA_DSC_CFG_2_OFFSET);

    if(RT_NULL == out)
        writel((uintptr_t)out, sec_base_addr + DMA_DSC_CFG_1_OFFSET);
    else
    {
        rt_hw_cpu_dcache_clean_flush(out_kvirt, len);
        out_pa = virt_to_phys((void*)out_kvirt);
        rt_hw_cpu_dcache_clean_flush((void *)out_pa, len);
        writel(out_pa, sec_base_addr + DMA_DSC_CFG_1_OFFSET);
    }

    if(RT_NULL == in)
        writel((uintptr_t)in, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    else    // virtual to physical
    {
        rt_memcpy(in_kvirt, (void *)in, len);

        rt_hw_cpu_dcache_clean_flush(in_kvirt, len);
        in_pa = virt_to_phys(in_kvirt);
        rt_hw_cpu_dcache_clean_flush((void *)in_pa, len);
        writel(in_pa, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    }
}

static void dma_write_key_config_0(pufs_key_type_t keytype, pufs_algo_type_t algo, rt_uint32_t size, rt_uint32_t slot_index)
{
    rt_uint32_t value = 0;

    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;

    writel(value, sec_base_addr + DMA_KEY_CFG_0_OFFSET);
}

//-----------------------------------------KWP FUNCTION----------------------------------------//
static pufs_status_t pufs_kwp_start(void)
{
    rt_uint32_t val32;
    writel(0x1, sec_base_addr + KWP_START_OFFSET);
    while(((val32 = readl(sec_base_addr + KWP_STATUS_OFFSET)) & KWP_STATUS_BUSY_MASK) != 0);

    if (val32 & (0x1 << 2))
        return E_DENY;
    else if (val32 & (0x1 << 3))
        return E_OVERFLOW;
    else if (val32 & (0x1 << 4))
        return E_UNDERFLOW;
    else if (val32 & (0x1 << 5))
        return E_VERFAIL;
    else if (val32 != 0)
    {
        rt_kprintf("Error, KWP status: 0x%08x\n", val32);
        return E_ERROR;
    }

    return SUCCESS;
}

//--------------------------------------KEY SLOT FUNCTION----------------------------------------//
static pufs_status_t ka_skslot_check(rt_bool_t valid, pufs_ka_slot_t slot, uint32_t keybits)
{
    // check keybits
    switch (slot)
    {
        case SK128_0:
        case SK128_1:
        case SK128_2:
        case SK128_3:
        case SK128_4:
        case SK128_5:
        case SK128_6:
        case SK128_7:
            if (keybits > 128)
                return E_OVERFLOW;
            break;
        case SK256_0:
        case SK256_1:
        case SK256_2:
        case SK256_3:
            if (keybits > 256)
                return E_OVERFLOW;
            else if ((keybits <= 128) && (keybits != 0))
                return E_UNDERFLOW;
            break;
        case SK512_0:
        case SK512_1:
            if (keybits > 512)
                return E_OVERFLOW;
            else if ((keybits <= 256) && (keybits != 0))
                return E_UNDERFLOW;
            break;
        default:
            return E_INVALID;
    }

    // check registers
    if (valid)
    {
        rt_uint32_t key_info;
        rt_uint32_t idx;
        rt_uint32_t tagbase;

        switch (slot)
        {
            case SK128_0:
            case SK128_1:
            case SK128_2:
            case SK128_3:
            case SK128_4:
            case SK128_5:
            case SK128_6:
            case SK128_7:
                idx = slot - SK128_0;
                tagbase = 0x30;
                key_info = readl(sec_base_addr + KA_SK_0_OFFSET + idx * 4);
                if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                    (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                    (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)))
                    return E_INVALID;
                break;
            case SK256_0:
            case SK256_1:
            case SK256_2:
            case SK256_3:
                idx = slot - SK256_0;
                tagbase = 0x50;
                key_info = readl(sec_base_addr + KA_SK_0_OFFSET + idx * 2 * 4);
                if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                    (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                    (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
                    (readl(sec_base_addr + KA_SK_0_OFFSET + idx * 2 * 4 + 4) != ((tagbase + idx) << 16)))
                    return E_INVALID;
                break;
            case SK512_0:
            case SK512_1:
                idx = slot - SK512_0;
                tagbase = 0x60;
                key_info = readl(sec_base_addr + KA_SK_0_OFFSET + idx * 4 * 4);
                if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                    (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                    (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
                    (readl(sec_base_addr + KA_SK_0_OFFSET + idx * 4 * 4 + 4) != ((tagbase + idx) << 16)) ||
                    (readl(sec_base_addr + KA_SK_0_OFFSET + idx * 4 * 4 + 8) != ((tagbase + idx) << 16)) ||
                    (readl(sec_base_addr + KA_SK_0_OFFSET + idx * 4 * 4 + 12) != ((tagbase + idx) << 16)))
                    return E_INVALID;
                break;
            default:
                return E_INVALID;
        }
    }

    return SUCCESS;
}

static int get_key_slot_idx(pufs_key_type_t keytype, rt_uint32_t keyslot)
{
    switch (keytype)
    {
        case SWKEY:
            return 0;
        case OTPKEY:
            // TODO
        case PUFKEY:
            // TODO
        case SSKEY:
            switch ((pufs_ka_slot_t)keyslot)
            {
                case SK128_0:
                case SK128_1:
                case SK128_2:
                case SK128_3:
                case SK128_4:
                case SK128_5:
                case SK128_6:
                case SK128_7:
                    return (keyslot - SK128_0);
                case SK256_0:
                case SK256_1:
                case SK256_2:
                case SK256_3:
                    return ((keyslot - SK256_0) * 2);
                case SK512_0:
                case SK512_1:
                    return ((keyslot - SK512_0) * 4);
                default:
                    return -1;
            }
            break;
        default:
            return -1;
    }
}

static pufs_status_t keyslot_check(rt_bool_t valid, pufs_key_type_t keytype, rt_uint32_t slot, rt_uint32_t keybits)
{
    switch (keytype)
    {
        case SSKEY:
            return ka_skslot_check(valid, (pufs_ka_slot_t)slot, keybits);
        case OTPKEY:
            // TODO
        case PRKEY:
            // TODO
        default:
            return E_INVALID;
    }
}

static pufs_status_t clear_ka_slot(pufs_ka_slot_t slot)
{
    rt_uint32_t val32;
    switch (slot)
    {
        case SK128_0:
        case SK128_1:
        case SK128_2:
        case SK128_3:
        case SK128_4:
        case SK128_5:
        case SK128_6:
        case SK128_7:
            val32 = 0x1<<(slot-SK128_0);
            writel(val32, sec_base_addr + KA_SK_FREE_OFFSET);
            return SUCCESS;
        case SK256_0:
        case SK256_1:
        case SK256_2:
        case SK256_3:
            val32 = 0x3<<(2*(slot-SK256_0));
            writel(val32, sec_base_addr + KA_SK_FREE_OFFSET);
            return SUCCESS;
        case SK512_0:
        case SK512_1:
            val32 = 0xf<<(4*(slot-SK512_0));
            writel(val32, sec_base_addr + KA_SK_FREE_OFFSET);
            return SUCCESS;
        case PRK_0:
        case PRK_1:
        case PRK_2:
            val32 = 0x1<<(slot-PRK_0);
            writel(val32, sec_base_addr + KA_SK_FREE_OFFSET);
            return SUCCESS;
        case SHARESEC_0:
            val32 = 0x1<<(slot-SHARESEC_0);
            writel(val32, sec_base_addr + KA_SK_FREE_OFFSET);
            return SUCCESS;
        default:
            return E_INVALID;
    }
}

static pufs_status_t pufs_import_plaintext_key(pufs_key_type_t keytype,
                                        pufs_ka_slot_t slot,
                                        const rt_uint8_t* key,
                                        rt_uint32_t keybits)
{
    pufs_status_t check;
    // keytype MUST be either SSKEY or PRKEY
    if ((keytype != SSKEY) && (keytype != PRKEY))
        return E_INVALID;
    // check KA key slot by key length
    if ((check = keyslot_check(RT_FALSE, keytype, slot, keybits)) != SUCCESS)
        return check;

    // Register manipulation
    if (readl(sec_base_addr + KWP_STATUS_OFFSET) & KWP_STATUS_BUSY_MASK)
        return E_BUSY;

    rt_uint32_t val32;

    val32 = 0x0<<0 | keybits<<8;
    switch (keytype)
    {
        case SSKEY:
            val32 |= 0x0<<19;
            break;
        case PRKEY:
            val32 |= 0x1<<19;
            break;
        default:
            return E_FIRMWARE;
    }
    val32 |= get_key_slot_idx(keytype, slot)<<20;
    writel(val32, sec_base_addr + KWP_CONFIG_OFFSET);

    rt_memset(pufs_buffer, 0, KWP_KEY_MAXLEN);
    rt_memcpy(pufs_buffer, key, b2B(keybits));

    rt_uint32_t *buf = (rt_uint32_t *)pufs_buffer;
    int i;
    for (i = 0; i < KWP_KEY_MAXLEN / 4; ++i)
        writel(be2le(buf[i]), sec_base_addr + KWP_KEY_IN_OUT_OFFSET + 4*i);

    return pufs_kwp_start();
}

static pufs_status_t pufs_clear_key(pufs_key_type_t keytype, pufs_ka_slot_t slot, rt_uint32_t keybits)
{
    pufs_status_t check;
    // keytype MUST be one of SSKEY, PRKEY, or SHARESEC
    if ((keytype != SSKEY) && (keytype != PRKEY) && (keytype != SHARESEC))
        return E_INVALID;
    // check KA key slot by key length
    if ((check = keyslot_check(RT_TRUE, keytype, slot, keybits)) != SUCCESS)
        return check;

    return clear_ka_slot(slot);
}


//--------------------------------CONTEXT FUNCTION----------------------------------------//
static pufs_status_t gcm_prepare(struct aes_context* aes_ctx, const rt_uint8_t* out, rt_uint32_t inlen)
{
    pufs_status_t check;
    rt_uint32_t val32;

    if (aes_ctx->keytype == SWKEY)
        crypto_write_sw_key(aes_ctx->key, SW_KEY_MAXLEN);

    dma_write_key_config_0(aes_ctx->keytype, ALGO_TYPE_GCM, (aes_ctx->keylen << 3), get_key_slot_idx(aes_ctx->keytype, aes_ctx->keyslot));

    if ((check = get_config(&val32, aes_ctx, out != RT_NULL, RT_FALSE, RT_FALSE)) != SUCCESS)
        return check;

    writel(val32, sec_base_addr + GCM_CFG_1_OFFSET);
    
    if (aes_ctx->inlen != ULLONG_MAX)
        writel(aes_ctx->incj0, sec_base_addr + GCM_CFG_2_OFFSET);
    else
        writel(0x0, sec_base_addr + GCM_CFG_2_OFFSET);

    // J_0
    if (out != RT_NULL)
    {
        crypto_write_iv(aes_ctx->j0, AES_BLOCK_SIZE);
        if (aes_ctx->inlen != ULLONG_MAX && inlen > 0)
            aes_ctx->incj0 += ((inlen - 1 + AES_BLOCK_SIZE) / AES_BLOCK_SIZE);
    }

    // Restore GHASH
    crypto_write_dgst(aes_ctx->ghash, AES_BLOCK_SIZE);

    return SUCCESS;
}

static pufs_status_t gcm_postproc(struct aes_context* aes_ctx)
{
    crypto_read_dgest(aes_ctx->ghash, AES_BLOCK_SIZE);
    return SUCCESS;
}

static pufs_status_t _ctx_update(struct aes_context* aes_ctx,
                                rt_uint8_t* out,
                                rt_uint32_t* outlen,
                                const rt_uint8_t* in,
                                rt_uint32_t inlen,
                                rt_bool_t last)
{
    rt_uint32_t val32;
    pufs_status_t check;
    // Register manipulation
    if (dma_check_busy_status(RT_NULL))
        return E_BUSY;

    debug_func("in", in, inlen);
    debug_func("out", out, inlen);

    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_data_block_config(aes_ctx->start ? RT_FALSE : RT_TRUE, last, RT_TRUE, RT_TRUE, 0);

    void *in_kvirt, *out_kvirt;
    in_kvirt = rt_malloc(inlen);
    out_kvirt = rt_malloc(inlen);
    if(!in_kvirt || !out_kvirt)
    {
        rt_kprintf("%s malloc fail!\n", __func__);
        check = -E_ERROR;
        return check;
    }
    dma_write_rwcfg(out, in, inlen, in_kvirt, out_kvirt);
  
    if ((check = gcm_prepare(aes_ctx, out, inlen)) != SUCCESS)
        return check;

    dma_write_start();
    while (dma_check_busy_status(&val32));
 
    if (val32 != 0)
    {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

    val32 = readl(sec_base_addr + GCM_STAT_OFFSET);
    if ((val32 & GCM_STATUS_RESP_MASK) != 0)
    {
        rt_kprintf("[ERROR] GCM status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

    if ((check = gcm_postproc(aes_ctx)) != SUCCESS)
        return check;

    if (out != RT_NULL) // output
    {
        *outlen = inlen;
        rt_memcpy(out, out_kvirt, inlen);
    }
    rt_free(in_kvirt);
    rt_free(out_kvirt);

    return SUCCESS;
}

static pufs_status_t gcm_tag(struct aes_context *aes_ctx, rt_uint8_t *tag, rt_uint32_t taglen, rt_bool_t from_reg)
{
    rt_uint32_t val32, tmplen = 0;
    pufs_status_t check;
    union {
        rt_uint8_t uc[AES_BLOCK_SIZE];
        rt_uint32_t u32[AES_BLOCK_SIZE / 4];
    } tmp;

    if (aes_ctx->op == GCM_GHASH_OP)
    {
        rt_memcpy(tag, aes_ctx->ghash, taglen);
        return SUCCESS;
    }

    // len(A) || len(C)
    tmp.u32[0] = be2le((rt_uint32_t)((aes_ctx->aadlen << 3) >> 32));
    tmp.u32[1] = be2le((rt_uint32_t)(aes_ctx->aadlen << 3));
    tmp.u32[2] = be2le((rt_uint32_t)((aes_ctx->inlen << 3) >> 32));
    tmp.u32[3] = be2le((rt_uint32_t)(aes_ctx->inlen << 3));
    if ((check = _ctx_update(aes_ctx, RT_NULL, RT_NULL, tmp.uc, AES_BLOCK_SIZE, RT_TRUE)) != SUCCESS)
        return check;

    // last GCTR
    aes_ctx->inlen = ULLONG_MAX;

    if (!from_reg)
    {
        if (((check = _ctx_update(aes_ctx, tmp.uc, &tmplen, aes_ctx->ghash, AES_BLOCK_SIZE, RT_TRUE)) != SUCCESS) ||
        (tmplen != AES_BLOCK_SIZE))
            return E_FIRMWARE;
    
        rt_memcpy(tag, tmp.uc, taglen);

        return SUCCESS;
    }

    crypto_write_iv(aes_ctx->j0, AES_BLOCK_SIZE);

    if (aes_ctx->keytype == SWKEY)
        crypto_write_sw_key(aes_ctx->key, SW_KEY_MAXLEN);

    if ((check = get_config(&val32, aes_ctx, RT_TRUE, RT_TRUE, RT_TRUE)) != SUCCESS)
        return check;

    writel(val32, sec_base_addr + GCM_CFG_1_OFFSET);
	writel(0, sec_base_addr + GCM_CFG_2_OFFSET);

    dma_write_data_block_config(RT_TRUE, RT_TRUE, RT_TRUE, RT_TRUE, 0);
    dma_write_rwcfg(RT_NULL, RT_NULL, 0, RT_NULL, RT_NULL);
    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_key_config_0(aes_ctx->keytype, ALGO_TYPE_GCM, aes_ctx->keylen << 3,
                           get_key_slot_idx(aes_ctx->keytype, aes_ctx->keyslot));
    
    dma_write_start();

    while (dma_check_busy_status(&val32));
 
    if (val32 != 0)
    {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

    val32 = readl(sec_base_addr + GCM_STAT_OFFSET);
    if ((val32 & GCM_STATUS_RESP_MASK) != 0)
    {
        rt_kprintf("[ERROR] GCM status 0: 0x%08x\n", val32);
        return E_ERROR;
    }
    crypto_read_dgest(tag, taglen);

    return SUCCESS;
}

static pufs_status_t ctx_update(gcm_op op,
                                struct aes_context* aes_ctx,
                                rt_bool_t encrypt,
                                rt_uint8_t* out,
                                rt_uint32_t* outlen,
                                const rt_uint8_t* in,
                                rt_uint32_t inlen)
{
    // check aes_ctx is owned by this operation (GCM mode)
    if ((aes_ctx->op != op) || (aes_ctx->encrypt != encrypt))
        return E_UNAVAIL;
    // continue if msg is NULL or msglen is zero
    if ((in == RT_NULL) || (inlen == 0))
    {
        if (outlen != RT_NULL)
            *outlen = 0;
        return SUCCESS;
    }

    switch (aes_ctx->stage)
    {
        case GCM_NONE_S:
            break;
        case GCM_AAD_S:
            aes_ctx->aadlen += (rt_uint64_t)inlen;
            break;
        case GCM_TEXT_S:
            aes_ctx->inlen += (rt_uint64_t)inlen;
            break;
        default:
            return E_FIRMWARE;
    }

    k_blsegs segs = segment(aes_ctx->buff, aes_ctx->buflen, in, inlen, AES_BLOCK_SIZE, aes_ctx->minlen);
    aes_ctx->buflen = 0;
    rt_uint32_t seglen = 0;
    pufs_status_t check = SUCCESS;
    if (aes_ctx->stage == GCM_TEXT_S)
        *outlen = 0;

    rt_uint32_t i;
    for (i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if (aes_ctx->stage == GCM_TEXT_S)
                check = _ctx_update(aes_ctx, out + *outlen, &seglen, segs.seg[i].addr, segs.seg[i].len, RT_FALSE);
            else
                check = _ctx_update(aes_ctx, RT_NULL, RT_NULL, segs.seg[i].addr, segs.seg[i].len, RT_FALSE);
            if (check != SUCCESS)
            {
                // release context
                aes_ctx->op = GCM_AVAILABLE_OP;
                return check;
            }
            if (aes_ctx->stage == GCM_TEXT_S)
                *outlen += seglen;
            if (aes_ctx->start == RT_FALSE)
                aes_ctx->start = RT_TRUE;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == aes_ctx->buff) && (aes_ctx->buflen == 0))
            { // skip copy what already in the right place
                aes_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                rt_memmove(aes_ctx->buff + aes_ctx->buflen, segs.seg[i].addr, segs.seg[i].len);
                aes_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return SUCCESS;
}

static pufs_status_t ctx_init(gcm_op op,
                                struct aes_context* aes_ctx,
                                rt_bool_t encrypt,
                                pufs_cipher_t cipher,
                                pufs_key_type_t keytype,
                                rt_size_t keyaddr,
                                rt_uint32_t keybits,
                                const rt_uint8_t* j0)
{
    rt_uint32_t val32;
    pufs_status_t check;

    // abort if aes_ctx is occupied
    if (aes_ctx->op != GCM_AVAILABLE_OP)
        return E_BUSY;

    // check if op is valid
    switch (op)
    {
        case GCM_GHASH_OP:
        case AES_GCM_OP:
        case GCM_GMAC_OP:
            break;
        default:
            return E_INVALID;
    }

    // check keytype
    if ((keytype == PUFKEY) || (keytype == SHARESEC))
        return E_DENY;

    // check key settings for block cipher
    if ((keytype != SWKEY) && ((check = keyslot_check(RT_TRUE, keytype, (rt_uint32_t)keyaddr, keybits)) != SUCCESS))
        return check;

    // check and set J_0 if needed
    if (op != GCM_GHASH_OP)
    {
        if (j0 == RT_NULL)
            return E_INVALID;
        rt_memcpy(aes_ctx->j0, j0, AES_BLOCK_SIZE);
    }

    // initialize for block-cipher GCM mode
    aes_ctx->aadlen = 0;
    aes_ctx->inlen = 0;
    aes_ctx->buflen = 0;
    aes_ctx->cipher = cipher;
    aes_ctx->encrypt = encrypt;
    aes_ctx->op = op;
    aes_ctx->minlen = 1;
    aes_ctx->incj0 = 1;
    aes_ctx->stage = GCM_NONE_S;
    memset(aes_ctx->ghash, 0, AES_BLOCK_SIZE);
    // set key
    aes_ctx->keylen = keybits >> 3;
    aes_ctx->keytype = keytype;
    if (keytype != SWKEY)
        aes_ctx->keyslot = (rt_uint32_t)keyaddr;
    else
        rt_memcpy(aes_ctx->key, (const void*)keyaddr, b2B(keybits));

    return SUCCESS;
}

static pufs_status_t step_stage(struct aes_context* aes_ctx, gcm_stage stage)
{
    switch (aes_ctx->stage)
    {
        case GCM_NONE_S:
            break;
        case GCM_AAD_S:
            switch (stage)
            {
                case GCM_AAD_S:
                    return SUCCESS;
                case GCM_TEXT_S:
                    if (aes_ctx->buflen != 0)
                    { // clear AAD and start input
                        pufs_status_t check;
                        check = _ctx_update(aes_ctx, RT_NULL, RT_NULL, aes_ctx->buff, aes_ctx->buflen, RT_TRUE);
                        if (check != SUCCESS)
                            return check;
                        aes_ctx->buflen = 0;
                    }
                    break;
                default:
                    return E_FIRMWARE;
            }
            break;
        case GCM_TEXT_S:
            if (stage != GCM_TEXT_S)
                return E_INVALID;
            break;
        default:
            return E_FIRMWARE;
    }

    if (aes_ctx->stage != stage)
    {
        aes_ctx->start = RT_FALSE;
        aes_ctx->stage = stage;
    }
    return SUCCESS;
}

//----------------------INIT-UPDATE-FINAL FUNCTION----------------------------------------//
static pufs_status_t pufs_gcm_final(gcm_op op,
                                    struct aes_context *aes_ctx,
                                    rt_bool_t encrypt,
                                    rt_uint8_t *out,
                                    rt_uint32_t *outlen,
                                    rt_uint8_t *tag,
                                    rt_uint32_t taglen,
                                    rt_bool_t from_reg)
{
    pufs_status_t check = SUCCESS;

    if (outlen != RT_NULL)
        *outlen = 0;

    // check aes_ctx is owned by this operation (GCM mode)
    if ((aes_ctx->op != op) || (aes_ctx->encrypt != encrypt))
        return E_UNAVAIL;

    if (aes_ctx->buflen != 0)
    {
        if ((check = _ctx_update(aes_ctx, out, outlen, aes_ctx->buff, aes_ctx->buflen, RT_TRUE)) != SUCCESS)
            goto release_gcm;
    }

    check = gcm_tag(aes_ctx, tag, taglen, from_reg);

release_gcm:
    // release gcm context
    aes_ctx->op = GCM_AVAILABLE_OP;

    return check;
}

static pufs_status_t build_j0(rt_uint8_t* j0,
                                pufs_cipher_t cipher,
                                pufs_key_type_t keytype,
                                rt_size_t keyaddr,
                                rt_uint32_t keybits,
                                const rt_uint8_t* iv,
                                rt_uint32_t ivlen)
{
    rt_uint8_t tmp[AES_BLOCK_SIZE];
    rt_uint64_t ivbits = ivlen << 3;
    pufs_status_t check;

    if ((iv == RT_NULL) || (ivlen == 0))
        return E_INVALID;

    if (ivlen == 12)
    {
        rt_memcpy(j0, iv, ivlen);
        rt_memset(j0 + ivlen, 0, 3);
        *(j0 + 15) = 1;
        return SUCCESS;
    }
    else
        return E_UNSUPPORT;

#if 0
    struct aes_context aes_ctx = { .op = GCM_AVAILABLE_OP };
    if ((check = ctx_init(GCM_GHASH_OP, &aes_ctx, RT_FALSE, cipher, keytype, keyaddr, keybits, RT_NULL)) != SUCCESS)
        return check;
    if ((check = ctx_update(GCM_GHASH_OP, &aes_ctx, RT_FALSE, RT_NULL, RT_NULL, iv, ivlen)) != SUCCESS)
        return check;
    rt_memset(tmp, 0, AES_BLOCK_SIZE);
    if ((ivlen % AES_BLOCK_SIZE) != 0)
    {
        rt_uint32_t padlen = AES_BLOCK_SIZE - (ivlen % AES_BLOCK_SIZE);
        if ((check = ctx_update(GCM_GHASH_OP, &aes_ctx, RT_FALSE, RT_NULL, RT_NULL, tmp, padlen)) != SUCCESS)
            return check;
    }
    *((rt_uint32_t*)(tmp + 8)) = be2le((rt_uint32_t)(ivbits >> 32));
    *((rt_uint32_t*)(tmp + 12)) = be2le((rt_uint32_t)ivbits);
    if ((check = ctx_update(GCM_GHASH_OP, &aes_ctx, RT_FALSE, RT_NULL, RT_NULL, tmp, AES_BLOCK_SIZE)) != SUCCESS)
        return check;

    return pufs_gcm_final(GCM_GHASH_OP, &aes_ctx, RT_FALSE, RT_NULL, RT_NULL, j0, AES_BLOCK_SIZE, RT_FALSE);
#endif
}

static pufs_status_t pufs_dec_gcm_final(struct aes_context *aes_ctx,
                                        rt_uint8_t *out,
                                        rt_uint32_t *outlen,
                                        const rt_uint8_t *tag,
                                        rt_uint32_t taglen)
{
    rt_uint8_t newtag[AES_BLOCK_SIZE];
    pufs_status_t check;

    if ((check = pufs_gcm_final(AES_GCM_OP, aes_ctx, RT_FALSE, out, outlen, newtag, taglen, RT_FALSE)) != SUCCESS)
        return check;

    return ((rt_memcmp(tag, newtag, taglen) == 0) ? SUCCESS : E_VERFAIL);
}

static pufs_status_t pufs_gcm_update(struct aes_context *aes_ctx,
                                    rt_uint8_t *out,
                                    rt_uint32_t *outlen,
                                    const rt_uint8_t *in,
                                    rt_uint32_t inlen,
                                    rt_bool_t encrypt)
{
    pufs_status_t check = step_stage(aes_ctx, (out == RT_NULL) ? GCM_AAD_S : GCM_TEXT_S);
    if (check != SUCCESS)
        return check;
    return ctx_update(AES_GCM_OP, aes_ctx, encrypt, out, outlen, in, inlen);
}

static pufs_status_t pufs_gcm_init(struct aes_context *aes_ctx,
                                    pufs_cipher_t cipher,
                                    pufs_key_type_t keytype,
                                    rt_size_t keyaddr,
                                    rt_uint32_t keybits,
                                    const rt_uint8_t *iv,
                                    rt_uint32_t ivlen,
                                    rt_bool_t encrypt)
{
    rt_uint8_t j0[AES_BLOCK_SIZE];
    pufs_status_t check;

    if ((check = build_j0(j0, cipher, keytype, keyaddr, keybits, iv, ivlen)) != SUCCESS)
        return check;

    return ctx_init(AES_GCM_OP, aes_ctx, encrypt, cipher, keytype, keyaddr, keybits, j0);
}

//----------------------ENCRYPT FUNCTION----------------------------------------//
static pufs_status_t pufs_enc_gcm(rt_uint8_t* out,
                            rt_uint32_t* outlen,
                            const rt_uint8_t* in,
                            rt_uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            rt_size_t keyaddr,
                            rt_uint32_t keybits,
                            const rt_uint8_t* iv,
                            rt_uint32_t ivlen,
                            const rt_uint8_t* aad,
                            rt_uint32_t aadlen,
                            rt_uint8_t* tag,
                            rt_uint32_t taglen)
{
    pufs_status_t check;
    rt_uint32_t toutlen;
    *outlen = 0;

    // Call I-U-F model
    if ((check = pufs_gcm_init(&aes_ctx, cipher, keytype, keyaddr, keybits, iv, ivlen, RT_TRUE)) != SUCCESS)
        return check;

    if ((check = pufs_gcm_update(&aes_ctx, RT_NULL, RT_NULL, aad, aadlen, RT_TRUE)) != SUCCESS)
        return check;

    if ((check = pufs_gcm_update(&aes_ctx, out, &toutlen, in, inlen, RT_TRUE)) != SUCCESS)
        return check;
    *outlen += toutlen;

    if ((check = pufs_gcm_final(AES_GCM_OP, &aes_ctx, RT_TRUE, out + *outlen, &toutlen, tag, taglen, RT_TRUE)) != SUCCESS)
        return check;
    *outlen += toutlen;

    return check;
}

//----------------------DECRYPT FUNCTION----------------------------------------//
static pufs_status_t pufs_dec_gcm(rt_uint8_t* out,
                            rt_uint32_t* outlen,
                            const rt_uint8_t* in,
                            rt_uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            rt_size_t keyaddr,
                            rt_uint32_t keybits,
                            const rt_uint8_t* iv,
                            int ivlen,
                            const rt_uint8_t* aad,
                            int aadlen,
                            const rt_uint8_t* tag,
                            int taglen)
{
    pufs_status_t check;
    rt_uint32_t toutlen;
    rt_uint8_t newtag[AES_BLOCK_SIZE];

    *outlen = 0;

    // Call I-U-F model
    if ((check = pufs_gcm_init(&aes_ctx, cipher, keytype, keyaddr, keybits, iv, ivlen, RT_FALSE)) != SUCCESS)
        return check;

    if ((check = pufs_gcm_update(&aes_ctx, RT_NULL, RT_NULL, aad, aadlen, RT_FALSE)) != SUCCESS)
        return check;

    if ((check = pufs_gcm_update(&aes_ctx, out, &toutlen, in, inlen, RT_FALSE)) != SUCCESS)
        return check;
    *outlen += toutlen;

    if ((check = pufs_gcm_final(AES_GCM_OP, &aes_ctx, RT_FALSE, out + *outlen, &toutlen, newtag, taglen, RT_FALSE)) != SUCCESS)
        return check;
    *outlen += toutlen;

    return ((rt_memcmp(tag, newtag, taglen) == 0) ? SUCCESS : E_VERFAIL);
}

static rt_err_t aes_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_aes_config_args *config_args = (struct rt_aes_config_args *)args;
    void *_aad, *_iv, *_key, *_in, *_out, *_tag;
    rt_uint32_t _aadlen, _ivlen, _keybits, _keylen, _inlen, _outlen, _taglen;
    void *total;
    pufs_status_t check;
    rt_err_t ret = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    _ivlen = config_args->ivlen;
    _keybits = config_args->keybits;
    _inlen = config_args->pclen;
    _outlen = config_args->pclen;
    _aadlen = config_args->aadlen;
    _taglen = AES_BLOCK_SIZE;
    _keylen = _keybits >> 3;

    // allocate one big address space, the space structure is:" key + iv + aad + pt/ct + tag + ct/pt "
    total = rt_malloc(_keylen + _ivlen + _aadlen + _inlen + _outlen + _taglen + 16);
    if(!total)
        return -RT_ENOMEM;

    _key = total;
    _iv = _key + _keylen;
    _aad = _iv + _ivlen;
    _in = _aad + _aadlen;
    if(_aadlen == 0)
        _in = _in + 1;
    _tag = _in + _inlen;
    if(_inlen == 0)
        _tag = _tag + 1;
    _out = _tag + _taglen;

    lwp_get_from_user(_key, config_args->key, _keylen);
    lwp_get_from_user(_iv, config_args->iv, _ivlen);
    lwp_get_from_user(_aad, config_args->in, _aadlen);
    lwp_get_from_user(_in, (config_args->in + _aadlen), _inlen);

    // rt_kprintf("keybits = %d, ivlen = %d, inlen = %d, outlen = %d, aadlen = %d, taglen = %d\n", _keybits, _ivlen, _inlen, _outlen, _aadlen, _taglen);

    switch(cmd)
    {
        case RT_AES_GCM_ENC:
        {
            // debug
            debug_func("key", _key, _keylen);
            debug_func("iv", _iv, _ivlen);
            debug_func("aad", _aad, _aadlen);
            debug_func("in", _in, _inlen);

            // initial outbuf
            rt_memset(_out, 0, _outlen);

            // input key to hardware
            g_keyslot = (_keybits > 128) ? SK256_0 : SK128_0;
            // add hardlock
            while(0 != kd_hardlock_lock(aes_hardlock));
            if((check = pufs_import_plaintext_key(SSKEY, g_keyslot, (const rt_uint8_t *)_key, _keybits)) != SUCCESS) {
                ret = -RT_ERROR;
                goto release_lock;
            }

            // do encrypt
            if((check = pufs_enc_gcm((rt_uint8_t *)_out, &_outlen,
                                    (const rt_uint8_t *)_in, _inlen,
                                    AES, SSKEY,
                                    g_keyslot, _keybits,
                                    (const rt_uint8_t *)_iv, _ivlen,
                                    (const rt_uint8_t *)_aad, _aadlen,
                                    (rt_uint8_t *)_tag, _taglen)) != SUCCESS)
            {
                ret = -RT_ERROR;
                goto release_lock;
            }
            
            // clear key from hardware
            if((check = pufs_clear_key(SSKEY, g_keyslot, _keybits) != SUCCESS))
            {
                ret = -RT_ERROR;
                goto release_lock;
            }
            kd_hardlock_unlock(aes_hardlock);

            // debug
            debug_func("out", _out, _outlen);
            
            // output result
            config_args->outlen = _aadlen + _outlen + _taglen;
            config_args->taglen = _taglen;
            lwp_put_to_user(config_args->out, _aad, _aadlen);
            lwp_put_to_user((config_args->out + _aadlen), _out, _outlen);
            lwp_put_to_user((config_args->out + _aadlen + _outlen), _tag, _taglen);

            rt_free(total);
            break;
        }
        case RT_AES_GCM_DEC:
        {
            lwp_get_from_user(_tag, (config_args->in + _aadlen + _inlen), _taglen);
            rt_memset(_out, 0, _outlen);

            // debug
            debug_func("key", _key, _keylen);
            debug_func("iv", _iv, _ivlen);
            debug_func("aad", _aad, _aadlen);
            debug_func("in", _in, _inlen);
            debug_func("tag", _tag, _taglen);

            // input key to hardware
            g_keyslot = (_keybits > 128) ? SK256_0 : SK128_0;
            // add hardlock
            while(0 != kd_hardlock_lock(aes_hardlock));
            if((check = pufs_import_plaintext_key(SSKEY, g_keyslot, (const rt_uint8_t *)_key, _keybits)) != SUCCESS) {
                ret = -RT_ERROR;
                goto release_lock;
            }

            // do decrypt
            if((check = pufs_dec_gcm((rt_uint8_t*)_out, &_outlen,
                                    (const rt_uint8_t *)_in, _inlen,
                                    AES, SSKEY,
                                    g_keyslot, _keybits,
                                    (const rt_uint8_t *)_iv, _ivlen,
                                    (const rt_uint8_t *)_aad, _aadlen,
                                    (const rt_uint8_t *)_tag, _taglen)) != SUCCESS)
            {
                ret = -RT_ERROR;
                goto release_lock;
            }

            // clear key from hardware
            if((check = pufs_clear_key(SSKEY, g_keyslot, _keybits) != SUCCESS))
            {
                ret = -RT_ERROR;
                goto release_lock;
            }
            kd_hardlock_unlock(aes_hardlock);

            // debug
            debug_func("out", _out, _outlen);

            // output result
            config_args->outlen = _aadlen + _outlen;
            config_args->taglen = 0;
            lwp_put_to_user(config_args->out, _aad, _aadlen);
            lwp_put_to_user((config_args->out + _aadlen), _out, _outlen);

            rt_free(total);
            break;
        }

        default:
            return -RT_EINVAL;
    }

    if(ret != RT_EOK)
        goto release_lock;

ret_aes:

    return ret;

release_lock:
    kd_hardlock_unlock(aes_hardlock);
    goto ret_aes;
}

static rt_err_t aes_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t aes_close(rt_device_t dev)
{
    return RT_EOK;
}

const static struct rt_device_ops aes_ops =
{
    RT_NULL,
    aes_open,
    aes_close,
    RT_NULL,
    RT_NULL,
    aes_control,
};

int rt_hw_aes_device_init(void)
{
    rt_device_t aes_device = &g_aes_device;
    rt_err_t ret = RT_EOK;

    sec_base_addr = rt_ioremap((void*)SECURITY_BASE_ADDR, SECURITY_IO_SIZE);
    if(RT_NULL == sec_base_addr) {
        rt_kprintf("Security module ioremap error!\n");
        return -RT_ERROR;
    }

    ret = rt_device_register(aes_device, K230_AES_NAME, RT_DEVICE_FLAG_RDWR);
    if(ret) {
        rt_kprintf("AES device register fail!\n");
        return ret;
    }

    aes_device->ops = &aes_ops;

    if(kd_request_lock(HARDLOCK_AES)) {
        rt_kprintf("Fail to request hardlock-%d\n", HARDLOCK_AES);
        aes_hardlock = -1;
    } else
        aes_hardlock = HARDLOCK_AES;

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_aes_device_init);