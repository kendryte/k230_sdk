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
#ifndef _CPU_PLATFORM_H_
#define _CPU_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
/*****************************************************************************
 * Device Specific Peripheral Registers structures
 ****************************************************************************/

#define __I                     volatile const	/* 'read only' permissions      */
#define __O                     volatile        /* 'write only' permissions     */
#define __IO                    volatile        /* 'read / write' permissions   */

/* k510 */
/*****************************************************************************
 * Memory Map
 ****************************************************************************/

#define _IO_(addr)              (addr)

#define USER_ALLOC_ADDR             (0x80230000U)
#define PATCH_CODE_ADDR             (0x80250000U)
#define FIRMWARE_HEAD_ADDR          (0x80230000U)
#define FIRMWARE_CODE_ADDR          (0x80300000U)
#define PLAINCODE_ADDR              (0x80300000U-4) // |4B firmware version|
#define CIPHERCODE_ADDR             (0x80380000U-4)
#define FIRMWARE_CODE_MAX_LEN       (512*1024)
#define SRAM_START_ADDRESS          (0x80000000U)
/* SMARTH */
#define PLIC_BASE_ADDR              (0x0f00000000UL)                          /*!< PLIC Base Address */

#define UART0_BASE_ADDR     		(0x91400000U)
#define UART1_BASE_ADDR     		(0x91401000U)
#define UART2_BASE_ADDR     		(0x91402000U)
#define UART3_BASE_ADDR     		(0x91403000U)
#define UART4_BASE_ADDR     		(0x91404000U)

#define SPI0_BASE_ADDR     			(0x91584000U)
#define SPI1_BASE_ADDR     			(0x91582000U)
#define SPI2_BASE_ADDR     			(0x91583000U)

/* I2C */
#define I2C0_BASE_ADDR              (0x91405000U)
#define I2C1_BASE_ADDR              (0x91406000U)
#define I2C2_BASE_ADDR              (0x91407000U)
#define I2C3_BASE_ADDR              (0x91408000U)
#define I2C4_BASE_ADDR              (0x91409000U)

/* IOMUX */
#define IOMUX_BASE_ADDR             (0x91105000U)

/* TIMER */
#define TIMER_BASE_ADDR            (0x91105800U)

/* WDT */
#define WDT0_BASE_ADDR  (0x91106000U)
#define WDT1_BASE_ADDR  (0x91106800U)

/* USB */
#define USB0_BASE_ADDR  (0x91500000U)
#define USB1_BASE_ADDR  (0x91540000U)

/* SDIO */
#define SDIO0_BASE_ADDR  (0x91580000U)
#define SDIO1_BASE_ADDR  (0x91581000U)

/* SYSCTL */
#define SYSCTL_CLK_BASE_ADDR    (0x91100000U)
#define SYSCTL_BOOT_BASE_ADDR   (0x91102000U)
#define SYSCTL_PWR_BASE_ADDR    (0x91103000U)
#define SYSCTL_RST_BASE_ADDR    (0x91101000U)
#define SYSCTL_STC_BASE_ADDR    (0x91108000U)


/*ugzip*/
#define GSDMA_CTRL_ADDR  0x80800000ULL
#define UGZIP_BASE_ADDR         0x80808000ULL
#define SDMA_CH_CFG  0x80800050ULL

#define GPIO_BASE_ADDR0     (0x9140B000U)
#define GPIO_BASE_ADDR1     (0x9140C000U)

/* puf */
#define PUFIOT_ADDR_START     0x91210000
#define PUFIOT_MAP_SIZE           0x6000

#define DMA_ADDR_OFFSET            0x000
#define CRYPTO_ADDR_OFFSET         0x100
#define SP38A_ADDR_OFFSET          0x200
#define CMAC_ADDR_OFFSET           0x220
#define SP38C_ADDR_OFFSET          0x240
#define SP38D_ADDR_OFFSET          0x260
#define SP38E_ADDR_OFFSET          0x280
#define KWP_ADDR_OFFSET            0x300
#define CHACHA_ADDR_OFFSET         0x400
#define HMAC_HASH_ADDR_OFFSET      0x800
#define KDF_ADDR_OFFSET            0x900
#define SP90A_ADDR_OFFSET          0xB00
#define KA_ADDR_OFFSET             0xC00
#define PKC_ADDR_OFFSET           0x1000
#define RT_ADDR_OFFSET            0x3000
#define CDE_ADDR_OFFSET           0x4000 

#define OSPI_CS			(14)
#define OSPI_CLK		(15)
#define OSPI_D0			(16)
#define OSPI_D1			(17)
#define OSPI_D2			(18)
#define OSPI_D3			(19)
#define OSPI_D4			(20)
#define OSPI_D5			(21)
#define OSPI_D6			(22)
#define OSPI_D7			(23)
#define OSPI_DQS		(24)

#define QSPI0_CS4		(50)
#define QSPI0_CS3		(51)
#define QSPI0_CS2		(60)
#define QSPI0_CS1		(61)

#define QSPI0_CONF0_CS0		(54)
#define QSPI0_CONF0_CLK		(55)
#define QSPI0_CONF0_D0		(56)
#define QSPI0_CONF0_D1		(57)
#define QSPI0_CONF0_D2		(58)
#define QSPI0_CONF0_D3		(59)

#define QSPI0_CONF1_CS0		(14)
#define QSPI0_CONF1_CLK		(15)
#define QSPI0_CONF1_D0		(16)
#define QSPI0_CONF1_D1		(17)
#define QSPI0_CONF1_D2		(18)
#define QSPI0_CONF1_D3		(19)

#define SD_CLK			(26)
#define SD_CMD			(27)
#define SD_D0			(28)
#define SD_D1			(29)
#define SD_D2			(30)
#define SD_D3			(31)

#define DEBUG_UART_TXD  (38)
#define DEBUG_UART_RXD  (39)

#define HS_REG_BASE (0x91585000UL)
#define SD0_CTRL       (HS_REG_BASE + 0x00)
#define USB0_TEST_CTL0 (HS_REG_BASE + 0x70)
#define USB0_TEST_CTL1 (HS_REG_BASE + 0x74)
#define USB0_TEST_CTL2 (HS_REG_BASE + 0x78)
#define USB0_TEST_CTL3 (HS_REG_BASE + 0x7c)
#define USB0_TEST_CTL4 (HS_REG_BASE + 0x80)
#define USB0_TEST_CTL5 (HS_REG_BASE + 0x84)
#define USB0_TEST_CTL6 (HS_REG_BASE + 0x88)

#define USB1_TEST_CTL0 (HS_REG_BASE + 0x90)
#define USB1_TEST_CTL1 (HS_REG_BASE + 0x94)
#define USB1_TEST_CTL2 (HS_REG_BASE + 0x98)
#define USB1_TEST_CTL3 (HS_REG_BASE + 0x9c)
#define USB1_TEST_CTL4 (HS_REG_BASE + 0xa0)
#define USB1_TEST_CTL5 (HS_REG_BASE + 0xa4)
#define USB1_TEST_CTL6 (HS_REG_BASE + 0xa8)

#define REG_USB0_TEST_CTL2         REG32(USB0_TEST_CTL2)
#define REG_USB0_TEST_CTL3         REG32(USB0_TEST_CTL3)
#define REG_USB1_TEST_CTL3         REG32(USB1_TEST_CTL3)

#define kendryte_enable_otg_phy() (REG_USB0_TEST_CTL2 |= 1)

#ifdef __cplusplus
}
#endif

#endif	/* _CPU_PLATFORM_H_ */
