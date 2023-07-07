/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: hcc layer frw task.
 * Author: CompanyNameame
 * Create: 2021-09-28
 */
#include "hcc_host.h"
#include "sdio_host.h"
#include "hcc_list.h"
#include "oam_ext_if.h"
#include "oal_mm.h"
#include "securec.h"
#include "oal_completion.h"
#include "soc_types.h"
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#include <linux/mmc/sdio.h>
#include <linux/scatterlist.h>

/* new fix: */
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include "soc_param.h"

#if (_PRE_KERVER != _PRE_KERVER_4D9)
#include <linux/sched/clock.h>
#endif
#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/time.h>

struct {
    oal_kthread_stru* heartbeat_kthr;
    struct semaphore heart_sema;
    struct hrtimer hrtimer;
    ktime_t kt;
#define HEART_INTVL_NS  (80*1000000)
} g_hrtimer;
#endif

#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_JZ)
#include <mach/jzmmc.h>
#include <soc/gpio.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 宏定义 */
#define FUNC_NUM_SHIFT_BIT          28
#define REGISTER_ADDR_SHIFT_BIT     9

#define SDIO_PROBLE_TIMES           3
#define DELAY_10_US                 10
#define TIMEOUT_MUTIPLE_10          10
#define TIMEOUT_MUTIPLE_5           5
#define HCC_TASK_RX_ISR_NAME        "rx_isr"
#define HCC_TASK_RX_ISR_PRIO        3
#define HCC_TASK_RX_ISR_SIZE        0x400

#define gpio_reg_writel(addr, val)            *((volatile unsigned int *)(addr)) = val;

/* 支持离散内存收发 */
#define _SCATTER_LIST_MEMORY

/* 全局变量定义 */
static oal_completion g_sdio_driver_complete;
static oal_channel_stru *g_channel_sdio_;

static struct sdio_device_id const g_oal_sdio_ids[] = {
    { SDIO_DEVICE(SDIO_VENDOR_ID, SDIO_PRODUCT_ID) },
    {},
};

static td_void oal_sdio_print_state(td_u32 old_state, td_u32 new_state)
{
    if (old_state != new_state) {
        oam_info_log4("sdio state changed, tx[%d=>%d],rx[%d=>%d] (1:on, 0:off)\n",
            (old_state & OAL_SDIO_TX) ? 1 : 0, (new_state & OAL_SDIO_TX) ? 1 : 0,
            (old_state & OAL_SDIO_RX) ? 1 : 0, (new_state & OAL_SDIO_RX) ? 1 : 0);
    }
}

td_void oal_free_sdio_stru(oal_channel_stru *pst_sdio)
{
    aich_unref_param(pst_sdio);
    oam_error_log0("oal_free_sdio_stru\n");
}

oal_channel_stru *oal_get_sdio_default_handler(td_void)
{
    return g_channel_sdio_;
}

oal_channel_stru *oal_sdio_alloc(struct sdio_func *func)
{
    oal_channel_stru* pst_sdio = TD_NULL;
    if (func == TD_NULL) {
        oam_error_log0("oal_sdio_alloc: func null!\n");
        return TD_NULL;
    }

    pst_sdio = oal_get_sdio_default_handler();
    if (pst_sdio == TD_NULL) {
        oam_error_log0("Failed to alloc pst_sdio!\n");
        return TD_NULL;
    }

    pst_sdio->func           = func;
    sdio_set_drvdata(func, pst_sdio);

    return pst_sdio;
}

td_void oal_disable_sdio_state(oal_channel_stru *pst_sdio, td_u32 mask)
{
    td_u32 old_state;
    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        oam_error_log0("oal_enable_sdio_state: pst_sdio null!\n");
        return;
    }

    oal_sdio_claim_host(pst_sdio->func);
    old_state = pst_sdio->state;
    pst_sdio->state &= ~mask;
    oal_sdio_print_state(old_state, pst_sdio->state);
    oal_sdio_release_host(pst_sdio->func);
}

static td_s32 oal_sdio_sleep_dev_internal(oal_channel_stru *pst_sdio)
{
    int    ret;
    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        return -OAL_EFAIL;
    }

    oal_sdio_claim_host(pst_sdio->func);
    sdio_f0_writeb(pst_sdio->func, ALLOW_TO_SLEEP_VALUE, SDIO_WAKEUP_DEV_REG, &ret);
    oal_sdio_release_host(pst_sdio->func);
    return ret;
}

td_s32 oal_sdio_send_msg(oal_channel_stru *pst_sdio, unsigned long val)
{
    td_s32       ret  = EXT_SUCCESS;
    struct sdio_func *func = TD_NULL;
    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        oam_error_log0("{oal_sdio_send_msg::sdio is not initialized,can't send sdio msg!}");
        return -OAL_EINVAL;
    }

    func = pst_sdio->func;

    if (pst_sdio->bus_ops &&
        pst_sdio->bus_ops->pm_wakeup_dev) {
        if (pst_sdio->bus_ops->pm_wakeup_dev(pst_sdio->bus_data) != EXT_SUCCESS) {
            oam_error_log0("{oal_sdio_send_msg::host wakeup device failed}");
            return -OAL_EBUSY;
        }
    }

    oal_sdio_claim_host(func);
    oal_sdio_writel(func, (1 << val), SDIO_REG_FUNC1_WRITE_MSG, &ret);
    if (ret) {
        oam_error_log2("{oal_sdio_send_msg::failed to send sdio msg[%lu]!ret=%d}", val, ret);
    }
    oal_sdio_release_host(func);
    return ret;
}

td_s32 oal_sdio_get_state(const oal_channel_stru *pst_sdio, td_u32 mask)
{
    if (pst_sdio == TD_NULL) {
        return TD_FALSE;
    }

    if ((pst_sdio->state & mask) == mask) {
        return TD_TRUE;
    } else {
        return TD_FALSE;
    }
}

td_void oal_enable_sdio_state(oal_channel_stru *pst_sdio, td_u32 mask)
{
    td_u32 old_state;
    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        oam_error_log0("oal_enable_sdio_state: pst_sdio null!\n");
        return;
    }

    oal_sdio_claim_host(pst_sdio->func);
    old_state = pst_sdio->state;
    pst_sdio->state |= mask;
    oal_sdio_print_state(old_state, pst_sdio->state);
    oal_sdio_release_host(pst_sdio->func);
}

td_void oal_sdio_dev_shutdown(struct device *dev)
{
    td_s32   ret;
    aich_unref_param(dev);
    oal_channel_stru *pst_sdio = oal_get_sdio_default_handler();
    if ((pst_sdio == TD_NULL) || (pst_sdio->func == NULL)) {
        return;
    }

    if (oal_sdio_sleep_dev_internal(pst_sdio) != EXT_SUCCESS) {
        return;
    }

    if (oal_sdio_send_msg(pst_sdio, H2D_MSG_PM_WLAN_OFF) != EXT_SUCCESS) {
        return;
    }

    if (TD_TRUE != oal_sdio_get_state(pst_sdio, OAL_SDIO_ALL)) {
        return;
    }

    oal_sdio_claim_host(pst_sdio->func);
    ret = sdio_disable_func(pst_sdio->func);
    oal_sdio_release_host(pst_sdio->func);
    if (ret) {
        oam_error_log0("sdio_disable_func fail!\n");
    }
}

static td_s32 oal_sdio_suspend(struct device *dev)
{
    struct sdio_func *func = TD_NULL;
    oal_channel_stru *pst_sdio = TD_NULL;

    oam_error_log0("+++++++sdio suspend+++++++++++++\n");
    if (dev == TD_NULL) {
         oam_error_log0("[WARN]dev is null\n");
        return EXT_SUCCESS;
    }
    func = dev_to_sdio_func(dev);
    pst_sdio = sdio_get_drvdata(func);
    if (pst_sdio == TD_NULL) {
         oam_error_log0("pst_sdio is null\n");
        return EXT_SUCCESS;
    }
    return EXT_SUCCESS;
}

static td_s32 oal_sdio_resume(struct device *dev)
{
    struct sdio_func *func = TD_NULL;
    oal_channel_stru *pst_sdio = TD_NULL;

    oam_error_log0("+++++++sdio resume+++++++++++++\n");
    if (dev == TD_NULL) {
        oam_error_log0("[WARN]dev is null\n");
        return EXT_SUCCESS;
    }
    func = dev_to_sdio_func(dev);
    pst_sdio = sdio_get_drvdata(func);
    if (pst_sdio == TD_NULL) {
        oam_error_log0("pst_sdio is null\n");
        return EXT_SUCCESS;
    }
    return EXT_SUCCESS;
}

td_s32 oal_sdio_transfer_rx_reserved_buff(const oal_channel_stru *pst_sdio)
{
    td_s32 ret;
    if (pst_sdio->sdio_extend == TD_NULL) {
        oam_error_log0("{pst_sdio->sdio_extend NULL!}");
        return -OAL_EINVAL;
    }

    td_u32 ul_extend_len = pst_sdio->sdio_extend->xfer_count;

    if (ul_extend_len == 0) {
        oam_error_log0("{extend_len is zero!}");
        return -OAL_EINVAL;
    }

    if (ul_extend_len > SDIO_MAX_XFER_LEN) {
        return -OAL_EINVAL;
    }

    ret = oal_sdio_readsb(pst_sdio->func, 0, pst_sdio->rx_buf, ul_extend_len);

    return ret;
}

td_void oal_copy_data_to_uncbuf(const oal_channel_stru *pst_sdio,
    const hcc_data_queue *head,
    td_u8* buf,
    td_u32 len)
{
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_unc_struc *tmp = TD_NULL;
    td_u32 offset = 0;
    td_s32 index = 0;
    osal_list_for_each_entry_safe(unc_buf, tmp, (&head->data_queue), list) {
        memcpy_s(oal_unc_data(unc_buf), oal_unc_len(unc_buf), buf + offset, oal_unc_len(unc_buf));
        offset += (pst_sdio->sdio_extend->comm_reg[index] << EXT_SDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS);
        len -= (pst_sdio->sdio_extend->comm_reg[index] << EXT_SDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS);
        if (len  <= 0) {
            break;
        }
        index++;
    }
}

td_s32 oal_sdio_tranfer_commit_memory(const oal_channel_stru *pst_sdio,
    const hcc_data_queue *head,
    td_s32 rw)
{
    td_s32 ret;
    td_u32 queue_len;
    td_u32 sum_len = 0;
    td_u32 idx = 0;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_unc_struc *tmp = TD_NULL;

    queue_len = hcc_list_len(head);

    osal_list_for_each_entry_safe(unc_buf, tmp, (&head->data_queue), list) {
        if (!oal_is_aligned((uintptr_t)oal_unc_data(unc_buf), 4)) { /* 4: 4字节对齐 */
            oam_error_log3("oal_sdio_transfer_netbuf_list netbuf 4 aligned fail!:: unc_buf[%p], len[%d], rw[%d]",
                oal_unc_data(unc_buf), oal_unc_len(unc_buf), rw);
            return -OAL_EINVAL;
        }

        if (WARN_ON(!oal_is_aligned(oal_unc_len(unc_buf), BUS_H2D_SCATT_BUFFLEN_ALIGN))) {
            return -OAL_EINVAL;
        }

        if (rw == SDIO_WRITE) {
            memcpy_s(pst_sdio->tx_buf + sum_len, SDIO_MAX_XFER_LEN, oal_unc_data(unc_buf), oal_unc_len(unc_buf));
        }

        sum_len += oal_unc_len(unc_buf);
        idx++;
    }

    if (oal_unlikely(idx > queue_len)) {
        return -OAL_EINVAL;
    }

    if (WARN_ON(sum_len > SDIO_MAX_XFER_LEN)) {
        return -OAL_EINVAL;
    }

    if (sum_len < EXT_SDIO_BLOCK_SIZE) {
        sum_len = aich_byte_align(sum_len, 4); /* 4: 4字节对齐 */
    } else {
        sum_len = aich_byte_align(sum_len, EXT_SDIO_BLOCK_SIZE);
    }

    oal_sdio_claim_host(pst_sdio->func);
    if (rw == SDIO_WRITE) {
        ret = oal_sdio_writesb(pst_sdio->func, 0, pst_sdio->tx_buf, sum_len);
    } else {
        ret = oal_sdio_readsb(pst_sdio->func, 0, pst_sdio->rx_buf, sum_len);
    }
    oal_sdio_release_host(pst_sdio->func);

    /* 将从device获取的数据拷贝到netbuf */
    if (rw == SDIO_READ) {
        oal_copy_data_to_uncbuf(pst_sdio, head, pst_sdio->rx_buf, sum_len);
    }
    return ret;
}

td_void check_sg_format(struct scatterlist *sg, td_u32 sg_len)
{
    td_u32 i = 0;
    struct scatterlist *sg_t = TD_NULL;
    for_each_sg(sg, sg_t, sg_len, i) {
        if (oal_unlikely(TD_NULL == sg_t)) {
            return;
        }
        if (oal_warn_on(((uintptr_t)sg_virt(sg_t) & 0x03) || (sg_t->length & 0x03))) {
            oam_error_log3("check_sg_format:[i:%d][addr:%p][len:%u]\n", i, sg_virt(sg_t), sg_t->length);
        }
    }
}

td_s32 oal_mmc_io_rw_scat_extended(const oal_channel_stru *pst_sdio,
    td_s32 write, td_u32 fn, td_u32 addr,
    td_s32 incr_addr, struct scatterlist *sg,
    td_u32 sg_len, td_u32 blocks, td_u32 blksz)
{
    struct mmc_request mrq = {0};
    struct mmc_command cmd = {0};
    struct mmc_data data = {0};

    if ((blksz == 0) || (addr & ~0x1FFFF)) {
        return -EINVAL;
    }

    struct mmc_card *card = pst_sdio->func->card;
    check_sg_format(sg, sg_len);

    mrq.cmd = &cmd;
    mrq.data = &data;

    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << FUNC_NUM_SHIFT_BIT;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << REGISTER_ADDR_SHIFT_BIT;
    if (blocks == 1 && blksz <= EXT_SDIO_BLOCK_SIZE) {
        cmd.arg |= (blksz == EXT_SDIO_BLOCK_SIZE) ? 0 : blksz;
    } else {
        cmd.arg |= 0x08000000 | blocks;
    }
    cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;

    data.blksz = blksz;
    data.blocks = blocks;
    data.flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
    data.sg = sg;
    data.sg_len = sg_len;

    mmc_set_data_timeout(&data, card);
    mmc_wait_for_req(card->host, &mrq);

    if (cmd.error) {
        return cmd.error;
    }
    if (data.error) {
        return data.error;
    }

    if (oal_warn_on(mmc_host_is_spi(card->host))) {
        oam_error_log0("WiFi driver do not support spi sg transfer!\n");
        return -EIO;
    }

    if (cmd.resp[0] & R5_ERROR) {
        return -EIO;
    }

    if (cmd.resp[0] & R5_FUNCTION_NUMBER) {
        return -EINVAL;
    }

    if (cmd.resp[0] & R5_OUT_OF_RANGE) {
        return -ERANGE;
    }

    return 0;
}

static td_s32 _oal_sdio_transfer_scatt(const oal_channel_stru *pst_sdio,
    td_s32 rw, td_u32 addr, struct scatterlist *sg,
    td_u32 sg_len, td_u32 rw_sz)
{
    td_s32 ret;
    td_s32 write = (rw == SDIO_READ) ? 0 : 1;
    struct sdio_func *func = pst_sdio->func;
    oal_sdio_claim_host(func);

    if (oal_unlikely(oal_sdio_get_state(pst_sdio, OAL_SDIO_ALL) != TD_TRUE)) {
        oal_sdio_release_host(func);
        return -OAL_EFAIL;
    }
    ret = oal_mmc_io_rw_scat_extended(pst_sdio, write, sdio_func_num(pst_sdio->func), addr, 0, sg, sg_len,
        (rw_sz / EXT_SDIO_BLOCK_SIZE) ? : 1, min(rw_sz, (td_u32)EXT_SDIO_BLOCK_SIZE));
    if (oal_unlikely(ret)) {
        if (write) {
            oam_error_log1("{oal_sdio_transfer_scatt::write failed=%d}", ret);
        } else {
            oam_error_log1("{oal_sdio_transfer_scatt::read failed=%d}", ret);
        }
    }
    oal_sdio_release_host(func);

    return ret;
}

td_s32 oal_sdio_transfer_scatt(const oal_channel_stru *pst_sdio, td_s32 rw, td_u32 addr, struct scatterlist *sg,
    td_u32 sg_len, td_u32 sg_max_len, td_u32 rw_sz)
{
    td_u32 align_len;
    td_u32 align_t;

    if ((pst_sdio == TD_NULL) || (rw_sz == 0) || (sg_max_len < sg_len)) {
        return -OAL_EINVAL;
    }

    if ((!pst_sdio) || (!rw_sz) || (sg_max_len < sg_len) || (sg == TD_NULL)) {
        oam_error_log3("oal_sdio_transfer_scatt: pst_sdio:%p,/rw_sz:%d,/sg_max_len<sg_len?:%d,/sg null}",
            (uintptr_t)pst_sdio, rw_sz, sg_max_len < sg_len);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(!sg_len)) {
        oam_error_log2("Sdio %d(1:read,2:write) Scatter list num should never be zero, total request len: %u}",
            rw == SDIO_READ ? 1 : 2, rw_sz); /* 1:read,2:write */
        return -OAL_EINVAL;
    }

    align_t = sdio_align_4_or_blk(rw_sz);
    align_len = align_t - rw_sz;

    if (oal_likely(align_len)) {
        if (oal_unlikely(sg_len + 1 > sg_max_len)) {
            oam_error_log2("{sg list over,sg_len:%u, sg_max_len:%u\n}", sg_len, sg_max_len);
            return -OAL_ENOMEM;
        }
        sg_set_buf(&sg[sg_len], pst_sdio->sdio_align_buff, align_len);
        sg_len++;
    }

    sg_mark_end(&sg[sg_len - 1]);
    rw_sz = align_t;

    oal_warn_on((rw_sz >= EXT_SDIO_BLOCK_SIZE) && (rw_sz & (EXT_SDIO_BLOCK_SIZE - 1)));
    oal_warn_on((rw_sz < EXT_SDIO_BLOCK_SIZE)  && (rw_sz & (4 - 1))); /* 4: 4字节对齐 */

    if (oal_warn_on(align_len & 0x3)) {
        oam_warning_log1("{not 4 bytes align:%u\n}", align_len);
    }

    return _oal_sdio_transfer_scatt(pst_sdio, rw, addr, sg, sg_len, rw_sz);
}

td_s32 oal_sdio_tranfer_sglist_memory(const oal_channel_stru *pst_sdio,
    const hcc_data_queue *head,
    td_s32 rw)
{
    td_s32 ret;
    td_u32 queue_len;
    td_u32 sum_len = 0;
    td_u32 idx = 0;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_unc_struc *tmp = TD_NULL;
    td_u32 request_sg_len;
    struct scatterlist *sg = TD_NULL;
    struct sg_table sgtable = {0};
    td_u8 sg_realloc = 0;

    queue_len = hcc_list_len(head);

    request_sg_len = queue_len + 1;
    if (oal_unlikely(request_sg_len > pst_sdio->scatt_info[rw].max_scatt_num)) {
        if (sg_alloc_table(&sgtable, request_sg_len, GFP_KERNEL)) {
            return -OAL_ENOMEM;
        }
        sg_realloc = 1;
        sg = sgtable.sgl;
    } else {
        sg = pst_sdio->scatt_info[rw].sglist;
    }

    memset_s(sg, sizeof(struct scatterlist) * request_sg_len, 0, sizeof(struct scatterlist) * request_sg_len);

    osal_list_for_each_entry_safe(unc_buf, tmp, (&head->data_queue), list) {
        if (!oal_is_aligned((uintptr_t)oal_unc_data(unc_buf), 4)) { /* 4: 4字节对齐 */
            return -OAL_EINVAL;
        }

        if (!oal_is_aligned(oal_unc_len(unc_buf), BUS_H2D_SCATT_BUFFLEN_ALIGN)) {
            return -OAL_EINVAL;
        }

        sg_set_buf(&sg[idx], oal_unc_data(unc_buf), oal_unc_len(unc_buf));
        sum_len += oal_unc_len(unc_buf);
        idx++;
    }

    if (oal_unlikely(idx > queue_len)) {
        return -OAL_EINVAL;
    }

    ret = oal_sdio_transfer_scatt(pst_sdio, rw, SDIO_REG_FUNC1_FIFO, sg, idx, request_sg_len, sum_len);
    if (sg_realloc) {
        sg_free_table(&sgtable);
    }
    return ret;
}

td_s32 oal_sdio_transfer_list(const oal_channel_stru *pst_sdio,
    const hcc_data_queue *head,
    td_s32 rw)
{
    td_s32 ret;

    if ((!pst_sdio) || (!head)) {
        oam_error_log0("pst_sdio head null\n");
        return -OAL_EINVAL;
    }

    if (WARN_ON(rw >= SDIO_OPT_BUTT)) {
        return -OAL_EINVAL;
    }

    if (WARN_ON(hcc_is_list_empty(head))) {
        return -OAL_EINVAL;
    }

    if (rw == SDIO_WRITE) {
        if (pst_sdio->bus_ops &&
            pst_sdio->bus_ops->pm_wakeup_dev) {
            if (pst_sdio->bus_ops->pm_wakeup_dev(pst_sdio->bus_data) != EXT_SUCCESS) {
                oam_error_log0("{oal_sdio_transfer_netbuf_list::host wakeup device failed}");
                return -OAL_EBUSY;
            }
        }
    }

#ifdef _SCATTER_LIST_MEMORY
    ret = oal_sdio_tranfer_sglist_memory(pst_sdio, head, rw);
#else
    ret = oal_sdio_tranfer_commit_memory(pst_sdio, head, rw);
#endif
    return ret;
}

td_s32 oal_sdio_build_rx_list(oal_channel_stru *pst_sdio, hcc_data_queue *head)
{
    td_s32 i;
    td_u8  buff_len;
    td_u16 buff_len_t;
    td_s32 ret = EXT_SUCCESS;
    td_u32 sum_len = 0;
    hcc_unc_struc *unc_buf = TD_NULL;

    if (!hcc_is_list_empty(head)) {
        oam_error_log0("oal_sdio_build_rx_list: oal netbuf list empty");
        return -OAL_EINVAL;
    }

    if ((pst_sdio == TD_NULL) || (pst_sdio->bus_ops == TD_NULL) ||
        (pst_sdio->bus_ops->alloc_unc_buf == TD_NULL)) {
        oam_error_log0("oal_sdio_build_rx_list: sdio initialization incomplete");
        return -OAL_EINVAL;
    }

    for (i = 0; i < SDIO_EXTEND_REG_COUNT; i++) {
        buff_len = pst_sdio->sdio_extend->comm_reg[i];
        if (buff_len == 0) {
            break;
        }

        buff_len_t = buff_len << EXT_SDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS;

        unc_buf = pst_sdio->bus_ops->alloc_unc_buf(buff_len_t, NETBUF_STRU_TYPE);
        if (unc_buf ==  TD_NULL) {
            oam_error_log0("oal_sdio_build_rx_list: alloc_unc_buf fail");
            goto failed_unc_buf_alloc;
        }
        sum_len += buff_len_t;

        hcc_list_add_tail(head, unc_buf);
    }

    if (oal_warn_on(sdio_align_4_or_blk(sum_len) != pst_sdio->sdio_extend->xfer_count)) {
        oam_warning_log3("{[WIFI][E]scatt total len[0x%x] should = xfercount[0x%x],after pad len:0x%x}",
            sum_len, pst_sdio->sdio_extend->xfer_count, sdio_align_4_or_blk(sum_len));
        goto failed_unc_buf_alloc;
    }

    if (oal_unlikely(hcc_is_list_empty(head))) {
        return -OAL_EINVAL;
    }

    return ret;
failed_unc_buf_alloc:
    hcc_list_purge(head);
    ret = oal_sdio_transfer_rx_reserved_buff(pst_sdio);
    if (ret != EXT_SUCCESS) {
        oam_error_log0("oal_sdio_transfer_rx_reserved_buff fail\n");
    }
    return -OAL_ENOMEM;
}

td_s32 oal_sdio_dev_init(oal_channel_stru *pst_sdio)
{
    struct sdio_func   *func = TD_NULL;
    td_s32               ret;

    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        return -OAL_EFAIL;
    }

    func = pst_sdio->func;
    oal_sdio_claim_host(pst_sdio->func);
     /* 超时时间为1000ms */
    sdio_en_timeout(func)  = 1000;

    ret = sdio_enable_func(func);
    if (ret < 0) {
        oam_error_log1("failed to enable sdio function! ret=%d\n", ret);
        goto failed_enabe_func;
    }

    ret = sdio_set_block_size(func, EXT_SDIO_BLOCK_SIZE);
    if (ret) {
        oam_error_log1("failed to set sdio blk size! ret=%d\n", ret);
        goto failed_set_block_size;
    }

    oal_sdio_writeb(func, SDIO_FUNC1_INT_MASK, SDIO_REG_FUNC1_INT_STATUS, &ret);
    if (ret) {
        oam_error_log1("failed to clear sdio interrupt! ret=%d\n", ret);
        goto failed_clear_func1_int;
    }

    oal_sdio_writeb(func, SDIO_FUNC1_INT_MASK, SDIO_REG_FUNC1_INT_ENABLE, &ret);
    if (ret < 0) {
        oam_error_log1("failed to enable sdio interrupt! ret=%d\n", ret);
        goto failed_enable_func1;
    }

    oal_enable_sdio_state(pst_sdio, OAL_SDIO_ALL);
    oal_sdio_release_host(pst_sdio->func);

    return EXT_SUCCESS;
failed_enable_func1:
failed_clear_func1_int:
failed_set_block_size:
    sdio_disable_func(func);
failed_enabe_func:
    oal_sdio_release_host(pst_sdio->func);
    return ret;
}

static td_void oal_sdio_dev_deinit(oal_channel_stru *pst_sdio)
{
    struct sdio_func   *func = TD_NULL;
    td_s32           ret = 0;
    func  = pst_sdio->func;
    sdio_claim_host(func);
    oal_sdio_writeb(func, 0, SDIO_REG_FUNC1_INT_ENABLE, &ret);
    sdio_disable_func(func);
    oal_disable_sdio_state(pst_sdio, OAL_SDIO_ALL);
    sdio_release_host(func);
}

/* new fix: implicitly call the kernel plat_sdio_rescan */
typedef td_s32 (*sdio_rescan_fn)(td_s32 slot);

__attribute__((unused)) static unsigned long kallsyms_lookup_kfunc(const char *name)
{
    struct kprobe kp;
    unsigned long addr;

    if (!name)
    {
        oam_error_log0("param name is invalid\n");
        return 0;
    }
    
    memset(&kp, 0, sizeof(struct kprobe));
    kp.symbol_name = name;
    int ret = register_kprobe(&kp);
    if (ret < 0) {
        oam_error_log1("register_kprobe failed, ret = %d\n", ret);
        return 0;
    }
    addr = (unsigned long)kp.addr;
    unregister_kprobe(&kp);

    return addr;
}

td_void sdio_card_detect_change(td_s32 sdio_dev_num)
{
#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_SOC)
    /* 如下为host平台函数，3518对应如下函数 */
    hisi_sdio_rescan(sdio_dev_num);
#elif (_PRE_OS_PLATFORM == _PRE_PLATFORM_JZ)
    jzmmc_manual_detect(sdio_dev_num, 1);
#elif (_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)
    plat_sdio_rescan(sdio_dev_num);
    /*
    sdio_rescan_fn plat_sdio_rescan = kallsyms_lookup_kfunc("plat_sdio_rescan");
    if (plat_sdio_rescan)
        plat_sdio_rescan(sdio_dev_num);
    else
        oam_error_log0("kallsyms_lookup_name plat_sdio_rescan failed\n");
    */
#endif
}

td_s32 oal_sdio_detectcard_to_core(td_s32 sdio_dev_num)
{
    sdio_card_detect_change(sdio_dev_num);
    return EXT_SUCCESS;
}
static td_void oal_sdio_remove(struct sdio_func *func)
{
    oal_channel_stru *pst_sdio = TD_NULL;
    if (func == TD_NULL) {
        oam_error_log0("[Error]oal_sdio_remove: Invalid NULL func!\n");
        return;
    }

    pst_sdio = (oal_channel_stru *)sdio_get_drvdata(func);
    if (pst_sdio == TD_NULL) {
        oam_error_log0("[Error]Invalid NULL pst_sdio!\n");
        return;
    }

    oal_sdio_dev_deinit(pst_sdio);
    sdio_set_drvdata(func, NULL);
    oam_error_log0("wifi connectivity sdio driver has been removed.");
}

static td_s32 oal_sdio_extend_buf_get(const oal_channel_stru *pst_sdio)
{
    td_s32 ret;

    ret = oal_sdio_memcpy_fromio(pst_sdio->func, (td_void *)pst_sdio->sdio_extend,
        SDIO_EXTEND_BASE_ADDR, sizeof(sdio_extend_func));
    if (ret != EXT_SUCCESS) {
        oam_info_log0("{[SDIO][Err]sdio read extend_buf fail!}");
    }
    return ret;
}

static td_s32 oal_sdio_get_func1_int_status(const oal_channel_stru *pst_sdio, td_u8 *int_stat)
{
    pst_sdio->sdio_extend->int_stat &= pst_sdio->func1_int_mask;
    *int_stat = (pst_sdio->sdio_extend->int_stat & 0xF);
    return EXT_SUCCESS;
}

static td_s32 oal_sdio_msg_stat(const oal_channel_stru *pst_sdio, td_u32 *msg)
{
    td_s32 ret = 0;

    *msg = oal_sdio_readl(pst_sdio->func, SDIO_REG_FUNC1_MSG_FROM_DEV, &ret);

    if (ret) {
        oam_info_log1("sdio readb error![ret=%d]\n", ret);
        return ret;
    }
    pst_sdio->sdio_extend->msg_stat = *msg;

    return EXT_SUCCESS;
}

td_s32 oal_sdio_msg_irq(oal_channel_stru *pst_sdio)
{
    td_u32 bit;
    struct sdio_func    *func;
    td_u32               msg = 0;
    td_s32               ret;
    unsigned long        msg_tmp;

    func       = pst_sdio->func;
    /* reading interrupt form ARM Gerneral Purpose Register(0x28)  */
    ret = oal_sdio_msg_stat(pst_sdio, &msg);
    if (ret) {
        oam_info_log1("[SDIO][Err]oal_sdio_msg_stat error![ret=%d]\n", ret);
        return ret;
    }
    msg_tmp = (unsigned long)msg;

    if (!msg) {
        return EXT_SUCCESS;
    }
    if (test_bit(D2H_MSG_DEVICE_PANIC, &msg_tmp)) {
        oal_disable_sdio_state(pst_sdio, OAL_SDIO_ALL);
    }
    oal_sdio_release_host(pst_sdio->func);
    if (test_and_clear_bit(D2H_MSG_DEVICE_PANIC, &msg_tmp)) {
        bit = D2H_MSG_DEVICE_PANIC;
        pst_sdio->msg[bit].count++;
        pst_sdio->msg[bit].cpu_time = cpu_clock(UINT_MAX);
        if (pst_sdio->msg[bit].msg_rx) {
            oam_info_log1("device panic msg come, 0x%8x\n", msg);
            pst_sdio->msg[bit].msg_rx(pst_sdio->msg[bit].data);
        }
    }
    bit = 0;
    for_each_set_bit(bit, (const unsigned long *)&msg_tmp, D2H_MSG_COUNT) {
        if (bit >= D2H_MSG_COUNT) {
            oam_info_log0("oal_sdio_msg_irq, bit >= D2H_MSG_COUNT\n");
            return -OAL_EFAIL;
        }

/* new fix: */
//#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)
#if (_PRE_OS_PLATFORM == 100)
        if (bit == D2H_MSG_HEARTBEAT)
            continue;
#endif
        pst_sdio->msg[bit].count++;
        pst_sdio->msg[bit].cpu_time = cpu_clock(UINT_MAX);
        if (pst_sdio->msg[bit].msg_rx) {
            pst_sdio->msg[bit].msg_rx(pst_sdio->msg[bit].data);
        }
    }
    oal_sdio_claim_host(pst_sdio->func);

    return EXT_SUCCESS;
}

td_s32 oal_sdio_get_credit(const oal_channel_stru *pst_sdio, td_u32 *uc_hipriority_cnt)
{
    td_s32 ret;
    sdio_claim_host(pst_sdio->func);
    ret = oal_sdio_memcpy_fromio(pst_sdio->func, (td_u8 *)uc_hipriority_cnt,
                                 SDIO_EXTEND_CREDIT_ADDR, sizeof(*uc_hipriority_cnt));
    sdio_release_host(pst_sdio->func);
    /* 此处要让出CPU */
    schedule();
    return ret;
}

__attribute__((unused)) static td_s32 oal_sdio_xfercount_get(const oal_channel_stru *pst_sdio, td_u32 *xfercount)
{
    td_s32 ret = 0;
    /* read from 0x0c */
    *xfercount = oal_sdio_readl(pst_sdio->func, SDIO_REG_FUNC1_XFER_COUNT, &ret);
    if (oal_unlikely(ret)) {
        oam_error_log1("[E]sdio read xercount failed ret=%d\n", ret);
        return ret;
    }
    pst_sdio->sdio_extend->xfer_count = *xfercount;
    return EXT_SUCCESS;
}

td_s32 oal_sdio_transfer_register(oal_channel_stru *pst_sdio, struct hcc_bus_ops *bus_ops)
{
    if (pst_sdio == TD_NULL) {
        return -OAL_EINVAL;
    }
    pst_sdio->bus_ops  = bus_ops;
    return EXT_SUCCESS;
}

td_void oal_sdio_transfer_unregister(oal_channel_stru *pst_sdio)
{
    pst_sdio->bus_ops = TD_NULL;
}

td_s32 oal_sdio_message_register(oal_channel_stru *pst_sdio, td_u8 msg, sdio_msg_rx cb, td_void *data)
{
    if (pst_sdio == TD_NULL || msg >= D2H_MSG_COUNT) {
        return -OAL_EFAIL;
    }
    pst_sdio->msg[msg].msg_rx = cb;
    pst_sdio->msg[msg].data = data;
    return EXT_SUCCESS;
}

td_void oal_sdio_message_unregister(oal_channel_stru *pst_sdio, td_u8 msg)
{
    if (pst_sdio == TD_NULL || msg >= D2H_MSG_COUNT) {
        return;
    }
    pst_sdio->msg[msg].msg_rx = TD_NULL;
    pst_sdio->msg[msg].data = TD_NULL;
}

td_s32 oal_sdio_data_sg_irq(oal_channel_stru *pst_sdio)
{
    struct sdio_func   *func = TD_NULL;

    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL || pst_sdio->bus_data == TD_NULL) {
        return -OAL_EINVAL;
    }

    func = pst_sdio->func;
    /* new fix: */
#if 0
    td_s32 ret;
    td_u32              xfer_count;

    ret = oal_sdio_xfercount_get(pst_sdio, &xfer_count);
    if (oal_unlikely(ret)) {
        return -OAL_EFAIL;
    }
#endif
    /* beacuse get buf may cost lot of time, so release bus first */
    if (pst_sdio->bus_ops == TD_NULL ||
        pst_sdio->bus_ops->rx == TD_NULL) {
        return -OAL_EINVAL;
    }

    oal_sdio_release_host(func);
    pst_sdio->bus_ops->rx(pst_sdio->bus_data);
    oal_sdio_claim_host(func);

    return EXT_SUCCESS;
}

td_s32 oal_sdio_do_isr(oal_channel_stru *pst_sdio)
{
    td_u8                   int_mask;
    td_s32                  ret;
    /*
    struct timeval start_time, end_time;
    long cost_time = 0;

    printk("enter isr, ");
    do_gettimeofday(&start_time);
    */
    if (oal_unlikely(TD_TRUE != oal_sdio_get_state(pst_sdio, OAL_SDIO_RX))) {
        oam_error_log0("oal_sdio_do_isr rx disable");
        return EXT_SUCCESS;
    }

    ret = oal_sdio_extend_buf_get(pst_sdio);
    if (oal_unlikely(ret)) {
        return -OAL_EFAIL;
    }

    ret = oal_sdio_get_func1_int_status(pst_sdio, &int_mask);
    if (oal_unlikely(ret)) {
        return ret;
    }

    if (oal_unlikely((int_mask & SDIO_FUNC1_INT_MASK) == 0)) {
        oam_info_log0("no sdio isr");
        return EXT_SUCCESS;
    }

    /* message interrupt, flow control */
    if (int_mask & SDIO_FUNC1_INT_MFARM) {
        if (oal_sdio_msg_irq(pst_sdio) != EXT_SUCCESS) {
            oam_error_log0("oal_sdio_msg_irq failed");
            return -OAL_EFAIL;
        }
    }

    if (int_mask & SDIO_FUNC1_INT_DREADY) {
        /* new fix: */
        //return oal_sdio_data_sg_irq(pst_sdio);
        ret = oal_sdio_data_sg_irq(pst_sdio);
        /*
        do_gettimeofday(&end_time);
        cost_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
        printk("cost time: %ld us\n", cost_time);
        */
        return ret;
    }
    /*
    do_gettimeofday(&end_time);
    cost_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
    printk("cost time: %ld us\n", cost_time);
    */
    return EXT_SUCCESS;
}

td_void oal_sdio_isr(struct sdio_func *func)
{
    oal_channel_stru     *pst_sdio = TD_NULL;
    td_s32                     ret;
    if (func == TD_NULL) {
        oam_error_log0("oal_sdio_isr func null\n");
        return;
    }

    pst_sdio = sdio_get_drvdata(func);
    if (pst_sdio == TD_NULL || pst_sdio->func == TD_NULL) {
        oam_error_log1("pst_sdio/pst_sdio->func is NULL :%p\n", (uintptr_t)pst_sdio);
        return;
    }

    oal_sdio_claim_host(pst_sdio->func);
    ret = oal_sdio_do_isr(pst_sdio);
    if (oal_unlikely(ret)) {
        oam_error_log0("oal_sdio_do_isr fail\n");
    }
    oal_sdio_release_host(pst_sdio->func);
}

static td_s32 hcc_task_rx_gpio_thread(td_void *data)
{
    oal_channel_stru *pst_sdio;
    pst_sdio = (oal_channel_stru *)data;
    oam_warning_log0("hcc_task_rx_gpio_thread start");
    allow_signal(SIGTERM);
    while (!down_interruptible(&pst_sdio->rx_sema)) {
        if (kthread_should_stop()) {
             oam_error_log0("exit gpio rx thread");
            break;
        }
        /* start to read GPIO interrupt */
        oal_sdio_isr(pst_sdio->func);
    }
    oam_info_log0("hcc_task_rx_gpio_thread is terminated");
    return EXT_SUCCESS;
}

static td_s32 sdio_start_gpio_thread(oal_channel_stru *pst_sdio)
{
    oal_kthread_param_stru thread_param = {0};
    memset_s(&thread_param, sizeof(oal_kthread_param_stru), 0, sizeof(oal_kthread_param_stru));

    thread_param.l_cpuid = 0;
    thread_param.l_policy = OAL_SCHED_FIFO;
    thread_param.l_prio = HCC_TASK_RX_ISR_PRIO;
    thread_param.ul_stacksize = HCC_TASK_RX_ISR_SIZE;

    pst_sdio->rx_handle = oal_kthread_create(HCC_TASK_RX_ISR_NAME,
        hcc_task_rx_gpio_thread, pst_sdio, &thread_param);
    if (pst_sdio->rx_handle == TD_NULL) {
        return EXT_FAILURE;
    }
    return EXT_SUCCESS;
}

static irqreturn_t oal_gpio_isr(int irq, void *data)
{
    oal_channel_stru* pst_sdio = (oal_channel_stru*)data;
    if (pst_sdio == NULL) {
        return IRQ_HANDLED;
    }
    /* 唤醒RX线程 */
    up(&pst_sdio->rx_sema);

    return IRQ_HANDLED;
}

int oal_sdio_interrupt_register(oal_channel_stru *pst_sdio)
{
    int ret;
    int irq;
#if(_PRE_OS_PLATFORM == _PRE_PLATFORM_JZ)
    jz_gpio_set_func(WLAN_GPIO_INT, GPIO_INT_RE);
#elif (_PRE_OS_PLATFORM == _PRE_PLATFORM_SOC)
    void __iomem *io_mux_base;
    io_mux_base = ioremap(0x112C0054, 0x04);
    gpio_reg_writel(io_mux_base, 0x1A00);
    iounmap(io_mux_base);
#elif(_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)

#endif
    /* new fix: */
    //ret = gpio_request_one(WLAN_GPIO_INT, GPIOF_IN, PROC_NAME_GPIO_WLAN_WAKEUP_HOST);
    ret = gpio_request_one(gpio, GPIOF_IN, PROC_NAME_GPIO_WLAN_WAKEUP_HOST);
    if (ret) {
        goto gpio_request_one_fail;
    }

    //irq = gpio_to_irq(WLAN_GPIO_INT);
    irq = gpio_to_irq(gpio);

    pst_sdio->ul_wlan_irq = irq;

    sema_init(&pst_sdio->rx_sema, 0);

    ret = request_irq(irq, oal_gpio_isr, IRQF_NO_SUSPEND |
        IRQF_TRIGGER_RISING, "wifi_gpio_intr", pst_sdio); /* | IRQF_DISABLED */
    if (ret < 0) {
        return ret;
    }

    disable_irq(irq);
    enable_irq(irq);
    return EXT_SUCCESS;
gpio_request_one_fail:
    return EXT_FAILURE;
}

static void oal_sdio_interrupt_unregister(oal_channel_stru *pst_sdio)
{
    disable_irq_nosync(pst_sdio->ul_wlan_irq);
    free_irq(pst_sdio->ul_wlan_irq, pst_sdio);
    /* new fix: */
    //gpio_free(WLAN_GPIO_INT);
    gpio_free(gpio);
    
    return;
}

/* new fix: */
//#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)
#if (_PRE_OS_PLATFORM == 100)
static td_s32 hcc_task_heartbeat_thread(td_void *data)
{
    oal_channel_stru *pst_sdio = (oal_channel_stru *)data;
    oam_warning_log0("hcc_task_heartbeat_thread start");
    allow_signal(SIGTERM);
    while (!down_interruptible(&g_hrtimer.heart_sema)) {
        if (kthread_should_stop()) {
             oam_error_log0("exit heartbeat thread");
            break;
        }
        pst_sdio->msg[D2H_MSG_HEARTBEAT].msg_rx(pst_sdio->msg[D2H_MSG_HEARTBEAT].data);
    }
    oam_info_log0("hcc_task_heartbeat_thread is terminated");
    return EXT_SUCCESS;
}

static td_s32 sdio_start_heartbeat_thread(oal_channel_stru *pst_sdio)
{
    oal_kthread_param_stru thread_param = {0};
    memset_s(&thread_param, sizeof(oal_kthread_param_stru), 0, sizeof(oal_kthread_param_stru));

    thread_param.l_cpuid = 1;
    thread_param.l_policy = OAL_SCHED_FIFO;
    thread_param.l_prio = HCC_TASK_RX_ISR_PRIO;
    thread_param.ul_stacksize = HCC_TASK_RX_ISR_SIZE;

    g_hrtimer.heartbeat_kthr = oal_kthread_create("heartbeat_isr",
        hcc_task_heartbeat_thread, pst_sdio, &thread_param);
    if (g_hrtimer.heartbeat_kthr == TD_NULL) {
        return EXT_FAILURE;
    }
    return EXT_SUCCESS;
}

static enum hrtimer_restart  hrtimer_timeout_hander(struct hrtimer *hrtimer)
{   
    up(&g_hrtimer.heart_sema);
    hrtimer_forward_now(hrtimer, g_hrtimer.kt);
    return HRTIMER_RESTART;  
}
#endif

static td_s32 oal_sdio_probe(struct sdio_func *func, const struct sdio_device_id *ids)
{
    oal_channel_stru *pst_sdio = TD_NULL;
    int ret;

    if (func == TD_NULL || func->card == TD_NULL || func->card->host == TD_NULL || (!ids)) {
        oam_error_log0("oal_sdio_probe:func func->card->host ids null\n");
        return -OAL_EFAIL;
    }

    pst_sdio = oal_sdio_alloc(func);
    if (pst_sdio == TD_NULL) {
        oam_error_log0("failed to alloc pst_sdio!\n");
        goto failed_sdio_alloc;
    }
/* new fix: */
//#if (_PRE_OS_PLATFORM == _PRE_PLATFORM_CANAAN)
#if (_PRE_OS_PLATFORM == 100)
    sema_init(&g_hrtimer.heart_sema, 0);

    ret = sdio_start_heartbeat_thread(pst_sdio);
    if (ret < 0) {
        goto failed_sdio_start_thread_fail;
    }

    g_hrtimer.kt = ktime_set(0, HEART_INTVL_NS);
    hrtimer_init(&g_hrtimer.hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    g_hrtimer.hrtimer.function = hrtimer_timeout_hander;

    hrtimer_start(&g_hrtimer.hrtimer, ktime_set(0, HEART_INTVL_NS), HRTIMER_MODE_REL);
#endif

    /* 注册GPIO中断 */
    ret = oal_sdio_interrupt_register(pst_sdio);
    if (ret < 0) {
        goto failed_sdio_dev_init;
    }

    ret = sdio_start_gpio_thread(pst_sdio);
    if (ret < 0) {
        goto failed_sdio_start_thread_fail;
    }

    oal_disable_sdio_state(pst_sdio, OAL_SDIO_ALL);

    if (oal_sdio_dev_init(pst_sdio) != EXT_SUCCESS) {
        oam_error_log0("sdio dev init failed!\n");
        goto failed_sdio_start_thread_fail;
    }

    oal_complete(&g_sdio_driver_complete);
    return EXT_SUCCESS;
failed_sdio_start_thread_fail:
    oal_sdio_interrupt_unregister(pst_sdio);
failed_sdio_dev_init:
    oal_free(pst_sdio);
failed_sdio_alloc:
    return -OAL_EFAIL;
}

static const struct dev_pm_ops oal_sdio_pm_ops = {
    .suspend = oal_sdio_suspend,
    .resume = oal_sdio_resume,
};

static  struct sdio_driver oal_sdio_driver = {
    .name       = "oal_sdio",
    .id_table   = g_oal_sdio_ids,
    .probe      = oal_sdio_probe,
    .remove     = oal_sdio_remove,
    .drv        = {
        .owner      = THIS_MODULE,
        .pm         = &oal_sdio_pm_ops,
        .shutdown   = oal_sdio_dev_shutdown,
    }
};

td_s32 oal_sdio_func_probe(oal_channel_stru *pst_sdio, td_s32 sdio_dev_num)
{
    td_s32 ret;

    if (pst_sdio == TD_NULL) {
        return -OAL_EFAIL;
    }
    oal_init_completion(&g_sdio_driver_complete);
    ret = oal_sdio_detectcard_to_core(sdio_dev_num);
    if (ret) {
        oam_error_log1("fail to detect sdio card, ret=%d\n", ret);
        goto failed_sdio_reg;
    }
    ret = sdio_register_driver(&oal_sdio_driver);
    if (ret) {
        oam_error_log1("register sdio driver Failed ret=%d\n", ret);
        goto failed_sdio_reg;
    }

    /* new fix: simplify the check process and reduce timeout */
#if 1
    if (!oal_wait_for_completion_timeout(&g_sdio_driver_complete, 30))
    {
        oam_error_log0("soc sdio load sucuess, sdio enum done.\n");
        goto failed_sdio_enum;
    }
#else
    td_s32 times = SDIO_PROBLE_TIMES;
    do {
        if (oal_wait_for_completion_timeout(&g_sdio_driver_complete, TIMEOUT_MUTIPLE_10 * HZ) != 0) {
            break;
        }

        ret = oal_sdio_detectcard_to_core(sdio_dev_num);
        if (ret) {
            oam_error_log1("fail to detect sdio card, ret=%d\n", ret);
            break;
        }
    } while (--times > 0);

    if (times <= 0) {
        oam_error_log0("soc sdio load sucuess, sdio enum done.\n");
        goto failed_sdio_enum;
    }
#endif

    oal_sdio_claim_host(pst_sdio->func);
    oal_disable_sdio_state(pst_sdio, OAL_SDIO_ALL);
    oal_sdio_release_host(pst_sdio->func);
    return EXT_SUCCESS;
failed_sdio_enum:
    sdio_unregister_driver(&oal_sdio_driver);
failed_sdio_reg:
    return -OAL_EFAIL;
}

td_void oal_sdio_credit_info_init(oal_channel_stru *pst_sdio)
{
    pst_sdio->sdio_credit_info.large_free_cnt = 0;
    pst_sdio->sdio_credit_info.short_free_cnt = 0;
    oal_spin_lock_init(&pst_sdio->sdio_credit_info.credit_lock);
}

oal_channel_stru *oal_sdio_init_module(hcc_handler_stru *hcc_handler, struct hcc_bus_ops *bus_ops)
{
    oal_channel_stru *pst_sdio = (oal_channel_stru *)oal_memalloc(sizeof(oal_channel_stru));
    if (pst_sdio == TD_NULL) {
        return TD_NULL;
    }
    memset_s(pst_sdio, sizeof(oal_channel_stru), 0, sizeof(oal_channel_stru));

    pst_sdio->rx_buf = (td_u8 *)oal_memalloc(SDIO_MAX_XFER_LEN);
    if (pst_sdio->rx_buf == TD_NULL) {
        goto alloc_sdio_struct_fail;
    }

    pst_sdio->tx_buf = (td_u8 *)oal_memalloc(SDIO_MAX_XFER_LEN);
    if (pst_sdio->tx_buf == TD_NULL) {
        goto alloc_rx_buf_fail;
    }

    oal_sdio_credit_info_init(pst_sdio);

    pst_sdio->sdio_extend = (sdio_extend_func *)oal_memalloc(sizeof(sdio_extend_func));
    if (pst_sdio->sdio_extend == TD_NULL) {
        goto failed_sdio_extend_alloc;
    }
    memset_s(pst_sdio->sdio_extend, sizeof(sdio_extend_func), 0, sizeof(sdio_extend_func));

    pst_sdio->scatt_info[SDIO_READ].max_scatt_num = EXT_SDIO_DEV2HOST_SCATT_MAX + 1;
    pst_sdio->scatt_info[SDIO_READ].sglist = oal_kzalloc(
        sizeof(struct scatterlist) * (EXT_SDIO_DEV2HOST_SCATT_MAX + 1), OAL_GFP_KERNEL);
    if (pst_sdio->scatt_info[SDIO_READ].sglist == TD_NULL) {
        goto failed_sdio_read_sg_alloc;
    }

    pst_sdio->scatt_info[SDIO_WRITE].max_scatt_num = EXT_SDIO_HOST2DEV_SCATT_MAX + 2; /* 2: 多预留2个 */
    pst_sdio->scatt_info[SDIO_WRITE].sglist = oal_kzalloc(
        sizeof(struct scatterlist) * (pst_sdio->scatt_info[SDIO_WRITE].max_scatt_num), OAL_GFP_KERNEL);
    if (pst_sdio->scatt_info[SDIO_WRITE].sglist == TD_NULL) {
        goto failed_sdio_write_sg_alloc;
    }

    pst_sdio->sdio_align_buff = oal_kzalloc(EXT_SDIO_BLOCK_SIZE, OAL_GFP_KERNEL);
    if (pst_sdio->sdio_align_buff == TD_NULL) {
        goto failed_sdio_align_buff_fail;
    }

    pst_sdio->func1_int_mask = SDIO_FUNC1_INT_MASK;
    pst_sdio->bus_data = (td_void*)hcc_handler;

    if (oal_sdio_transfer_register(pst_sdio, bus_ops) != EXT_SUCCESS) {
        goto failed_sdio_align_buff_fail;
    }

    g_channel_sdio_ = pst_sdio;

    return pst_sdio;
failed_sdio_align_buff_fail:
    oal_free(pst_sdio->scatt_info[SDIO_WRITE].sglist);
    pst_sdio->scatt_info[SDIO_WRITE].sglist = TD_NULL;
    pst_sdio->scatt_info[SDIO_WRITE].max_scatt_num = 0;
failed_sdio_write_sg_alloc:
    oal_free(pst_sdio->scatt_info[SDIO_READ].sglist);
    pst_sdio->scatt_info[SDIO_READ].sglist = TD_NULL;
    pst_sdio->scatt_info[SDIO_READ].max_scatt_num = 0;
failed_sdio_read_sg_alloc:
    oal_free(pst_sdio->sdio_extend);
    pst_sdio->sdio_extend = TD_NULL;
failed_sdio_extend_alloc:
    oal_free(pst_sdio->tx_buf);
    pst_sdio->tx_buf = TD_NULL;
alloc_rx_buf_fail:
    oal_free(pst_sdio->rx_buf);
    pst_sdio->rx_buf = TD_NULL;
alloc_sdio_struct_fail:
    oal_free(pst_sdio);
    return TD_NULL;
}


td_void  oal_sdio_exit_module(oal_channel_stru *pst_sdio)
{
    oal_free(pst_sdio->tx_buf);
    oal_free(pst_sdio->rx_buf);
    oal_free(pst_sdio->sdio_extend);
    oal_sdio_transfer_unregister(pst_sdio);
    oal_free(pst_sdio);
    g_channel_sdio_ = TD_NULL;
}

/* td_s32 val卡选择 */
td_s32 oal_sdio_init(void *data, td_s32 sdio_dev_num, struct hcc_bus_ops *bus_ops)
{
    oal_channel_stru *pst_sdio;
    td_s32 ret;
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;

    pst_sdio = oal_sdio_init_module(hcc_handler, bus_ops);
    if (pst_sdio == NULL) {
        oam_error_log0("oal_sdio_init_module: failed");
        return EXT_FAILURE;
    }

    ret = oal_sdio_func_probe(pst_sdio, sdio_dev_num);
    if (ret != EXT_SUCCESS) {
        oam_error_log1("regeste_sdio_drv:: oal_sdio_func_probe failed", ret);
        goto oal_sdio_func_probe_fail;
    }

    oal_enable_sdio_state(pst_sdio, OAL_SDIO_ALL);

    hcc_handler->channel = (td_void*)pst_sdio;

    return EXT_SUCCESS;
oal_sdio_func_probe_fail:
    oal_free(pst_sdio);
    return EXT_FAILURE;
}

td_void oal_sdio_exit(oal_channel_stru *pst_sdio)
{
    oal_disable_sdio_state(pst_sdio, OAL_SDIO_ALL);
    oal_sdio_interrupt_unregister(pst_sdio);
    oal_kthread_stop(pst_sdio->rx_handle);
    up(&pst_sdio->rx_sema);
    sdio_unregister_driver(&oal_sdio_driver);
    oal_sdio_exit_module(pst_sdio);
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

