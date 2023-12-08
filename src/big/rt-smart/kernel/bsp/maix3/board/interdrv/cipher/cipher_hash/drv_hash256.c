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
#include "drv_hash256.h"

static struct rt_device g_hash_device = {0};
void *sec_base_addr = RT_NULL;
static int hash_hardlock;
struct hash_context hash_ctx = { .op = HMAC_AVAILABLE };

// ----------------------BASE FUNCTION ----------------------------------------//
#define b2B(bits) (((bits) + 7) / 8)

rt_uint32_t be2le(rt_uint32_t var)
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

#if 0
rt_size_t crypto_copy_from_user(void *dst, void *src, rt_size_t size)
{
    struct rt_lwp *lwp = RT_NULL;
    rt_mmu_info *mmu_info = RT_NULL;
    void *addr_start = RT_NULL, *addr_end = RT_NULL, *next_page = RT_NULL;
    void *tmp_dst = RT_NULL, *tmp_src = RT_NULL;
    RT_ASSERT(size != 0);

    lwp = lwp_self();
    if(!lwp)
        return 0;
    mmu_info = &lwp->mmu_info;

    tmp_dst = dst;
    addr_start = src;
    addr_end = (void *)((rt_uint8_t *)src + size);
    next_page = (void *)(((rt_size_t)addr_start + ARCH_PAGE_SIZE) & ~(ARCH_PAGE_SIZE - 1));

    do
    {
        rt_size_t len = (rt_uint8_t *)next_page - (rt_uint8_t *)addr_start;

        if (size < len)
        {
            len = size;
        }
        tmp_src = rt_hw_mmu_v2p(mmu_info, addr_start);  // kernel variable physical address
        if (!tmp_src)
        {
            break;
        }
        tmp_src = (void *)((rt_uint8_t *)tmp_src - PV_OFFSET);  // translate kernel variable virtual address
        rt_memcpy(tmp_dst, tmp_src, len);   // copy data
        tmp_dst = (void *)((rt_uint8_t *)tmp_dst + len);
        addr_start = (void *)((rt_uint8_t *)addr_start + len);
        size -= len;
        next_page = (void *)((rt_uint8_t *)next_page + ARCH_PAGE_SIZE);
    } while (addr_start < addr_end);

    return 0;
}
#endif

rt_uint64_t virt_to_phys(void *virt)
{
    return (rt_uint64_t)virt + PV_OFFSET;
}

/*************************************** DMA function ****************************************/
void dma_write_start(void)
{
    writel(0x1, sec_base_addr + DMA_START_OFFSET);
}

rt_bool_t dma_check_busy_status(rt_uint32_t *status)
{
    rt_uint32_t stat = readl(sec_base_addr + DMA_STAT_0_OFFSET);
    rt_bool_t busy = (stat & DMA_STATUS_0_BUSY_MASK) != 0;
    
    if (status != RT_NULL)
        *status = stat;

    return busy;
}

void dma_write_config_0(rt_bool_t rng_enable, rt_bool_t sgdma_enable, rt_bool_t no_cypt)
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

void dma_write_data_block_config(rt_bool_t head, rt_bool_t tail, rt_bool_t dn_intrpt, rt_bool_t dn_pause, rt_uint32_t offset)
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

void dma_write_rwcfg(const rt_uint8_t *out, const rt_uint8_t *in, rt_uint32_t len)
{
    rt_uint64_t pa;
    void *tmp_kvirt;
    tmp_kvirt = rt_malloc(len);
    if(!tmp_kvirt)
    {
        rt_kprintf("%s malloc fail!\n", __func__);
        return;
    }
    rt_memcpy(tmp_kvirt, (void*)in, len);

    writel(len, sec_base_addr + DMA_DSC_CFG_2_OFFSET);
    writel((uintptr_t)out, sec_base_addr + DMA_DSC_CFG_1_OFFSET);

    if(RT_NULL == in)
        writel((uintptr_t)in, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    else    // virtual to physical
    {
        rt_hw_cpu_dcache_clean_flush(tmp_kvirt, len);
        pa = virt_to_phys(tmp_kvirt);
        rt_hw_cpu_dcache_clean_flush((void *)pa, len);
        writel(pa, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    }
}

void dma_write_key_config_0(k_pufs_key_type_t keytype, k_pufs_algo_type_t algo, rt_uint32_t size, rt_uint32_t slot_index)
{
    rt_uint32_t value = 0;

    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;

    writel(value, sec_base_addr + DMA_KEY_CFG_0_OFFSET);
}

rt_int32_t get_key_slot_idx(k_pufs_key_type_t keytype, rt_uint32_t keyslot)
{
    switch (keytype)
    {
        case SWKEY:
            return 0;
        default:
            return -1;
    }
}

rt_bool_t crypto_write_dgst(rt_uint8_t *dgst, rt_size_t length)
{
    if(length > DGST_INT_STATE_LEN)
        return -RT_EINVAL;

    write_data((rt_uint32_t *)(sec_base_addr + CRYPTO_DGST_IN_OFFSET), dgst, length, RT_TRUE);

    return RT_EOK;
}

void crypto_read_dgest(rt_uint8_t *out, rt_size_t length)
{
    length = length < DGST_INT_STATE_LEN ? length : DGST_INT_STATE_LEN;
    read_data(out, (void *)(sec_base_addr + CRYPTO_DGST_OUT_OFFSET), length, RT_TRUE);
}

/*************************************** segment function ****************************************/
k_blsegs segment(rt_uint8_t * buf, rt_uint32_t buflen, const rt_uint8_t* in, rt_uint32_t inlen, rt_uint32_t blocksize, rt_uint32_t minlen)
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


static rt_err_t ctx_update(struct hash_context *hash_ctx, k_pufs_dgst_st *md, const rt_uint8_t *msg, rt_uint32_t msglen, rt_bool_t last)
{
    rt_uint32_t val32;
    rt_int32_t i;
    rt_err_t ret;

    if(last && (md == RT_NULL))
        return -RT_ERROR;

    if (dma_check_busy_status(RT_NULL))
        return -RT_EBUSY;

    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_data_block_config(hash_ctx->start ? RT_FALSE : RT_TRUE, last, RT_TRUE, RT_TRUE, 0);
    dma_write_rwcfg(RT_NULL, msg, msglen);
    dma_write_key_config_0(hash_ctx->keytype, ALGO_TYPE_HMAC, (hash_ctx->keybits < 512) ? hash_ctx->keybits : 512, get_key_slot_idx(hash_ctx->keytype, hash_ctx->keyslot));

    if (hash_ctx->start)
        crypto_write_dgst(hash_ctx->state, DGST_INT_STATE_LEN);

    val32 = ((hash_ctx->hash == SHA_256) ? 0x03 : 0x08);
    writel(val32, sec_base_addr + HASH_CONFIG_OFFSET);
    writel(hash_ctx->curlen, sec_base_addr + HASH_PLEN_OFFSET);

    dma_write_start();
    while(dma_check_busy_status(&val32));
    if (val32 != 0)
    {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        return -RT_ERROR;
    }

    val32 = readl(sec_base_addr + HASH_STATUS_OFFSET);
    if (val32 != 0)
    {
        rt_kprintf("[ERROR] HMAC status: 0x%08x\n", val32);
        return -RT_ERROR;
    }

    if (!last)
    {
        crypto_read_dgest(hash_ctx->state, DGST_INT_STATE_LEN);
        hash_ctx->curlen = readl(sec_base_addr + HASH_ALEN_OFFSET);
    }

    return RT_EOK;
}

static rt_err_t ctx_finish(struct hash_context *hash_ctx, k_pufs_dgst_st *md)
{
    if(md == RT_NULL)
        return -RT_EINVAL;

    crypto_read_dgest(md->dgst, DGST_INT_STATE_LEN);
    md->dlen = 32;

    return RT_EOK;
}

static rt_err_t sha256_init(struct hash_context *hash_ctx)
{
    rt_size_t keyaddr;
    rt_uint32_t keybits;
    hash_ctx->blocklen = 64;

    // initialize for hash
    hash_ctx->buflen = 0;
    hash_ctx->keybits = 0;
    hash_ctx->minlen = 1;
    hash_ctx->keytype = 0;
    hash_ctx->curlen = 0;
    hash_ctx->op = HMAC_HASH;
    hash_ctx->hash = SHA_256;
    hash_ctx->start = RT_FALSE;

    keyaddr = 0;
    keybits = 0;
    rt_memset(hash_ctx->key, 0, HMAC_BLOCK_MAXLEN);
    rt_memcpy(hash_ctx->key, (const void*)keyaddr, b2B(keybits));
    
    return RT_EOK;
}

static rt_err_t sha256_update(struct hash_context *hash_ctx, const rt_uint8_t *msg, rt_size_t msglen)
{
    rt_uint32_t i;
    rt_err_t ret;

    if((msg == RT_NULL) || (msglen == 0))
        return RT_EOK;

    k_blsegs segs = segment(hash_ctx->buff, hash_ctx->buflen, msg, msglen, hash_ctx->blocklen, hash_ctx->minlen);
    hash_ctx->buflen = 0;

    for (i = 0; i < segs.nsegs; i++)
    {
        if (segs.seg[i].process) // process
        {
            if ((ret = ctx_update(hash_ctx, RT_NULL, segs.seg[i].addr, segs.seg[i].len, RT_FALSE)) != RT_EOK)
            {
                // release hmac context
                hash_ctx->op = HMAC_AVAILABLE;
                return ret;
            }
            if (hash_ctx->start == RT_FALSE)
                hash_ctx->start = RT_TRUE;
        }
        else // keep in the internal buffer
        {
            if ((segs.seg[i].addr == hash_ctx->buff) && (hash_ctx->buflen == 0))
            { // skip copy what already in the right place
                hash_ctx->buflen += segs.seg[i].len;
            }
            else // copy into the buffer
            {
                rt_memmove(hash_ctx->buff + hash_ctx->buflen, segs.seg[i].addr, segs.seg[i].len);
                hash_ctx->buflen += segs.seg[i].len;
            }
        }
    }

    return RT_EOK;
}

static rt_err_t sha256_finish(struct hash_context *hash_ctx, k_pufs_dgst_st *md)
{
    rt_err_t ret = RT_EOK;

    ret = ctx_update(hash_ctx, md, hash_ctx->buff, hash_ctx->buflen, RT_TRUE);
    if(ret != RT_EOK)
        goto done;

    ret = ctx_finish(hash_ctx, md);

done:
    hash_ctx->op = HMAC_AVAILABLE;
    return ret;
}

#if 0
rt_err_t pufs_hash(k_pufs_dgst_st *md, const rt_uint8_t *msg, rt_uint32_t msglen)
{
    rt_err_t ret;
    struct hash_context hash_ctx = { .op = HMAC_AVAILABLE };

    if((ret = sha256_init(&hash_ctx)) != RT_EOK)
        return ret;

    if((ret = sha256_update(&hash_ctx, msg, msglen)) != RT_EOK)
        return ret;

    return sha256_finish(&hash_ctx, md);
}
#endif

static rt_err_t hash_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_hash_config_args *config_args = (struct rt_hash_config_args *)args;
    void *msg = RT_NULL;
    k_pufs_dgst_st md;
    rt_uint32_t msglen = RT_NULL;
    rt_err_t ret = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    msglen = config_args->msglen;
    msg = rt_malloc(msglen);
    if(!msg)
        return -RT_ENOMEM;
    lwp_get_from_user(msg, config_args->msg, msglen);

    switch(cmd)
    {
        case RT_HWHASH_CTRL_INIT:
        {
            // add hardlock
            while(0 != kd_hardlock_lock(hash_hardlock));

            if((ret = sha256_init(&hash_ctx)) != RT_EOK)
                goto release_lock;
            
            break;
        }

        case RT_HWHASH_CTRL_UPDATE:
        {
            if((ret = sha256_update(&hash_ctx, msg, msglen)) != RT_EOK)
                goto release_lock;
            
            break;
        }

        case RT_HWHASH_CTRL_FINISH:
        {
            if((ret = sha256_finish(&hash_ctx, &md)) != RT_EOK)
                goto release_lock;

            lwp_put_to_user(config_args->dgst, md.dgst, md.dlen);
            config_args->dlen = md.dlen;

            kd_hardlock_unlock(hash_hardlock);
            break;
        }

        default:
            return RT_EINVAL;
    }

ret_hash:
    return ret;

release_lock:
    if(ret != RT_EOK)
        kd_hardlock_unlock(hash_hardlock);
    goto ret_hash;
}

static rt_err_t hash_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t hash_close(rt_device_t dev)
{
    return RT_EOK;
}

const static struct rt_device_ops hash_ops = 
{
    RT_NULL,
    hash_open,
    hash_close,
    RT_NULL,
    RT_NULL,
    hash_control
};

int rt_hw_hash_device_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_device_t hash_device = &g_hash_device;

    sec_base_addr = rt_ioremap((void *)SECURITY_BASE_ADDR, SECURITY_IO_SIZE);
    if(RT_NULL == sec_base_addr) {
        rt_kprintf("Security module ioremap error!\n");
        return -RT_ERROR;
    }

    ret = rt_device_register(hash_device, K230_HASH_NAME, RT_DEVICE_FLAG_RDWR);
    if(ret) {
        rt_kprintf("hwhash device register fail\n");
        return ret;
    }

    hash_device->ops = &hash_ops;

    if(kd_request_lock(HARDLOCK_HASH)) {
        rt_kprintf("fail to request hardlock-%d\n", HARDLOCK_HASH);
        hash_hardlock = -1;
    } else
        hash_hardlock = HARDLOCK_HASH;

    return ret;
}
INIT_BOARD_EXPORT(rt_hw_hash_device_init);