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

#ifndef DRV_SPI_H__
#define DRV_SPI_H__
#include <stdint.h>
#include <stdbool.h>

#define DW_SPI_TMOD_BIT10       0x0400
#define DW_SPI_TMOD_BIT11       0x0800

typedef enum {
    PLL0_DIV2_800M = 0,
    PLL2_DIV4_667M
} spi_ssi0_clk_sel_e;

typedef enum {
    SPI_AITM_STANDARD = 0,
    SPI_AITM_ADDR_STANDARD,
    SPI_AITM_AS_FRAME_FORMAT
} spi_inst_addr_trans_mode_e;

typedef enum {
    SPI_FF_STANDARD = 0,
    SPI_FF_DUAL,
    SPI_FF_QUAD,
    SPI_FF_OCTAL,
} spi_frame_mode_e;

/*----- SPI Control Codes: Mode Parameters: Frame Format -----*/
typedef enum {
    SPI_FORMAT_CPOL0_CPHA0 = 0, ///< Clock Polarity 0, Clock Phase 0
    SPI_FORMAT_CPOL0_CPHA1,     ///< Clock Polarity 0, Clock Phase 1
    SPI_FORMAT_CPOL1_CPHA0,     ///< Clock Polarity 1, Clock Phase 0
    SPI_FORMAT_CPOL1_CPHA1,     ///< Clock Polarity 1, Clock Phase 1
} spi_work_mode_e;

typedef struct {
    void *base_;
    int32_t idx_;
} dw_spi_priv_t;

typedef struct {
    dw_spi_priv_t spi_;
    spi_work_mode_e mode_;
    spi_frame_mode_e frame_format_;
    uint32_t chip_select_mask_;
    uint32_t data_bit_length_;
    uint32_t instruction_length_;
    uint32_t address_length_;
    uint32_t inst_width_;
    uint32_t addr_width_;
    uint32_t wait_cycles_;
    spi_inst_addr_trans_mode_e trans_mode_;
    uint32_t baud_rate_;
    spi_ssi0_clk_sel_e sel_;
    uint32_t buffer_width_;
    uint32_t endian_;
    uint32_t rx_delay_;
    uint32_t addr_data_ddr_en_;
    uint32_t inst_ddr_en_;
    uint32_t ddr_drive_edge_;
    uint32_t rxds_en_;
    uint32_t xip_inst_en_;
    uint32_t xip_dfs_hc_;
    uint32_t ssic_xip_cont_xfer_en_;
    bool is_first_config;
} csi_spidev_t;


typedef struct _spi_
{
    /* SPI Control Register 0                                    (0x00)*/
    volatile uint32_t ctrlr0;
    /* SPI Control Register 1                                    (0x04)*/
    volatile uint32_t ctrlr1;
    /* SPI Enable Register                                       (0x08)*/
    volatile uint32_t ssienr;
    /* SPI Microwire Control Register                            (0x0c)*/
    volatile uint32_t mwcr;
    /* SPI Slave Enable Register                                 (0x10)*/
    volatile uint32_t ser;
    /* SPI Baud Rate Select                                      (0x14)*/
    volatile uint32_t baudr;
    /* SPI Transmit FIFO Threshold Level                         (0x18)*/
    volatile uint32_t txftlr;
    /* SPI Receive FIFO Threshold Level                          (0x1c)*/
    volatile uint32_t rxftlr;
    /* SPI Transmit FIFO Level Register                          (0x20)*/
    volatile uint32_t txflr;
    /* SPI Receive FIFO Level Register                           (0x24)*/
    volatile uint32_t rxflr;
    /* SPI Status Register                                       (0x28)*/
    volatile uint32_t sr;
    /* SPI Interrupt Mask Register                               (0x2c)*/
    volatile uint32_t imr;
    /* SPI Interrupt Status Register                             (0x30)*/
    volatile uint32_t isr;
    /* SPI Raw Interrupt Status Register                         (0x34)*/
    volatile uint32_t risr;
    /* SPI Transmit FIFO Underflow Interrupt Clear Register      (0x38)*/
    volatile uint32_t txeicr;
    /* SPI Receive FIFO Overflow Interrupt Clear Register        (0x3c)*/
    volatile uint32_t rxoicr;
    /* SPI Receive FIFO Underflow Interrupt Clear Register       (0x40)*/
    volatile uint32_t rxuicr;
    /* SPI Multi-Master Interrupt Clear Register                 (0x44)*/
    volatile uint32_t msticr;
    /* SPI Interrupt Clear Register                              (0x48)*/
    volatile uint32_t icr;
    /* SPI DMA Control Register                                  (0x4c)*/
    volatile uint32_t dmacr;
#if SSIC_HAS_DMA == 1
    /* SPI DMA Transmit Data Level                               (0x50)*/
    volatile uint32_t dmatdlr;
    /* SPI DMA Receive Data Level                                (0x54)*/
    volatile uint32_t dmardlr;
#elif SSIC_HAS_DMA == 2
    /* SPI Destination Burst Length                              (0x50)*/
    volatile uint32_t axiawlen;
    /* SPI Source Burst Length                                   (0x54)*/
    volatile uint32_t axiarlen;
#else
    uint32_t resv0[2];
#endif
    /* SPI Identification Register                               (0x58)*/
    volatile const uint32_t idr;
    /* SPI DWC_ssi component version                             (0x5c)*/
    volatile uint32_t ssic_version_id;
    /* SPI Data Register 0-36                                    (0x60 -- 0xec)*/
    volatile uint32_t dr[36];
    /* SPI RX Sample Delay Register                              (0xf0)*/
    volatile uint32_t rx_sample_delay;
    /* SPI SPI Control Register                                  (0xf4)*/
    volatile uint32_t spi_ctrlr0;
    /* SPI Transmit Drive Edge Register                          (0xf8)*/
    volatile uint32_t ddr_drive_edge;
    /* SPI XIP Mode bits                                         (0xfc)*/
    volatile uint32_t xip_mode_bits;
    /* SPI XIP INCR transfer opcode                              (0x100)*/
    volatile uint32_t xip_incr_inst;
    /* SPI XIP WRAP transfer opcode                              (0x104)*/
    volatile uint32_t xip_wrap_inst;
#if SSIC_CONCURRENT_XIP_EN
    /* SPI XIP Control Register                                  (0x108)*/
    volatile uint32_t xip_ctrl;
    /* SPI XIP Slave Enable Register                             (0x10c)*/
    volatile uint32_t xip_ser;
    /* SPI XIP Receive FIFO Overflow Interrupt Clear Register    (0x110)*/
    volatile uint32_t xrxoicr;
    /* SPI XIP time out register for continuous transfers        (0x114)*/
    volatile uint32_t xip_cnt_time_out;
    /* not support dyn ws                                        (0x118)*/
    uint32_t resv1[1];
    /* SPI Transmit Error Interrupt Clear Register               (0x11c)*/
    volatile uint32_t spitecr;
#else
    uint32_t resv1[6];
#endif
#if SSIC_HAS_DMA == 2
    /* SPI Device Register                                       (0x120)*/
    volatile uint32_t spidr;
    /* SPI Device Address Register                               (0x124)*/
    volatile uint32_t spiar;
    /* AXI Address Register 0                                    (0x128)*/
    volatile uint32_t axiar0;
    /* AXI Address Register 1                                    (0x12c)*/
    volatile uint32_t axiar1;
    /* AXI Master Error Interrupt Clear Register                 (0x130)*/
    volatile uint32_t axiecr;
    /* Transfer Done Clear Interrupt Clear Register              (0x134)*/
    volatile uint32_t donecr;
#endif
    /* resv                                                      (0x138 ~ 0x13c)*/
    uint32_t resv3[2];
#if SSIC_XIP_WRITE_REG_EN
    /* XIP_WRITE_INCR_INST - XIP Write INCR transfer opcode      (0x140)*/
    volatile uint32_t xip_write_incr_inst;
    /* XIP_WRITE_WRAP_INST - XIP Write WRAP transfer opcode      (0x144)*/
    volatile uint32_t xip_write_wrap_inst;
    /* XIP_WRITE_CTRL - XIP Write Control Register               (0x148)*/
    volatile uint32_t xip_write_ctrl;
#else
    uint32_t resv4[3];
#endif
    // volatile uint32_t endian;
} __attribute__((packed, aligned(4))) kd_spi_reg_t;



typedef struct _spi_imr_t
{
    uint32_t txeim:1;//Transmit FIFO Empty Interrupt Mask.
    uint32_t txoim:1;//Transmit FIFO Overflow Interrupt Mask.
    uint32_t rxuim:1;//Receive FIFO Underflow Interrupt Mask.
    uint32_t rxoim:1;//Receive FIFO Overflow Interrupt Mask.
    uint32_t rxfim:1;//Receive FIFO Full Interrupt Mask.
    uint32_t mstim:1;
    uint32_t rsvd0:1;
    uint32_t txuim:1;//Transmit FIFO Underflow Interrupt Mask.
    uint32_t axiem:1;//AXI Error Interrupt Mask.
    uint32_t rsvd1:2;
    uint32_t donem:1;//SSI Done Interrupt Mask.
    uint32_t rsvd2:20;
} __attribute__((packed, aligned(4))) spi_imr_t;

typedef union _spi_imr_u
{
    spi_imr_t imr;
    uint32_t data;
} spi_imr_u;


typedef struct _spi_txftlr_t
{
    //When the number of transmit FIFO entries is less than orequal to this value, the transmit FIFO empty interrupt is triggered.
    uint32_t tft:8;
    uint32_t rsvd0:8;
    //In Internal DMA mode, this field sets the minimum amount of data frames present in the FIFO after which DWC_ssi starts the transfer.
    uint32_t txfthr:11;
    uint32_t rsvd1:5;
} __attribute__((packed, aligned(4))) spi_txftlr_t;

typedef union _spi_txftlr_u
{
    spi_txftlr_t txftlr;
    uint32_t data;
} spi_txftlr_u;


typedef struct _spi_spi_ctrlr0_t
{
    uint32_t trans_type:2;//Address and instruction transfer format..
    uint32_t addr_l:4;//This bit defines Length of Address to be transmitted.
    uint32_t rsvd0:1;
    uint32_t xip_md_bit_en:1;
    uint32_t inst_l:2;//Dual/Quad/Octal mode instruction length in bits.
    uint32_t rsvd1:1;
    uint32_t wait_cycles:5;//Receive FIFO Full Interrupt Status.
    uint32_t spi_ddr_en:1;
    uint32_t inst_ddr_en:1;
    uint32_t spi_rxds_en:1;
    uint32_t xip_dfs_hc:1;
    uint32_t xip_inst_en:1;
    uint32_t ssic_xip_cont_xfer_en:1;
    uint32_t rsvd2:4;
    uint32_t xip_mbl:2;
    uint32_t rsvd3:4;
} __attribute__((packed, aligned(4))) spi_spi_ctrlr0_t;

typedef union _spi_spi_ctrlr0_u
{
    spi_spi_ctrlr0_t spi_ctrlr0;
    uint32_t data;
} spi_spi_ctrlr0_u;

#endif