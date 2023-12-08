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
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>
#include <crypto/internal/hash.h>
#include <crypto/sha.h>
#include <linux/canaan-hardlock.h>

#include "kendryte-hash.h"

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
void *hash_dma_virt_in;
dma_addr_t hash_dma_phys_in;
static int hardlock;
static bool request_status = false;
struct kendryte_sha256_info sha256_info;

//----------------------BASE FUNCTION ----------------------------------------//
static uint32_t be2le(uint32_t var)
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

static blsegs segment(uint8_t *buf, uint32_t buflen, const uint8_t *in, uint32_t inlen, uint32_t blocksize, uint32_t minlen)
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

static int get_key_slot_idx(kendryte_key_type_t keytype)
{
	switch(keytype)
	{
        case SWKEY:
            return 0;
		default:
        	return -1;
	}
}

//---------------------------------CRYTPO FUNCTION----------------------------------------//
static kendryte_status_t crypto_write_dgst(uint8_t *dgst, size_t length)
{
	if(length > DGST_INT_STATE_LEN)
		return E_INVALID;

	write_data((uint32_t *)(puf_crypto_base + CRYPTO_DGST_IN_OFFSET), dgst, length, true);

	return SUCCESS;
}

static void crypto_read_dgest(uint8_t *out, size_t length)
{
	length = length < DGST_INT_STATE_LEN ? length : DGST_INT_STATE_LEN;
    read_data(out, (void *)(puf_crypto_base + CRYPTO_DGST_OUT_OFFSET), length, true);
}

//----------------------------------------DMA FUNCTION----------------------------------------//
static void dma_write_start(void)
{
	writel(0x1, puf_dma_base + DMA_START_OFFSET);
}

static bool dma_check_busy_status(uint32_t *status)
{
	uint32_t stat = readl(puf_dma_base + DMA_STAT_0_OFFSET);
	bool busy = (stat & DMA_STATUS_0_BUSY_MASK) != 0;

	if(status != NULL)
		*status = stat;

	return busy;
}

static void dma_write_config_0(bool rng_enable, bool sgdma_enable, bool no_cypt)
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

static void dma_write_data_block_config(bool head, bool tail, bool dn_intrpt, bool dn_pause, uint32_t offset)
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

static void dma_write_rwcfg(struct kendryte_sha256_ctx *ctx, const uint8_t *out, const uint8_t *in, uint32_t len)
{
    hash_dma_virt_in = dma_alloc_coherent(ctx->dev->dev, (len + 32), &hash_dma_phys_in, GFP_KERNEL);

	writel(len, puf_dma_base + DMA_DSC_CFG_2_OFFSET);
    writel((uintptr_t)out, puf_dma_base + DMA_DSC_CFG_1_OFFSET);

	if(NULL == in)
	{
		writel((uintptr_t)in, puf_dma_base + DMA_DSC_CFG_0_OFFSET);
	}
	else
	{
		memcpy(hash_dma_virt_in, in, len);
		writel(hash_dma_phys_in, puf_dma_base + DMA_DSC_CFG_0_OFFSET);
	}
    dma_free_coherent(ctx->dev->dev, (len + 32), hash_dma_virt_in, hash_dma_phys_in);
}

static void dma_write_key_config_0(kendryte_key_type_t keytype, kendryte_algo_type_t algo, uint32_t size, uint32_t slot_index)
{
	uint32_t value = 0;

    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;

	writel(value, puf_dma_base + DMA_KEY_CFG_0_OFFSET);
}

//--------------------------------------------HASH FUNCTION--------------------------------------------------//
static kendryte_status_t _ctx_final(struct kendryte_sha256_ctx *hash_ctx, kendryte_dgst_st *md)
{
    if (md == NULL)
        return E_INVALID;

    crypto_read_dgest(md->dgst, DGST_INT_STATE_LEN);
    md->dlen = 32;

    return SUCCESS;
}

static kendryte_status_t _ctx_update(struct kendryte_sha256_ctx *hash_ctx, kendryte_dgst_st *md, const uint8_t *msg, uint32_t msglen, bool last)
{
    uint32_t val32;
    kendryte_status_t check;

    if (last && (md == NULL))
        return E_INVALID;

    if(dma_check_busy_status(NULL))
        return E_BUSY;

    dma_write_config_0(false, false, false);
    dma_write_data_block_config(hash_ctx->start ? false : true, last, true, true, 0);
    dma_write_rwcfg(hash_ctx, NULL, msg, msglen);
    dma_write_key_config_0(hash_ctx->keytype, ALGO_TYPE_HMAC, (hash_ctx->keybits < 512) ? hash_ctx->keybits : 512, get_key_slot_idx(hash_ctx->keytype));

    if (hash_ctx->start)
        crypto_write_dgst(hash_ctx->state, DGST_INT_STATE_LEN);

    val32 = ((hash_ctx->hash == SHA_256) ? 0x03 : 0x08);
    writel(val32, hash_ctx->dev->base + HASH_CONFIG_OFFSET);
    writel(hash_ctx->curlen, hash_ctx->dev->base + HASH_PLEN_OFFSET);

    flush_cache_all();
    dma_write_start();
    while(dma_check_busy_status(&val32));
    if (val32 != 0)
    {
        pr_err("DMA status 0: 0x%08x\n", val32);
        return E_ERROR;
    }

    val32 = readl(hash_ctx->dev->base + HASH_STATUS_OFFSET);
    if (val32 != 0)
    {
        pr_err("[ERROR] HMAC status: 0x%08x\n", val32);
        return E_ERROR;
    }

    if (!last)
    {
        crypto_read_dgest(hash_ctx->state, DGST_INT_STATE_LEN);
        hash_ctx->curlen = readl(hash_ctx->dev->base + HASH_ALEN_OFFSET);
    }

    return SUCCESS;
}

static kendryte_status_t _sha256_final(struct kendryte_sha256_ctx *hash_ctx, kendryte_dgst_st *md)
{
    kendryte_status_t check = SUCCESS;

    check = _ctx_update(hash_ctx, md, hash_ctx->buff, hash_ctx->buflen, true);
    if(check != SUCCESS)
        goto done;
    
    check = _ctx_final(hash_ctx, md);

done:
    hash_ctx->op = HMAC_AVAILABLE;
    return check;
}

static kendryte_status_t _sha256_update(struct kendryte_sha256_ctx *hash_ctx, const uint8_t *msg, uint32_t msglen)
{
    kendryte_status_t check;
    uint32_t i;
    // continue if msg is NULL or msglen is zero
    if ((msg == NULL) || (msglen == 0))
        return SUCCESS;

    blsegs segs = segment(hash_ctx->buff, hash_ctx->buflen, msg, msglen, hash_ctx->blocklen, hash_ctx->minlen);
    hash_ctx->buflen = 0;

    for (i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if ((check = _ctx_update(hash_ctx, NULL, segs.seg[i].addr, segs.seg[i].len, false)) != SUCCESS)
            {
                // release hmac context
                hash_ctx->op = HMAC_AVAILABLE;
                return check;
            }
            if (hash_ctx->start == false)
                hash_ctx->start = true;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == hash_ctx->buff) && (hash_ctx->buflen == 0))
            { // skip copy what already in the right place
                hash_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                memmove(hash_ctx->buff + hash_ctx->buflen, segs.seg[i].addr, segs.seg[i].len);
                hash_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return SUCCESS;
}

static kendryte_status_t _sha256_init(struct kendryte_sha256_ctx *hash_ctx)
{
    size_t keyaddr;
    uint32_t keybits;

    hash_ctx->blocklen = 64;
    // initialize for hash
    hash_ctx->buflen = 0;
    hash_ctx->keybits = 0;
    hash_ctx->minlen = 1;
    hash_ctx->keytype = 0;
    hash_ctx->curlen = 0;
    hash_ctx->op = HMAC_HASH;
    hash_ctx->hash = SHA_256;
    hash_ctx->start = false;

    keyaddr = 0;
    keybits = 0;
    memset(hash_ctx->key, 0, HMAC_BLOCK_MAXLEN);
    memcpy(hash_ctx->key, (const void*)keyaddr, b2B(keybits));
    
    return SUCCESS;
}

void sha256_free(struct kendryte_sha256_ctx *ctx, uint32_t msglen)
{
    iounmap(puf_dma_base);
    iounmap(puf_crypto_base);
    hardlock_unlock(hardlock);
}

static int kendryte_sha256_final(struct ahash_request *req)
{
    struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
    struct kendryte_sha256_ctx *ctx = crypto_ahash_ctx(tfm);
    kendryte_dgst_st md;
    kendryte_status_t check;
    int ret = 0;

    if((check = _sha256_final(ctx, &md)) != SUCCESS)
        goto done;

    memcpy(req->result, md.dgst, md.dlen);

done:
    if(check != SUCCESS)
    {
        pr_err("Hash-%s Error!\n", __func__);
        ret = -1;
        hardlock_unlock(hardlock);
    }

    sha256_free(ctx, req->nbytes);

    return 0;
}

static int kendryte_sha256_update(struct ahash_request *req)
{
    struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
    struct kendryte_sha256_ctx *ctx = crypto_ahash_ctx(tfm);
    const uint8_t *msg;
    uint32_t msglen;
    kendryte_status_t check;
    int ret = 0;

    msglen = req->nbytes;
    msg = kzalloc(msglen, GFP_KERNEL);
    if(NULL == msg)
    {
        pr_err("Could not kzalloc buffer in %s.\n", __func__);
        goto done;
    }
    scatterwalk_map_and_copy(msg, req->src, 0, msglen, 0);

    if((check = _sha256_update(ctx, msg, msglen)) != SUCCESS)
        goto done;

done:
    if(check != SUCCESS)
    {
        pr_err("Hash-%s Error!\n", __func__);
        ret = -1;
        hardlock_unlock(hardlock);
    }

    kfree(msg);
    return ret;
}

static int kendryte_sha256_init(struct ahash_request *req)
{
    struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
    struct kendryte_sha256_ctx *ctx = crypto_ahash_ctx(tfm);
    kendryte_status_t check;
    int ret = 0;

    // ioremap
    puf_dma_base = ioremap(0x91210000, 0x100);
    puf_crypto_base = ioremap(0x91210100, 0x100);

    // add hardlock
    while(hardlock_lock(hardlock));

    // hash-256
    ctx->op = HMAC_AVAILABLE;
    if((check = _sha256_init(ctx)) != SUCCESS)
        goto done;

done:
    if(check != SUCCESS)
    {
        pr_err("Hash-%s Error!\n", __func__);
        ret = -1;
        hardlock_unlock(hardlock);
    }

    return ret;
}

static int kendryte_sha256_digest(struct ahash_request *req)
{
    int ret = 0;

    while(hardlock_lock(hardlock));
    ret = kendryte_sha256_init(req);
    ret += kendryte_sha256_update(req);
    ret += kendryte_sha256_final(req);
    hardlock_unlock(hardlock);
    if(ret)
    {
        pr_err("Hash-%s Error!\n", __func__);
        return ret;
    }

    return 0;
}

static int kendryte_sha256_import(struct ahash_request *req, const void *in)
{
    return 0;
}

static int kendryte_sha256_export(struct ahash_request *req, void *out)
{
    return 0;
}

static int kendryte_sha256_cra_init(struct crypto_tfm *tfm)
{
    struct kendryte_sha256_ctx *ctx = crypto_tfm_ctx(tfm);

    ctx->dev = &sha256_info;
	return 0;
}

static struct ahash_alg kendryte_sha256_alg = {
    .init               = kendryte_sha256_init,
    .update	            = kendryte_sha256_update,
    .final              = kendryte_sha256_final,
    .digest             = kendryte_sha256_digest,
    .export             = kendryte_sha256_export,
    .import             = kendryte_sha256_import,
    .halg.digestsize    = SHA256_DIGEST_SIZE,
    .halg.statesize     = sizeof(struct kendryte_sha256_ctx),
	.halg.base          = {
		.cra_name               = "sha256",
		.cra_driver_name        = "sha256-kendryte",
		.cra_priority           = 300,
        .cra_flags              = (CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC),
		.cra_blocksize          = SHA256_BLOCK_SIZE,
		.cra_ctxsize            = sizeof(struct kendryte_sha256_ctx),
		.cra_init               = kendryte_sha256_cra_init,
        .cra_module             = THIS_MODULE,
	}
};

static int kendryte_sha256_probe(struct platform_device *pdev)
{
    struct resource *res;
	struct device *dev = &pdev->dev;
    int err = -1;

    pr_info("Kendryte sha256 driver probe!\n");

	sha256_info.dev = dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    sha256_info.base = devm_ioremap_resource(dev, res);
    if(IS_ERR(sha256_info.base))
        return PTR_ERR(sha256_info.base);

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

    platform_set_drvdata(pdev, &sha256_info);

	err = crypto_register_ahash(&kendryte_sha256_alg);
    if(err < 0)
		goto hash_err;
out:
    return err;

hash_err:
	dev_err(&pdev->dev, "Unable to register hash algorithm.\n");
    crypto_unregister_ahash(&kendryte_sha256_alg);
    goto out;
}

static int kendryte_sha256_remove(struct platform_device *pdev)
{
    crypto_unregister_ahash(&kendryte_sha256_alg);

    return 0;
}

static const struct of_device_id of_kendryte_sha256_ids[] = {
    { .compatible = "canaan,k230-hash" },
    {  }
};
MODULE_DEVICE_TABLE(of, of_kendryte_sha256_ids);

static struct platform_driver kendryte_sha256_driver = {
    .probe = kendryte_sha256_probe,
    .remove = kendryte_sha256_remove,
    .driver = {
        .name = "kendryte-sha256",
        .of_match_table = of_kendryte_sha256_ids,
    },
};
module_platform_driver(kendryte_sha256_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CANAAN K230 Hardware Accelerator SHA256 Driver");
