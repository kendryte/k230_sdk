/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: hcc layer frw task.
 * Author: CompanyName
 * Create: 2021-09-28
 */
/* 头文件包含 */
#include "hcc_task.h"
#include "securec.h"
#include "oal_mm.h"
#include "oal_channel_host_if.h"
#include "hcc_list.h"
#include <linux/delay.h>

/* new fix: */
#include "soc_param.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 宏定义 */
#define THRESHOLD_SIZE_128_BYTES    128
#define THRESHOLD_SIZE_256_BYTES    256
#define THRESHOLD_SIZE_512_BYTES    512
#define THRESHOLD_SIZE_1024_BYTES   1024

#define oal_min(a, b)         (((a) > (b)) ? (b) : (a))
#define oal_max(a, b)         (((a) > (b)) ? (a) : (b))
/* 全局变量定义 */
hcc_handler_stru *g_hcc_host_handler = TD_NULL;
td_u32  g_hcc_assemble_count = 5;
static td_u32  g_lo_buf_times = 0;
#define MAX_TIMES                           1
#define MAX_TIME_VALUE                      15
#define MIN_TIME_VALUE                      15

/* 函数定义 */
hcc_handler_stru *hcc_host_get_handler(td_void)
{
    return g_hcc_host_handler;
}

static td_void hcc_tx_list_free(hcc_data_queue *hcc_queue)
{
    hcc_unc_struc *unc_buf = TD_NULL;
    for (;;) {
        unc_buf = hcc_list_dequeue(hcc_queue);
        if (unc_buf == TD_NULL) {
            break;
        }
        unc_buf->free(unc_buf);
    }
}

td_void hcc_clear_queues(hcc_handler_stru *hcc, hcc_chan_type type)
{
    td_s32 i;
    hcc_data_queue *head = TD_NULL;
    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        head = &hcc->hcc_transer_info.hcc_queues[type].queues[i].queue_info;
        if (hcc_list_len(head) > 0) {
            hcc_tx_list_free(head);
        }
    }
}

td_void hcc_sched_transfer(hcc_handler_stru *hcc_handler)
{
    if (oal_warn_on(hcc_handler == TD_NULL)) {
        oam_error_log0("hcc_sched_transfer:: hcc_handler is null!");
        return;
    }

    aich_wait_queue_wake_up_interrupt(&hcc_handler->hcc_transer_info.hcc_tx_wq);
}

/* init hcc transfer_queue */
td_void hcc_transfer_queues_init(hcc_handler_stru *hcc_handler)
{
    td_s32 i, j;
    for (i = 0; i < HCC_DIR_COUNT; i++) {
        for (j = 0; j < HCC_QUEUE_COUNT; j++) {
            hcc_list_head_init(&hcc_handler->hcc_transer_info.hcc_queues[i].queues[j].queue_info);
        }
    }
}

static td_void hcc_build_next_assem_descr(hcc_handler_stru *hcc_handler,
    hcc_queue_type_enum type,
    hcc_data_queue *head,
    hcc_data_queue *next_assembled_head,
    const hcc_unc_struc *descr,
    td_u32 remain_len)
{
    td_s32 i = 0;
    td_s32 len;
    td_u8 *buf = TD_NULL;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_unc_struc *unc_buf_t = TD_NULL;
    td_u32 assemble_max_count, queue_len, current_trans_len;

    buf = (td_u8 *)oal_unc_data(descr);
    len = (td_s32)oal_unc_len(descr);
    assemble_max_count = oal_max(1, g_hcc_assemble_count);
    queue_len = hcc_list_len(head);
    current_trans_len = oal_min(queue_len, assemble_max_count);
    current_trans_len = oal_min(current_trans_len, remain_len);
    buf[0] = 0;

    if (current_trans_len == 0) {
        return;
    }

    for (;;) {
        /* move the netbuf from head queue to prepare-send queue, head->tail */
        unc_buf = hcc_list_dequeue(head);
        if (unc_buf == TD_NULL) {
            break;
        }

        /* align the buff len to 32B */
        if (hcc_handler->hcc_bus_ops && hcc_handler->hcc_bus_ops->private_len_align) {
            unc_buf_t = hcc_handler->hcc_bus_ops->private_len_align(unc_buf, BUS_H2D_SCATT_BUFFLEN_ALIGN);
            if (oal_unlikely(unc_buf_t == TD_NULL)) {
                hcc_list_add_head(head, unc_buf);
                break;
            }
        }

        current_trans_len--;

        hcc_list_add_tail(next_assembled_head, unc_buf_t);
        if (oal_likely(i >= len)) {
            break;
        }

        buf[i++] = (td_u8)(oal_unc_len(unc_buf_t) >> BUS_H2D_SCATT_BUFFLEN_ALIGN_BITS);
        if (current_trans_len == 0) {
            /* send empty */
            if (i != len) {
                buf[i] = 0;
            }
            break;
        }
    }

    if (oal_likely(!hcc_is_list_empty(next_assembled_head))) {
        hcc_handler->hcc_transer_info.tx_assem_info.assembled_queue_type = type;
    }
}

hcc_unc_struc *hcc_tx_assem_descr_get(hcc_handler_stru *hcc_handler)
{
    return hcc_list_dequeue(&hcc_handler->tx_descr_info.tx_assem_descr_hdr);
}

static  td_void hcc_tx_assem_descr_put(hcc_handler_stru *hcc_handler,
    hcc_unc_struc *unc_buf)
{
    hcc_list_add_tail(&hcc_handler->tx_descr_info.tx_assem_descr_hdr, unc_buf);
}

static td_s32 hcc_send_data_packet(hcc_handler_stru *hcc_handler,
                                   hcc_data_queue *head,
                                   hcc_queue_type_enum type,
                                   hcc_data_queue *next_assembled_head,
                                   hcc_send_mode mode,
                                   td_u32 *remain_len)
{
    td_s32 ret = EXT_SUCCESS;
    hcc_data_queue head_send;
    hcc_unc_struc *unc_buf_t = TD_NULL;

    if (*remain_len == 0) {
        return EXT_SUCCESS;
    }

    hcc_unc_struc *descr_netbuf = hcc_tx_assem_descr_get(hcc_handler);
    if (descr_netbuf == TD_NULL) {
        ret = -OAL_ENOMEM;
        goto failed_get_assem_descr;
    }

    td_u32 *info = hcc_handler->hcc_transer_info.tx_assem_info.info;

    hcc_list_head_init(&head_send);

    if (hcc_is_list_empty(next_assembled_head)) {
        /* single send */
        hcc_unc_struc *unc_buf = hcc_list_dequeue(head);
        if (unc_buf == TD_NULL) {
            ret = EXT_FAIL;
            goto failed_get_sig_buff;
        }

        if (hcc_handler->hcc_bus_ops && hcc_handler->hcc_bus_ops->private_len_align) {
            unc_buf_t = hcc_handler->hcc_bus_ops->private_len_align(unc_buf, BUS_H2D_SCATT_BUFFLEN_ALIGN);
            if (oal_unlikely(unc_buf_t == TD_NULL)) {
                hcc_list_add_head(head, unc_buf);
                ret = EXT_FAIL;
                goto failed_align_unc_buf;
            }
        } else {
            goto failed_get_sig_buff;
        }

        hcc_list_add_tail(&head_send, unc_buf_t);
        info[0]++;
    } else {
        td_u32 assemble_len = hcc_list_len(next_assembled_head);
        /* move the assem list to send queue */
        hcc_list_splice_init(next_assembled_head, &head_send);
        info[assemble_len]++;
    }

    td_u32 total_send = hcc_list_len(&head_send);
    if (*remain_len >= total_send) {
        *remain_len -= total_send;
    } else {
        *remain_len = 0;
    }

    if (mode == HCC_ASSEM_SEND) {
        hcc_build_next_assem_descr(hcc_handler, type, head, next_assembled_head, descr_netbuf, *remain_len);
    } else {
        td_u8 *buf = oal_unc_data(descr_netbuf);
        *((td_u32 *) buf) = 0;
    }

    /* add the assem descr buf */
    hcc_list_add_head(&head_send, descr_netbuf);

    ret = oal_channel_transfer_list(hcc_handler->channel, &head_send, SDIO_WRITE);

    if ((hcc_handler->hcc_bus_ops != TD_NULL) && (hcc_handler->hcc_bus_ops->wlan_pm_set_packet_cnt)) {
        hcc_handler->hcc_bus_ops->wlan_pm_set_packet_cnt(total_send);
    }
    hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[type].total_pkts += total_send;

    descr_netbuf = hcc_list_dequeue(&head_send);
    if (descr_netbuf == TD_NULL) {
        ret = EXT_SUCCESS;
        goto failed_get_assem_descr;
    }
    hcc_tx_assem_descr_put(hcc_handler, descr_netbuf);
    /* free the sent netbuf,release the wakelock */
    hcc_tx_list_free(&head_send);
    return ret;
failed_align_unc_buf:
failed_get_sig_buff:
    hcc_tx_assem_descr_put(hcc_handler, descr_netbuf);
failed_get_assem_descr:
    return ret;
}

static td_s32 hcc_send_single_descr(hcc_handler_stru *hcc_handler, hcc_unc_struc *unc_buf)
{
    td_s32 ret;
    hcc_data_queue head_send;
    oal_reference(hcc_handler);
    hcc_list_head_init(&head_send);
    hcc_list_add_tail(&head_send, unc_buf);
    ret = oal_channel_transfer_list(hcc_handler->channel, &head_send, SDIO_WRITE);
    return ret;
}

td_s32 hcc_send_descr_control_data(hcc_handler_stru *hcc_handler, hcc_descr_type descr_type,
                                   const td_void *data, td_u32 ul_size)
{
    td_s32 ret;
    hcc_unc_struc *unc_buf = TD_NULL;
    struct hcc_descr_header *dscr_hdr = TD_NULL;
    unc_buf = hcc_tx_assem_descr_get(hcc_handler);
    if (unc_buf == TD_NULL) {
        return -OAL_ENOMEM;
    }

    dscr_hdr = (struct hcc_descr_header *)oal_unc_data(unc_buf);
    dscr_hdr->descr_type = descr_type;

    if (ul_size) {
        if (oal_warn_on(data == TD_NULL)) {
            hcc_tx_assem_descr_put(hcc_handler, unc_buf);
            return -OAL_EINVAL;
        }
        if (oal_warn_on(ul_size + sizeof(struct hcc_descr_header) > oal_unc_len(unc_buf))) {
            hcc_tx_assem_descr_put(hcc_handler, unc_buf);
            return -OAL_EINVAL;
        }

        /* lint -e124 */
        if (memcpy_s((td_void *)((td_u8 *)oal_unc_data(unc_buf) + sizeof(struct hcc_descr_header)),
            ul_size, data, ul_size) != 0) {
            return -OAL_EINVAL;
        }
    }
    ret = hcc_send_single_descr(hcc_handler, unc_buf);

    hcc_tx_assem_descr_put(hcc_handler, unc_buf);
    return ret;
}

static td_void  hcc_restore_assemble_netbuf_list(hcc_handler_stru *hcc_handler)
{
    hcc_queue_type_enum type;
    hcc_data_queue *assembled_head;

    type = hcc_handler->hcc_transer_info.tx_assem_info.assembled_queue_type;
    assembled_head = &hcc_handler->hcc_transer_info.tx_assem_info.assembled_head;

    if (type >= HCC_QUEUE_COUNT) {
        type = DATA_LO_QUEUE;
    }
    /* new fix: */ 
    //将hcc_unc_struc从assembled_head尾部依次取出逐个添加到queue_info的头部，注意这里是能够保证hcc_unc_struc顺序的
    //或许这里使用hcc_list_splice_init是个更好的选择，hcc_list_splice_sync使用了循环
    //但是hcc_list_splice_sync使用了自旋锁，可能更安全
    if (!hcc_is_list_empty(assembled_head)) {
        hcc_list_splice_sync(&hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[type].queue_info,
            assembled_head);
    }
}

static  td_s32 hcc_send_assemble_reset(hcc_handler_stru *hcc_handler)
{
    td_s32 ret;
    hcc_handler->hcc_transer_info.tx_flow_ctrl.flowctrl_reset_count++;
    /* 当只发送一个聚合描述符包，并且聚合个数为0描述通知Device 重置聚合信息 */
    ret = hcc_send_descr_control_data(hcc_handler, HCC_DESCR_ASSEM_RESET, TD_NULL, 0);
    hcc_restore_assemble_netbuf_list(hcc_handler);
    return ret;
}

td_s32 hcc_tx_queue_switch(hcc_handler_stru *hcc_handler, hcc_netbuf_queue_type queue_type)
{
    return hcc_send_descr_control_data(hcc_handler, HCC_QUEUE_SWITCH, &queue_type, sizeof(queue_type));
}

td_s32 hcc_switch_high_pri_queue(hcc_handler_stru *hcc_handler,
    hcc_netbuf_queue_type pool_type)
{
    td_s32 ret = OAL_EFAIL;
    if (pool_type == HCC_HIGH_QUEUE) {
        /* 此处在最高位表示是否是配置事件 */
        ret = hcc_tx_queue_switch(hcc_handler, HCC_HIGH_QUEUE);
    }
    return ret;
}

td_s32 hcc_restore_normal_pri_queue(hcc_handler_stru *hcc_handler, hcc_netbuf_queue_type pool_type)
{
    td_s32 ret = OAL_EFAIL;
    if (pool_type == HCC_HIGH_QUEUE) {
        ret = hcc_tx_queue_switch(hcc_handler, HCC_NORMAL_QUEUE);
    }
    return ret;
}

static td_u32 hcc_tx_flow_ctrl_handle(hcc_handler_stru *hcc_handler, hcc_queue_type_enum type)
{
    td_u32 priority_cnt;
    td_s32 ret = EXT_SUCCESS;
    hcc_trans_queue_stru* hcc_queue;

    hcc_queue = &hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[type];
    if (hcc_handler->channel && oal_get_channel_func(hcc_handler->channel) != TD_NULL) {
        ret = oal_channel_get_credit(hcc_handler->channel, &priority_cnt);
        if (ret < 0) {
            oam_error_log2("hcc_tx_flow_ctrl_handle fail, type = %d, ret = %d!", type, ret);
            hcc_handler->hcc_transer_info.channel_exception_flag = (1 << EXCEPTION_CMD_SEND_FAILED);
            return EXT_FAIL;
        }
    }
    oal_spin_lock(&(hcc_handler->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));
    hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = hcc_mgmt_pkt_get(priority_cnt);
    hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_lopriority_cnt = hcc_large_pkt_get(priority_cnt);
    oal_spin_unlock(&(hcc_handler->hcc_transer_info.tx_flow_ctrl.st_hipri_lock));
    if (hcc_handler->hcc_bus_ops &&
        hcc_handler->hcc_bus_ops->tx_flow_ctrl_handle) {
        if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
            ret = hcc_handler->hcc_bus_ops->tx_flow_ctrl_handle(hcc_queue->flow_ctrl.flow_type,
                hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt);
        } else if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_SDIO) {
            ret = hcc_handler->hcc_bus_ops->tx_flow_ctrl_handle(hcc_queue->flow_ctrl.flow_type,
                hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_lopriority_cnt);
        }
    }

    return ret;
}

td_void hcc_tx_sleep(td_void)
{
    g_lo_buf_times++;
    if (g_lo_buf_times > MAX_TIMES) {
        usleep_range(MIN_TIME_VALUE, MAX_TIME_VALUE);
        g_lo_buf_times = 0;
    }
}

td_s32 hcc_host_proc_tx_queue(hcc_handler_stru *hcc_handler, hcc_queue_type_enum type)
{
    td_s32 count = 0;

    if (type >= HCC_QUEUE_COUNT) {
        oam_error_log1("hcc_send_tx_queue:: invalid hcc_queue type[%d]", type);
        return count;
    }

    if (hcc_handler->hcc_transer_info.tx_flow_ctrl.flowctrl_flag == D2H_MSG_FLOWCTRL_OFF) {
        hcc_tx_sleep();
        return  count;
    }

    hcc_trans_queue_stru *hcc_queue = &hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[type];
    hcc_data_queue *head = &hcc_queue->queue_info;

    td_u32 remain_len = hcc_list_len(head);
    if (!remain_len) {
        goto hcc_tx_exit;
    }

    /* hcc流控处理 */
    td_s32 ret = hcc_tx_flow_ctrl_handle(hcc_handler, type);
    if (ret != EXT_SUCCESS) {
        hcc_tx_sleep();
        goto hcc_tx_exit;
    }

    g_lo_buf_times = 0;

    if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        remain_len = oal_min(hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt, remain_len);
    } else if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_SDIO) {
        remain_len = oal_min(hcc_handler->hcc_transer_info.tx_flow_ctrl.uc_lopriority_cnt, remain_len);
    }

    td_u32 remain_len_t = remain_len;
    hcc_data_queue *next_assembled_head = &hcc_handler->hcc_transer_info.tx_assem_info.assembled_head;

    if (!hcc_is_list_empty(next_assembled_head)) {
        if (hcc_send_assemble_reset(hcc_handler) != EXT_SUCCESS) {
            /* send one pkt */
            count = 1;
            goto hcc_tx_exit;
        }
    }
    //DATA_LO_QUEUE---HCC_ASSEM_SEND; DATA_HI_QUEUE---HCC_SINGLE_SEND
    hcc_send_mode send_mode = hcc_queue->send_mode;
    //DATA_LO_QUEUE---HCC_NORMAL_QUEUE; DATA_HI_QUEUE---HCC_HIGH_QUEUE
    hcc_queue_type pool_type = hcc_queue->pool_type;

    /* new fix: 从for循环体中移动到此处 */
    if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        ret = hcc_switch_high_pri_queue(hcc_handler, pool_type);
        if (ret != EXT_SUCCESS) {
            count = 1;
            goto hcc_tx_exit;
        }
    }

    while (remain_len != 0) {
        /* new fix: 从for循环体中移出到体外 */
#if 0
        if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
            ret = hcc_switch_high_pri_queue(hcc_handler, pool_type);
            if (ret != EXT_SUCCESS) {
                break;
            }
        }
#endif
        if (hcc_handler->hcc_transer_info.tx_flow_ctrl.flowctrl_flag == D2H_MSG_FLOWCTRL_OFF) {
            break;
        }

        ret = hcc_send_data_packet(hcc_handler, head, type, next_assembled_head, send_mode, &remain_len);
        if (ret != EXT_SUCCESS) {
            break;
        }
        count += (td_s32)(remain_len_t - remain_len);
    }

    if (hcc_queue->flow_ctrl.flow_type == HCC_FLOWCTRL_CREDIT) {
        hcc_restore_normal_pri_queue(hcc_handler, pool_type);
    }

hcc_tx_exit:
    return count;
}

td_u32 hcc_host_proc_rx_queue(hcc_handler_stru *hcc_handler, hcc_queue_type_enum  type)
{
    td_s32 count = 0;
    td_s32 ret;
    hcc_data_queue *head = TD_NULL;
    hcc_unc_struc *unc_buf = TD_NULL;
    head = &hcc_handler->hcc_transer_info.hcc_queues[HCC_RX].queues[type].queue_info;

    for (;;) {
        unc_buf = hcc_list_dequeue(head);
        if (unc_buf == TD_NULL) {
            break;
        }
        if (hcc_handler->hcc_bus_ops != NULL && hcc_handler->hcc_bus_ops->rx_proc_queue) {
            ret = hcc_handler->hcc_bus_ops->rx_proc_queue(unc_buf);
            count++;
        } else {
            unc_buf->free(unc_buf);
        }
    }
    return count;
}

td_void hcc_trans_flow_ctrl_info_init(hcc_handler_stru *hcc_handle)
{
    td_s32 i;

    hcc_handle->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_ON;
    hcc_handle->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count = 0;
    hcc_handle->hcc_transer_info.tx_flow_ctrl.flowctrl_on_count = 0;
    oal_spin_lock_init(&hcc_handle->hcc_transer_info.tx_flow_ctrl.lock);

    hcc_handle->hcc_transer_info.tx_flow_ctrl.uc_hipriority_cnt = HCC_FLOW_HIGH_PRI_BUFF_CNT;
    hcc_handle->hcc_transer_info.tx_flow_ctrl.uc_lopriority_cnt = HCC_FLOW_LOW_PRI_BUFF_CNT;
    oal_spin_lock_init(&hcc_handle->hcc_transer_info.tx_flow_ctrl.st_hipri_lock);

    hcc_trans_queues_stru *hcc_tx_queues = &hcc_handle->hcc_transer_info.hcc_queues[HCC_TX];
    hcc_trans_queues_stru *hcc_rx_queues = &hcc_handle->hcc_transer_info.hcc_queues[HCC_RX];

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        /* TX queue */
        hcc_tx_queues->queues[i].flow_ctrl.enable = TD_TRUE;
        hcc_tx_queues->queues[i].flow_ctrl.flow_type = HCC_FLOWCTRL_SDIO;
        hcc_tx_queues->queues[i].flow_ctrl.is_stopped = TD_FALSE;
        hcc_tx_queues->queues[i].flow_ctrl.low_waterline = THRESHOLD_SIZE_512_BYTES;
        hcc_tx_queues->queues[i].flow_ctrl.high_waterline = THRESHOLD_SIZE_1024_BYTES;
        hcc_tx_queues->queues[i].pool_type = HCC_NORMAL_QUEUE;

        /* RX queue */
        hcc_rx_queues->queues[i].flow_ctrl.enable = TD_TRUE;
        hcc_rx_queues->queues[i].flow_ctrl.is_stopped = TD_FALSE;
        hcc_rx_queues->queues[i].flow_ctrl.low_waterline = THRESHOLD_SIZE_128_BYTES;
        hcc_rx_queues->queues[i].flow_ctrl.high_waterline = THRESHOLD_SIZE_512_BYTES;
    }

    /* Additional high priority flow_ctrl settings */
    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.flow_type = HCC_FLOWCTRL_CREDIT;
    hcc_tx_queues->queues[DATA_HI_QUEUE].flow_ctrl.enable = TD_FALSE;
    hcc_tx_queues->queues[DATA_HI_QUEUE].pool_type = HCC_HIGH_QUEUE;
}

td_void hcc_trans_send_mode_init(hcc_handler_stru *hcc_handler)
{
    td_s32 i;

    for (i = 0; i < HCC_QUEUE_COUNT; i++) {
        hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[i].send_mode = HCC_ASSEM_SEND;
    }
    hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[DATA_HI_QUEUE].send_mode = HCC_SINGLE_SEND;
}

td_s32 hcc_message_register(const hcc_handler_stru *hcc_handler, td_u8 msg, hcc_msg_cb cb, td_void *data)
{
    return oal_channel_message_register(hcc_handler->channel, msg, cb, data);
}

td_void hcc_message_unregister(const hcc_handler_stru *hcc_handler, td_u8 msg)
{
    oal_channel_message_unregister(hcc_handler->channel, msg);
}

static td_u32 hcc_host_check_header_vaild(const hcc_header_stru *hcc_hdr)
{
    if ((hcc_hdr->main_type >= HCC_TYPE_BUFF)) {
        oam_error_log1("hcc_host_check_header_vaild:: invalid type[%d]", hcc_hdr->main_type);
        return TD_FALSE;
    }
    return TD_TRUE;
}

static td_void hcc_transfer_rx_handler_replace(hcc_handler_stru *hcc_handler, hcc_unc_struc *unc_buf_new)
{
    hcc_unc_struc *unc_buf_old = TD_NULL;
    hcc_trans_queue_stru *hcc_queue = &hcc_handler->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE];
    unc_buf_old = hcc_list_dequeue(&hcc_queue->queue_info);
    if (unc_buf_old != TD_NULL) {
        hcc_queue->loss_pkts++;
        unc_buf_old->free(unc_buf_old);
    }
    hcc_list_add_tail(&hcc_queue->queue_info, unc_buf_new);
}

static td_void hcc_transfer_rx_handler(hcc_handler_stru *hcc_handler, hcc_unc_struc *unc_buf)
{
    hcc_list_add_tail(&hcc_handler->hcc_transer_info.hcc_queues[HCC_RX].queues[DATA_LO_QUEUE].queue_info, unc_buf);
}

td_void hcc_rx_sched_transfer(hcc_handler_stru *hcc_handler)
{
    if (oal_warn_on(hcc_handler == TD_NULL)) {
        oam_error_log0("hcc_sched_transfer:: hcc_handler is null!");
        return;
    }
    aich_wait_queue_wake_up_interrupt(&hcc_handler->hcc_transer_info.hcc_rx_wq);
}

static td_void hcc_rx_list_handler(hcc_handler_stru *hcc_handler, hcc_data_queue *head)
{
    td_u32 scatt_count;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_header_stru *hcc_hdr = TD_NULL;

    scatt_count = hcc_list_len(head);
    if (scatt_count > BUS_DEV2HOST_SCATT_MAX) {
        oam_error_log1("hcc_rx_netbuf_list_handler:: scatt buffs overflow, scatt_count[%d]", scatt_count);
        scatt_count = 0;
    }

    hcc_handler->hcc_transer_info.rx_assem_info.info[scatt_count]++;
    hcc_trans_queues_stru *hcc_rx = &hcc_handler->hcc_transer_info.hcc_queues[HCC_RX];

    for (;;) {
        unc_buf = hcc_list_dequeue(head);
        if (unc_buf == TD_NULL) {
            break;
        }
        hcc_hdr = (hcc_header_stru *)oal_unc_data(unc_buf);
        if (hcc_host_check_header_vaild(hcc_hdr) != TD_TRUE) {
            unc_buf->free(unc_buf);
            oam_error_log0("invalid hcc header: ");
            break;
        }

        if (hcc_rx->queues[DATA_LO_QUEUE].flow_ctrl.enable == TD_TRUE) {
            if (hcc_list_len(&hcc_rx->queues[DATA_LO_QUEUE].queue_info) >
                hcc_rx->queues[DATA_LO_QUEUE].flow_ctrl.high_waterline) {
                hcc_transfer_rx_handler_replace(hcc_handler, unc_buf);
            } else {
                hcc_transfer_rx_handler(hcc_handler, unc_buf);
            }
        } else {
            hcc_transfer_rx_handler(hcc_handler, unc_buf);
        }
    }

    hcc_rx_sched_transfer(hcc_handler);
}

td_s32 hcc_transfer_rx_data_handler(td_void *data)
{
    td_s32 ret;
    hcc_data_queue head;
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;

    hcc_list_head_init(&head);

    ret = oal_channel_build_rx_list(hcc_handler->channel, &head);
    if (ret != EXT_SUCCESS) {
        oam_error_log1("oal_channel_build_rx_list:: oal_channel_build_rx_list failed[%d]", ret);
        return ret;
    }

    ret = oal_channel_transfer_list(hcc_handler->channel, &head, SDIO_READ);
    if (ret != EXT_SUCCESS) {
        hcc_list_purge(&head);
        oam_error_log1("oal_channel_transfer_list:: oal_channel_transfer_list failed[%d]", ret);
        return -OAL_EFAIL;
    }

    hcc_rx_list_handler(hcc_handler, &head);
    return EXT_SUCCESS;
}

td_void hcc_tx_assem_descr_exit(hcc_handler_stru *hcc_handler)
{
    hcc_list_purge(&hcc_handler->tx_descr_info.tx_assem_descr_hdr);
}

td_void hcc_host_tx_assem_info_reset(hcc_handler_stru *hcc_handler)
{
    memset_s(hcc_handler->hcc_transer_info.tx_assem_info.info,
        sizeof(hcc_handler->hcc_transer_info.tx_assem_info.info),
        0,
        sizeof(hcc_handler->hcc_transer_info.tx_assem_info.info));
}

td_void hcc_host_rx_assem_info_reset(hcc_handler_stru *hcc_handler)
{
    memset_s(hcc_handler->hcc_transer_info.rx_assem_info.info,
        sizeof(hcc_handler->hcc_transer_info.rx_assem_info.info),
        0,
        sizeof(hcc_handler->hcc_transer_info.rx_assem_info.info));
}

td_void hcc_assem_info_init(hcc_handler_stru *hcc_handler)
{
    hcc_handler->hcc_transer_info.tx_assem_info.assemble_max_count = g_hcc_assemble_count;
    hcc_host_tx_assem_info_reset(hcc_handler);
    hcc_host_rx_assem_info_reset(hcc_handler);
    hcc_list_head_init(&hcc_handler->hcc_transer_info.tx_assem_info.assembled_head);
}

td_s32 hcc_tx_assem_descr_init(hcc_handler_stru *hcc_handler)
{
    td_s32 i;
    td_s32 ret = EXT_SUCCESS;
    hcc_unc_struc *accem_desm = TD_NULL;

    if (hcc_handler->hcc_bus_ops == TD_NULL ||
        hcc_handler->hcc_bus_ops->alloc_unc_buf == TD_NULL) {
        return EXT_FAIL;
    }

    hcc_list_head_init(&hcc_handler->tx_descr_info.tx_assem_descr_hdr);
    hcc_handler->tx_descr_info.descr_num = MAX_ASSEM_DESC_CNT;

    for (i = 0; i < hcc_handler->tx_descr_info.descr_num; i++) {
        accem_desm = hcc_handler->hcc_bus_ops->alloc_unc_buf(BUS_HOST2DEV_SCATT_SIZE, NORMAL_STRU_TYPE);
        if (accem_desm == TD_NULL) {
            goto failed_netbuf_alloc;
        }

        hcc_list_add_tail(&hcc_handler->tx_descr_info.tx_assem_descr_hdr, accem_desm);
        if (!oal_is_aligned((uintptr_t)oal_unc_data(accem_desm), 4))  { /* 4 字节对齐 */
            oam_error_log0("hcc_tx_assem_descr_init:: 4 aligned failed!");
        }
    }
    return ret;
failed_netbuf_alloc:
    hcc_list_purge(&hcc_handler->tx_descr_info.tx_assem_descr_hdr);
    return -OAL_ENOMEM;
}

td_s32 hcc_heartbeat_callback(td_void *data)
{
    hcc_handler_stru *hcc_handle = (hcc_handler_stru *)data;
    if (oal_channel_send_msg(hcc_handle->channel, H2D_MSG_HEARTBEAT_ACK) != EXT_SUCCESS) {
        /* new fix: */
        oam_error_log0("sndmsg H2D_MSG_HEARTBEAT_ACK failed!");
        return EXT_FAIL;
    }

    return EXT_SUCCESS;
}

td_void hcc_dev_flowctrl_off(hcc_handler_stru *hcc_handler)
{
    hcc_handler->hcc_transer_info.tx_flow_ctrl.flowctrl_flag = D2H_MSG_FLOWCTRL_OFF;
    hcc_handler->hcc_transer_info.tx_flow_ctrl.flowctrl_off_count++;
}

td_s32 hcc_flow_off_callback(td_void *data)
{
    hcc_dev_flowctrl_off((hcc_handler_stru *)data);
    return EXT_SUCCESS;
}

#ifdef _PRE_SOCCHANNEL_DEBUG
td_s32 hcc_test_write_over_callback(td_void *data)
{
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;

    if (oal_warn_on(hcc_handler == TD_NULL)) {
        oam_error_log0("hcc_sched_transfer:: hcc_handler is null!");
        return EXT_FAIL;
    }
    oal_complete(&hcc_handler->hcc_transer_info.hcc_test_tx);
    return EXT_SUCCESS;
}
#endif

/* new fix: */
#if (_PRE_KERVER == _PRE_KERVER_4D9)
static void hcc_trig_tx_sched(unsigned long data)
#else
static void hcc_trig_tx_sched(oal_timer_list_stru *timer_list)
#endif
{
    /* new fix: */
#if (_PRE_KERVER == _PRE_KERVER_4D9)
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;
#else
    hcc_handler_stru *hcc_handler = hcc_host_get_handler();
#endif
    if (hcc_handler == TD_NULL) {
        hcc_handler->hcc_transer_info.hcc_timer_status = TIMER_STOP;
        return;
    }
    hcc_handler->hcc_transer_info.hcc_timer_status = TIMER_RUNING;

    /* stop tcpip tx queue */
    hcc_trans_queue_stru *hcc_queue = &hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[DATA_LO_QUEUE];
    if ((hcc_handler->hcc_bus_ops != TD_NULL) &&
        (hcc_handler->hcc_bus_ops->awake_tcpip_tx_queue != TD_NULL)) {
        hcc_handler->hcc_bus_ops->awake_tcpip_tx_queue(hcc_queue);
    }

    hcc_sched_transfer(hcc_handler);
    /* new fix: */
#if (_PRE_KERVER == _PRE_KERVER_4D9)
    hi_u32 ret = oal_timer_start(&hcc_handler->hcc_transer_info.hcc_timer, HCC_TRIG_TX_SCHED_TIMEOUT);
#else
    td_u32 ret = oal_timer_start(timer_list, HCC_TRIG_TX_SCHED_TIMEOUT);
#endif
    if (ret != 0 && ret != 1) {
        oam_error_log1("{hcc_trig_tx_sched timer fail: fail ret = %d}", ret);
    }
    hcc_handler->hcc_transer_info.hcc_timer_status = TIMER_STOP;
}

static hcc_unc_struc* hcc_alloc_unc_buf(td_s32 len, hcc_stru_type type)
{
    hcc_handler_stru *hcc_handler = hcc_host_get_handler();
    if (hcc_handler == TD_NULL) {
        return TD_NULL;
    }

    if (hcc_handler->hcc_bus_ops &&
        hcc_handler->hcc_bus_ops->alloc_unc_buf) {
        return hcc_handler->hcc_bus_ops->alloc_unc_buf(len, type);
    }

    return TD_NULL;
}

static td_u32 hcc_wlan_pm_wakeup_dev(td_void *data)
{
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;
    if (hcc_handler == TD_NULL) {
        return EXT_FAIL;
    }

    if (hcc_handler->hcc_bus_ops &&
        hcc_handler->hcc_bus_ops->wlan_pm_wakeup_dev) {
        return hcc_handler->hcc_bus_ops->wlan_pm_wakeup_dev();
    }
    return EXT_SUCCESS;
}

static struct hcc_bus_ops g_bus_ops = {
    .rx             = hcc_transfer_rx_data_handler,
    .alloc_unc_buf  = hcc_alloc_unc_buf,
    .pm_wakeup_dev  = hcc_wlan_pm_wakeup_dev,
};

td_u32 hcc_host_init(struct hcc_bus_adpta_ops* hcc_bus_adpta_opt)
{
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)oal_memalloc(sizeof(hcc_handler_stru));
    if (hcc_handler == TD_NULL) {
        oam_error_log0("hcc_host_init:: malloc hcc_handler fail!");
        return EXT_FAIL;
    }
    /* new fix: */
#if (_PRE_KERVER != _PRE_KERVER_4D9)
    else
    {
        g_hcc_host_handler = hcc_handler;
        oam_info_log0("hcc_host_init:: malloc hcc_handler success!");
    }
#endif
    if (memset_s(hcc_handler, sizeof(hcc_handler_stru), 0, sizeof(hcc_handler_stru)) != EOK) {
        goto sdio_init_err;
    }
    /* new fix: */
    //if (oal_channel_init((void*)hcc_handler, 1, &g_bus_ops) != EXT_SUCCESS) {
    //if (oal_channel_init((void*)hcc_handler, 2, &g_bus_ops) != EXT_SUCCESS) {
    if (oal_channel_init((void*)hcc_handler, mmc, &g_bus_ops) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: oal_sdio_init fail!");
        goto sdio_init_err;
    }

    aich_wait_queue_init_head(&hcc_handler->hcc_transer_info.hcc_tx_wq);
    aich_wait_queue_init_head(&hcc_handler->hcc_transer_info.hcc_rx_wq);
#ifdef _PRE_SOCCHANNEL_DEBUG
    oal_init_completion(&hcc_handler->hcc_transer_info.hcc_test_tx);
#endif
    hcc_transfer_queues_init(hcc_handler);
    hcc_trans_flow_ctrl_info_init(hcc_handler);
    oal_mutex_init(&hcc_handler->tx_transfer_lock);
    hcc_handler->hcc_bus_ops = hcc_bus_adpta_opt;
    td_u32 ret = hcc_task_init(hcc_handler);
    if (ret != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: hcc_task_init failed");
        goto hcc_tast_init_err;
    }

    /* tx线程防呆 */
    oal_timer_init(&hcc_handler->hcc_transer_info.hcc_timer, HCC_TRIG_TX_SCHED_TIMEOUT,
        hcc_trig_tx_sched,
        (uintptr_t)hcc_handler);
    oal_timer_add(&hcc_handler->hcc_transer_info.hcc_timer);
    hcc_handler->hcc_transer_info.hcc_timer_status = TIMER_ADD;

    hcc_assem_info_init(hcc_handler);
    hcc_trans_send_mode_init(hcc_handler);

    if (hcc_tx_assem_descr_init(hcc_handler) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: hcc_tx_assem_descr_init failed");
        goto hcc_tast_init_err;
    }

    if (hcc_message_register(hcc_handler,
        D2H_MSG_FLOWCTRL_OFF, hcc_flow_off_callback, hcc_handler) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: hcc_message_register_flowctrl_off failed!");
        goto failed_reg_flowoff_msg;
    }
  
    if (hcc_message_register(hcc_handler,
        D2H_MSG_HEARTBEAT, hcc_heartbeat_callback, hcc_handler) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: heartbeat_cb register failed!");
        goto failed_reg_heartbeat_msg;
    }

    /* new fix: disable sdio heartbeat */
#if 0
    /* notify SDIO device to start heartbeat */
    if (oal_channel_send_msg(hcc_handler->channel, H2D_MSG_HEART_BEAT_OPEN) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: start heartbeat fail");
        goto failed_reg_heartbeat_msg;
    }
#endif
#ifdef _PRE_SOCCHANNEL_HDR_OPT
    if (oal_channel_send_msg(hcc_handler->channel, H2D_MSG_SOCCHANNEL_HDR_OPT) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: socchannel_hdr opt fail");
        goto failed_reg_heartbeat_msg;
    }
#endif

#ifdef _PRE_SOCCHANNEL_DEBUG
    if (hcc_message_register(hcc_handler,
        D2H_MSG_TEST_WRITE_OVER,
        hcc_test_write_over_callback, hcc_handler) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: hcc_message_register_flowctrl_off failed!");
        goto failed_reg_test_write_over;
    }
#endif

    /* new fix: */
#if (_PRE_KERVER == _PRE_KERVER_4D9)
    g_hcc_host_handler = hcc_handler; 
#endif
    if (oal_channel_send_msg(hcc_handler->channel, H2D_MSG_SDIO_READY) != EXT_SUCCESS) {
        oam_error_log0("hcc_host_init:: send ready msg fail");
        goto failed_send_ready_msg;
    }
    oam_warning_log0("hcc_host_init SUCCESSFULLY");
    return EXT_SUCCESS;
failed_send_ready_msg:
#ifdef _PRE_SOCCHANNEL_DEBUG
failed_reg_test_write_over:
    hcc_message_unregister(hcc_handler, D2H_MSG_HEARTBEAT);
    hcc_message_unregister(hcc_handler, D2H_MSG_FLOWCTRL_OFF);
#endif
failed_reg_heartbeat_msg:
    hcc_message_unregister(hcc_handler, D2H_MSG_FLOWCTRL_OFF);
failed_reg_flowoff_msg:
    hcc_tx_assem_descr_exit(hcc_handler);
    oal_timer_delete(&hcc_handler->hcc_transer_info.hcc_timer);
    hcc_handler->hcc_transer_info.hcc_timer_status = TIMER_DEL;
hcc_tast_init_err:
    oal_kthread_stop(hcc_handler->hcc_transer_info.hcc_transfer_thread);
    hcc_handler->hcc_transer_info.hcc_transfer_thread = TD_NULL;
    oal_mutex_destroy(&hcc_handler->tx_transfer_lock);
    oal_channel_exit(hcc_handler->channel);
sdio_init_err:
    oal_free(hcc_handler);
    return EXT_FAIL;
}

td_void hcc_delete_tx_sched_timer(hcc_handler_stru *hcc)
{
    td_u16 retry_time = 10000;
    if (hcc == TD_NULL || hcc->hcc_transer_info.hcc_timer_status == TIMER_DEL) {
        return;
    }

    while (retry_time > 0 && hcc->hcc_transer_info.hcc_timer_status == TIMER_RUNING) {
        usleep_range(1, 1);
        retry_time--;
    }
    oal_timer_delete(&hcc->hcc_transer_info.hcc_timer);
    hcc->hcc_transer_info.hcc_timer_status = TIMER_DEL;
}

td_void hcc_host_exit(hcc_handler_stru *hcc)
{
    if (hcc == TD_NULL) {
        return;
    }

    hcc_message_unregister(hcc, D2H_MSG_HEARTBEAT);
#ifdef _PRE_SOCCHANNEL_DEBUG
    hcc_message_unregister(hcc, D2H_MSG_TEST_WRITE_OVER);
#endif
    hcc_tx_assem_descr_exit(hcc);
    hcc_delete_tx_sched_timer(hcc);
    oal_disable_channel_state(hcc->channel, OAL_SDIO_ALL);
    hcc_exit_task_thread(hcc);
    oal_kthread_stop(hcc->hcc_transer_info.hcc_transfer_thread);
    oal_kthread_stop(hcc->hcc_transer_info.hcc_rx_thread);
    hcc_clear_queues(hcc, HCC_TX);
    hcc_clear_queues(hcc, HCC_RX);
    hcc->hcc_transer_info.hcc_transfer_thread = TD_NULL;
    oal_mutex_destroy(&hcc->tx_transfer_lock);
    oal_channel_exit(hcc->channel);
    oal_free(hcc);
    g_hcc_host_handler = TD_NULL;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
