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

#include <stdint.h>
#include <rtthread.h>
#include <drivers/mtd_nand.h>
#include <drivers/spi.h>

#define DBG_TAG "winbond nand"
#ifdef RT_SPI_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_WARNING
#endif
#include <rtdbg.h>

static struct rt_mtd_nand_device nand_dev;
static struct rt_qspi_device spi_nand_dev;

static rt_err_t standard_transfer(uint8_t* in, uint8_t* out, uint32_t size)
{
    struct rt_qspi_message msg = {
        .parent.cs_take = 0,
        .parent.cs_release = 0,
        .parent.next = NULL,
        .parent.send_buf = in,
        .parent.recv_buf = out,
        .parent.length = size,
        .instruction.content = 0,
        .instruction.size = 0,
        .instruction.qspi_lines = 0,
        .address.content = 0,
        .address.size = 0,
        .address.qspi_lines = 0,
        .dummy_cycles = 0,
        .qspi_data_lines = 1,
    };

    if (0 > rt_qspi_transfer_message(&spi_nand_dev, &msg))
        return -RT_EIO;
    return 0;
}

static rt_err_t device_reset(void)
{
    rt_err_t ret;
    uint8_t buf[1] = { 0xFF };

    ret = standard_transfer(buf, NULL, 1);

    return ret;
}

static rt_err_t read_id(void)
{
    rt_err_t ret;
    uint8_t buf[5] = { 0x9F };

    ret = standard_transfer(buf, buf, 5);
    if (ret == 0)
        ret = ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 8) | buf[4];

    return ret;
}

static rt_err_t read_status_reg(uint8_t reg)
{
    rt_err_t ret;
    uint8_t buf[3] = { 0x0F, reg };

    ret = standard_transfer(buf, buf, 3);
    if (ret == 0)
        ret = buf[2];

    return ret;
}

static rt_err_t write_status_reg(uint8_t reg, uint8_t value)
{
    rt_err_t ret;
    uint8_t buf[3] = { 0x1F, reg, value };

    ret = standard_transfer(buf, NULL, 3);
    if (ret == 0)
        ret = buf[2];

    return ret;
}

static rt_err_t write_enable(void)
{
    rt_err_t ret;
    uint8_t buf[1] = { 0x06 };

    ret = standard_transfer(buf, NULL, 1);

    return ret;
}

static rt_err_t write_disable(void)
{
    rt_err_t ret;
    uint8_t buf[1] = { 0x04 };

    ret = standard_transfer(buf, NULL, 1);

    return ret;
}

static rt_err_t block_erase(uint32_t page)
{
    rt_err_t ret;
    uint8_t buf[4] = { 0xD8, page >> 16, page >> 8, page };

    ret = standard_transfer(buf, NULL, 4);

    return ret;
}

static rt_err_t page_data_read(uint32_t page)
{
    rt_err_t ret;
    uint8_t buf[4] = { 0x13, page >> 16, page >> 8, page };

    ret = standard_transfer(buf, NULL, 4);

    return ret;
}

static rt_err_t page_read(uint32_t offset, uint8_t* data, uint32_t size)
{
    rt_err_t ret;

    struct rt_qspi_message msg = {
        .parent.cs_take = 0,
        .parent.cs_release = 0,
        .parent.next = NULL,
        .parent.send_buf = NULL,
        .parent.recv_buf = data,
        .parent.length = size,
        .instruction.content = 0xEB,
        .instruction.size = 8,
        .instruction.qspi_lines = 1,
        .address.content = offset,
        .address.size = 16,
        .address.qspi_lines = 4,
        .dummy_cycles = 4,
        .qspi_data_lines = 4,
    };

    if (0 > rt_qspi_transfer_message(&spi_nand_dev, &msg))
        return -RT_EIO;
    return 0;

    return ret;
}

static rt_err_t program_execute(uint32_t page)
{
    rt_err_t ret;
    uint8_t buf[4] = { 0x10, page >> 16, page >> 8, page };

    ret = standard_transfer(buf, NULL, 4);

    return ret;
}

static rt_err_t page_write(uint32_t offset, uint8_t* data, uint32_t size, uint8_t random)
{
    rt_err_t ret;

    struct rt_qspi_message msg = {
        .parent.cs_take = 0,
        .parent.cs_release = 0,
        .parent.next = NULL,
        .parent.send_buf = data,
        .parent.recv_buf = NULL,
        .parent.length = size,
        .instruction.content = random ? 0x34 : 0x32,
        .instruction.size = 8,
        .instruction.qspi_lines = 1,
        .address.content = offset,
        .address.size = 16,
        .address.qspi_lines = 1,
        .dummy_cycles = 0,
        .qspi_data_lines = 4,
    };

    if (0 > rt_qspi_transfer_message(&spi_nand_dev, &msg))
        return -RT_EIO;
    return 0;

    return ret;
}

static rt_err_t wait_idle(uint32_t ms)
{
    rt_err_t ret;
    int retry = ms;

    do {
        ret = read_status_reg(0xC0);
        if (ret < 0)
            return ret;
        if ((ret & 1) == 0)
            return ret;
        if (ms)
            rt_thread_mdelay(1);
    } while (retry--);
    if (ms)
        LOG_E("wait idle timeout\n");

    return -RT_ETIMEOUT;
}

static rt_err_t nand_read_id(struct rt_mtd_nand_device* device)
{
    return read_id();
}

static rt_err_t nand_read_page(struct rt_mtd_nand_device* device, rt_off_t page,
    rt_uint8_t* data, rt_uint32_t data_len,
    rt_uint8_t* spare, rt_uint32_t spare_len)
{
    rt_err_t ret;
    uint8_t oob[device->oob_size];

    wait_idle(1);
    page_data_read(page);
    while (wait_idle(0) == -RT_ETIMEOUT)
        ;
    ret = read_status_reg(0xC0);
    if (ret & 0x20)
        return -2;
    if (data)
        page_read(0, data, data_len);
    if (spare) {
        page_read(device->page_size, oob, device->oob_size);
        spare[0] = 0xFF;
        if (device->plane_num == 0) {
            rt_memcpy(spare + 1, oob + 4, 4);
            rt_memcpy(spare + 5, oob + 20, 4);
            rt_memcpy(spare + 9, oob + 36, 1);
        } else if (device->plane_num == 1) {
            rt_memcpy(spare + 1, oob + 4, 9);
        }
    }

    return 0;
}

static rt_err_t nand_write_page(struct rt_mtd_nand_device* device, rt_off_t page,
    const rt_uint8_t* data, rt_uint32_t data_len,
    const rt_uint8_t* spare, rt_uint32_t spare_len)
{
    rt_err_t ret;
    uint8_t oob[device->oob_size];

    wait_idle(1);
    write_enable();
    if (data)
        page_write(0, data, data_len, 0);
    if (spare) {
        rt_memset(oob, 0xff, sizeof(oob));
        if (device->plane_num == 0) {
            rt_memcpy(oob + 4, spare + 1, 4);
            rt_memcpy(oob + 20, spare + 5, 4);
            rt_memcpy(oob + 36, spare + 9, 1);
        } else if (device->plane_num == 1) {
            rt_memcpy(oob + 4, spare + 1, 9);
        }
        page_write(device->page_size, oob, device->oob_size, 1);
    }
    program_execute(page);
    wait_idle(2);
    ret = read_status_reg(0xC0);
    if (ret & 0x08)
        return -2;

    return 0;
}

static rt_err_t nand_erase_block(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    rt_err_t ret;

    wait_idle(1);
    write_enable();
    block_erase(block * device->pages_per_block);
    wait_idle(10);
    ret = read_status_reg(0xC0);
    if (ret & 0x04)
        return -2;

    return 0;
}

static rt_err_t nand_check_block(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    uint8_t oob[device->oob_size];

    wait_idle(1);
    page_data_read(block * device->pages_per_block);
    while (wait_idle(0) == -RT_ETIMEOUT)
        ;
    page_read(device->page_size, oob, device->oob_size);
    if (oob[0] != 0xFF)
        return 1;

    return 0;
}

static rt_err_t nand_mark_badblock(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    rt_err_t ret;
    uint8_t oob[device->oob_size];

    wait_idle(1);
    write_enable();
    rt_memset(oob, 0, sizeof(oob));
    page_write(device->page_size, oob, device->oob_size, 0);
    program_execute(block * device->pages_per_block);
    wait_idle(1);
    ret = read_status_reg(0xC0);
    if (ret & 0x08)
        return -2;

    return 0;
}

static struct rt_mtd_nand_driver_ops ops = {
    .read_id = nand_read_id,
    .read_page = nand_read_page,
    .write_page = nand_write_page,
    .move_page = NULL,
    .erase_block = nand_erase_block,
    .check_block = nand_check_block,
    .mark_badblock = nand_mark_badblock,
};

static int winbond_nand_init(void)
{
    int ret;

    struct rt_qspi_configuration cfg = {
        .parent.mode = 0,
        .parent.cs_mode = 0,
        .parent.cs = 0,
        .parent.data_width = 8,
        .parent.max_hz = 100000000,
        .ddr_mode = 0,
        .medium_size = 0,
        .qspi_dl_width = 4,
    };

    ret = rt_spi_bus_attach_device(&spi_nand_dev, "winbond_nand", RT_USING_WINBOND_NAND_DEV, NULL);
    if (ret != RT_EOK) {
        LOG_E("winbond_nand attach %s failed!\n", RT_USING_WINBOND_NAND_DEV);
        return ret;
    }

    ret = rt_qspi_configure(&spi_nand_dev, &cfg);
    if (ret != RT_EOK) {
        LOG_E("winbond_nand configure failed!\n");
        return ret;
    }

    wait_idle(10);
    device_reset();
    wait_idle(1);
    write_status_reg(0xA0, 0);

    uint32_t id = read_id();
    if (id == 0xEFBA21) {
        nand_dev.plane_num = 0;
        nand_dev.oob_size = 64;
        nand_dev.oob_free = 64;
        nand_dev.block_start = 160;
        nand_dev.block_end = 1023;
    } else if (id == 0xEFBA22) {
        nand_dev.plane_num = 1;
        nand_dev.oob_size = 64;
        nand_dev.oob_free = 64;
        nand_dev.block_start = 160;
        nand_dev.block_end = 2047;
    } else if (id == 0xEFBA23) {
        nand_dev.plane_num = 1;
        nand_dev.oob_size = 64;
        nand_dev.oob_free = 64;
        nand_dev.block_start = 160;
        nand_dev.block_end = 4095;
    }

    nand_dev.page_size = 2048;
    nand_dev.pages_per_block = 64;
    nand_dev.block_total = nand_dev.block_end - nand_dev.block_start;
    nand_dev.ops = &ops;
    ret = rt_mtd_nand_register_device("nand0", &nand_dev);

    return ret;
}
INIT_COMPONENT_EXPORT(winbond_nand_init);
