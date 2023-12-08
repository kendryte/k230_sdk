/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/crypto.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <crypto/internal/aead.h>
#include <crypto/aes.h>
#include <crypto/akcipher.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>
#include <crypto/gcm.h>
#include <linux/canaan-hardlock.h>
#include "kendryte-aes.h"

// #define _debug_print

/*****************************************************************************
 * Macros
 ****************************************************************************/
/**
 * @brief Convert number of bits to number of bytes
 *
 * @param[in] bits  Number of bits.
 * @return          Number of bytes.
 */
#define b2B(bits) (((bits) + 7) / 8)
/**
 * @brief Convert number of bytes to number of bits
 *
 * @param[in] len  Number of bytes.
 * @return         Number of bits.
 */
#define B2b(len) (8 * (len))


static void *puf_dma_base;
static void *puf_crypto_base;
static void *puf_kwp_base;
static void *puf_ka_base;
static int hardlock;
static bool request_status = false;
void *dma_virt_in;
void *dma_virt_out;
dma_addr_t dma_phys_in;
dma_addr_t dma_phys_out;
uint8_t kwp_buffer[BUFFER_SIZE];
struct kendryte_crypto_info crypto_info;

//----------------------BASE FUNCTION ----------------------------------------//
uint32_t be2le(uint32_t var)
{
    return (((0xff000000 & var) >> 24) |
            ((0x00ff0000 & var) >> 8) |
            ((0x0000ff00 & var) << 8) |
            ((0x000000ff & var) << 24));
}

static void write_data(uint32_t *dst, uint8_t *src, size_t length, bool le)
{
    uint32_t *src32 = (uint32_t *)src;
    length = length / 4;
	size_t i;

    for (i = 0; i < length; ++i)
    {
        if (le)
            *(dst + i) = be2le(*(src32 + i));
        else
            *(dst + i) = *(src32 + i);
    }
}

static void read_data(uint8_t *dst, uint32_t *src, size_t length, bool le)
{
    uint32_t *dst32 = (uint32_t *)dst;
    uint32_t word_nums = 0, rest = 0, last_word;
    word_nums = length / 4;
    rest = length % 4;
	uint32_t i;

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

        memcpy(dst + (word_nums * 4), &last_word, rest);
    }
}

static kendryte_status_t get_config(uint32_t *cfg, struct kendryte_aes_ctx *ctx, bool gctr, bool reg_in, bool reg_out)
{
	uint32_t val32;
	switch(ctx->cipher)
	{
		case AES:
			switch((ctx->keylen) << 3)
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

		default:
            return E_FIRMWARE;
	}

	if((ctx->inlen) != ULLONG_MAX)
		val32 |= 0x1 << GCM_CFG_GHASH_BITS;
	
	if(gctr)
		val32 |= 0x1 << GCM_CFG_GCTR_BITS;

	val32 |= (ctx->encrypt ? 0x1 : 0x0 ) << GCM_CFG_ENCRYPT_BITS;

    if (reg_in)
        val32 |= 0x1 << GCM_CFG_REG_IN_BITS;

    if (reg_out)
        val32 |= 0x1 << GCM_CFG_REG_OUT_BITS;
    
    *cfg = val32;

	return SUCCESS;
}

blsegs segment(uint8_t *buf, uint32_t buflen, const uint8_t *in, uint32_t inlen, uint32_t blocksize, uint32_t minlen)
{
	blsegs ret = { .nsegs =0 };
	// calculate total number of blocks to be processed
    uint32_t nprocblocks = 0;
    if ((buflen + inlen) >= (minlen + blocksize))
        nprocblocks = (buflen + inlen - minlen) / blocksize;

	// no available block for processing, keep input in the internal buffer.
    if (nprocblocks == 0)
    {
        ret.seg[ret.nsegs++] = (segstr){ false, buf, buflen };
        ret.seg[ret.nsegs++] = (segstr){ false, in, inlen };
        return ret;
    }

	const uint8_t *start = in;
    // some blocks are ready for processing, using bytes in the internal buffer first
    if (buflen != 0)
    {
        // if all data in the internal buffer will be processed
        if (nprocblocks * blocksize >= buflen)
        {
            // fill buffer if not a complete block
            uint32_t proclen = blocksize;
            nprocblocks--;
            while (proclen < buflen)
            {
                proclen += blocksize;
                nprocblocks--;
            }
            memcpy(buf + buflen, start, proclen - buflen);
            ret.seg[ret.nsegs++] = (segstr){ true, buf, proclen };
            start += (proclen - buflen);
            inlen -= (proclen - buflen);
        }
        else // some data will be remained in the internal buffer
        {
            ret.seg[ret.nsegs++] = (segstr){ true, buf, nprocblocks * blocksize };
            ret.seg[ret.nsegs++] = (segstr){ false, buf + nprocblocks * blocksize, buflen - nprocblocks * blocksize };
            nprocblocks = 0;
        }
    }
    // deal with input data
    if (nprocblocks > 0)
    {
        ret.seg[ret.nsegs++] = (segstr){ true, start, nprocblocks * blocksize };
    }
    ret.seg[ret.nsegs++] = (segstr){ false, start + nprocblocks * blocksize, inlen - nprocblocks * blocksize };
	
	return ret;
}

//----------------------KWP FUNCTION----------------------------------------//
static kendryte_status_t kwp_start(void)
{
	uint32_t val32;
	writel(0x1, puf_kwp_base + KWP_START_OFFSET);
	while(((val32 = readl(puf_kwp_base + KWP_STATUS_OFFSET)) & KWP_STATUS_BUSY_MASK) != 0);

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
        pr_err("Error, KWP status: 0x%08x\n", val32);
        return E_ERROR;
    }

	return SUCCESS;
}

static kendryte_status_t ka_skslot_check(bool valid, kendryte_ka_slot_t slot, uint32_t keybits)
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

#if 1
	// check registers
    if (valid)
    {
        uint32_t key_info;
        uint32_t idx;
        uint32_t tagbase;

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
				key_info = readl(puf_ka_base + KA_SK_0_OFFSET + idx * 4);
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
				key_info = readl(puf_ka_base + KA_SK_0_OFFSET + idx * 2 * 4);
				if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
					(((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
					(((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
					(readl(puf_ka_base + KA_SK_0_OFFSET + idx * 2 * 4 + 4) != ((tagbase + idx) << 16)))
					return E_INVALID;
				break;
			case SK512_0:
			case SK512_1:
				idx = slot - SK512_0;
				tagbase = 0x60;
				key_info = readl(puf_ka_base + KA_SK_0_OFFSET + idx * 4 * 4);
				if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
					(((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
					(((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
					(readl(puf_ka_base + KA_SK_0_OFFSET + idx * 4 * 4 + 4) != ((tagbase + idx) << 16)) ||
					(readl(puf_ka_base + KA_SK_0_OFFSET + idx * 4 * 4 + 8) != ((tagbase + idx) << 16)) ||
					(readl(puf_ka_base + KA_SK_0_OFFSET + idx * 4 * 4 + 12) != ((tagbase + idx) << 16)))
					return E_INVALID;
				break;
			default:
				return E_INVALID;
        }
    }
#endif

	return SUCCESS;
}

static kendryte_status_t keyslot_check(bool valid, kendryte_key_type_t keytype, uint32_t slot, uint32_t keybits)
{
	switch (keytype)
    {
		case SSKEY:
			return ka_skslot_check(valid, (kendryte_ka_slot_t)slot, keybits);
		default:
			return E_INVALID;
    }
}

int get_key_slot_idx(kendryte_key_type_t keytype, kendryte_ka_slot_t keyslot)
{
	switch(keytype)
	{
		case SSKEY:
			switch (keyslot)
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

//---------------------------------CRYTPO FUNCTION----------------------------------------//
kendryte_status_t crypto_write_iv(uint8_t *iv, size_t length)
{
	if(length > IV_MAXLEN)
		return E_INVALID;
	
	write_data((uint32_t *)(puf_crypto_base + CRYPTO_IV_OFFSET), iv, length, true);

    return SUCCESS;
}

kendryte_status_t crypto_write_dgst(uint8_t *dgst, size_t length)
{
	if(length > DGST_INT_STATE_LEN)
		return E_INVALID;

	write_data((uint32_t *)(puf_crypto_base + CRYPTO_DGST_IN_OFFSET), dgst, length, true);

	return SUCCESS;
}

void crypto_read_dgest(uint8_t *out, size_t length)
{
	length = length < DGST_INT_STATE_LEN ? length : DGST_INT_STATE_LEN;
    read_data(out, (void *)(puf_crypto_base + CRYPTO_DGST_OUT_OFFSET), length, true);
}

//----------------------------------------DMA FUNCTION----------------------------------------//
void dma_write_start(void)
{
	writel(0x1, puf_dma_base + DMA_START_OFFSET);
}

bool dma_check_busy_status(uint32_t *status)
{
	uint32_t stat = readl(puf_dma_base + DMA_STAT_0_OFFSET);
	bool busy = (stat & DMA_STATUS_0_BUSY_MASK) != 0;

	if(status != NULL)
		*status = stat;

	return busy;
}

void dma_write_config_0(bool rng_enable, bool sgdma_enable, bool no_cypt)
{
    uint32_t value = 0;

    if (rng_enable)
        value |= 0x1;
    if (sgdma_enable)
        value |= 0x1 << 1;
    if (no_cypt)
        value |= 0x1 << 2;

	writel(value, puf_dma_base + DMA_CFG_0_OFFSET);
}

void dma_write_data_block_config(bool head, bool tail, bool dn_intrpt, bool dn_pause, uint32_t offset)
{
    uint32_t value = 0;

    if (head)
        value |= 0x1 << DMA_DSC_CFG_4_HEAD_BITS;
    if (tail)
        value |= 0x1 << DMA_DSC_CFG_4_TAIL_BITS;
    if (dn_intrpt)
        value |= 0x1 << DMA_DSC_CFG_4_DN_INTRPT_BITS;
    if (dn_pause);
        value |= 0x1 << DMA_DSC_CFG_4_DN_PAUSE_BITS;
    value |= offset << DMA_DSC_CFG_4_OFFSET_BITS;

	writel(value, puf_dma_base + DMA_DSC_CFG_4_OFFSET);
}

void dma_write_rwcfg(const uint8_t *out, const uint8_t *in, uint32_t len)
{
	writel(len, puf_dma_base + DMA_DSC_CFG_2_OFFSET);

	if(NULL == out)
	{
		writel((uintptr_t)out, puf_dma_base + DMA_DSC_CFG_1_OFFSET);
	}
	else
	{
		writel(dma_phys_out, puf_dma_base + DMA_DSC_CFG_1_OFFSET);
	}

	if(NULL == in)
	{
		writel((uintptr_t)in, puf_dma_base + DMA_DSC_CFG_0_OFFSET);
	}
	else
	{
		memcpy(dma_virt_in, in, len);
		writel(dma_phys_in, puf_dma_base + DMA_DSC_CFG_0_OFFSET);
	}
}

void dma_write_key_config_0(kendryte_key_type_t keytype, kendryte_algo_type_t algo, uint32_t size, uint32_t slot_index)
{
	uint32_t value = 0;

    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;

	writel(value, puf_dma_base + DMA_KEY_CFG_0_OFFSET);
}


//--------------------------------------------GCM FUNCTION--------------------------------------------------//
static kendryte_status_t gcm_prepare(struct kendryte_aes_ctx *ctx, const uint8_t *out, uint32_t inlen)
{
	kendryte_status_t check;
	uint32_t val32;

	dma_write_key_config_0(ctx->keytype, ALGO_TYPE_GCM, (ctx->keylen << 3), get_key_slot_idx(ctx->keytype, ctx->keyslot));

	if ((check = get_config(&val32, ctx, out != NULL, false, false)) != SUCCESS)
        return check;

	writel(val32, ctx->dev->base + GCM_CFG_1_OFFSET);

	if ((ctx->inlen) != ULLONG_MAX)
		writel(ctx->incj0, ctx->dev->base + GCM_CFG_2_OFFSET);
    else
		writel(0x0, ctx->dev->base + GCM_CFG_2_OFFSET);

    // J_0
    if (out != NULL)
    {
        crypto_write_iv(ctx->j0, AES_BLOCK_SIZE);
		if ((ctx->inlen) != ULLONG_MAX && inlen > 0)
            ctx->incj0 += ((inlen - 1 + AES_BLOCK_SIZE) / AES_BLOCK_SIZE);
    }

    // Restore GHASH
    crypto_write_dgst(ctx->ghash, AES_BLOCK_SIZE);

	return SUCCESS;
}

static kendryte_status_t gcm_postproc(struct kendryte_aes_ctx *ctx)
{
	crypto_read_dgest(ctx->ghash, AES_BLOCK_SIZE);
	return SUCCESS;
}


/*
	Pass the input into GCM hardware
*/
static kendryte_status_t _ctx_update(struct kendryte_aes_ctx *ctx,
								uint8_t *out,
								uint32_t *outlen,
								const uint8_t *in,
								uint32_t inlen,
								bool last)
{
	uint32_t val32;
	kendryte_status_t check;

	if (dma_check_busy_status(NULL))
        return E_BUSY;

	dma_write_config_0(false, false, false);
    dma_write_data_block_config(ctx->start ? false : true, last, true, true, 0);
    dma_write_rwcfg(out, in, inlen);   // config physical address of DMA read and write

    if ((check = gcm_prepare(ctx, out, inlen)) != SUCCESS)
        return check;

	flush_cache_all();
    dma_write_start();
    while (dma_check_busy_status(&val32));

    if (val32 != 0)
    {
        pr_err("DMA status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

	val32 = readl(ctx->dev->base + GCM_STAT_OFFSET);
    if ((val32 & GCM_STATUS_RESP_MASK) != 0)
    {
        pr_err("GCM status: 0x%08x\n", val32);
        return E_ERROR;
    }

    if ((check = gcm_postproc(ctx)) != SUCCESS)
        return check;

	if (out != NULL)
    {
        *outlen = inlen;
		memcpy(out, dma_virt_out, inlen);
    }

	return SUCCESS;
}

static kendryte_status_t ctx_update(gcm_op op,
								struct kendryte_aes_ctx *ctx,
								bool encrypt,
								uint8_t *out,
								uint32_t *outlen,
								const uint8_t *in,
								uint32_t inlen)
{
	uint32_t i;
	// check ctx is owned by this operation (GCM mode)
    if ((ctx->op != op) || (ctx->encrypt != encrypt))
        return E_UNAVAIL;
	
	// continue if msg is NULL or msglen is zero
    if ((in == NULL) || (inlen == 0))
    {
        if (outlen != NULL)
            *outlen = 0;
        return SUCCESS;
    }

	switch(ctx->stage)
	{
		case GCM_NONE_S:
			break;
		case GCM_AAD_S:
			ctx->aadlen += (uint64_t)inlen;
			break;
		case GCM_TEXT_S:
			ctx->inlen += (uint64_t)inlen;
		
		default:
			E_FIRMWARE;
	}

	int j;

	blsegs segs = segment(ctx->buffer, ctx->buflen, in, inlen, AES_BLOCK_SIZE, ctx->minlen);
	ctx->buflen = 0;
	uint32_t seglen = 0;
	kendryte_status_t check = SUCCESS;
	if (ctx->stage == GCM_TEXT_S)
        *outlen = 0;

	for(i = 0; i < segs.nsegs; i++)
	{
		if(segs.seg[i].process)// process
		{
			if(ctx->stage == GCM_TEXT_S)
			{
				check = _ctx_update(ctx, out + *outlen, &seglen, segs.seg[i].addr, segs.seg[i].len, false);
			}
			else
			{
				check = _ctx_update(ctx, NULL, NULL, segs.seg[i].addr, segs.seg[i].len, false);
			}
			
			if(check != SUCCESS)
			{
				// release context
				ctx->op = GCM_AVAILABLE_OP;
				return check;
			}

			if(ctx->stage == GCM_TEXT_S)
				*outlen += seglen;
			
			if(ctx->start == false)
				ctx->start = true;
		}
		else// keep in the internal buffer
		{
			if((segs.seg[i].addr == ctx->buffer) && (ctx->buflen == 0))
				// skip copy what already in the right place
				ctx->buflen += segs.seg[i].len;
			else
			{
				// copy into the buffer
				memmove(ctx->buffer + ctx->buflen, segs.seg[i].addr, segs.seg[i].len);
				ctx->buflen += segs.seg[i].len;
			}
		}
	}
	
	return SUCCESS;
}

static kendryte_status_t gcm_tag(struct kendryte_aes_ctx *ctx, uint8_t *tag, uint32_t taglen, bool from_reg)
{
	uint32_t val32, tmplen = 0;
	kendryte_status_t check;

	union {
        uint8_t uc[AES_BLOCK_SIZE];
        uint32_t u32[AES_BLOCK_SIZE / 4];
    } tmp;

    if (ctx->op == GCM_GHASH_OP)
    {
        memcpy(tag, ctx->ghash, taglen);
        return SUCCESS;
    }

	// len(A) || len(C)
    tmp.u32[0] = be2le((uint32_t)((ctx->aadlen << 3) >> 32));
    tmp.u32[1] = be2le((uint32_t)(ctx->aadlen << 3));
    tmp.u32[2] = be2le((uint32_t)((ctx->inlen << 3) >> 32));
    tmp.u32[3] = be2le((uint32_t)(ctx->inlen << 3));
	if((check = _ctx_update(ctx, NULL, NULL, tmp.uc, AES_BLOCK_SIZE, true)) != SUCCESS)
		return check;

	// last GCTR
    ctx->inlen = ULLONG_MAX;

	if(!from_reg)
	{
		if(((check = _ctx_update(ctx, tmp.uc, &tmplen, ctx->ghash, AES_BLOCK_SIZE, true)) != SUCCESS) || (tmplen != AES_BLOCK_SIZE))
			return E_FIRMWARE;

		memcpy(tag, tmp.uc, taglen);
        return SUCCESS;
	}

	crypto_write_iv(ctx->j0, AES_BLOCK_SIZE);

    if ((check = get_config(&val32, ctx, true, true, true)) != SUCCESS)
        return check;

	writel(val32, ctx->dev->base + GCM_CFG_1_OFFSET);
	writel(0, ctx->dev->base + GCM_CFG_2_OFFSET);

    dma_write_data_block_config(true, true, true, true, 0);
    dma_write_rwcfg(NULL, NULL, 0);
    dma_write_config_0(false, false, false);
    dma_write_key_config_0(ctx->keytype, ALGO_TYPE_GCM, (ctx->keylen << 3), get_key_slot_idx(ctx->keytype, ctx->keyslot));
    
	flush_cache_all();
    dma_write_start();

    while (dma_check_busy_status(&val32));
 
    if (val32 != 0)
    {
        pr_err("DMA status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

	val32 = readl(ctx->dev->base + GCM_STAT_OFFSET);
    if ((val32 & GCM_STATUS_RESP_MASK) != 0)
    {
        pr_err("GCM status: 0x%08x\n", val32);
        return E_ERROR;
    }
    crypto_read_dgest(tag, taglen);

    return SUCCESS;
}

static kendryte_status_t ctx_final(gcm_op op,
								struct kendryte_aes_ctx *ctx,
								bool encrypt,
								uint8_t *out,
								uint32_t *outlen,
								uint8_t *tag,
								uint32_t taglen,
								bool from_reg)
{
	kendryte_status_t check = SUCCESS;

	if(outlen != NULL)
		*outlen = 0;

	// check ctx is owned by this operation (GCM mode)
    if((ctx->op != op) || (ctx->encrypt != encrypt))
        return E_UNAVAIL;

	if(ctx->buflen != 0)
	{
		if((check = _ctx_update(ctx, out, outlen, ctx->buffer, ctx->buflen, true)) != SUCCESS)
			goto release_gcm;
	}

	if(encrypt)
		check = gcm_tag(ctx, tag, taglen, from_reg);
	else
	{
		check = gcm_tag(ctx, tag, taglen, !from_reg);
	}

release_gcm:
	ctx->op = GCM_AVAILABLE_OP;

	return check;
}

static kendryte_status_t ctx_init(gcm_op op,
								struct kendryte_aes_ctx *ctx,
								bool encrypt,
								kendryte_cipher_t cipher,
								kendryte_key_type_t keytype,
								size_t keyaddr,
								uint32_t keybits,
								const uint8_t *j0)
{
	uint32_t val32;
	kendryte_status_t check;

	// abort if gcm ctx is occupied
    if (ctx->op != GCM_AVAILABLE_OP)
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

	// check key settings for block cipher
    if ((keytype != SWKEY) && ((check = keyslot_check(true, keytype, (uint32_t)keyaddr, keybits)) != SUCCESS))
		return check;

	// check and set J_0 if needed
    if (op != GCM_GHASH_OP)
    {
        if (j0 == NULL)
            return E_INVALID;
        memcpy(ctx->j0, j0, AES_BLOCK_SIZE);
    }

	// initialize for block-cipher GCM mode
	ctx->keylen = 0;
	ctx->ivlen = 0;
	ctx->aadlen = 0;
	ctx->taglen = 0;
	ctx->pclen = 0;
	ctx->inlen = 0;
	ctx->minlen = 1;
	ctx->buflen = 0;
	ctx->op = op;
	ctx->incj0 = 1;
	ctx->stage = GCM_NONE_S;
    ctx->cipher = cipher;
	ctx->encrypt = encrypt;
	memset(ctx->ghash, 0, AES_BLOCK_SIZE);
	// set key
	ctx->keylen = keybits >> 3;
	ctx->keytype = keytype;
	if (keytype != SWKEY)
        ctx->keyslot = (uint32_t)keyaddr;
    else
        memcpy(ctx->key, (const void*)keyaddr, b2B(keybits));

    return SUCCESS;
}

static kendryte_status_t build_j0(uint8_t *j0,
								kendryte_cipher_t cipher,
								kendryte_key_type_t keytype,
								size_t keyaddr,
								uint32_t keybits,
								const uint8_t *iv,
								uint32_t ivlen)
{
	uint8_t tmp[AES_BLOCK_SIZE];
	uint64_t ivbits = ivlen << 3;
	kendryte_status_t check;

	if((iv == NULL) || (ivlen == 0)) {
		pr_err("%s() Error: need more params!\n", __func__);
		return E_INVALID;
	}

	if (ivlen == 12)
    {
        memcpy(j0, iv, ivlen);
        memset(j0 + ivlen, 0, 3);
        *(j0 + 15) = 1;
        return SUCCESS;
    }

	struct kendryte_aes_ctx ctx = { .op = GCM_AVAILABLE_OP };
	// GHASH init
	if((check = ctx_init(GCM_GHASH_OP, &ctx, false, cipher, keytype, keyaddr, keybits, NULL)) != SUCCESS)
		return check;
	// GHASH update
	if((check = ctx_update(GCM_GHASH_OP, &ctx, false, NULL, NULL, iv, ivlen)) != SUCCESS)
		return check;
	

	memset(tmp, 0, AES_BLOCK_SIZE);
	if((ivlen % AES_BLOCK_SIZE) != 0)
	{
		uint32_t padlen = AES_BLOCK_SIZE - (ivlen % AES_BLOCK_SIZE);
		if((check = ctx_update(GCM_GHASH_OP, &ctx, false, NULL, NULL, tmp, padlen)) != SUCCESS)
			return check;
	}

	*((uint32_t*)(tmp + 8)) = be2le((uint32_t)(ivbits >> 32));
    *((uint32_t*)(tmp + 12)) = be2le((uint32_t)ivbits);
    if((check = ctx_update(GCM_GHASH_OP, &ctx, false, NULL, NULL, tmp, AES_BLOCK_SIZE)) != SUCCESS)
			return check;

    return ctx_final(GCM_GHASH_OP, &ctx, false, NULL, NULL, j0, AES_BLOCK_SIZE, false);
}

static kendryte_status_t step_stage(struct kendryte_aes_ctx *ctx, gcm_stage stage)
{
    switch (ctx->stage)
    {
        case GCM_NONE_S:
            break;
        case GCM_AAD_S:
            switch (stage)
            {
                case GCM_AAD_S:
                    return SUCCESS;
                case GCM_TEXT_S:
                    if (ctx->buflen != 0)
                    { // clear AAD and start input
                        kendryte_status_t check;
                        check = _ctx_update(ctx, NULL, NULL, ctx->buffer, ctx->buflen, true);
                        if (check != SUCCESS)
                            return check;
                        ctx->buflen = 0;
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

    if (ctx->stage != stage)
    {
        ctx->start = false;
        ctx->stage = stage;
    }

    return SUCCESS;
}

static kendryte_status_t gcm_init(struct kendryte_aes_ctx *ctx,
								kendryte_cipher_t cipher,
								kendryte_key_type_t keytype,
								size_t keyaddr,
								uint32_t keybits,
								const uint8_t *iv,
								uint32_t ivlen,
								bool encrypt)
{
	kendryte_status_t check;
	uint8_t j0[AES_BLOCK_SIZE];

	if((check = build_j0(j0, cipher, keytype, keyaddr, keybits, iv, ivlen)) != SUCCESS)
		return check;

	return ctx_init(AES_GCM_OP, ctx, encrypt, cipher, keytype, keyaddr, keybits, j0);
}

static kendryte_status_t gcm_update(struct kendryte_aes_ctx *ctx, uint8_t *out, uint32_t *outlen, const uint8_t *in, uint32_t inlen, bool encrypt)
{
    kendryte_status_t check = step_stage(ctx, (out == NULL) ? GCM_AAD_S : GCM_TEXT_S);

    if(check != SUCCESS)
        return check;

    return ctx_update(AES_GCM_OP, ctx, encrypt, out, outlen, in, inlen);
}

static kendryte_status_t import_plaintext_key_to_slot(struct kendryte_aes_ctx *ctx, kendryte_key_type_t keytype, kendryte_ka_slot_t slot, const uint8_t *key, uint32_t keybits)
{
	kendryte_status_t check;
	int i;
	uint32_t val32;

	// keytype MUST be SSKEY
	if (keytype != SSKEY)
        return E_INVALID;

	// check KA key slot by key length
    if ((check = keyslot_check(false, keytype, slot, keybits)) != SUCCESS)
        return check;

	if(readl(puf_kwp_base + KWP_STATUS_OFFSET) & KWP_STATUS_BUSY_MASK)
		return E_BUSY;

    val32 = 0x0<<0 | keybits<<8;

    switch (keytype)
    {
		case SSKEY:
			val32 |= 0x0<<19;
			break;
		default:
			return E_FIRMWARE;
    }

	val32 |= get_key_slot_idx(keytype, slot)<<20;
	writel(val32, puf_kwp_base + KWP_CONFIG_OFFSET);

    memset(kwp_buffer, 0, KWP_KEY_MAXLEN);
    memcpy(kwp_buffer, key, b2B(keybits));
	

    uint32_t *buf = (uint32_t *)kwp_buffer;
    for (i = 0; i < KWP_KEY_MAXLEN / 4; ++i)
		writel(be2le(buf[i]), puf_kwp_base + KWP_KEY_IN_OUT_OFFSET + 4*i);	// store key into KWP

    return kwp_start();
}

static kendryte_status_t clear_ka_slot(struct kendryte_aes_ctx *ctx, kendryte_ka_slot_t slot)
{
	uint32_t val32;
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
			writel(val32, puf_ka_base + KA_SK_FREE_OFFSET);
			return SUCCESS;
		case SK256_0:
		case SK256_1:
		case SK256_2:
		case SK256_3:
			val32 = 0x3<<(2*(slot-SK256_0));
			writel(val32, puf_ka_base + KA_SK_FREE_OFFSET);
			return SUCCESS;
		case SK512_0:
		case SK512_1:
			val32 = 0xf<<(4*(slot-SK512_0));
			writel(val32, puf_ka_base + KA_SK_FREE_OFFSET);
			return SUCCESS;

		default:
			return E_INVALID;
	}
}

static kendryte_status_t clear_key(struct kendryte_aes_ctx *ctx, kendryte_key_type_t keytype, kendryte_ka_slot_t slot, uint32_t keybits)
{
	kendryte_status_t check;

	if((check = keyslot_check(true, keytype, slot, keybits)) != SUCCESS)
		return check;

	return clear_ka_slot(ctx, slot);
}

static int kendryte_gcm_aes_setkey(struct crypto_aead *tfm, const uint8_t *key, unsigned int keylen)
{
	struct kendryte_aes_ctx *ctx = crypto_aead_ctx(tfm);

	ctx->keylen = keylen;	    // key length in bytes
	memcpy(ctx->key, key, keylen);

	return 0;
}

static int kendryte_gcm_aes_setauthsize(struct crypto_aead *tfm, uint32_t authsize)
{
	switch(authsize)
	{
		case 16:
			break;
		
		default:
			return -EINVAL;
	}

	return 0;
}

/*
* Encryption case:
*  INPUT  =   Assocdata  ||   Plaintext
*           <-  aadlen ->   <-  pclen  ->
*          <---------- total ----------->
*
*  OUTPUT =   Assocdata  ||  Ciphertext  ||   Tag
*          <- aadlen ->     <- outlen ->    <- taglen ->
*          <--------- total ----------->
*
* Decryption case:
*  INPUT  =   Assocdata  ||  Ciphertext  ||     Tag
*          <- aadlen ->    <------- pclen ------------>
*                                          <- taglen ->
*          <--------- total -------------->
*
*  OUTPUT =   Assocdata  ||   Plaintext
*          <- aadlen ->   <- crypten - tag ->
*          <---------- total ----->
*/
static int kendryte_gcm_aes_crypt(struct aead_request *req, bool encrypt)
{
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct kendryte_aes_ctx *ctx = crypto_aead_ctx(tfm);
	kendryte_status_t check;
    void *buf_src;
    void *buf_dst;
    const uint8_t *in;
	const uint8_t *tag;
	const uint8_t *tmptag;
	const uint8_t *out;
    uint32_t inlen;
	uint32_t outlen;
    uint32_t toutlen;
	uint32_t total;
	kendryte_cipher_t cipher;
	kendryte_key_type_t keytype;
	kendryte_ka_slot_t keyslot;
	const uint8_t *key;
	uint32_t keybits;
	const uint8_t *iv;
	uint32_t ivlen;
	const uint8_t *aad;
	uint32_t aadlen;
	uint32_t taglen;
	uint32_t pclen;
	int ret = 0;

	// add hardlock
    while(hardlock_lock(hardlock));

	puf_dma_base = ioremap(0x91210000, 0x100);
	puf_kwp_base = ioremap(0x91210300, 0x100);
	puf_crypto_base = ioremap(0x91210100, 0x100);
	puf_ka_base = ioremap(0x91210C00, 0x100);

	cipher = AES;
	keytype = SSKEY;
	iv = req->iv;
	ivlen = crypto_aead_ivsize(tfm);
	aadlen = req->assoclen;
	taglen = crypto_aead_authsize(tfm);
	keybits = (ctx->keylen << 3);
	key = ctx->key;
	keyslot = (keybits > 128) ? SK256_0 : SK128_0;
	pclen = req->cryptlen;
	total = aadlen + pclen;

	buf_src = kzalloc(total, GFP_KERNEL);
	if(encrypt)
		buf_dst = kzalloc((total + taglen), GFP_KERNEL);
	else
		buf_dst = kzalloc((total - taglen), GFP_KERNEL);

	if((NULL == buf_src) || (NULL == buf_dst))
	{
		pr_err("Could not kzalloc buffer.\n");
        goto done;
	}
	scatterwalk_map_and_copy(buf_src, req->src, 0, total, 0);

	if(aadlen > 0)
	{
		aad = kzalloc(aadlen, GFP_KERNEL);
		if(NULL == aad)
		{
			pr_err("Could not kzalloc buffer.\n");
			goto done;
		}
		memcpy(aad, buf_src, aadlen);
	}

	tag = kzalloc(AES_BLOCK_SIZE, GFP_KERNEL);
	tmptag = kzalloc(AES_BLOCK_SIZE, GFP_KERNEL);
	if((NULL == tag) || (NULL == tmptag))
	{
		pr_err("Could not kzalloc buffer.\n");
        goto done;
	}

	// input data, plaintext or ciphertext
    in = buf_src + aadlen;
    inlen = pclen;
    if(!encrypt)
	{
		inlen -= taglen;
		memcpy(tmptag, in+inlen, taglen);
	}

	if(inlen > 0)
		out = kzalloc(inlen, GFP_KERNEL);
	else
		out = kzalloc((inlen + 1), GFP_KERNEL);
	if(NULL == out)
	{
		pr_err("Could not kzalloc buffer.\n");
        goto done;
	}

	dma_virt_in = dma_alloc_coherent(ctx->dev->dev, (total + ivlen), &dma_phys_in, GFP_KERNEL);
	dma_virt_out = dma_alloc_coherent(ctx->dev->dev, (total + ivlen), &dma_phys_out, GFP_KERNEL);

#ifdef _debug_print
	pr_info("[%s:%d]taglen:%d aadlen:%d ivlen:%d keylen:%d pclen:%d total:%d\n", __FUNCTION__, __LINE__, taglen, aadlen, ivlen, keybits, pclen, total);
#endif

	// import key to internal slot
	if((check = import_plaintext_key_to_slot(ctx, keytype, keyslot, key, keybits)) != SUCCESS)
		goto done;

    ctx->op = GCM_AVAILABLE_OP;
	outlen = 0;

	// GCM init
	if((check = gcm_init(ctx, cipher, keytype, keyslot, keybits, iv, ivlen, encrypt)) != SUCCESS)
		goto done;

    // GCM update
    if((check = gcm_update(ctx, NULL, NULL, aad, aadlen, encrypt)) != SUCCESS)
        goto done;

    if((check = gcm_update(ctx, out, &toutlen, in, inlen, encrypt)) != SUCCESS)
        goto done;
    outlen += toutlen;

    // GCM done
	if ((check = ctx_final(AES_GCM_OP, ctx, encrypt, out + outlen, &toutlen, tag, taglen, true)) != SUCCESS)
		goto done;

	if(!encrypt)
	{
		if(memcmp(tag, tmptag, taglen) != 0)
		{
			check = E_VERFAIL;
			goto done;
		}
	}

	outlen += toutlen;

    // clear key from internal slot
    if((check = clear_key(ctx, keytype, keyslot, keybits)) != SUCCESS )
		goto done;

	if(aadlen > 0)
	{
		memcpy(buf_dst, aad, aadlen);
		memcpy(buf_dst + aadlen, out, outlen);
		// output data, include <aad || ciphertext || tag> or <aad || plaintext>
		if(encrypt)
		{
			memcpy(buf_dst + aadlen + outlen, tag, taglen);
			scatterwalk_map_and_copy(buf_dst, req->dst, 0, total + taglen, 1);
		}
		else
		{
			scatterwalk_map_and_copy(buf_dst, req->dst, 0, total - taglen, 1);
		}
	}
	else
	{
		memcpy(buf_dst, out, outlen);
		// output data, include <aad || ciphertext || tag> or <aad || plaintext>
		if(encrypt)
		{
			memcpy(buf_dst + outlen, tag, taglen);
			scatterwalk_map_and_copy(buf_dst, req->dst, 0, total + taglen, 1);
		}
		else
		{
			scatterwalk_map_and_copy(buf_dst, req->dst, 0, total - taglen, 1);
		}
	}

#ifdef _debug_print
	pr_info("[K230-GCM]: \n\r");
	int i;
	for(i=0; i<(total + taglen); i++)
	{
		pr_info("[%s:%d] aad+ciphertext+tag:0x%x", __func__, __LINE__, *(uint8_t *)(buf_dst + i));
	}
#endif

done:
    if(check != SUCCESS)
    {
        pr_err("Encrypt or Decrypt Error!\n");
        ret = -1;
    }

	// release hardlock
	hardlock_unlock(hardlock);
    
	iounmap(puf_dma_base);
	iounmap(puf_kwp_base);
	iounmap(puf_crypto_base);
	iounmap(puf_ka_base);
	kfree(buf_src);
	kfree(buf_dst);
	if(aadlen > 0)
		kfree(aad);
	kfree(tag);
	kfree(tmptag);
	kfree(out);
	dma_free_coherent(ctx->dev->dev, (total + ivlen), dma_virt_in, dma_phys_in);
	dma_free_coherent(ctx->dev->dev, (total + ivlen), dma_virt_out, dma_phys_out);

	return ret;
}

static int kendryte_cra_init(struct crypto_tfm *tfm)
{
	struct kendryte_aes_ctx *ctx = crypto_tfm_ctx(tfm);

    ctx->dev = &crypto_info;

	return 0;
}

static int kendryte_gcm_aes_encrypt(struct aead_request *req)
{
	return kendryte_gcm_aes_crypt(req, true);
}

static int kendryte_gcm_aes_decrypt(struct aead_request *req)
{
	return kendryte_gcm_aes_crypt(req, false);
}

static int kendryte_gcm_aes_init(struct crypto_aead *tfm)
{
	return kendryte_cra_init(&tfm->base);
}

struct aead_alg kendryte_gcm_aes_alg = {
	.setkey			= 	kendryte_gcm_aes_setkey,
	.setauthsize	= 	kendryte_gcm_aes_setauthsize,
	.encrypt		=	kendryte_gcm_aes_encrypt,
	.decrypt		=	kendryte_gcm_aes_decrypt,
	.init			= 	kendryte_gcm_aes_init,
	.ivsize			= 	GCM_AES_IV_SIZE,
	.maxauthsize	= 	AES_BLOCK_SIZE,

	.base = {
		.cra_name		= "gcm(aes)",
		.cra_driver_name	= "gcm-aes-canaan",
		.cra_priority		= 400,
		.cra_blocksize		= 1,
        .cra_flags			= CRYPTO_ALG_TYPE_AEAD,
		.cra_ctxsize		= sizeof(struct kendryte_aes_ctx),
		.cra_alignmask		= 0xf,
		.cra_module		= THIS_MODULE,
	},
};

static int kendryte_crypto_probe(struct platform_device *pdev)
{
    struct resource *res;
	struct device *dev = &pdev->dev;
    int err = -1;

    pr_info("Kendryte crypto driver probe!\n");

	crypto_info.dev = dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    crypto_info.base = devm_ioremap_resource(dev, res);
    if(IS_ERR(crypto_info.base))
        return PTR_ERR(crypto_info.base);

	if (!request_status) {
		if(of_property_read_u32(pdev->dev.of_node, "hardlock", &hardlock))
        {
            dev_err(&pdev->dev, "fail to parse hardlock num\n");
            return -EINVAL;
        }
		if (request_lock(hardlock))
		{
			dev_err(&pdev->dev, "request hardlock %d failed!\n", hardlock);
			hardlock = -1;
		}
		request_status = true;
		dev_err(&pdev->dev, "request hardlock %d success!\n", hardlock);
	}

    platform_set_drvdata(pdev, &crypto_info);

	err = crypto_register_aead(&kendryte_gcm_aes_alg);
    if(err < 0)
		goto gcm_err;

out:
    return err;

gcm_err:
	dev_err(&pdev->dev, "Unable to register cipher algorithm.\n");
    crypto_unregister_aead(&kendryte_gcm_aes_alg);
    goto out;
}

static int kendryte_crypto_remove(struct platform_device *pdev)
{
    crypto_unregister_aead(&kendryte_gcm_aes_alg);

    return 0;
}

static const struct of_device_id of_kendryte_crypto_ids[] = {
    { .compatible = "canaan,k230-crypto" },
    {  }
};
MODULE_DEVICE_TABLE(of, of_kendryte_crypto_ids);

static struct platform_driver kendryte_crypto_driver = {
    .probe = kendryte_crypto_probe,
    .remove = kendryte_crypto_remove,
    .driver = {
        .name = "kendryte-crypto",
        .of_match_table = of_kendryte_crypto_ids,
    },
};
module_platform_driver(kendryte_crypto_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CANAAN K230 Hardware Accelerator Crypto Driver");