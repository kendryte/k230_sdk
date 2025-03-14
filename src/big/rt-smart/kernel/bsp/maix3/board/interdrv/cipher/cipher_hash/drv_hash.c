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
#include <byteswap.h>
#include "riscv_io.h"
#include "riscv_mmu.h"
#include "drv_hardlock.h"
#include "drv_hash.h"

static struct rt_device g_hash_device = { 0 };
void* sec_base_addr = RT_NULL;
struct hash_context hash_ctx = { .op = HMAC_AVAILABLE, .busy = RT_FALSE };

// ----------------------BASE FUNCTION ----------------------------------------//
static rt_uint64_t virt_to_phys(void* virt)
{
    return (rt_uint64_t)virt + PV_OFFSET;
}

/*************************************** DMA function ****************************************/
static void dma_write_start(void)
{
    writel(0x1, sec_base_addr + DMA_START_OFFSET);
}

static rt_bool_t dma_check_busy_status(rt_uint32_t* status)
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
    if (dn_pause)
        value |= 0x1 << DMA_DSC_CFG_4_DN_PAUSE_BITS;
    value |= offset << DMA_DSC_CFG_4_OFFSET_BITS;

    writel(value, sec_base_addr + DMA_DSC_CFG_4_OFFSET);
}

static void dma_write_rwcfg(const rt_uint8_t* in, rt_uint32_t len, void* in_kvirt)
{
    rt_uint64_t pa;

    writel(len, sec_base_addr + DMA_DSC_CFG_2_OFFSET);
    writel(0, sec_base_addr + DMA_DSC_CFG_1_OFFSET);

    if (RT_NULL == in) {
        writel(0, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    } else {
        if (0 == lwp_get_from_user(in_kvirt, in, len))
            rt_memcpy(in_kvirt, in, len);
        rt_hw_cpu_dcache_clean(in_kvirt, len);
        pa = virt_to_phys(in_kvirt);
        writel(pa, sec_base_addr + DMA_DSC_CFG_0_OFFSET);
    }
}

static void dma_write_key_config_0(k_pufs_key_type_t keytype, k_pufs_algo_type_t algo, rt_uint32_t size, rt_uint32_t slot_index)
{
    rt_uint32_t value = 0;

    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;

    writel(value, sec_base_addr + DMA_KEY_CFG_0_OFFSET);
}

static void crypto_write_dgst(rt_uint8_t* dgst, rt_size_t length)
{
    rt_uint32_t* src = dgst;
    rt_uint32_t* dst = sec_base_addr + CRYPTO_DGST_IN_OFFSET;
    length = length >> 2;

    while (length--)
        *dst++ = *src++;
}

static void crypto_read_dgest(rt_uint8_t* out, rt_size_t length)
{
    rt_uint32_t* src = sec_base_addr + CRYPTO_DGST_OUT_OFFSET;
    rt_uint32_t* dst = out;
    length = length >> 2;

    while (length--)
        *dst++ = *src++;
}

/*************************************** segment function ****************************************/
static k_blsegs segment(rt_uint8_t* buf, rt_uint32_t buflen, const rt_uint8_t* in, rt_uint32_t inlen, rt_uint32_t blocksize, rt_uint32_t minlen)
{
    k_blsegs ret = { .nsegs = 0 };

    // calculate total number of blocks to be processed
    rt_uint32_t nprocblocks = 0;
    if ((buflen + inlen) >= (minlen + blocksize))
        nprocblocks = (buflen + inlen - minlen) / blocksize;

    // no available block for processing, keep input in the internal buffer.
    if (nprocblocks == 0) {
        ret.seg[ret.nsegs++] = (k_segstr) { RT_FALSE, buf, buflen };
        ret.seg[ret.nsegs++] = (k_segstr) { RT_FALSE, in, inlen };
        return ret;
    }

    const rt_uint8_t* start = in;
    // some blocks are ready for processing,
    // using bytes in the internal buffer first
    if (buflen != 0) {
        // if all data in the internal buffer will be processed
        if (nprocblocks * blocksize >= buflen) {
            // fill buffer if not a complete block
            rt_uint32_t proclen = blocksize;
            nprocblocks--;
            while (proclen < buflen) {
                proclen += blocksize;
                nprocblocks--;
            }
            if (0 == lwp_get_from_user(buf + buflen, start, proclen - buflen))
                rt_memcpy(buf + buflen, start, proclen - buflen);
            ret.seg[ret.nsegs++] = (k_segstr) { RT_TRUE, buf, proclen };
            start += (proclen - buflen);
            inlen -= (proclen - buflen);
        } else { // some data will be remained in the internal buffer
            ret.seg[ret.nsegs++] = (k_segstr) { RT_TRUE, buf, nprocblocks * blocksize };
            ret.seg[ret.nsegs++] = (k_segstr) { RT_FALSE, buf + nprocblocks * blocksize, buflen - nprocblocks * blocksize };
            nprocblocks = 0;
        }
    }
    // deal with input data
    if (nprocblocks > 0) {
        ret.seg[ret.nsegs++] = (k_segstr) { RT_TRUE, start, nprocblocks * blocksize };
    }
    ret.seg[ret.nsegs++] = (k_segstr) { RT_FALSE, start + nprocblocks * blocksize, inlen - nprocblocks * blocksize };

    return ret;
}

static rt_err_t ctx_update(struct hash_context* hash_ctx, k_pufs_dgst_st* md, const rt_uint8_t* msg, rt_uint32_t msglen, rt_bool_t last)
{
    rt_uint32_t val32;
    rt_err_t ret = RT_EOK;

    if (last && (md == RT_NULL))
        return -RT_ERROR;

    if (dma_check_busy_status(RT_NULL))
        return -RT_EBUSY;

    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_data_block_config(hash_ctx->start ? RT_FALSE : RT_TRUE, last, RT_TRUE, RT_TRUE, 0);
    void* in_kvirt;
    in_kvirt = rt_malloc_align(msglen, 64);
    if (!in_kvirt) {
        rt_kprintf("%s malloc fail!\n", __func__);
        return -RT_ERROR;
    }
    dma_write_rwcfg(msg, msglen, in_kvirt);
    dma_write_key_config_0(hash_ctx->keytype, ALGO_TYPE_HMAC, 0, 0);
    if (hash_ctx->start)
        crypto_write_dgst(hash_ctx->state, DGST_INT_STATE_LEN);

    val32 = ((hash_ctx->hash == SHA_256) ? 0x03 : 0x08);
    writel(val32, sec_base_addr + HASH_CONFIG_OFFSET);
    writel(hash_ctx->curlen, sec_base_addr + HASH_PLEN_OFFSET);
    dma_write_start();
    while (dma_check_busy_status(&val32))
        ;
    if (val32 != 0) {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto error;
    }

    val32 = readl(sec_base_addr + HASH_STATUS_OFFSET);
    if (val32 != 0) {
        rt_kprintf("[ERROR] HMAC status: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto error;
    }

    if (!last) {
        crypto_read_dgest(hash_ctx->state, DGST_INT_STATE_LEN);
        hash_ctx->curlen = readl(sec_base_addr + HASH_ALEN_OFFSET);
    }
error:
    rt_free_align(in_kvirt);
    return ret;
}

static rt_err_t ctx_finish(struct hash_context* hash_ctx, k_pufs_dgst_st* md)
{
    if (md == RT_NULL)
        return -RT_EINVAL;

    crypto_read_dgest(md->dgst, DGST_INT_STATE_LEN);
    uint32_t* src = md->dgst;
    for (int i = 0; i < DGST_INT_STATE_LEN / 4; i++)
        *src++ = bswap_32(*src);

    return RT_EOK;
}

static int hash_deinit(void)
{
    if (hash_ctx.busy == RT_FALSE)
        return 0;

    hash_ctx.busy = RT_FALSE;
    kd_hardlock_unlock(HARDLOCK_HASH);
    return 0;
}

static int hash_init(union rt_hash_control_args* ctl)
{
    if (hash_ctx.busy == RT_FALSE) {
        if (0 != kd_hardlock_lock(HARDLOCK_HASH))
            return -EBUSY;
        hash_ctx.busy = RT_TRUE;
    }

    hash_ctx.blocklen = 64;
    hash_ctx.buflen = 0;
    hash_ctx.keybits = 0;
    hash_ctx.minlen = 1;
    hash_ctx.keytype = 0;
    hash_ctx.curlen = 0;
    hash_ctx.op = HMAC_HASH;
    hash_ctx.hash = ctl->init.mode;
    hash_ctx.start = RT_FALSE;

    return 0;
}

static int hash_update(union rt_hash_control_args* ctl)
{
    int ret = 0;

    if (hash_ctx.busy == RT_FALSE)
        return -EPERM;

    if ((ctl->update.msg == RT_NULL) || (ctl->update.msglen == 0))
        return 0;

    k_blsegs segs = segment(hash_ctx.buff, hash_ctx.buflen, ctl->update.msg, ctl->update.msglen, hash_ctx.blocklen, hash_ctx.minlen);
    hash_ctx.buflen = 0;

    for (int i = 0; i < segs.nsegs; i++) {
        if (segs.seg[i].process) {
            ret = ctx_update(&hash_ctx, RT_NULL, segs.seg[i].addr, segs.seg[i].len, RT_FALSE);
            if (ret != RT_EOK) {
                hash_ctx.op = HMAC_AVAILABLE;
                goto error;
            }
            if (hash_ctx.start == RT_FALSE)
                hash_ctx.start = RT_TRUE;
        } else {
            if ((segs.seg[i].addr == hash_ctx.buff) && (hash_ctx.buflen == 0)) {
                hash_ctx.buflen += segs.seg[i].len;
            } else {
                rt_memmove(hash_ctx.buff + hash_ctx.buflen, segs.seg[i].addr, segs.seg[i].len);
                hash_ctx.buflen += segs.seg[i].len;
            }
        }
    }
    return 0;
error:
    hash_deinit();
    return ret;
}

static int hash_final(union rt_hash_control_args* ctl)
{
    int ret;
    k_pufs_dgst_st md;

    if (hash_ctx.busy == RT_FALSE)
        return -EPERM;

    ret = ctx_update(&hash_ctx, &md, hash_ctx.buff, hash_ctx.buflen, RT_TRUE);
    if (ret != RT_EOK)
        goto exit;
    ctx_finish(&hash_ctx, &md);
    lwp_put_to_user(ctl->final.dgst, md.dgst, ctl->final.dlen);
exit:
    hash_ctx.op = HMAC_AVAILABLE;
    hash_deinit();
    return ret;
}

static int hash_fast_double(union rt_hash_control_args* ctl)
{
    int ret = 0;
    void* msg = ctl->fast.msg;
    rt_uint32_t msglen = ctl->fast.msglen;
    void* in_kvirt = NULL;
    rt_uint32_t dgst[32 / 4];
    rt_uint32_t val32;

    if ((msg == RT_NULL) || (msglen == 0))
        return -EINVAL;

    if (hash_ctx.busy == RT_FALSE) {
        if (0 != kd_hardlock_lock(HARDLOCK_HASH))
            return -EBUSY;
        hash_ctx.busy = RT_TRUE;
    }

    hash_ctx.buflen = 0;
    hash_ctx.curlen = 0;
    hash_ctx.op = HMAC_HASH;
    hash_ctx.hash = SHA_256;
    hash_ctx.start = RT_FALSE;

    if (dma_check_busy_status(RT_NULL)) {
        ret = -RT_EBUSY;
        goto exit;
    }
    dma_write_config_0(RT_FALSE, RT_FALSE, RT_FALSE);
    dma_write_key_config_0(0, ALGO_TYPE_HMAC, 0, 0);
    writel(0x03, sec_base_addr + HASH_CONFIG_OFFSET);
    in_kvirt = rt_malloc_align(msglen, 64);
    if (!in_kvirt) {
        rt_kprintf("%s malloc fail!\n", __func__);
        ret = -RT_ENOMEM;
        goto exit;
    }

    dma_write_data_block_config(RT_TRUE, RT_TRUE, RT_TRUE, RT_TRUE, 0);
    dma_write_rwcfg(msg, msglen, in_kvirt);
    writel(0, sec_base_addr + HASH_PLEN_OFFSET);
    dma_write_start();
    while (dma_check_busy_status(&val32))
        ;
    if (val32 != 0) {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto exit;
    }
    val32 = readl(sec_base_addr + HASH_STATUS_OFFSET);
    if (val32 != 0) {
        rt_kprintf("[ERROR] HMAC status: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto exit;
    }
    crypto_read_dgest(dgst, 32);
    for (int i = 0; i < 32 / 4; i++)
        dgst[i] = bswap_32(dgst[i]);

    dma_write_data_block_config(RT_TRUE, RT_TRUE, RT_TRUE, RT_TRUE, 0);
    dma_write_rwcfg(dgst, 32, in_kvirt);
    writel(0, sec_base_addr + HASH_PLEN_OFFSET);
    dma_write_start();
    while (dma_check_busy_status(&val32))
        ;
    if (val32 != 0) {
        rt_kprintf("[ERROR] DMA status 0: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto exit;
    }
    val32 = readl(sec_base_addr + HASH_STATUS_OFFSET);
    if (val32 != 0) {
        rt_kprintf("[ERROR] HMAC status: 0x%08x\n", val32);
        ret = -RT_ERROR;
        goto exit;
    }
    crypto_read_dgest(dgst, 32);
    for (int i = 0; i < 32 / 4; i++)
        dgst[i] = bswap_32(dgst[i]);

    lwp_put_to_user(ctl->fast.dgst, dgst, ctl->fast.dlen);
exit:
    if (in_kvirt)
        rt_free_align(in_kvirt);
    hash_ctx.op = HMAC_AVAILABLE;
    hash_deinit();
    return ret;
}

static rt_err_t hash_control(rt_device_t dev, int cmd, void* args)
{
    int ret;
    union rt_hash_control_args ctl;

    lwp_get_from_user(&ctl, args, sizeof(ctl));

    switch (cmd) {
    case RT_HASH_INIT:
        ret = hash_init(&ctl);
        break;
    case RT_HASH_UPDATE:
        ret = hash_update(&ctl);
        break;
    case RT_HASH_FINAL:
        ret = hash_final(&ctl);
        break;
    case RT_HASH_FAST_DOUBLE:
        ret = hash_fast_double(&ctl);
        break;
    default:
        ret = -EINVAL;
    }
    return ret;
}

static rt_err_t hash_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t hash_close(rt_device_t dev)
{
    return RT_EOK;
}

const static struct rt_device_ops hash_ops = {
    RT_NULL,
    hash_open,
    hash_close,
    RT_NULL,
    RT_NULL,
    hash_control
};

int rt_hw_hash_device_init(void)
{
    if (kd_request_lock(HARDLOCK_HASH)) {
        rt_kprintf("Fail to request hardlock-%d\n", HARDLOCK_HASH);
        return -RT_ERROR;
    }

    sec_base_addr = rt_ioremap((void*)SECURITY_BASE_ADDR, SECURITY_IO_SIZE);
    if (RT_NULL == sec_base_addr) {
        rt_kprintf("Security module ioremap error!\n");
        return -RT_ERROR;
    }

    if (RT_EOK != rt_device_register(&g_hash_device, K230_HASH_NAME, RT_DEVICE_FLAG_RDWR)) {
        rt_kprintf("hwhash device register fail\n");
        return -RT_ERROR;
    }

    g_hash_device.ops = &hash_ops;
    hash_ctx.blocklen = 64;
    hash_ctx.keybits = 0;
    hash_ctx.minlen = 1;
    hash_ctx.keytype = 0;

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_hash_device_init);
