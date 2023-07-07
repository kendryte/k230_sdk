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
#ifndef __SYSCTL_H__
#define __SYSCTL_H__

#include "platform.h"
#include <asm/io.h>
// #include "common.h"
#define REG32(REG)  readl((const volatile void __iomem *)REG)
typedef enum
{
    SYSCTL_BOOT_NORFLASH        = 0,
    SYSCTL_BOOT_NANDFLASH       = 1,
    SYSCTL_BOOT_SDIO0           = 2,
    SYSCTL_BOOT_SDIO1           = 3,
    SYSCTL_BOOT_MAX,
}sysctl_boot_mode_e;

#ifdef BUILD_FPGA
#define DEFAULT_CPU0_CLK    (50*MHz)
#define DEFAULT_SDIO0_CCLK  (50*MHz) 
#define DEFAULT_SDIO1_CCLK  (25*MHz)
#define DEFAULT_OSPI_CLK    (50*MHz)
#define DEFAULT_QSPI_CLK    (50*MHz)
#define DEFAULT_UART_CLK    (25*MHz)
#else
#define DEFAULT_CPU0_CLK    (800*MHz)
#define DEFAULT_SDIO0_CCLK  (200*MHz)
#define DEFAULT_SDIO1_CCLK  (100*MHz)
#define DEFAULT_OSPI_CLK    (800*MHz)
#define DEFAULT_QSPI_CLK    (200*MHz)
#define DEFAULT_UART_CLK    (50*MHz)
#endif

#define USB_RST_CTL             (SYSCTL_RST_BASE_ADDR + 0x3c)
#define REG_USB_RST_CTL         REG32(USB_RST_CTL)

#define SDC_RST_CTL             (SYSCTL_RST_BASE_ADDR + 0x34)
#define REG_SDC_RST_CTL         REG32(SDC_RST_CTL)

#define SPI_RST_CTL             (SYSCTL_RST_BASE_ADDR + 0x44)
#define REG_SPI_RST_CTL         REG32(SPI_RST_CTL)

#define LOSYS_RST_CTL           (SYSCTL_RST_BASE_ADDR + 0x24)
#define REG_LOSYS_RST_CTL       REG32(LOSYS_RST_CTL)

#define SEC_RST_CTL             (SYSCTL_RST_BASE_ADDR + 0x4c)
#define REG_SEC_RST_CTL         REG32(SEC_RST_CTL)

#define SOC_BOOT_CTL  (SYSCTL_BOOT_BASE_ADDR + 0x40)
#define REG_SOC_BOOT_CTL         REG32(SOC_BOOT_CTL)

// typedef sysctl_boot_mode_e (*func_sysctl_boot_get_boot_mode)(void);

// extern func_sysctl_boot_get_boot_mode cb_sysctl_boot_get_boot_mode;
extern sysctl_boot_mode_e sysctl_boot_get_boot_mode(void);
#endif