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

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

typedef struct _uart
{
    union
    {
        volatile uint32_t RBR;
        volatile uint32_t DLL;
        volatile uint32_t THR;
    };

    union
    {
        volatile uint32_t DLH;
        volatile uint32_t IER;
    };

    union
    {
        volatile uint32_t FCR;
        volatile uint32_t IIR;
    };

    volatile uint32_t LCR;
    volatile uint32_t MCR;
    volatile uint32_t LSR;
    volatile uint32_t MSR;

    volatile uint32_t SCR;
    volatile uint32_t LPDLL;
    volatile uint32_t LPDLH;

    volatile uint32_t reserved1[2];

    union
    {
        volatile uint32_t SRBR[16];
        volatile uint32_t STHR[16];
    };

    volatile uint32_t FAR;
    volatile uint32_t TFR;
    volatile uint32_t RFW;
    volatile uint32_t USR;
    volatile uint32_t TFL;
    volatile uint32_t RFL;
    volatile uint32_t SRR;
    volatile uint32_t SRTS;
    volatile uint32_t SBCR;
    volatile uint32_t SDMAM;
    volatile uint32_t SFE;
    volatile uint32_t SRT;
    volatile uint32_t STET;
    volatile uint32_t HTX;
    volatile uint32_t DMASA;
    volatile uint32_t TCR;
    volatile uint32_t DE_EN;
    volatile uint32_t RE_EN;
    volatile uint32_t DET;
    volatile uint32_t TAT;
    volatile uint32_t DLF;
    volatile uint32_t RAR;
    volatile uint32_t TAR;
    volatile uint32_t LCR_EXT;
    volatile uint32_t reserved2[9];
    volatile uint32_t CPR;
    volatile uint32_t UCV;
    volatile uint32_t CTR;
} uart_t;

typedef enum _uart_receive_trigger
{
    UART_RECEIVE_FIFO_1,
    UART_RECEIVE_FIFO_8,
    UART_RECEIVE_FIFO_16,
    UART_RECEIVE_FIFO_30,
} uart_receive_trigger_t;

struct uart_configure
{
    rt_uint32_t baud_rate;

    rt_uint32_t data_bits               :4;
    rt_uint32_t stop_bits               :2;
    rt_uint32_t parity                  :2;
    rt_uint32_t fifo_lenth              :2;
    rt_uint32_t auto_flow               :1;
    rt_uint32_t reserved                :21;
};

#endif /* __DRV_UART_H__ */
