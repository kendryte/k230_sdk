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
#include <rtdevice.h>

#include "drv_spi.h"
#include <drivers/spi.h>
#include <string.h>
#include "riscv_io.h"
#include "board.h"
#include "ioremap.h"
#include "sysctl_rst.h"
#include "sysctl_clk.h"

static csi_spidev_t spi_stand;

#define SPI_SUPPORT_CHK( x ) if( ( x ) == 0 ) {             \
    return (-1);  			            \
}

static int kd_spi_setup_device(csi_spidev_t *spidev)
{
    kd_spi_reg_t *addr = (kd_spi_reg_t *)(spidev->spi_.base_);
    addr->baudr = spidev->baud_rate_;
    addr->rx_sample_delay = spidev->rx_delay_;

    spi_imr_u imr;
    imr.data = 0;

    addr->imr = imr.data;
    addr->ser = 0x00;
    addr->ssienr = 0x00;
    addr->ctrlr0 = (spidev->mode_ << 8) | (spidev->frame_format_ << 22) | ((spidev->data_bit_length_ - 1) << 0);

    spi_txftlr_u txftlr;
    txftlr.data = 0;
    txftlr.txftlr.txfthr = 0;
    addr->txftlr = txftlr.data;
    addr->rxftlr = 128;

    return 0;
}

static int kd_spi_set_tmode(csi_spidev_t *spidev, int mode)
{

    kd_spi_reg_t *addr = (kd_spi_reg_t *)(spidev->spi_.base_);

    /* It is impossible to write to this register when the SSI is enabled.*/
    /* we can set the TMOD to config transfer mode as below:
     *     TMOD_BIT11  TMOD_BIT10    transfer mode
     *         0          0         transmit & receive
     *         0          1           transmit only
     *         1          0           receive only
     *         1          1           eeprom read
     */
    switch (mode) {
        case RT_SPI_MODE_0:
            addr->ctrlr0 &= (~DW_SPI_TMOD_BIT10);
            addr->ctrlr0 &= (~DW_SPI_TMOD_BIT11);
            break;

        case RT_SPI_MODE_1:
            addr->ctrlr0 |= DW_SPI_TMOD_BIT10;
            addr->ctrlr0 &= (~DW_SPI_TMOD_BIT11);
            break;

        case RT_SPI_MODE_2:
            addr->ctrlr0 &= (~DW_SPI_TMOD_BIT10);
            addr->ctrlr0 |= DW_SPI_TMOD_BIT11;
            break;

        default:
            addr->ctrlr0 |= DW_SPI_TMOD_BIT10;
            addr->ctrlr0 |= DW_SPI_TMOD_BIT11;
            break;
    }
    // addr->ctrlr0 |= BIT13;
    return 0;
}

static uint32_t get_buffer_width(size_t data_bit_length)
{
    if (data_bit_length <= 8)
        return 1;
    else if (data_bit_length <= 16)
        return 2;
    return 4;
}

static int kd_spi_initialize(csi_spidev_t *spidev, int32_t idx, spi_work_mode_e mode, spi_frame_mode_e frame_format, uint32_t chip_select_mask, uint32_t data_bit_length)
{
    spidev->mode_  = mode;
    spidev->frame_format_ = frame_format;
    spidev->chip_select_mask_  = chip_select_mask;
    spidev->data_bit_length_  = data_bit_length;
    spidev->buffer_width_ = get_buffer_width(data_bit_length);

    spidev->instruction_length_ = 0;
    spidev->address_length_ = 0;
    spidev->inst_width_ = 0;
    spidev->addr_width_ = 0;
    spidev->wait_cycles_ = 0;
    spidev->trans_mode_  = SPI_AITM_STANDARD;
    spidev->baud_rate_  = 40;
    spidev->buffer_width_ = get_buffer_width(data_bit_length);
    spidev->endian_  = 0;
    spidev->rx_delay_  = 2 | (0 << 16);
    spidev->addr_data_ddr_en_  = 0;
    spidev->inst_ddr_en_  = 0;
    spidev->ddr_drive_edge_ = 0;
    spidev->rxds_en_ = 0;
    spidev->xip_inst_en_ = 0;
    spidev->xip_dfs_hc_ = 1;
    spidev->ssic_xip_cont_xfer_en_= 0;
    spidev->is_first_config = true;

    return 0;
}

static rt_err_t drv_spi_configure(struct rt_spi_device *device,
                                  struct rt_spi_configuration *configuration)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t index;
    rt_uint32_t max_hz;
    rt_uint32_t val;
    rt_uint8_t mode = -1;
    rt_uint8_t width;
    struct rt_spi_bus *spi_bus = (struct rt_spi_bus *)device->bus;
    csi_spidev_t *spi_dev = (csi_spidev_t *)spi_bus->parent.user_data;
    index = spi_dev->spi_.idx_;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    if (configuration->data_width <= 8)
    {
        width = 8;   /* data_width */
    }
    else
    {
        return -RT_EIO;
    }

    max_hz = configuration->max_hz;
    if (max_hz > 100 * 1000000)
        max_hz = 100 * 1000000;     /* baudrate */

    rt_uint32_t spi_clock;

    if (index == 0)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_0_CLK);
    else if (index == 1)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_1_CLK);
    else if (index == 2)
        spi_clock = sysctl_clk_get_leaf_freq(SYSCTL_CLK_SSI_2_CLK);

    rt_uint32_t div = spi_clock / max_hz;
    if (div & 1)
        div ++;

    switch (configuration->mode & RT_SPI_MODE_3) /* mode */
    {
    case RT_SPI_MODE_0:
        mode = 0;
        break;
    case RT_SPI_MODE_1:
        mode = 1;
        break;
    case RT_SPI_MODE_2:
        mode = 2;
        break;
    case RT_SPI_MODE_3:
        mode = 3;
        break;
    }

    spi_dev->data_bit_length_ = width;
    spi_dev->baud_rate_ = div;

    kd_spi_setup_device(spi_dev);

    if(mode >= 0)
        kd_spi_set_tmode(spi_dev, mode);

    return ret;
}

static rt_uint32_t drv_spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct rt_spi_bus *spi_bus = (struct rt_spi_bus *)device->bus;
    csi_spidev_t *spi_dev = (csi_spidev_t *)spi_bus->parent.user_data;
    kd_spi_reg_t *addr = (kd_spi_reg_t *)(spi_dev->spi_.base_);
    struct rt_spi_configuration * config = &device->config;
    uint32_t ret = 0;

    static uint8_t * buffer_read;
    static const uint8_t * tx_buffer;
    static uint32_t tx_length;
    uint32_t i = 0;

    if ((config->data_width <= 8) && (message->length > 0))
    {

        if(message->cs_take)
        {
            addr->ser = spi_dev->chip_select_mask_;
        }

        if (message->cs_take && message->cs_release)
        {
            if (message->send_buf)
            {
                size_t tx_buffer_len = message->length - (spi_dev->inst_width_ + spi_dev->addr_width_);
                size_t tx_frames = tx_buffer_len / spi_dev->buffer_width_;
                const uint8_t *buffer_write = message->send_buf;

                size_t index, fifo_len;
                addr->ssienr = 0x01;

                while (tx_frames)
                {
                    fifo_len = 256 - addr->txflr;
                    fifo_len = fifo_len < tx_frames ? fifo_len : tx_frames;
                    switch (spi_dev->buffer_width_)
                    {
                    case 4:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint32_t *)buffer_write)[i++];
                        break;
                    case 2:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint16_t *)buffer_write)[i++];
                        break;
                    default:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = buffer_write[i++];
                        break;
                    }
                    tx_frames -= fifo_len;
                }

                while ((addr->sr & 0x05) != 0x04);
            } else
            if (message->recv_buf)
            {
                size_t rx_buffer_len = message->length;
                uint8_t * buffer_read = message->recv_buf;
                SPI_SUPPORT_CHK(!(rx_buffer_len % spi_dev->buffer_width_))

                size_t rx_frames = rx_buffer_len / spi_dev->buffer_width_;

                addr->ctrlr1 = rx_frames - 1;
                addr->ssienr = 0x01;

                if (spi_dev->frame_format_ == SPI_FF_STANDARD)
                {
                    addr->dr[0] = 0xFFFFFFFF;
                }

                size_t index, fifo_len;

                while (rx_frames)
                {
                    fifo_len = addr->rxflr;
                    fifo_len = fifo_len < rx_frames ? fifo_len : rx_frames;
                    switch (spi_dev->buffer_width_)
                    {
                    case 4:
                        for (index = 0; index < fifo_len; index++)
                            ((uint32_t *)buffer_read)[i++] = addr->dr[0];
                        break;
                    case 2:
                        for (index = 0; index < fifo_len; index++)
                            ((uint16_t *)buffer_read)[i++] = (uint16_t)addr->dr[0];
                        break;
                    default:
                        for (index = 0; index < fifo_len; index++)
                            buffer_read[i++] = (uint8_t)addr->dr[0];
                        break;
                    }
                    rx_frames -= fifo_len;
                }
            }
        } else
        if (message->cs_take && !message->cs_release)
        {
            if (message->send_buf)
            {
                tx_buffer = message->send_buf;
                tx_length = message->length;
            }
        } else
        if (!message->cs_take && message->cs_release)
        {
            if (message->recv_buf)
            {
                size_t tx_buffer_len = tx_length;
                size_t rx_buffer_len = message->length;
                SPI_SUPPORT_CHK(!(spi_dev->frame_format_ != SPI_FF_STANDARD))
                SPI_SUPPORT_CHK(!(tx_buffer_len % spi_dev->buffer_width_) && !(rx_buffer_len % spi_dev->buffer_width_))
                size_t tx_frames = tx_buffer_len / spi_dev->buffer_width_;
                size_t rx_frames = rx_buffer_len / spi_dev->buffer_width_;
                uint8_t * buffer_read = message->recv_buf;
                const uint8_t * buffer_write = tx_buffer;
                size_t index, fifo_len;
                addr->ctrlr1 = rx_frames - 1;
                addr->ssienr = 0x01;

                while (tx_frames)
                {
                    fifo_len = 256 - addr->txflr;
                    fifo_len = fifo_len < tx_frames ? fifo_len : tx_frames;
                    switch (spi_dev->buffer_width_)
                    {
                    case 4:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint32_t *)buffer_write)[i++];
                        break;
                    case 2:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint16_t *)buffer_write)[i++];
                        break;
                    default:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = buffer_write[i++];
                        break;
                    }
                    tx_frames -= fifo_len;
                }
                i = 0;
                while (rx_frames)
                {
                    fifo_len = addr->rxflr;
                    fifo_len = fifo_len < rx_frames ? fifo_len : rx_frames;
                    switch (spi_dev->buffer_width_)
                    {
                    case 4:
                        for (index = 0; index < fifo_len; index++)
                            ((uint32_t *)buffer_read)[i++] = addr->dr[0];
                        break;
                    case 2:
                        for (index = 0; index < fifo_len; index++)
                            ((uint16_t *)buffer_read)[i++] = (uint16_t)addr->dr[0];
                        break;
                    default:
                        for (index = 0; index < fifo_len; index++)
                            buffer_read[i++] = (uint8_t)addr->dr[0];
                        break;
                    }
                    rx_frames -= fifo_len;
                }
            } else if (message->send_buf)
            {
                uint8_t *data = (uint8_t*)rt_malloc(tx_length + message->length);
                if (!data)
                {
                    rt_kprintf(" spi malloc space falied!\n");
                    return -RT_ENOMEM;
                }
                rt_memcpy(data, tx_buffer, tx_length);
                rt_memcpy(data + tx_length, message->send_buf, message->length);

                size_t tx_buffer_len = tx_length + message->length - (spi_dev->inst_width_ + spi_dev->addr_width_);
                SPI_SUPPORT_CHK(!(tx_buffer_len % spi_dev->buffer_width_));
                size_t tx_frames = tx_buffer_len / spi_dev->buffer_width_;

                const uint8_t *buffer_write = data;
                size_t index, fifo_len;
                addr->ssienr = 0x01;

                while (tx_frames)
                {
                    fifo_len = 256 - addr->txflr;
                    fifo_len = fifo_len < tx_frames ? fifo_len : tx_frames;
                    switch (spi_dev->buffer_width_)
                    {
                    case 4:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint32_t *)buffer_write)[i++];
                        break;
                    case 2:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = ((uint16_t *)buffer_write)[i++];
                        break;
                    default:
                        for (index = 0; index < fifo_len; index++)
                            addr->dr[0] = buffer_write[i++];
                        break;
                    }
                    tx_frames -= fifo_len;
                }

                while ((addr->sr & 0x05) != 0x04);
                rt_free(data);
            }
        }

        if(message->cs_release)
        {
            addr->ser = 0x0;
            addr->ssienr = 0x0;
        }
    }

    return message->length;
}

const static struct rt_spi_ops kd_spi_ops =
{
    drv_spi_configure,
    drv_spi_xfer
};

int rt_hw_spi_bus_init(void)
{
    rt_err_t ret;
    static struct rt_spi_bus spi_bus1;
    spi_stand.spi_.base_ = rt_ioremap((void *)SPI_QOPI_BASE_ADDR, SPI_QOPI_IO_SIZE);
    spi_stand.spi_.idx_ = 1;
    spi_stand.baud_rate_ = 40;
    spi_stand.rx_delay_ = 2;

    kd_spi_initialize(&spi_stand, 1, SPI_FORMAT_CPOL0_CPHA0, SPI_FF_STANDARD, 0xf, 8);

    spi_bus1.parent.user_data = (void *)&spi_stand;
    ret = rt_spi_bus_register(&spi_bus1, "spi1", &kd_spi_ops);
#ifndef RT_FASTBOOT
    rt_kprintf("register spi[1] bus OK.\n");
#endif
    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_spi_bus_init);

