/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oal_netbuff.c.
 * Author: CompanyName
 * Create: 2021-08-04
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_netbuf.h"
#include "oam_ext_if.h"
#include "oal_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 向netbu_head的尾部串接netbuf
*****************************************************************************/
td_u32 oal_netbuf_concat(oal_netbuf_stru* netbuf_head, oal_netbuf_stru* netbuf)
{
     /* 判断空间是否足够 */
    if (((td_u32)netbuf_head->end - (td_u32)netbuf_head->tail) < netbuf->len) {
        oal_netbuf_free(netbuf);
        oam_error_log3("oal_netbuf_concat::no enough space: end:%d, tail:%d, len:%d.",
            netbuf_head->end, netbuf_head->tail, netbuf->len);
        return EXT_FAIL;
    }
    if (memcpy_s(skb_tail_pointer(netbuf_head), netbuf->len, netbuf->data, netbuf->len) != EOK) {
        oam_error_log0("oal_netbuf_concat:: memcpy_s failed");
        oal_netbuf_free(netbuf);
        return EXT_FAIL;
    }

    skb_put(netbuf_head, netbuf->len);
    oal_netbuf_free(netbuf);
    return EXT_SUCCESS;
}

/*****************************************************************************
 功能描述  : 判断ipv4 tcp报文是否为tcp ack
*****************************************************************************/
td_u8 oal_netbuf_is_tcp_ack(oal_ip_header_stru  *ip_hdr)
{
    oal_tcp_header_stru    *tcp_hdr = TD_NULL;
    td_u32                 ip_pkt_len;
    td_u32                 ip_hdr_len;
    td_u32                 tcp_hdr_len;

    tcp_hdr     = (oal_tcp_header_stru *)(ip_hdr + 1);
    ip_pkt_len   = oal_net2host_short(ip_hdr->us_tot_len);
    ip_hdr_len   = (oal_low_half_byte(ip_hdr->us_ihl)) << 2;   /* 2: 左移2位 */
    tcp_hdr_len  = (oal_high_half_byte(tcp_hdr->offset)) << 2; /* 2: 左移2位 */
    if (tcp_hdr_len + ip_hdr_len == ip_pkt_len) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

/*****************************************************************************
 功能描述  : 为netbuf申请内存
 输入参数  : ul_size: 分配内存的大小
             l_reserve: data跟指针头之间要预留的长度
             ul_align: 需要几字节对齐
 返 回 值  : 成功返回结构体指针；失败返回OAL_PTR_NULL
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_alloc(td_u32 ul_size, td_u32 l_reserve, td_u32 l_align)
{
    oal_netbuf_stru *pst_netbuf = TD_NULL;
    td_u32       ul_offset;

    /* 保证data部分的size不会再字节对齐后小于预先想分配的大小 */
    if (l_align) {
        ul_size += (l_align - 1);
    }

    if (NET_SKB_PAD < 64) {                         /* net skb pad less than 64 */
        pst_netbuf = dev_alloc_skb(ul_size + 32);   /* 预留32字节给头部使其满足需求 */
    } else {
        pst_netbuf = dev_alloc_skb(ul_size);
    }

    if (oal_unlikely(pst_netbuf == TD_NULL)) {
        return TD_NULL;
    }

    if (NET_SKB_PAD < 64) {                         /* net skb pad less than 64 */
        skb_reserve(pst_netbuf, l_reserve + 32);    /* 预留32字节给头部使其满足需求 */
    } else {
        skb_reserve(pst_netbuf, l_reserve);
    }

    if (l_align) {
        /* 计算为了能使4字节对齐的偏移量 */
        ul_offset = (td_u32)(((unsigned long)pst_netbuf->data) % (unsigned long)l_align);
        if (ul_offset) {
            skb_reserve(pst_netbuf, l_align - ul_offset);
        }
    }

    return pst_netbuf;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

