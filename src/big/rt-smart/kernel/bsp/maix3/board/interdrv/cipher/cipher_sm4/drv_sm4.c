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
#include "drv_sm4.h"

// #define sm4_debug

static struct rt_device g_sm4_device = {0};
static void *sec_base_addr = RT_NULL;
static int sm4_hardlock;
static rt_uint8_t pufs_buffer[BUFFER_SIZE];
static pufs_ka_slot_t g_keyslot;
static void *in_kvirt;
static void *out_kvirt;
struct sm4_context sm4_ctx = { .op = SP38A_AVAILABLE };

//----------------------BASE FUNCTION ----------------------------------------//
#ifdef sm4_debug
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

pufs_status_t crypto_read_iv(rt_uint8_t *out, rt_size_t length)
{
    length = length < IV_MAXLEN ? length : IV_MAXLEN;
    read_data(out, (void *)(sec_base_addr + CRYPTO_IV_OUT_OFFSET), length, RT_TRUE);
    return SUCCESS;
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

static void dma_write_rwcfg(const rt_uint8_t *out, const rt_uint8_t *in, rt_uint32_t len, void *in_buff, void *out_buff)
// static void dma_write_rwcfg(const rt_uint8_t *out, const rt_uint8_t *in, rt_uint32_t len)
{
    rt_uint64_t in_pa, out_pa;

    writel(len, sec_base_addr + DMA_DSC_CFG_2_OFFSET);

    if(RT_NULL == out)
        writel((uintptr_t)out, sec_base_addr + DMA_DSC_CFG_1_OFFSET);
    else
    {
        rt_hw_cpu_dcache_clean_flush(out_buff, len);
        out_pa = virt_to_phys((void*)out_buff);
        rt_hw_cpu_dcache_clean_flush((void *)out_pa, len);
        writel(out_pa, sec_base_addr + DMA_DSC_CFG_1_OFFSET);
    }

    if(RT_NULL == in)
        writel((uintptr_t)in, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    else    // virtual to physical
    {
        rt_memcpy(in_buff, (void *)in, len);

        rt_hw_cpu_dcache_clean_flush(in_buff, len);
        in_pa = virt_to_phys(in_buff);
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

static rt_bool_t check_sp38a_op(sp38a_op opf, sp38a_op opctx)
{
    // ECB, CFB, and OFB
    if (opf == opctx)
        return RT_TRUE;

    // CBC
    if (opf == SP38A_CBC_CLR)
    {
        switch (opctx)
        {
        case SP38A_CBC_CLR:
        case SP38A_CBC_CS1:
        case SP38A_CBC_CS2:
        case SP38A_CBC_CS3:
            return RT_TRUE;
        default:
            return RT_FALSE;
        }
    }

    // CTR
    if (opf == SP38A_CTR_128)
    {
        switch (opctx)
        {
        case SP38A_CTR_32:
        case SP38A_CTR_64:
        case SP38A_CTR_128:
            return RT_TRUE;
        default:
            return RT_FALSE;
        }
    }

    return RT_FALSE;
}

static pufs_status_t sp38a_get_cfg(struct sm4_context* sm4_ctx, rt_uint32_t *cfg)
{
    switch (sm4_ctx->cipher)
    {
        case AES:
            switch (sm4_ctx->keylen << 3)
            {
                case 128:
                    *cfg = 0x0;
                    break;
                case 192:
                    *cfg = 0x1;
                    break;
                case 256:
                    *cfg = 0x2;
                    break;
                default:
                    return E_FIRMWARE;
            }
            break;
        case SM4:
            switch (sm4_ctx->keylen << 3)
            {
                case 128:
                    *cfg = 0x3;
                    break;
                default:
                    return E_FIRMWARE;
            }
            break;
        default:
            return E_FIRMWARE;
    }
    switch (sm4_ctx->op)
    {
        case SP38A_ECB_CLR:
            (*cfg) |= 0x0<<4;
            break;
        case SP38A_CFB_CLR:
            (*cfg) |= 0x1<<4;
            break;
        case SP38A_OFB:
            (*cfg) |= 0x2<<4;
            break;
        case SP38A_CBC_CLR:
            (*cfg) |= 0x3<<4;
            break;
        case SP38A_CBC_CS1:
            (*cfg) |= 0x4<<4;
            break;
        case SP38A_CBC_CS2:
            (*cfg) |= 0x5<<4;
            break;
        case SP38A_CBC_CS3:
            (*cfg) |= 0x6<<4;
            break;
        case SP38A_CTR_32:
            (*cfg) |= 0x7<<4;
            break;
        case SP38A_CTR_64:
            (*cfg) |= 0x8<<4;
            break;
        case SP38A_CTR_128:
            (*cfg) |= 0x9<<4;
            break;
        default:
            return E_FIRMWARE;
    }

    (*cfg) |= (sm4_ctx->encrypt ? 0x1 : 0x0 )<<8;
    return SUCCESS;
}

static pufs_status_t sp38a_postproc(struct sm4_context* sm4_ctx)
{
    crypto_read_iv(sm4_ctx->iv, AES_BLOCK_SIZE);
    return SUCCESS;
}

static pufs_status_t sp38a_prepare(struct sm4_context* sm4_ctx)
{
    rt_uint32_t val32;
    pufs_status_t check;

    if (sm4_ctx->keytype == SWKEY)
        crypto_write_sw_key(sm4_ctx->key, SW_KEY_MAXLEN);

    dma_write_key_config_0(sm4_ctx->keytype,
                           ALGO_TYPE_SP38A,
                           (sm4_ctx->keylen << 3),
                           get_key_slot_idx(sm4_ctx->keytype, sm4_ctx->keyslot));

    crypto_write_iv(sm4_ctx->iv, AES_BLOCK_SIZE);

    if ((check = sp38a_get_cfg(sm4_ctx, &val32)) != SUCCESS)
        return check;

    writel(val32, sec_base_addr + SP38A_CFG_OFFSET);

    return SUCCESS;
}

static pufs_status_t _ctx_update(struct sm4_context* sm4_ctx,
                                rt_uint8_t* out,
                                rt_uint32_t* outlen,
                                const rt_uint8_t* in,
                                rt_uint32_t inlen,
                                rt_bool_t last)
{
    rt_uint32_t val32;
    pufs_status_t check;

    if (out == NULL || outlen == NULL)
        return E_INVALID;

    // rt_kprintf("[debug]: %s, %d\n", __func__, __LINE__);
    debug_func("in", (void*)in, inlen);
    debug_func("iv", (void*)sm4_ctx->iv, AES_BLOCK_SIZE);

    // Register manipulation
    if (dma_check_busy_status(RT_NULL))
        return E_BUSY;
    
    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_data_block_config(sm4_ctx->start ? RT_FALSE : RT_TRUE, last, RT_TRUE, RT_TRUE, 0);

    if(inlen > 448)
    {
        rt_uint8_t *in_buff = RT_NULL;
        rt_uint8_t *out_buff = RT_NULL;
        rt_uint8_t array[2*inlen + 16];
        rt_memset(array, 0, 2*inlen + 16);
        in_buff = array;
        out_buff = (array + inlen);
        dma_write_rwcfg(out, in, inlen, in_buff, out_buff);

        if ((check = sp38a_prepare(sm4_ctx)) != SUCCESS)
            return check;

        dma_write_start();
        while (dma_check_busy_status(&val32));
    
        if (val32 != 0)
        {
            rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
            return E_ERROR;
        }

        val32 = readl(sec_base_addr + SP38A_STAT_OFFSET);
        if ((val32 & SP38A_STATUS_ERROR_MASK) != 0)
        {
            rt_kprintf("[ERROR] SM4 status 0: 0x%08x\n", val32);
            return E_ERROR;
        }

        // post-processing
        if (last == RT_FALSE)
            sp38a_postproc(sm4_ctx);

        // rt_memcpy(out, out_kvirt, inlen);
        rt_memcpy(out, out_buff, inlen);
        *outlen = inlen;
    }
    else
    {
        rt_uint8_t *in_buff = (rt_uint8_t*)in_kvirt;
        rt_uint8_t *out_buff = (rt_uint8_t*)out_kvirt;
        dma_write_rwcfg(out, in, inlen, in_buff, out_buff);

        if ((check = sp38a_prepare(sm4_ctx)) != SUCCESS)
            return check;

        dma_write_start();
        while (dma_check_busy_status(&val32));
    
        if (val32 != 0)
        {
            rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
            return E_ERROR;
        }

        val32 = readl(sec_base_addr + SP38A_STAT_OFFSET);
        if ((val32 & SP38A_STATUS_ERROR_MASK) != 0)
        {
            rt_kprintf("[ERROR] SM4 status 0: 0x%08x\n", val32);
            return E_ERROR;
        }

        // post-processing
        if (last == RT_FALSE)
            sp38a_postproc(sm4_ctx);

        // rt_memcpy(out, out_kvirt, inlen);
        rt_memcpy(out, out_buff, inlen);
        *outlen = inlen;
    }

    debug_func("out", (void*)out, inlen);

    return SUCCESS;
}


//----------------------INIT-UPDATE-FINAL FUNCTION----------------------------------------//
static pufs_status_t ctx_final(sp38a_op op,
                                struct sm4_context *sm4_ctx,
                                rt_bool_t encrypt,
                                rt_uint8_t* out,
                                rt_uint32_t* outlen)
{
    pufs_status_t check = SUCCESS;

    // check sm4_ctx is owned by this operation (GCM mode)
    if ((check_sp38a_op(op, sm4_ctx->op) == RT_FALSE) || (sm4_ctx->encrypt != encrypt))
        return E_UNAVAIL;

    // in final call, it must be minimum-length bytes depending on modes to
    //  pass into the modes of operation module
    if (sm4_ctx->buflen < sm4_ctx->minlen)
        check = E_INVALID;
    else
        check = _ctx_update(sm4_ctx, out, outlen, sm4_ctx->buff, sm4_ctx->buflen, RT_TRUE);

    // release context
    sm4_ctx->op = SP38A_AVAILABLE;
    return check;
}

static pufs_status_t ctx_update(sp38a_op op,
                                struct sm4_context* sm4_ctx,
                                rt_bool_t encrypt,
                                rt_uint8_t* out,
                                rt_uint32_t* outlen,
                                const rt_uint8_t* in,
                                rt_uint32_t inlen)
{
    // check sm4_ctx is owned by this operation (GCM mode)
    if ((check_sp38a_op(op, sm4_ctx->op) == RT_FALSE) || (sm4_ctx->encrypt != encrypt))
        return E_UNAVAIL;
    // continue if msg is NULL or msglen is zero
    *outlen = 0;
    if ((in == RT_NULL) || (inlen == 0))
    { 
        return SUCCESS;
    }

    k_blsegs segs = segment(sm4_ctx->buff, sm4_ctx->buflen, in, inlen, AES_BLOCK_SIZE, sm4_ctx->minlen);
    sm4_ctx->buflen = 0;
    rt_uint32_t seglen = 0;
    pufs_status_t check = SUCCESS;

    rt_uint32_t i;
    for (i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if((check = _ctx_update(sm4_ctx, out + *outlen, &seglen, segs.seg[i].addr, segs.seg[i].len, RT_FALSE)) != SUCCESS)
            {
                // release context
                sm4_ctx->op = SP38A_AVAILABLE;
                return check;
            }
            *outlen += seglen;
            if (sm4_ctx->start == RT_FALSE)
                sm4_ctx->start = RT_TRUE;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == sm4_ctx->buff) && (sm4_ctx->buflen == 0))
            { // skip copy what already in the right place
                sm4_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                rt_memmove(sm4_ctx->buff + sm4_ctx->buflen, segs.seg[i].addr, segs.seg[i].len);
                sm4_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return SUCCESS;
}

static pufs_status_t ctx_init(sp38a_op op,
                                struct sm4_context* sm4_ctx,
                                rt_bool_t encrypt,
                                pufs_cipher_t cipher,
                                pufs_key_type_t keytype,
                                rt_size_t keyaddr,
                                rt_uint32_t keybits,
                                const rt_uint8_t* iv,
                                int option)
{
    rt_uint32_t val32;
    pufs_status_t check;

    // abort if sm4_ctx is occupied
    if (sm4_ctx->op != SP38A_AVAILABLE)
        return E_BUSY;

    // check keytype
    if ((keytype == PUFKEY) || (keytype == SHARESEC))
        return E_DENY;

    // check key settings for block cipher
    if ((keytype != SWKEY) && ((check = keyslot_check(RT_TRUE, keytype, (rt_uint32_t)keyaddr, keybits)) != SUCCESS))
        return check;

    val32 = readl(sec_base_addr + SP38A_FEATURE_OFFSET);
    if (((encrypt == RT_TRUE) && ((val32 & SP38A_FEATURE_ENC_MASK) == 0)) ||
        ((encrypt == RT_FALSE) && ((val32 & SP38A_FEATURE_DEC_MASK) == 0)))
        return E_UNSUPPORT;
    // check if op is valid
    switch (op)
    {
        case SP38A_ECB_CLR:
            if ((val32 & SP38A_FEATURE_ECB_CLR_MASK) == 0)
                return E_UNSUPPORT;
            break;
        case SP38A_CFB_CLR:
            if ((val32 & SP38A_FEATURE_CFB_MASK) == 0)
                return E_UNSUPPORT;
            break;
        case SP38A_OFB:
            if ((val32 & SP38A_FEATURE_OFB_MASK) == 0)
                return E_UNSUPPORT;
            break;
        case SP38A_CBC_CLR:
            if (((option == 0) && ((val32 & SP38A_FEATURE_CBC_CLR_MASK) == 0)) ||
                ((option == 1) && ((val32 & SP38A_FEATURE_CBC_CS1_MASK) == 0)) ||
                ((option == 2) && ((val32 & SP38A_FEATURE_CBC_CS2_MASK) == 0)) ||
                ((option == 3) && ((val32 & SP38A_FEATURE_CBC_CS3_MASK) == 0)))
                return E_UNSUPPORT;
            break;
        case SP38A_CTR_128:
            if ((val32 & SP38A_FEATURE_CTR_MASK) == 0)
                return E_UNSUPPORT;
            break;
        default:
            return E_FIRMWARE;
    }

    // check and set iv if needed
    if (op != SP38A_ECB_CLR)
    {
        if (iv == RT_NULL)
            return E_INVALID;
        else
        rt_memcpy(sm4_ctx->iv, iv, AES_BLOCK_SIZE);
    }

    // initialize for block-cipher mode of operation
    sm4_ctx->buflen = 0;
    sm4_ctx->cipher = cipher;
    sm4_ctx->encrypt = encrypt;
    sm4_ctx->start = RT_FALSE;
    // set key
    sm4_ctx->keylen = keybits >> 3;
    sm4_ctx->keytype = keytype;

    if (keytype != SWKEY)
        sm4_ctx->keyslot = (rt_uint32_t)keyaddr;
    else
        rt_memcpy(sm4_ctx->key, (const void*)keyaddr, b2B(keybits));

    // set mode of operation, and the minimum length of the last input
    if (op == SP38A_ECB_CLR) {
        sm4_ctx->op = SP38A_ECB_CLR;
        sm4_ctx->minlen = 1;
    } else if (op == SP38A_CFB_CLR) {
        sm4_ctx->op = SP38A_CFB_CLR;
        sm4_ctx->minlen = 1;
    } else if (op == SP38A_OFB) {
        sm4_ctx->op = SP38A_OFB;
        sm4_ctx->minlen = 1;
    } else if (op == SP38A_CBC_CLR) {
        if (option == 0) {
            sm4_ctx->op = SP38A_CBC_CLR;
            sm4_ctx->minlen = 1;
        } else if (option == 1) {
            sm4_ctx->op = SP38A_CBC_CS1;
            sm4_ctx->minlen = 17;
        } else if (option == 2) {
            sm4_ctx->op = SP38A_CBC_CS2;
            sm4_ctx->minlen = 17;
        } else if (option == 3) {
            sm4_ctx->op = SP38A_CBC_CS3;
            sm4_ctx->minlen = 17;
        } else {
            return E_INVALID;
        }
    } else if (op == SP38A_CTR_128) {
        if (option == 32) {
            sm4_ctx->op = SP38A_CTR_32;
            sm4_ctx->minlen = 1;
        } else if (option == 64) {
            sm4_ctx->op = SP38A_CTR_64;
            sm4_ctx->minlen = 1;
        } else if (option == 128) {
            sm4_ctx->op = SP38A_CTR_128;
            sm4_ctx->minlen = 1;
        } else {
            return E_INVALID;
        }
    } else {
        return E_INVALID;
    }

    return SUCCESS;
}

//----------------------ENCRYPT FUNCTION----------------------------------------//
static pufs_status_t pufs_enc_sm4(rt_uint8_t* out,
                            rt_uint32_t* outlen,
                            const rt_uint8_t* in,
                            rt_uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            rt_size_t keyaddr,
                            rt_uint32_t keybits,
                            const rt_uint8_t* iv,
                            const char* sm4_mode)
{
    pufs_status_t check;
    rt_uint32_t toutlen;
    sp38a_op op;
    int opt;

    *outlen = 0;

    if(!rt_strcmp(sm4_mode, "ecb-sm4")) {
        iv = RT_NULL;
        op = SP38A_ECB_CLR;
        opt = 0;
    }
    else if(!rt_strcmp(sm4_mode, "cfb-sm4")) {
        op = SP38A_CFB_CLR;
        opt = 0;
    } 
    else if(!rt_strcmp(sm4_mode, "ofb-sm4")) {
        opt = 0;
        op = SP38A_OFB;
    }
    else if(!rt_strcmp(sm4_mode, "cbc-sm4")) {
        opt = 0;
        op = SP38A_CBC_CLR;
    }
    else if(!rt_strcmp(sm4_mode, "ctr-sm4")) {
        opt = 128;
        op = SP38A_CTR_128;
    }
    else
        op = SP38A_AVAILABLE;

    // Call I-U-F model
    if((check = ctx_init(op, &sm4_ctx, RT_TRUE, cipher, keytype, keyaddr, keybits, iv, opt)) != SUCCESS)
        return check;

    if((check = ctx_update(op, &sm4_ctx, RT_TRUE, out, &toutlen, in, inlen)) != SUCCESS)
        return check;
    *outlen += toutlen;

    if((check = ctx_final(op, &sm4_ctx, RT_TRUE, out + *outlen, &toutlen)) != SUCCESS)
        return check;
    *outlen += toutlen;

    return SUCCESS;
}

//----------------------DECRYPT FUNCTION----------------------------------------//
static pufs_status_t pufs_dec_sm4(rt_uint8_t* out,
                            rt_uint32_t* outlen,
                            const rt_uint8_t* in,
                            rt_uint32_t inlen,
                            pufs_cipher_t cipher,
                            pufs_key_type_t keytype,
                            rt_size_t keyaddr,
                            rt_uint32_t keybits,
                            const rt_uint8_t* iv,
                            const char* sm4_mode)
{
    pufs_status_t check;
    rt_uint32_t toutlen;
    sp38a_op op;
    int opt;

    *outlen = 0;

    if(!rt_strcmp(sm4_mode, "ecb-sm4")) {
        iv = RT_NULL;
        op = SP38A_ECB_CLR;
        opt = 0;
    }
    else if(!rt_strcmp(sm4_mode, "cfb-sm4")) {
        op = SP38A_CFB_CLR;
        opt = 0;
    } 
    else if(!rt_strcmp(sm4_mode, "ofb-sm4")) {
        opt = 0;
        op = SP38A_OFB;
    }
    else if(!rt_strcmp(sm4_mode, "cbc-sm4")) {
        opt = 0;
        op = SP38A_CBC_CLR;
    }
    else if(!rt_strcmp(sm4_mode, "ctr-sm4")) {
        opt = 128;
        op = SP38A_CTR_128;
    }
    else
        op = SP38A_AVAILABLE;

    // Call I-U-F model
    if((check = ctx_init(op, &sm4_ctx, RT_FALSE, cipher, keytype, keyaddr, keybits, iv, opt)) != SUCCESS)
        return check;

    if((check = ctx_update(op, &sm4_ctx, RT_FALSE, out, &toutlen, in, inlen)) != SUCCESS)
        return check;
    *outlen += toutlen;

    if((check = ctx_final(op, &sm4_ctx, RT_FALSE, out + *outlen, &toutlen)) != SUCCESS)
        return check;
    *outlen += toutlen;

    return SUCCESS;
}

static rt_err_t sm4_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_sm4_config_args *config_args = (struct rt_sm4_config_args *)args;
    void *_iv, *_key, *_in, *_out;
    rt_uint32_t _ivlen, _keybits, _keylen, _inlen, _outlen;
    void *total;
    char *_mode;
    pufs_status_t check;
    rt_err_t ret = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    _keybits = config_args->keybits;
    _inlen = config_args->pclen;
    _outlen = config_args->pclen;
    _keylen = _keybits >> 3;
    _mode = config_args->mode;

    if(!rt_strcmp(_mode, "ecb-sm4"))
    {
        // allocate one big address space, the space structure is:" key + pt/ct + ct/pt "
        total = rt_malloc(_keylen + _inlen + _outlen + 16);
        if(!total)
            return -RT_ENOMEM;

        _key = total;
        _in = _key + _keylen;
        _out = _in + _inlen;
        if(_inlen == 0)
            _out = _out + 1;

        lwp_get_from_user(_key, config_args->key, _keylen);
        lwp_get_from_user(_in, config_args->in, _inlen);
    }
    else if((!rt_strcmp(_mode, "cfb-sm4")) || (!rt_strcmp(_mode, "ofb-sm4")) || (!rt_strcmp(_mode, "cbc-sm4")) || (!rt_strcmp(_mode, "ctr-sm4")))
    {
        _ivlen = config_args->ivlen;

        // allocate one big address space, the space structure is:" key + iv + pt/ct + ct/pt "
        total = rt_malloc(_keylen + _ivlen + _inlen + _outlen + 16);
        if(!total)
            return -RT_ENOMEM;

        _key = total;
        _iv = _key + _keylen;
        _in = _iv + _ivlen;
        _out = _in + _inlen;
        if(_inlen == 0)
            _out = _out + 1;

        lwp_get_from_user(_key, config_args->key, _keylen);
        lwp_get_from_user(_iv, config_args->iv, _ivlen);
        lwp_get_from_user(_in, config_args->in, _inlen);
    }
    else
    {
        rt_kprintf("enc/dec mode not supported!\n");
        return -RT_EINVAL;
    }

    // alloc global buffer for store val from PUF-DMA
    in_kvirt = rt_malloc(_inlen + 16);
    out_kvirt = rt_malloc(_inlen + 16);
    if(!in_kvirt || !out_kvirt)
    {
        rt_kprintf("%s malloc fail!\n", __func__);
        ret = -RT_ERROR;
        return ret;
    }

    switch(cmd)
    {
        case RT_SM4_ENC:
        {
            rt_uint32_t _toutlen;
            // debug
            debug_func("key", _key, _keylen);
            debug_func("iv", _iv, _ivlen);
            debug_func("in", _in, _inlen);

            // initial outbuf
            rt_memset(_out, 0, _outlen);

            // input key to hardware
            g_keyslot = (_keybits > 128) ? SK256_0 : SK128_0;
            // add hardlock
            while(0 != kd_hardlock_lock(sm4_hardlock));
            if((check = pufs_import_plaintext_key(SSKEY, g_keyslot, (const rt_uint8_t *)_key, _keybits)) != SUCCESS) {
                ret = -RT_ERROR;
                return ret;
            }

            // do encrypt
            if((check = pufs_enc_sm4((rt_uint8_t *)_out, &_toutlen,
                                    (const rt_uint8_t *)_in, _inlen,
                                    SM4, SSKEY,
                                    g_keyslot, _keybits,
                                    (const rt_uint8_t *)_iv, _mode)) != SUCCESS)
            {
                ret = -RT_ERROR;
                return ret;
            }
            
            // clear key from hardware
            if((check = pufs_clear_key(SSKEY, g_keyslot, _keybits) != SUCCESS) || (_outlen != _toutlen))
            {
                ret = -RT_ERROR;
                return ret;
            }
            kd_hardlock_unlock(sm4_hardlock);

            // debug
            debug_func("out", _out, _outlen);
            
            // output result
            config_args->outlen = _outlen;
            lwp_put_to_user(config_args->out, _out, _outlen);
            
            break;
        }
        case RT_SM4_DEC:
        {
            // debug
            debug_func("key", _key, _keylen);
            debug_func("iv", _iv, _ivlen);
            debug_func("in", _in, _inlen);

            // initial outbuf
            rt_memset(_out, 0, _outlen);

            // input key to hardware
            g_keyslot = (_keybits > 128) ? SK256_0 : SK128_0;
            // add hardlock
            while(0 != kd_hardlock_lock(sm4_hardlock));
            if((check = pufs_import_plaintext_key(SSKEY, g_keyslot, (const rt_uint8_t *)_key, _keybits)) != SUCCESS) {
                ret = -RT_ERROR;
                return ret;
            }

            // do decrypt
            if((check = pufs_dec_sm4((rt_uint8_t *)_out, &_outlen,
                                    (const rt_uint8_t *)_in, _inlen,
                                    SM4, SSKEY,
                                    g_keyslot, _keybits,
                                    (const rt_uint8_t *)_iv, _mode)) != SUCCESS)
            {
                ret = -RT_ERROR;
                return ret;
            }
            
            // clear key from hardware
            if((check = pufs_clear_key(SSKEY, g_keyslot, _keybits) != SUCCESS) || (_outlen != _inlen))
            {
                ret = -RT_ERROR;
                return ret;
            }
            kd_hardlock_unlock(sm4_hardlock);

            // debug
            debug_func("out", _out, _outlen);
            
            // output result
            config_args->outlen = _outlen;
            lwp_put_to_user(config_args->out, _out, _outlen);
            
            break;
        }

        default:
            return -RT_EINVAL;
    }
    rt_free(total);
    // release global buffer
    rt_free(in_kvirt);
    rt_free(out_kvirt);

    return ret;
}

static rt_err_t sm4_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t sm4_close(rt_device_t dev)
{
    return RT_EOK;
}

const static struct rt_device_ops sm4_ops =
{
    RT_NULL,
    sm4_open,
    sm4_close,
    RT_NULL,
    RT_NULL,
    sm4_control,
};

int rt_hw_sm4_device_init(void)
{
    rt_device_t sm4_device = &g_sm4_device;
    rt_err_t ret = RT_EOK;

    sec_base_addr = rt_ioremap((void*)SECURITY_BASE_ADDR, SECURITY_IO_SIZE);
    if(RT_NULL == sec_base_addr) {
        rt_kprintf("Security module ioremap error!\n");
        return -RT_ERROR;
    }

    ret = rt_device_register(sm4_device, K230_SM4_NAME, RT_DEVICE_FLAG_RDWR);
    if(ret) {
        rt_kprintf("SM4 device register fail!\n");
        return ret;
    }

    sm4_device->ops = &sm4_ops;

    if(kd_request_lock(HARDLOCK_SM4)) {
        rt_kprintf("Fail to request hardlock-%d\n", HARDLOCK_SM4);
        sm4_hardlock = -1;
    } else
        sm4_hardlock = HARDLOCK_SM4;

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_sm4_device_init);