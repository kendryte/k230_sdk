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
#include <riscv_io.h>
#include "ioremap.h"
#include "drv_otp.h"
#include "board.h"
#include "sbi.h"
#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>
#endif

#include <rthw.h>

static struct rt_device g_otp_device = {0};
void *otp_base_addr = RT_NULL;
void *otp_bypass_addr = RT_NULL;

static rt_uint32_t be2le(rt_uint32_t var)
{
    return (((0xff000000 & var) >> 24) |
            ((0x00ff0000 & var) >> 8) |
            ((0x0000ff00 & var) << 8) |
            ((0x000000ff & var) << 24));
}

static rt_bool_t sysctl_boot_get_otp_bypass(void)
{
    if(readl(otp_bypass_addr) & (0x1 << 4))
        return RT_TRUE;
    else
        return RT_FALSE;
}

static rt_bool_t otp_get_status(k_otp_status_e eStatus)
{
    if(OTP_BYPASS_STATUS == eStatus)
        return sysctl_boot_get_otp_bypass();
    else
        return RT_FALSE;
}

rt_size_t otp_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_uint32_t *outbuf = (rt_uint32_t *)buffer;
    rt_uint32_t wlen = size / WORD_SIZE;
    rt_uint32_t word;
    rt_uint32_t ret = (uint32_t)size;
    rt_uint32_t i = 0;
    void *otp_read_addr = (void *)((char *)otp_base_addr + OTP_READ_OFFSET);

    if(RT_TRUE == otp_get_status(OTP_BYPASS_STATUS))
        return -1;

    if((0x300 <= pos) || (0x300 < size) || (0x0 >= size) || (0x300 < (pos + size)))
        return -1;

    for(i=0; i<wlen; i++)
    {
        outbuf[i] = readl(otp_read_addr + pos + i*WORD_SIZE);
        outbuf[i] = be2le(outbuf[i]);
    }

    if(size % WORD_SIZE != 0)
    {
        outbuf += wlen * WORD_SIZE;
        word = readl(otp_read_addr + pos + wlen*WORD_SIZE);
        word = be2le(word);
        memcpy(outbuf, &word, size % WORD_SIZE);
        ret = (uint32_t)(size - size % WORD_SIZE + WORD_SIZE);

        return ret;
    }

    return ret;
}

rt_size_t otp_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_uint8_t *inbuf = (rt_uint8_t *)buffer;
    rt_uint32_t ret = (uint32_t)size;
    rt_uint32_t i = 0, j = 0;
    void *otp_write_addr = (void *)((char *)otp_base_addr + OTP_WRITE_OFFSET + pos);

    if(RT_TRUE == otp_get_status(OTP_BYPASS_STATUS))
        return -1;

    if((0x300 <= pos) || (0x300 < size) || (0x0 >= size)|| (0x300 < (pos + size)))
        return -1;

    // 1. config pmp reg for writing OTP2
    sbi_pmp_write_enable(1);

    // 2. write OTP2
    for (i=0; i<size; i+=4)
    {
        union {
            rt_uint32_t word;
            rt_uint8_t byte[4];
        } otp_word;
        for (int8_t j=3; j>=0; j--)
            otp_word.byte[j] = ((i+3-j) < size) ? inbuf[i+3-j] : 0x00;

        writel(otp_word.word, (rt_uint32_t*)otp_write_addr+i);
    }
    
    // 3. config pmp reg for read only
    sbi_pmp_write_enable(0);

    return (j * WORD_SIZE);
}

const static struct rt_device_ops otp_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    otp_device_read,
    otp_device_write,
    RT_NULL
};

int rt_hw_otp_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_device_t otp_device = &g_otp_device;

    otp_base_addr = rt_ioremap((void*)SECURITY_BASE_ADDR, SECURITY_IO_SIZE);
    otp_bypass_addr = rt_ioremap((void*)BOOT_BASE_ADDR, BOOT_IO_SIZE);
    if((otp_base_addr == RT_NULL) || (otp_bypass_addr == RT_NULL)) {
        rt_kprintf("otp ioremap error\n");
        return -1;
    }

    ret = rt_device_register(otp_device, "otp", RT_DEVICE_FLAG_RDWR);
    if(ret) {
        rt_kprintf("otp device register fail\n");
        return ret;
    }

    otp_device->ops = &otp_ops;
#ifndef RT_FASTBOOT
    if(!ret)
        rt_kprintf("%s OK\n", __func__);
#endif
    return ret;
}
INIT_BOARD_EXPORT(rt_hw_otp_init);
