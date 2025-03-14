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

#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "drivers/spi.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "riscv_io.h"
#include "board.h"
#include "ioremap.h"
#include "sysctl_rst.h"
#include "sysctl_clk.h"
#include "cache.h"

#define DBG_TAG "spi"
#ifdef RT_SPI_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_WARNING
#endif
#include <rtdbg.h>

#define SSIC_HAS_DMA 2
#define SSIC_AXI_BLW 8
#define SSIC_TX_ABW 256
#define SSIC_RX_ABW 256

#define IRQN_SPI0 146
#define IRQN_SPI1 155
#define IRQN_SPI2 164

#define CACHE_ALIGN_TOP(x) (((x) + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1))
#define CACHE_ALIGN_BOTTOM(x) ((x) & ~(L1_CACHE_BYTES - 1))
#define BIT(n) (1UL << (n))

enum {
    SSI_TXE = 0,
    SSI_TXO,
    SSI_RXF,
    SSI_RXO,
    SSI_TXU,
    SSI_RXU,
    SSI_MST,
    SSI_DONE,
    SSI_AXIE,
};

enum {
    SPI_FRF_STD_SPI,
    SPI_FRF_DUAL_SPI,
    SPI_FRF_QUAD_SPI,
    SPI_FRF_OCT_SPI,
};

enum {
    SPI_TMOD_TR,
    SPI_TMOD_TO,
    SPI_TMOD_RO,
    SPI_TMOD_EPROMREAD,
};

typedef struct {
    struct rt_spi_bus dev;
    void* base;
    uint8_t idx;
    uint8_t rdse;
    uint8_t rdsd;
    uint8_t max_line;
    uint32_t max_hz;
    struct rt_event event;
    void *send_buf;
    void *recv_buf;
    rt_size_t send_length;
    rt_size_t recv_length;
    uint8_t cell_size;
} dw_spi_bus_t;

typedef struct {
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
} __attribute__((packed, aligned(4))) dw_spi_reg_t;

static rt_err_t drv_spi_configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);
    dw_spi_bus_t* spi_bus = (dw_spi_bus_t*)device->bus;
    dw_spi_reg_t* spi = (dw_spi_reg_t*)spi_bus->base;
    struct rt_qspi_device* dev = (struct rt_qspi_device*)device;
    struct rt_qspi_configuration* cfg = (struct rt_qspi_configuration*)configuration;
    uint8_t idx = spi_bus->idx;
    uint8_t dfs, mode;
    uint32_t max_hz, spi_clock, div;

    if (cfg->qspi_dl_width > spi_bus->max_line || cfg->qspi_dl_width == 0)
        return -RT_EINVAL;

    if (configuration->data_width < 4 || configuration->data_width > 32)
        return -RT_EINVAL;
    dfs = configuration->data_width - 1;

    max_hz = configuration->max_hz;
    if (max_hz > spi_bus->max_hz)
        max_hz = spi_bus->max_hz;
    if (idx == 0)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_0_CLK);
    else if (idx == 1)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_1_CLK);
    else if (idx == 2)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_2_CLK);
    else
        return -RT_EINVAL;
    div = spi_clock / max_hz;

    mode = configuration->mode & RT_SPI_MODE_3;

    if (configuration->cs_mode == RT_SPI_SOFT_CS) {
        kd_pin_mode(configuration->cs_pin, GPIO_DM_OUTPUT);
        kd_pin_write(configuration->cs_pin, configuration->mode & RT_SPI_CS_HIGH ? GPIO_PV_LOW : GPIO_PV_HIGH);
    }

    spi->ssienr = 0;
    spi->ser = 0;
    spi->baudr = div;
    spi->rx_sample_delay = spi_bus->rdse << 16 | spi_bus->rdsd;
    spi->axiawlen = SSIC_AXI_BLW << 8;
    spi->axiarlen = SSIC_AXI_BLW << 8;
    spi->ctrlr0 = (dfs) | (mode << 8);

    rt_memcpy(&dev->config, cfg, sizeof(struct rt_qspi_configuration));

    return 0;
}

static rt_uint32_t drv_spi_xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    dw_spi_bus_t* spi_bus = (dw_spi_bus_t*)device->bus;
    dw_spi_reg_t* spi = (dw_spi_reg_t*)spi_bus->base;
    struct rt_qspi_device* dev = (struct rt_qspi_device*)device;
    struct rt_qspi_message* msg = (struct rt_qspi_message*)message;
    struct rt_qspi_configuration* cfg = (struct rt_qspi_configuration*)&dev->config;

    if (msg->qspi_data_lines > 1) { // 多线模式
        uint8_t spi_ff;
        if (msg->qspi_data_lines > cfg->qspi_dl_width) {
            LOG_E("data line is invalid");
            return 0;
        }
        if (msg->qspi_data_lines == 2)
            spi_ff = SPI_FRF_DUAL_SPI;
        else if (msg->qspi_data_lines == 4)
            spi_ff = SPI_FRF_QUAD_SPI;
        else if (msg->qspi_data_lines == 8)
            spi_ff = SPI_FRF_OCT_SPI;
        else {
            LOG_E("data line is invalid");
            return 0;
        }
        if (cfg->parent.data_width & (msg->qspi_data_lines - 1)) {
            LOG_E("data line and data width do not match");
            return 0;
        }
        if (msg->instruction.size & 3 || msg->instruction.size > 16 || msg->instruction.size == 12) {
            LOG_E("instruction size is invalid");
            return 0;
        }
        if (msg->instruction.size && msg->instruction.qspi_lines != 1 && msg->instruction.qspi_lines != msg->qspi_data_lines) {
            LOG_E("instruction line is invalid");
            return 0;
        }
        if (msg->address.size & 3 || msg->address.size > 32) {
            LOG_E("address size is invalid");
            return 0;
        }
        if (msg->address.size && msg->address.qspi_lines != 1 && msg->address.qspi_lines != msg->qspi_data_lines) {
            LOG_E("address line is invalid");
            return 0;
        }
        if (msg->instruction.size + msg->address.size == 0) {
            LOG_E("at least one instruction or address is required");
            return 0;
        }
        if (msg->parent.length > 0x10000) {
            LOG_E("data length is invalid, more than 0x10000");
            return 0;
        }
        uint8_t trans_type = 0;
        if (msg->instruction.size && msg->instruction.qspi_lines != 1)
            trans_type = 2;
        if (msg->address.size) {
            if (msg->address.qspi_lines != 1)
                trans_type = trans_type ? trans_type : 1;
            else if (trans_type != 0) {
                LOG_E("instruction or address line is invalid");
                return 0;
            }
        }
        if (msg->dummy_cycles > 31) {
            LOG_E("dummy cycle is invalid");
            return 0;
        }
        uint8_t tmod = msg->parent.recv_buf ? SPI_TMOD_RO : SPI_TMOD_TO;
        uint8_t inst_l = __builtin_ffs(msg->instruction.size);
        rt_size_t length = msg->parent.length;
        rt_size_t txfthr = length > (SSIC_TX_ABW / 2) ? (SSIC_TX_ABW / 2) : length - 1;
        uint8_t cell_size = (cfg->parent.data_width + 7) >> 3;
        uint8_t* buf = NULL;
        if (length) {
            buf = rt_malloc_align(CACHE_ALIGN_TOP(length * cell_size), L1_CACHE_BYTES);
            if (buf == NULL) {
                LOG_E("alloc mem error");
                return 0;
            }
        }
        spi->spi_ctrlr0 = trans_type | (msg->address.size >> 2 << 2) | ((inst_l ? inst_l - 2 : 0) << 8) | (msg->dummy_cycles << 11);
        spi->ctrlr0 &= ~((3 << 22) | (3 << 10));
        if (length) {
            spi->ctrlr0 |= (spi_ff << 22) | (tmod << 10);
            spi->txftlr = (txfthr << 16) | (SSIC_TX_ABW / 2);
            spi->rxftlr = (SSIC_RX_ABW - 1);
            spi->imr = (1 << 11) | (1 << 8);
            spi->dmacr = (1 << 6) | (3 << 3) | (1 << 2);
            spi->ctrlr1 = length - 1;
            spi->spidr = msg->instruction.content;
            spi->spiar = msg->address.content;
            if (tmod == 1) {
                rt_memcpy(buf, msg->parent.send_buf, length * cell_size);
                rt_hw_cpu_dcache_clean(buf, CACHE_ALIGN_TOP(length * cell_size));
            }
            spi->axiar0 = (uint32_t)((uint64_t)buf);
            spi->axiar1 = (uint32_t)((uint64_t)buf >> 32);
        } else {
            tmod = SPI_TMOD_TO;
            spi->ctrlr0 |= (spi_ff << 22) | (tmod << 10);
            spi->txftlr = ((SSIC_TX_ABW - 1) << 16) | (SSIC_TX_ABW - 1);
            spi->rxftlr = (SSIC_RX_ABW - 1);
            spi->imr = 0;
            spi->dmacr = 0;
        }
        if (cfg->parent.cs_mode == RT_SPI_SOFT_CS && msg->parent.cs_take)
            kd_pin_write(cfg->parent.cs_pin, cfg->parent.mode & RT_SPI_CS_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
        rt_event_control(&spi_bus->event, RT_IPC_CMD_RESET, 0);
        spi->ser = 1 << cfg->parent.cs;
        spi->ssienr = 1;
        rt_uint32_t event;
        rt_err_t err;
        if (length) {
            err = rt_event_recv(&spi_bus->event, BIT(SSI_DONE) | BIT(SSI_AXIE),
                RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 1000, &event);
        } else {
            err = 0;
            event = 0;
            if (msg->instruction.size) {
                spi->dr[0] = msg->instruction.content;
                length++;
            }
            if (msg->address.size) {
                spi->dr[0] = msg->address.content;
                length++;
            }
            spi->txftlr = 0;
            while ((spi->sr & 0x5) != 0x4);
        }
        spi->ser = 0;
        spi->ssienr = 0;
        if (cfg->parent.cs_mode == RT_SPI_SOFT_CS && msg->parent.cs_release)
            kd_pin_write(cfg->parent.cs_pin, cfg->parent.mode & RT_SPI_CS_HIGH ? GPIO_PV_LOW : GPIO_PV_HIGH);
        if (err == -RT_ETIMEOUT) {
            LOG_E("spi%d transfer data timeout", spi_bus->idx);
            length = 0;
            goto multi_exit;
        }
        if (event & BIT(SSI_AXIE)) {
            LOG_E("spi%d dma error", spi_bus->idx);
            length = 0;
            goto multi_exit;
        }
        if (tmod == SPI_TMOD_RO) {
            rt_hw_cpu_dcache_invalidate(buf, CACHE_ALIGN_TOP(length * cell_size));
            rt_memcpy(msg->parent.recv_buf, buf, length * cell_size);
        }
    multi_exit:
        if (buf)
            rt_free_align(buf);
        return length;
    } else { // 单线模式
        if (cfg->parent.cs_mode == RT_SPI_SOFT_CS && msg->parent.cs_take)
            kd_pin_write(cfg->parent.cs_pin, cfg->parent.mode & RT_SPI_CS_HIGH ? GPIO_PV_HIGH : GPIO_PV_LOW);
        if (msg->parent.length == 0)
            goto single_exit;
        uint8_t cell_size = (cfg->parent.data_width + 7) >> 3;
        rt_size_t length = msg->parent.length;
        rt_size_t count = length > 0x10000 ? 0x10000 : length;
        rt_size_t send_single = 0, send_length = 0, recv_single = 0, recv_length = 0;
        void *send_buf = msg->parent.send_buf;
        void *recv_buf = msg->parent.recv_buf;
        uint8_t tmod = send_buf ? SPI_TMOD_TO : SPI_TMOD_EPROMREAD;
        tmod = recv_buf ? tmod & SPI_TMOD_RO : tmod;
        if (tmod == SPI_TMOD_EPROMREAD) {
            LOG_E("send_buf and recv_buf cannot both be empty");
            return 0;
        }
        if (tmod == SPI_TMOD_RO && cfg->parent.data_width == 8) {
            if ((msg->instruction.size & 7) || (msg->address.size & 7) || (msg->dummy_cycles & 7)) {
                LOG_E("instruction, address, dummy_cycles invalid");
                LOG_E("For read-only mode the instruction, address, dummy_cycles must be set to zero");
                LOG_E("For eeprom-read mode the instruction, address, dummy_cycles must be set to multiples of 8");
                return 0;
            } else if (msg->instruction.size || msg->address.size) {
                if (length > 0x10000) {
                    LOG_E("For eeprom-read mode, data length cannot exceed 0x10000");
                    return 0;
                }
                tmod = SPI_TMOD_EPROMREAD;
            }
        }
        if (send_buf) {
            send_single = count;
            send_buf = rt_malloc(count * cell_size);
            if (send_buf == NULL) {
                LOG_E("alloc mem error");
                return 0;
            }
            rt_memcpy(send_buf, msg->parent.send_buf, count * cell_size);
        } else if (tmod == SPI_TMOD_EPROMREAD) {
            send_single = msg->instruction.size / 8 + msg->address.size / 8 + msg->dummy_cycles / 8;
            send_buf = rt_malloc(send_single);
            if (send_buf == NULL) {
                LOG_E("alloc mem error");
                return 0;
            }
            uint8_t *temp = send_buf;
            for (int i = msg->instruction.size / 8; i; i--)
                *temp++ = msg->instruction.content >> ((i - 1) * 8);
            for (int i = msg->address.size / 8; i; i--)
                *temp++ = msg->address.content >> ((i - 1) * 8);
            for (int i = msg->dummy_cycles / 8; i; i--)
                *temp++ = 0xFF;
        }
        if (recv_buf) {
            recv_single = count;
            recv_buf = rt_malloc(count * cell_size);
            if (recv_buf == NULL) {
                LOG_E("alloc mem error");
                if (send_buf)
                    rt_free(send_buf);
                return 0;
            }
        }
        send_length = 0;
        recv_length = 0;
        spi_bus->cell_size = cell_size;
        spi_bus->send_buf = send_buf;
        spi_bus->recv_buf = recv_buf;
        spi_bus->send_length = send_single;
        spi_bus->recv_length = recv_single;
        spi->ctrlr0 &= ~((3 << 22) | (3 << 10));
        spi->ctrlr0 |= (tmod << 10);
        spi->ctrlr1 = count - 1;
        spi->txftlr = ((SSIC_TX_ABW / 2) << 16) | (SSIC_TX_ABW / 2);
        spi->rxftlr = count >= (SSIC_RX_ABW / 2) ? (SSIC_RX_ABW / 2 - 1) : count - 1;
        spi->dmacr = 0;
        spi->imr = (1 << 4) | (1 << 0);
        rt_event_control(&spi_bus->event, RT_IPC_CMD_RESET, 0);
        spi->ser = 1 << cfg->parent.cs;
        spi->ssienr = 1;
        if (tmod == SPI_TMOD_RO)
            spi->dr[0] = 0;
        rt_uint32_t event;
        rt_err_t err;
    loop:
        err = rt_event_recv(&spi_bus->event, BIT(SSI_TXE) | BIT(SSI_RXF),
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 10000, &event);
        if (err == -RT_ETIMEOUT) {
            LOG_E("spi%d transfer data timeout", spi_bus->idx);
            length = 0;
            goto single_error;
        }
        if (event & BIT(SSI_TXE)) {
            send_length += send_single;
            if (send_length < length && tmod <= SPI_TMOD_TO) {
                count = length - send_length;
                count = count > 0x10000 ? 0x10000 : count;
                rt_memcpy(send_buf, msg->parent.send_buf + send_length * cell_size, count * cell_size);
                spi_bus->send_buf = send_buf;
                spi_bus->send_length = count;
                send_single = count;
                spi->txftlr = ((SSIC_TX_ABW / 2) << 16) | (SSIC_TX_ABW / 2);
                if (tmod == SPI_TMOD_TO)
                    spi->imr |= (1<< 0);
            } else if (tmod == SPI_TMOD_TO) {
                while ((spi->sr & 0x5) != 0x4);
                goto single_error;
            }
        }
        if (event & BIT(SSI_RXF)) {
            rt_memcpy(msg->parent.recv_buf + recv_length * cell_size, recv_buf, recv_single * cell_size);
            recv_length += recv_single;
            if (recv_length >= length)
                goto single_error;
            count = length - recv_length;
            count = count > 0x10000 ? 0x10000 : count;
            spi_bus->recv_buf = recv_buf;
            spi_bus->recv_length = count;
            recv_single = count;
            spi->rxftlr = count >= (SSIC_RX_ABW / 2) ? (SSIC_RX_ABW / 2 - 1) : count - 1;
            if (tmod == SPI_TMOD_TR) {
                spi->imr |= (1<< 0) | (1 << 4);
            } else if (tmod == SPI_TMOD_RO) {
                spi->imr |= (1 << 4);
                spi->ssienr = 0;
                spi->ctrlr1 = count - 1;
                spi->ssienr = 1;
                spi->dr[0] = 0;
                spi->dr[0] = 0;
            }
        }
        goto loop;
    single_error:
        spi->ser = 0;
        spi->ssienr = 0;
        if (send_buf)
            rt_free(send_buf);
        if (recv_buf)
            rt_free(recv_buf);
    single_exit:
        if (cfg->parent.cs_mode == RT_SPI_SOFT_CS && msg->parent.cs_release)
            kd_pin_write(cfg->parent.cs_pin, cfg->parent.mode & RT_SPI_CS_HIGH ? GPIO_PV_LOW : GPIO_PV_HIGH);
        return length;
    }
    return 0;
}

static void spi_irq(int vector, void* param)
{
    dw_spi_bus_t* spi_bus = param;
    dw_spi_reg_t* spi = (dw_spi_reg_t*)spi_bus->base;
    vector -= IRQN_SPI0;
    vector %= (IRQN_SPI1 - IRQN_SPI0);
    if (vector == SSI_TXE) {
        if (spi_bus->send_buf == NULL) {
            spi->imr &= ~1;
        } else if (spi_bus->cell_size == 1) {
            while ((spi_bus->send_length) && (spi->sr & 2)) {
                spi->dr[0] = *((uint8_t *)spi_bus->send_buf);
                spi_bus->send_buf++;
                spi_bus->send_length--;
            }
        } else if (spi_bus->cell_size == 2) {
            while ((spi_bus->send_length) && (spi->sr & 2)) {
                spi->dr[0] = *((uint16_t *)spi_bus->send_buf);
                spi_bus->send_buf += 2;
                spi_bus->send_length--;
            }
        } else if (spi_bus->cell_size == 4) {
            while ((spi_bus->send_length) && (spi->sr & 2)) {
                spi->dr[0] = *((uint32_t *)spi_bus->send_buf);
                spi_bus->send_buf += 4;
                spi_bus->send_length--;
            }
        } else {
            LOG_E("spi%d datawidth error", spi_bus->idx);
        }
        if (spi_bus->send_length == 0) {
            if ((spi->ctrlr0 >> 10) & SPI_TMOD_EPROMREAD == SPI_TMOD_TO) {
                if (spi->txftlr)
                    return;
            }
            spi->txftlr = 0;
            spi->imr &= ~1;
            rt_event_send(&spi_bus->event, BIT(SSI_TXE));
        }
    } else if (vector == SSI_RXF) {
        if (spi_bus->recv_buf == NULL) {
            spi->imr &= ~0x10;
        } else if (spi_bus->cell_size == 1) {
            while ((spi_bus->recv_length) && (spi->sr & 8)) {
                *((uint8_t *)spi_bus->recv_buf) = spi->dr[0];
                spi_bus->recv_buf++;
                spi_bus->recv_length--;
            }
        } else if (spi_bus->cell_size == 2) {
            while ((spi_bus->recv_length) && (spi->sr & 8)) {
                *((uint16_t *)spi_bus->recv_buf) = spi->dr[0];
                spi_bus->recv_buf += 2;
                spi_bus->recv_length--;
            }
        } else if (spi_bus->cell_size == 4) {
            while ((spi_bus->recv_length) && (spi->sr & 8)) {
                *((uint32_t *)spi_bus->recv_buf) = spi->dr[0];
                spi_bus->recv_buf += 4;
                spi_bus->recv_length--;
            }
        } else {
            LOG_E("spi%d datawidth error", spi_bus->idx);
        }
        if (spi_bus->recv_length == 0) {
            spi->imr &= ~0x10;
            rt_event_send(&spi_bus->event, BIT(SSI_RXF));
        } else if (spi_bus->recv_length <= spi->rxftlr) {
            spi->rxftlr = spi_bus->recv_length - 1;
        }
    } else if (vector == SSI_DONE) {
        spi->donecr;
        rt_event_send(&spi_bus->event, BIT(SSI_DONE));
    } else if (vector == SSI_AXIE) {
        spi->axiecr;
        rt_event_send(&spi_bus->event, BIT(SSI_AXIE));
    }
}

static struct rt_spi_ops spi_ops = {
    drv_spi_configure,
    drv_spi_xfer
};

int rt_hw_spi_bus_init(void)
{
    rt_err_t ret;
#ifdef RT_USING_SPI0
    static dw_spi_bus_t spi_bus0;
    spi_bus0.base = rt_ioremap((void*)SPI_OPI_BASE_ADDR, SPI_OPI_IO_SIZE);
    spi_bus0.idx = 0;
    spi_bus0.rdse = 0;
    spi_bus0.rdsd = 6;
    spi_bus0.max_line = 8;
    spi_bus0.max_hz = 200000000;

    ret = rt_qspi_bus_register(&spi_bus0.dev, "spi0", &spi_ops);
    if (ret) {
        LOG_E("spi0 register fail");
        return ret;
    }
    rt_event_init(&spi_bus0.event, "spi0_event", RT_IPC_FLAG_PRIO);
    rt_hw_interrupt_install(IRQN_SPI0 + SSI_TXE, spi_irq, &spi_bus0, "spi0");
    rt_hw_interrupt_umask(IRQN_SPI0 + SSI_TXE);
    rt_hw_interrupt_install(IRQN_SPI0 + SSI_RXF, spi_irq, &spi_bus0, "spi0");
    rt_hw_interrupt_umask(IRQN_SPI0 + SSI_RXF);
    rt_hw_interrupt_install(IRQN_SPI0 + SSI_DONE, spi_irq, &spi_bus0, "spi0");
    rt_hw_interrupt_umask(IRQN_SPI0 + SSI_DONE);
    rt_hw_interrupt_install(IRQN_SPI0 + SSI_AXIE, spi_irq, &spi_bus0, "spi0");
    rt_hw_interrupt_umask(IRQN_SPI0 + SSI_AXIE);
#endif
#ifdef RT_USING_SPI1
    static dw_spi_bus_t spi_bus1;
    spi_bus1.base = rt_ioremap((void*)SPI_QOPI_BASE_ADDR, SPI_OPI_IO_SIZE);
    spi_bus1.idx = 1;
    spi_bus1.rdse = 0;
    spi_bus1.rdsd = 0;
    spi_bus1.max_line = 4;
    spi_bus1.max_hz = 100000000;

    ret = rt_qspi_bus_register(&spi_bus1.dev, "spi1", &spi_ops);
    if (ret) {
        LOG_E("spi1 register fail");
        return ret;
    }
    rt_event_init(&spi_bus1.event, "spi1_event", RT_IPC_FLAG_PRIO);
    rt_hw_interrupt_install(IRQN_SPI1 + SSI_TXE, spi_irq, &spi_bus1, "spi1");
    rt_hw_interrupt_umask(IRQN_SPI1 + SSI_TXE);
    rt_hw_interrupt_install(IRQN_SPI1 + SSI_RXF, spi_irq, &spi_bus1, "spi1");
    rt_hw_interrupt_umask(IRQN_SPI1 + SSI_RXF);
    rt_hw_interrupt_install(IRQN_SPI1 + SSI_DONE, spi_irq, &spi_bus1, "spi1");
    rt_hw_interrupt_umask(IRQN_SPI1 + SSI_DONE);
    rt_hw_interrupt_install(IRQN_SPI1 + SSI_AXIE, spi_irq, &spi_bus1, "spi1");
    rt_hw_interrupt_umask(IRQN_SPI1 + SSI_AXIE);
#endif
#ifdef RT_USING_SPI2
    static dw_spi_bus_t spi_bus2;
    spi_bus2.base = rt_ioremap((void*)SPI_QOPI_BASE_ADDR + SPI_OPI_IO_SIZE, SPI_OPI_IO_SIZE);
    spi_bus2.idx = 2;
    spi_bus2.rdse = 0;
    spi_bus2.rdsd = 0;
    spi_bus2.max_line = 4;
    spi_bus2.max_hz = 100000000;

    ret = rt_qspi_bus_register(&spi_bus2.dev, "spi2", &spi_ops);
    if (ret) {
        LOG_E("spi2 register fail");
        return ret;
    }
    rt_event_init(&spi_bus2.event, "spi2_event", RT_IPC_FLAG_PRIO);
    rt_hw_interrupt_install(IRQN_SPI2 + SSI_TXE, spi_irq, &spi_bus2, "spi2");
    rt_hw_interrupt_umask(IRQN_SPI2 + SSI_TXE);
    rt_hw_interrupt_install(IRQN_SPI2 + SSI_RXF, spi_irq, &spi_bus2, "spi2");
    rt_hw_interrupt_umask(IRQN_SPI2 + SSI_RXF);
    rt_hw_interrupt_install(IRQN_SPI2 + SSI_DONE, spi_irq, &spi_bus2, "spi2");
    rt_hw_interrupt_umask(IRQN_SPI2 + SSI_DONE);
    rt_hw_interrupt_install(IRQN_SPI2 + SSI_AXIE, spi_irq, &spi_bus2, "spi2");
    rt_hw_interrupt_umask(IRQN_SPI2 + SSI_AXIE);
#endif
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_spi_bus_init);
