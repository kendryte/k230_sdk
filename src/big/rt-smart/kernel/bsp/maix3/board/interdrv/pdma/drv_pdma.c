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

#include <rtdbg.h>
#include "drv_pdma.h"
#include <stdbool.h>
#include <stdlib.h>
#include <cache.h>
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <rtdevice.h>
#include <riscv_io.h>
#include <rtdef.h>
#include "ioremap.h"
#include "board.h"
#include <stdio.h>
#include <page.h>


// static volatile pdma_reg_t *pdma_reg = (volatile pdma_reg_t *)(intptr_t)(PDMA_BASE_ADDR);

static size_t g_pdma_page_size[8];
static bool g_pdma_alloc_page[8];
/***************************************************************
* general cfg
***************************************************************/
static void pdma_ch_enable(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    pdma_reg->pdma_ch_en |= (1 << ch);
}

static void pdma_ch_disable(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    pdma_reg->pdma_ch_en &= (~(1 << ch));
}

static int pdma_interrupt_en(pdma_reg_t *pdma_reg, rt_uint8_t ch, rt_uint32_t mask)
{
    if (ch >= PDMA_CH_MAX)
        return -1;

    pdma_reg->dma_int_mask &= (~(mask << ch));

    return 0;
}

static int pdma_interrupt_mask(pdma_reg_t *pdma_reg, rt_uint8_t ch, rt_uint32_t mask)
{
    if (ch >= PDMA_CH_MAX)
        return -1;

    pdma_reg->dma_int_mask |= (mask << ch);

    return 0;
}

rt_uint32_t k_pdma_interrupt_stat(pdma_reg_t *pdma_reg)
{
    return pdma_reg->dma_int_stat;
}

void k_pdma_int_clear_all(pdma_reg_t *pdma_reg, rt_uint32_t clear)
{
    int ch = 0;
    rt_uint32_t reg;

    reg = pdma_reg->dma_int_stat;
    pdma_reg->dma_int_stat = clear;

    /* free llt */
    if (reg & 0xff) {
        for (ch = 0; ch < 8; ch++) {
            if ((reg >> ch) & 0x1) {
                k_pdma_llt_free(pdma_reg, ch);
            }
        }
    } 
}

int k_pdma_int_clear(pdma_reg_t *pdma_reg, rt_uint8_t ch, rt_uint32_t clear)
{
    if (ch >= PDMA_CH_MAX)
        return -1;

    pdma_reg->dma_int_stat = (clear << ch);

    if (clear == PDONE_INT) {
        k_pdma_llt_free(pdma_reg, ch);
    }

    return 0;
}

void k_pdma_start(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    pdma_reg->pdma_ch_reg[ch].ch_ctl = 0x1;
}

void k_pdma_stop(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    pdma_reg->pdma_ch_reg[ch].ch_ctl = 0x2;
}

void k_pdma_resume(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    pdma_reg->pdma_ch_reg[ch].ch_ctl = 0x4;
}

bool jamlnik_pdma_is_busy(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    if (pdma_reg->pdma_ch_reg[ch].ch_status & 0x1)
        return true;
    else
        return false;
}

bool k_pdma_is_pause(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    if (pdma_reg->pdma_ch_reg[ch].ch_status & 0x2)
        return true;
    else 
        return false;
}

static rt_uint32_t *pdma_llt_cal(usr_pdma_cfg_t pdma_cfg)
{
    int i;
    rt_uint32_t list_num;
    pdma_llt_t *llt_list;

    list_num = (pdma_cfg.line_size - 1) / PDMA_MAX_LINE_SIZE + 1;

    g_pdma_page_size[pdma_cfg.ch] = rt_page_bits(sizeof(pdma_llt_t) * list_num);
    g_pdma_alloc_page[pdma_cfg.ch] = true;
    llt_list = (pdma_llt_t *)rt_pages_alloc(g_pdma_page_size[pdma_cfg.ch]);

    for (i = 0; i < list_num; i++) {
        llt_list[i].src_addr = ((rt_uint32_t)(intptr_t)pdma_cfg.src_addr + PDMA_MAX_LINE_SIZE*i);
        llt_list[i].dst_addr = ((rt_uint32_t)(intptr_t)pdma_cfg.dst_addr + PDMA_MAX_LINE_SIZE*i);

        if (i == list_num -1) {
            llt_list[i].line_size = pdma_cfg.line_size % PDMA_MAX_LINE_SIZE;
            llt_list[i].next_llt_addr = 0;
        } else {
            llt_list[i].line_size = PDMA_MAX_LINE_SIZE;
            llt_list[i].next_llt_addr = (rt_uint32_t)(intptr_t)(&llt_list[i+1]);
        }

        llt_list[i].pause = 0;
    }


    rt_hw_cpu_dcache_clean_flush((uint64_t*)llt_list, sizeof(pdma_llt_t)*list_num);

    // return (rt_uint32_t *)llt_list;
    return (void *)((char *)llt_list + PV_OFFSET);
}

int k_pdma_config(pdma_reg_t *pdma_reg, usr_pdma_cfg_t pdma_cfg)
{
    rt_uint8_t ch = pdma_cfg.ch;

    if (ch >= PDMA_CH_MAX)
        return -1;

    k_pdma_stop(pdma_reg, ch);
    while (pdma_reg->pdma_ch_reg[ch].ch_status & 0x1)
        ;
    pdma_ch_enable(pdma_reg,ch);
    k_pdma_int_clear(pdma_reg, ch, PDONE_INT | PITEM_INT | PPAUSE_INT | PTOUT_INT);

    pdma_reg->pdma_ch_reg[ch].ch_cfg = pdma_cfg.pdma_ch_cfg;
    pdma_reg->ch_peri_dev_sel[ch] = pdma_cfg.device;
    pdma_reg->pdma_ch_reg[ch].ch_llt_saddr = (rt_uint32_t)(intptr_t)pdma_llt_cal(pdma_cfg);

    return 0;
}

void k_pdma_llt_free(pdma_reg_t *pdma_reg, rt_uint8_t ch)
{
    rt_uint32_t *llt_list;
    llt_list = (rt_uint32_t *)(intptr_t)pdma_reg->pdma_ch_reg[ch].ch_llt_saddr;
    if (g_pdma_alloc_page[ch])
    {
        rt_pages_free(llt_list, g_pdma_page_size[ch]);
        g_pdma_alloc_page[ch] = false;
        //rt_free(llt_list);
    }
}












