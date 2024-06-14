/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-10-10     Tanek        first version
 * 2021-07-07     linzhenxing  add sd card drivers in mmu
 * 2021-07-14     linzhenxing  add emmc
 */

#include <rtthread.h>
#include <rthw.h>
#include <drivers/mmcsd_core.h>

#include "board.h"
#include "drv_sdhci.h"
#include "riscv_io.h"
#include <string.h>
#include <ioremap.h>
#ifdef RT_USING_SDIO

#define DBG_TAG "drv_sdhci"
#ifdef RT_SDIO_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_WARNING
#endif /* RT_SDIO_DEBUG */
#include <rtdbg.h>

#if defined(RT_USING_SDIO0) || defined(RT_USING_SDIO1)

#define CACHE_LINESIZE (64)

#define KD_MAX_FREQ (25UL * 1000UL * 1000UL)

#define USDHC_ADMA_TABLE_WORDS (8U) /* define the ADMA descriptor table length */
#define USDHC_ADMA2_ADDR_ALIGN (4U) /* define the ADMA2 descriptor table addr align size */
#define USDHC_DATA_TIMEOUT (0xFU) /*!< data timeout counter value */

#define REG_SD_EMMC_SEL 0x64 // bit0 for sd0/emmc0  bit1 for sd1/emmc1  (0:sd  1:emmc)
#define KD_SDMMC_VER_ID 0x2A
#define SDHCI_VENDOR_PTR_R 0xE8
#define SYS_CTRL_SDEMMC_DIV_CTRL 0x3C
#define SYS_CTRL_SDEMMC_CTRL_EN_CLR 0x68
#define REG_SDMMC_SOFTRST_SEL 0x4

#define SDHCI_DEFAULT_BOUNDARY_SIZE (512 * 1024)
#define SDHCI_DEFAULT_BOUNDARY_ARG (7)

/* Synopsys vendor specific registers */
#define reg_offset_addr_vendor (sdhci_readw(host, SDHCI_VENDOR_PTR_R))
#define SDHC_MHSC_VER_ID_R (reg_offset_addr_vendor)
#define SDHC_MHSC_VER_TPYE_R (reg_offset_addr_vendor + 0X4)
#define SDHC_MHSC_CTRL_R (reg_offset_addr_vendor + 0X8)
#define SDHC_MBIU_CTRL_R (reg_offset_addr_vendor + 0X10)
#define SDHC_EMMC_CTRL_R (reg_offset_addr_vendor + 0X2C)
#define SDHC_BOOT_CTRL_R (reg_offset_addr_vendor + 0X2E)
#define SDHC_GP_IN_R (reg_offset_addr_vendor + 0X30)
#define SDHC_GP_OUT_R (reg_offset_addr_vendor + 0X34)
#define SDHC_AT_CTRL_R (reg_offset_addr_vendor + 0X40)
#define SDHC_AT_STAT_R (reg_offset_addr_vendor + 0X44)

void* emmc0_base;
void* emmc1_base;

/* ENABLE CACHER . */
#define SDHCI_ENABLE_CACHE_CONTROL
#define SDHCI_SDMA_ENABLE

struct sdhci_64bit_adma2_descriptor adma2_tbl[32] __attribute__((aligned(32)));
struct rt_mmcsd_host* host1;
struct rt_mmcsd_host* host2;
static rt_mutex_t mmcsd_mutex = RT_NULL;

static rt_err_t sdhci_receive_command_response(struct sdhci_host* sdhci_host, struct sdhci_command* command);
static void sdhci_reset(struct sdhci_host* host, uint8_t mask);
extern void rt_hw_cpu_dcache_invalidate(void* addr, int size);
extern void rt_hw_cpu_dcache_clean(void* addr, int size);
extern void rt_hw_cpu_dcache_clean_and_invalidate(void* addr, int size);

static inline void sdhci_writel(struct sdhci_host* host, uint32_t val, int reg)
{
    writel(val, (void*)host->mapbase + reg);
}

static inline void sdhci_writew(struct sdhci_host* host, uint16_t val, int reg)
{
    writew((uint16_t)val, (void*)host->mapbase + reg);
}

static inline void sdhci_writeb(struct sdhci_host* host, uint8_t val, int reg)
{
    writeb((uint8_t)val, (void*)host->mapbase + reg);
}

static inline uint32_t sdhci_readl(struct sdhci_host* host, int reg)
{
    return (uint32_t)readl((void*)host->mapbase + reg);
}

static inline uint16_t sdhci_readw(struct sdhci_host* host, int reg)
{
    return (uint16_t)readw((void*)host->mapbase + reg);
}

static inline uint8_t sdhci_readb(struct sdhci_host* host, int reg)
{
    return (uint8_t)readb((void*)host->mapbase + reg);
}

void emmc_reg_display(struct sdhci_host* host)
{
    rt_kprintf("SD_MASA_R:%x\n", sdhci_readl(host, SDHCI_DMA_ADDRESS));
    rt_kprintf("BLCOKSIZE_R:%x\n", sdhci_readw(host, SDHCI_BLOCK_SIZE));
    rt_kprintf("BLOCKCOUNT_R:%x\n", sdhci_readw(host, SDHCI_BLOCK_COUNT));
    rt_kprintf("ARGUMENT_R:%x\n", sdhci_readl(host, SDHCI_ARGUMENT));
    rt_kprintf("XFER_MODE_R:%x\n", sdhci_readw(host, SDHCI_TRANSFER_MODE));
    rt_kprintf("CMD_R:%x\n", sdhci_readw(host, SDHCI_COMMAND));
    rt_kprintf("RESP0_R:%x\n", sdhci_readl(host, SDHCI_RESPONSE));
    rt_kprintf("RESP1_R:%x\n", sdhci_readl(host, SDHCI_RESPONSE + 4));
    rt_kprintf("RESP2_R:%x\n", sdhci_readl(host, SDHCI_RESPONSE + 8));
    rt_kprintf("RESP3_R:%x\n", sdhci_readl(host, SDHCI_RESPONSE + 12));
    rt_kprintf("BUF_DATA_R:%x\n", sdhci_readl(host, SDHCI_BUFFER));
    rt_kprintf("PSTATE_REG_R:%x\n", sdhci_readl(host, SDHCI_PRESENT_STATE));
    rt_kprintf("HOST_CTL_R:%x\n", sdhci_readb(host, SDHCI_HOST_CONTROL));
    rt_kprintf("PWR_CTRL_R:%x\n", sdhci_readb(host, SDHCI_POWER_CONTROL));
    rt_kprintf("BGAP_CTRL_R:%x\n", sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
    rt_kprintf("WUP_CTRL_R:%x\n", sdhci_readb(host, SDHCI_WAKE_UP_CONTROL));
    rt_kprintf("CLK_CTRL_R:%x\n", sdhci_readw(host, SDHCI_CLOCK_CONTROL));
    rt_kprintf("TOUT_CTRL_R:%x\n", sdhci_readb(host, SDHCI_TIMEOUT_CONTROL));
    rt_kprintf("SW_RSR_R:%x\n", sdhci_readb(host, SDHCI_SOFTWARE_RESET));
    rt_kprintf("NORMAL_INT_STAT_R:%x\n", sdhci_readw(host, SDHCI_INT_STATUS));
    rt_kprintf("ERROR_INT_STAT_R:%x\n", sdhci_readw(host, SDHCI_INT_STATUS + 2));
    rt_kprintf("NORMAL_INT_STAT_EN_R:%x\n", sdhci_readw(host, SDHCI_INT_ENABLE));
    rt_kprintf("ERROR_INT_STAT_EN_R:%x\n", sdhci_readw(host, SDHCI_INT_ENABLE + 2));
    rt_kprintf("NORNAL_INT_SIGNAL_EN_R:%x\n", sdhci_readw(host, SDHCI_SIGNAL_ENABLE));
    rt_kprintf("ERROR_INT_SIGNAL_EN_R:%x\n", sdhci_readw(host, SDHCI_SIGNAL_ENABLE + 2));
    rt_kprintf("AUTO_CMD_STAT_R:%x\n", sdhci_readw(host, SDHCI_AUTO_CMD_STATUS));
    rt_kprintf("HOST_CTRL2_R:%x\n", sdhci_readw(host, SDHCI_HOST_CONTROL2));
    rt_kprintf("CAPABILITIES1_R:%x\n", sdhci_readl(host, SDHCI_CAPABILITIES));
    rt_kprintf("CAPABILITIES2_R:%x\n", sdhci_readl(host, SDHCI_CAPABILITIES_1));
    rt_kprintf("FORCE_AUTO_CMD_STAT_R:%x\n", sdhci_readw(host, SDHCI_MAX_CURRENT));
    rt_kprintf("FORCE_ERROR_INT_STAT_R:%x\n", sdhci_readw(host, SDHCI_SET_ACMD12_ERROR));
    rt_kprintf("AMDA_ERR_STAT_STAT_R:%x\n", sdhci_readl(host, SDHCI_ADMA_ERROR));
    rt_kprintf("AMDA_SA_LOW_STAT_R:%x\n", sdhci_readl(host, SDHCI_ADMA_ADDRESS));
    rt_kprintf("AMDA_SA_HIGH_STAT_R:%x\n", sdhci_readl(host, SDHCI_ADMA_ADDRESS_HI));
}

void host_change(void);

static inline void delay_1k(unsigned int uicnt)
{
    int i, j;

    for (i = 0; i < uicnt; i++)
        for (j = 0; j < 1000; j++)
            asm("nop");
}

static void sdhci_reset(struct sdhci_host* host, uint8_t mask)
{
    unsigned long timeout;

    /* Wait max 100 ms */
    timeout = 100;
    sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
    while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
        if (timeout == 0) {
            LOG_E("%s: Reset 0x%x never completed.\n",
                __func__, (int)mask);
            return;
        }
        timeout--;
        delay_1k(1);
    }
}

static uint32_t sdhci_get_present_status_flag(struct sdhci_host* sdhci_host)
{
    return sdhci_readl(sdhci_host, SDHCI_PRESENT_STATE);
}

static uint32_t sdhci_get_int_status_flag(struct sdhci_host* sdhci_host)
{
    return sdhci_readl(sdhci_host, SDHCI_INT_STATUS);
}

static void sdhci_clear_int_status_flag(struct sdhci_host* sdhci_host, uint32_t mask)
{
    sdhci_writel(sdhci_host, mask, SDHCI_INT_STATUS);
}

static void sdhic_error_recovery(struct sdhci_host* sdhci_host)
{
    uint32_t status;
    /* get host present status */
    status = sdhci_get_present_status_flag(sdhci_host);
    /* check command inhibit status flag */
    if ((status & SDHCI_CMD_INHIBIT) != 0U) {
        /* reset command line */
        sdhci_reset(sdhci_host, SDHCI_RESET_CMD);
    }
    /* check data inhibit status flag */
    if ((status & SDHCI_DATA_INHIBIT) != 0U) {
        /* reset data line */
        sdhci_reset(sdhci_host, SDHCI_RESET_DATA);
    }
}

static void sdhci_interrupt_init(struct sdhci_host* sdhci_host)
{
    /* Enable only interrupts served by the SD controller */
    sdhci_writel(sdhci_host, (SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK), SDHCI_INT_ENABLE);

    /* Mask all sdhci interrupt sources */
    sdhci_writel(sdhci_host, 0x0, SDHCI_SIGNAL_ENABLE);
}

static void _mmcsd_host_init(struct sdhci_host* mmcsd)
{
    sdhci_interrupt_init(mmcsd);

    sdhci_writeb(mmcsd, 0x00, 0x508);
}

static void _mmcsd_clk_init(struct sdhci_host* mmcsd)
{
    uint8_t pwr = 0;
    uint8_t ctl;

    ctl = sdhci_readb(mmcsd, SDHCI_HOST_CONTROL);
    ctl &= ~SDHCI_CTRL_DMA_MASK;
    ctl |= SDHCI_CTRL_HISPD;
    sdhci_writeb(mmcsd, ctl, SDHCI_HOST_CONTROL);
    // sdhci_writew(mmcsd,sdhci_readw(mmcsd, SDHCI_HOST_CONTROL)|SDHCI_CTRL_HISPD, SDHCI_HOST_CONTROL);
    if (mmcsd->mapbase == (void*)emmc0_base) {
#ifdef RT_SDIO0_SD
        pwr |= SDHCI_POWER_ON | SDHCI_POWER_330;
#else
        sdhci_writew(mmcsd, SDHCI_CTRL_VDD_180, SDHCI_HOST_CONTROL2);
        pwr |= SDHCI_POWER_ON | SDHCI_POWER_180;
#endif
    } else {
        pwr |= SDHCI_POWER_ON | SDHCI_POWER_330;
    }

    sdhci_writeb(mmcsd, pwr, SDHCI_POWER_CONTROL);
}

static void sdhci_send_command(struct sdhci_host* sdhci_host, struct sdhci_command* command, rt_bool_t enDMA)
{
    RT_ASSERT(RT_NULL != command);

    uint32_t cmd_r, xfer_mode;
    struct sdhci_data* sdhci_data = sdhci_host->sdhci_data;

    cmd_r = SDHCI_MAKE_CMD(command->index, command->flags);

    if (sdhci_data != RT_NULL) {
#ifdef SDHCI_SDMA_ENABLE
        uint32_t start_addr;
        if (sdhci_data->rxData)
            start_addr = (rt_ubase_t)((uint8_t*)sdhci_data->rxData + PV_OFFSET);
        else
            start_addr = (rt_ubase_t)((uint8_t*)sdhci_data->txData + PV_OFFSET);
        rt_hw_cpu_dcache_clean((void*)start_addr, sdhci_data->blockSize * sdhci_data->blockCount);
        command->flags2 |= sdhci_enable_dma_flag;
        sdhci_writel(sdhci_host, start_addr, SDHCI_DMA_ADDRESS);
#endif
        xfer_mode = command->flags2 & 0x1ff;
        sdhci_writew(sdhci_host, SDHCI_MAKE_BLKSZ(7, sdhci_data->blockSize), SDHCI_BLOCK_SIZE);
        sdhci_writew(sdhci_host, sdhci_data->blockCount, SDHCI_BLOCK_COUNT);
        sdhci_writew(sdhci_host, xfer_mode, SDHCI_TRANSFER_MODE);
    }
    sdhci_writel(sdhci_host, command->argument, SDHCI_ARGUMENT);
    sdhci_writew(sdhci_host, cmd_r, SDHCI_COMMAND);
}

static rt_err_t sdhci_wait_command_done(struct sdhci_host* sdhci_host, struct sdhci_command* command, rt_bool_t executeTuning)
{
    RT_ASSERT(RT_NULL != command);

    int error = 0;
    uint32_t interruptStatus = 0U;
    /* tuning cmd do not need to wait command done */
    if (!executeTuning) {
        /* Wait command complete or USDHC encounters error. */
        while (!(sdhci_get_int_status_flag(sdhci_host) & (SDHCI_INT_RESPONSE | sdhci_command_error_flag))) {
        }

        interruptStatus = sdhci_get_int_status_flag(sdhci_host);

        if ((interruptStatus & sdhci_sdr104_tuning_flag) != 0U) {
            error = sdhci_tuning_error_flag;
        } else if ((interruptStatus & sdhci_command_error_flag) != 0U) {
            error = 1;
        } else {
        }
        /* Receive response when command completes successfully. */
        if (error == 0) {
            error = sdhci_receive_command_response(sdhci_host, command);
        }

        sdhci_clear_int_status_flag(
            sdhci_host, (sdhci_command_complete_flag | sdhci_command_error_flag | sdhci_tuning_error_flag));
    }

    return error;
}

static rt_err_t sdhci_transfer_data_blocking(struct sdhci_host* sdhci_host, struct sdhci_data* data, rt_bool_t enDMA)
{
    rt_err_t error = RT_EOK;
    uint32_t interruptStatus = 0U;
    unsigned int stat, rdy, mask, timeout, block = 0;
    rt_bool_t transfer_done = false;
    int i = 0;

    if (enDMA) {
        /* Wait data complete or USDHC encounters error. */
        while (!(sdhci_get_int_status_flag(sdhci_host) & (sdhci_data_complete_flag | sdhci_data_error_flag | sdhci_dma_error_flag | sdhci_tuning_error_flag))) {
        }

        interruptStatus = sdhci_get_int_status_flag(sdhci_host);
        if ((interruptStatus & sdhci_tuning_error_flag) != 0U) {
            error = sdhci_status_tuning_error;
        } else if ((interruptStatus & (sdhci_data_error_flag | sdhci_dma_error_flag)) != 0U) {
            if (!(data->enableIgnoreError) || (interruptStatus & sdhci_data_timeout_flag)) {
                error = RT_ERROR;
                emmc_reg_display(sdhci_host);
            }
        } else {
        }

        sdhci_clear_int_status_flag(sdhci_host, (sdhci_data_complete_flag | sdhci_data_error_flag | sdhci_dma_error_flag | sdhci_tuning_error_flag));
    } else {
        timeout = 1000000;
        rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
        mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;

        do {
            stat = sdhci_readl(sdhci_host, SDHCI_INT_STATUS);
            if (stat & SDHCI_INT_ERROR) {
                LOG_D("%s: Error detected in status(0x%X)!\n",
                    __func__, stat);
                emmc_reg_display(sdhci_host);
                return -1;
            }

            if (!transfer_done && (stat & rdy)) {
                if (!(sdhci_readl(sdhci_host, SDHCI_PRESENT_STATE) & mask)) {
                    continue;
                }
                sdhci_writel(sdhci_host, rdy, SDHCI_INT_STATUS);
                if (data->rxData) {
                    for (i = 0; i < data->blockSize / 4; i++) {
                        data->rxData[i + block * data->blockSize] = sdhci_readl(sdhci_host, SDHCI_BUFFER);
                    }
                } else {
                    for (i = 0; i < data->blockSize / 4; i++) {
                        sdhci_writel(sdhci_host, data->txData[i + block * data->blockSize], SDHCI_BUFFER);
                    }
                }
                if (++block >= data->blockCount) {
                    /* Keep looping until the SDHCI_INT_DATA_END is
                     * cleared, even if we finished sending all the
                     * blocks.
                     */
                    transfer_done = true;
                    continue;
                }
            }

#ifdef SDHCI_SDMA_ENABLE
            if (!transfer_done && (stat & SDHCI_INT_DMA_END)) {
                sdhci_writel(sdhci_host, SDHCI_INT_DMA_END, SDHCI_INT_STATUS);
                sdhci_writel(sdhci_host, sdhci_readl(sdhci_host, SDHCI_DMA_ADDRESS), SDHCI_DMA_ADDRESS);
            }
#endif
            if (timeout-- > 0) {
                delay_1k(1);
            } else {
                rt_kprintf("%s: Transfer data timeout\n", __func__);
                return -1;
            }
        } while (!(stat & SDHCI_INT_DATA_END));
#ifdef SDHCI_SDMA_ENABLE
        /* invalidate cache for read */
        if (data && data->rxData)
            rt_hw_cpu_dcache_invalidate((void*)data->rxData, data->blockSize * data->blockCount);
#endif
    }
    return error;
}

rt_err_t sdhci_set_transfer_config(struct sdhci_host* sdhci_host, struct sdhci_command* sdhci_command, struct sdhci_data* sdhci_data)
{
    RT_ASSERT(sdhci_command);
    /* Define the flag corresponding to each response type. */
    switch (sdhci_command->responseType) {
    case card_response_type_none:
        break;
    case card_response_type_r1: /* Response 1 */
    case card_response_type_r5: /* Response 5 */
    case card_response_type_r6: /* Response 6 */
    case card_response_type_r7: /* Response 7 */

        sdhci_command->flags |= (sdhci_cmd_resp_short | sdhci_enable_cmd_crc_flag | sdhci_enable_cmd_index_chk_flag);
        break;

    case card_response_type_r1b: /* Response 1 with busy */
    case card_response_type_r5b: /* Response 5 with busy */
        sdhci_command->flags |= (sdhci_cmd_resp_short_busy | sdhci_enable_cmd_crc_flag | sdhci_enable_cmd_index_chk_flag);
        break;

    case card_response_type_r2: /* Response 2 */
        sdhci_command->flags |= (sdhci_cmd_resp_long | sdhci_enable_cmd_crc_flag);
        break;

    case card_response_type_r3: /* Response 3 */
    case card_response_type_r4: /* Response 4 */
        sdhci_command->flags |= (sdhci_cmd_resp_short);
        break;

    default:
        break;
    }

    if (sdhci_command->type == card_command_type_abort) {
        sdhci_command->flags |= sdhci_enable_command_type_abort;
    } else if (sdhci_command->type == card_command_type_resume) {
        sdhci_command->flags |= sdhci_enable_command_type_resume;
    } else if (sdhci_command->type == card_command_type_suspend) {
        sdhci_command->flags |= sdhci_enable_command_type_suspend;
    } else if (sdhci_command->type == card_command_type_normal) {
        sdhci_command->flags |= sdhci_enable_command_type_normal;
    }
    sdhci_writeb(sdhci_host, 0xe, SDHCI_TIMEOUT_CONTROL);
    if (sdhci_data) {
        sdhci_command->flags |= sdhci_enable_cmd_data_present_flag;
        sdhci_command->flags2 |= sdhci_enable_block_count_flag | sdhci_enable_auto_command12_flag;

        if (sdhci_data->rxData) {
            sdhci_command->flags2 |= sdhci_data_read_flag;
        }
        if (sdhci_data->blockCount > 1U) {
            sdhci_command->flags2 |= (sdhci_multiple_block_flag);
            /* auto command 12 */
            if (sdhci_data->enableAutoCommand12) {
                /* Enable Auto command 12. */
                sdhci_command->flags2 |= sdhci_enable_auto_command12_flag;
            }
            /* auto command 23 */
            if (sdhci_data->enableAutoCommand23) {
                sdhci_command->flags2 |= sdhci_enable_auto_command23_flag;
            }
        }
    }
    return 0;
}

static rt_err_t sdhci_receive_command_response(struct sdhci_host* sdhci_host, struct sdhci_command* command)
{
    uint32_t i;

    if (command->responseType == card_response_type_r2) {
        /* CRC is stripped so we need to do some shifting. */
        for (i = 0; i < 4; i++) {
            command->response[3 - i] = sdhci_readl(sdhci_host, SDHCI_RESPONSE + (3 - i) * 4) << 8;
            if (i != 3) {
                command->response[3 - i] |= sdhci_readb(sdhci_host, SDHCI_RESPONSE + (3 - i) * 4 - 1);
            }
        }

    } else {
        command->response[0] = sdhci_readl(sdhci_host, SDHCI_RESPONSE);
    }
    /* check response error flag */
    if ((command->responseErrorFlags != 0U) && ((command->responseType == card_response_type_r1) || (command->responseType == card_response_type_r1b) || (command->responseType == card_response_type_r6) || (command->responseType == card_response_type_r5))) {
        if (((command->responseErrorFlags) & (command->response[0U])) != 0U) {
            return -1; // kStatus_USDHC_SendCommandFailed;
        }
    }

    return 0;
}

rt_err_t sdhci_transfer_blocking(struct sdhci_host* sdhci_host)
{
    RT_ASSERT(sdhci_host);
    struct sdhci_command* sdhci_command = sdhci_host->sdhci_command;
    struct sdhci_data* sdhci_data = sdhci_host->sdhci_data;
    rt_bool_t enDMA = false;
    int ret = RT_EOK;

    /* Wait until command/data bus out of busy status. */
    while (sdhci_get_present_status_flag(sdhci_host) & sdhci_command_inhibit_flag) {
    }
    while (sdhci_data && (sdhci_get_present_status_flag(sdhci_host) & sdhci_data_inhibit_flag)) {
    }
    sdhci_writel(sdhci_host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

    ret = sdhci_set_transfer_config(sdhci_host, sdhci_command, sdhci_data);
    if (ret != 0) {
        return ret;
    }
    sdhci_send_command(sdhci_host, sdhci_command, enDMA);
    /* wait command done */
    ret = sdhci_wait_command_done(sdhci_host, sdhci_command, ((sdhci_data == RT_NULL) ? false : sdhci_data->executeTuning));
    /* transfer data */
    if ((sdhci_data != RT_NULL) && (ret == 0)) {
        ret = sdhci_transfer_data_blocking(sdhci_host, sdhci_data, enDMA);
    }
    sdhci_writel(sdhci_host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
    sdhci_reset(sdhci_host, SDHCI_RESET_CMD);
    sdhci_reset(sdhci_host, SDHCI_RESET_DATA);
    return ret;
}

static void kd_mmc_request(struct rt_mmcsd_host* host, struct rt_mmcsd_req* req)
{
    struct sdhci_host* mmcsd;
    struct rt_mmcsd_cmd* cmd;
    struct rt_mmcsd_data* data;
    rt_err_t error;
    struct sdhci_data sdhci_data = { 0 };
    struct sdhci_command sdhci_command = { 0 };

    rt_mutex_take(mmcsd_mutex, RT_WAITING_FOREVER);

    RT_ASSERT(host != RT_NULL);
    RT_ASSERT(req != RT_NULL);

    mmcsd = (struct sdhci_host*)host->private_data;
    RT_ASSERT(mmcsd != RT_NULL);

    cmd = req->cmd;
    RT_ASSERT(cmd != RT_NULL);

    LOG_D("\tcmd->cmd_code: %02d, cmd->arg: %08x, cmd->flags: %08x --> ", cmd->cmd_code, cmd->arg, cmd->flags);

    data = cmd->data;

    sdhci_command.index = cmd->cmd_code;
    sdhci_command.argument = cmd->arg;

    if (cmd->cmd_code == STOP_TRANSMISSION)
        sdhci_command.type = card_command_type_abort;
    else
        sdhci_command.type = card_command_type_normal;

    switch (cmd->flags & RESP_MASK) {
    case RESP_NONE:
        sdhci_command.responseType = card_response_type_none;
        break;
    case RESP_R1:
        sdhci_command.responseType = card_response_type_r1;
        break;
    case RESP_R1B:
        sdhci_command.responseType = card_response_type_r1b;
        break;
    case RESP_R2:
        sdhci_command.responseType = card_response_type_r2;
        break;
    case RESP_R3:
        sdhci_command.responseType = card_response_type_r3;
        break;
    case RESP_R4:
        sdhci_command.responseType = card_response_type_r4;
        break;
    case RESP_R6:
        sdhci_command.responseType = card_response_type_r6;
        break;
    case RESP_R7:
        sdhci_command.responseType = card_response_type_r7;
        break;
    case RESP_R5:
        sdhci_command.responseType = card_response_type_r5;
        break;
    default:
        RT_ASSERT(RT_NULL);
    }

    sdhci_command.flags = 0;
    mmcsd->sdhci_command = &sdhci_command;

    if (data) {
        if (req->stop != RT_NULL)
            sdhci_data.enableAutoCommand12 = true;
        else
            sdhci_data.enableAutoCommand12 = false;

        sdhci_data.enableAutoCommand23 = false;

        sdhci_data.blockSize = data->blksize;
        sdhci_data.blockCount = data->blks;

        LOG_D(" buf: %p, blksize:%d, blks:%d ", data->buf, sdhci_data.blockSize, sdhci_data.blockCount);
        if ((cmd->cmd_code == WRITE_BLOCK) || (cmd->cmd_code == WRITE_MULTIPLE_BLOCK)) {
            sdhci_data.txData = data->buf;
            sdhci_data.rxData = RT_NULL;
        } else {
            sdhci_data.rxData = data->buf;
            sdhci_data.txData = RT_NULL;
        }
#ifdef SDHCI_SDMA_ENABLE
        if ((uint64_t)(sdhci_data.rxData) & 0x3f) {
            sdhci_data.rxData = rt_malloc_align(sdhci_data.blockSize * sdhci_data.blockCount, 64);
        } else if ((uint64_t)(sdhci_data.txData) & 0x3f) {
            sdhci_data.txData = rt_malloc_align(sdhci_data.blockSize * sdhci_data.blockCount, 64);
            rt_memcpy(sdhci_data.txData, data->buf, sdhci_data.blockSize * sdhci_data.blockCount);
        }
#endif
        mmcsd->sdhci_data = &sdhci_data;
    } else {
        mmcsd->sdhci_data = RT_NULL;
    }
    error = sdhci_transfer_blocking(mmcsd);
#ifdef SDHCI_SDMA_ENABLE
    if (data && sdhci_data.rxData && sdhci_data.rxData != data->buf) {
        rt_memcpy(data->buf, sdhci_data.rxData, sdhci_data.blockSize * sdhci_data.blockCount);
        rt_free_align(sdhci_data.rxData);
    } else if (data && sdhci_data.txData && sdhci_data.txData != data->buf) {
        rt_free_align(sdhci_data.txData);
    }
#endif
    if (error == 1) {
        sdhic_error_recovery(mmcsd);
        LOG_D(" ***USDHC_TransferBlocking error: %d*** --> \n", error);
        cmd->err = -RT_ERROR;
    }

    if ((cmd->flags & RESP_MASK) == RESP_R2) {
        cmd->resp[3] = sdhci_command.response[0];
        cmd->resp[2] = sdhci_command.response[1];
        cmd->resp[1] = sdhci_command.response[2];
        cmd->resp[0] = sdhci_command.response[3];
        LOG_D(" resp 0x%08X 0x%08X 0x%08X 0x%08X\n",
            cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    } else {
        cmd->resp[0] = sdhci_command.response[0];
        LOG_D(" resp 0x%08X\n", cmd->resp[0]);
    }
    mmcsd_req_complete(host);

    rt_mutex_release(mmcsd_mutex);
    return;
}

void kd_card_clk_enable(struct sdhci_host* host, uint32_t enable)
{
    if (enable) {
        sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) | SDHCI_CLOCK_CARD_EN | SDHCI_PROG_CLOCK_MODE, SDHCI_CLOCK_CONTROL);
    } else {
        sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) & (~SDHCI_CLOCK_CARD_EN), SDHCI_CLOCK_CONTROL);
    }
}

void kd_pll_clk_enable(struct sdhci_host* host, uint32_t enable)
{
    if (enable) {
        sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) | SDHCI_CLOCK_PLL_EN, SDHCI_CLOCK_CONTROL);
    } else {
        sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) & (~SDHCI_CLOCK_PLL_EN), SDHCI_CLOCK_CONTROL);
    }
}

void kd_mmc_clock_freq_change(struct sdhci_host* host, uint32_t clock)
{
    uint32_t div = 0, clk = 0;

    // Execute SD Clock Stop Sequence
    kd_card_clk_enable(host, 0);
    // Set CLK_CTRL_R.PLL_ENABLE to 0
    kd_pll_clk_enable(host, 0);
    if (clock == 0)
        return;

    if (host->max_clk <= clock) {
        div = 1;
    } else {
        for (div = 2;
             div < SDHCI_MAX_DIV_SPEC_300;
             div += 2) {
            if ((host->max_clk / div) <= clock)
                break;
        }
    }
    div >>= 1;
    clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
    clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
        << SDHCI_DIVIDER_HI_SHIFT;

    sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

    kd_pll_clk_enable(host, 1);
    kd_card_clk_enable(host, 1);
    sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) | SDHCI_CLOCK_INT_EN, SDHCI_CLOCK_CONTROL);
}

static void kd_set_iocfg(struct rt_mmcsd_host* host, struct rt_mmcsd_io_cfg* io_cfg)
{
    struct sdhci_host* mmcsd;
    unsigned int sdhci_clk;
    unsigned int bus_width;
    uint8_t ctrl = 0;
    RT_ASSERT(host != RT_NULL);
    RT_ASSERT(host->private_data != RT_NULL);
    RT_ASSERT(io_cfg != RT_NULL);

    mmcsd = (struct sdhci_host*)host->private_data;
    sdhci_clk = io_cfg->clock;
    bus_width = io_cfg->bus_width;

    /* ToDo : Use the Clock Framework */
    LOG_D("%s: sdhci_clk=%d, bus_width:%d\n", __func__, sdhci_clk, bus_width);

    kd_mmc_clock_freq_change(mmcsd, sdhci_clk);
    ctrl = sdhci_readb(mmcsd, SDHCI_HOST_CONTROL);
    if (bus_width == 3) {
        ctrl &= ~SDHCI_CTRL_4BITBUS;
        ctrl |= SDHCI_CTRL_8BITBUS;
    } else {
        ctrl &= ~SDHCI_CTRL_8BITBUS;
        if (bus_width == 2) {
            ctrl |= SDHCI_CTRL_4BITBUS;
        } else {
            ctrl &= ~SDHCI_CTRL_4BITBUS;
        }
    }
    sdhci_writeb(mmcsd, ctrl, SDHCI_HOST_CONTROL);
}

static const struct rt_mmcsd_host_ops ops = {
    kd_mmc_request,
    kd_set_iocfg,
    RT_NULL, //_mmc_get_card_status,
    RT_NULL, //_mmc_enable_sdio_irq,
};

static int init_kd_mmc_core(struct sdhci_host* host)
{
    if ((sdhci_readb(host, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL)) {
        LOG_E("%s: sd host controller reset error\n", __func__);
        return -1;
    }

    return 0;
}

static void kd_sdhci_clk_reset(void)
{
    static char* hi_sys_virt_addr;
    static char* sysctl_virt_addr;
    unsigned int data;
    unsigned int hi_sys_config_addr = 0x91585000;
    unsigned int sysctl_addr = 0x91101000;

    /*sdio:Hi_sys_config space,sdio write protect open */
    hi_sys_virt_addr = rt_ioremap((void*)hi_sys_config_addr, 0x400);
    sysctl_virt_addr = rt_ioremap((void*)sysctl_addr, 0xb0);
    data = readl(hi_sys_virt_addr + 8);
    data |= 0x4;
    writel(data, hi_sys_virt_addr + 8);
    /*sysctl reset sdio1*/
    writel(0x2, sysctl_virt_addr + 0x34);
    rt_thread_mdelay(1);
    while (!(readl(sysctl_virt_addr + 0x34) & 0x20000000))
        ;
    writel(0x20000000, sysctl_virt_addr + 0x34);
    rt_thread_mdelay(5);
    rt_iounmap(hi_sys_virt_addr);
    rt_iounmap(sysctl_virt_addr);
}

rt_int32_t kd_sdhci_init(void)
{
#ifdef RT_USING_SDIO0
    struct sdhci_host* mmcsd1;

    host1 = mmcsd_alloc_host();
    if (!host1) {
        goto err;
    }

    mmcsd1 = rt_malloc(sizeof(struct sdhci_host));
    if (!mmcsd1) {
        LOG_E("alloc mci failed\n");
        goto err;
    }

    rt_memset(mmcsd1, 0, sizeof(struct sdhci_host));
    mmcsd1->mapbase = (void*)rt_ioremap((void*)SDEMMC0_BASE, 0x1000);
    emmc0_base = (void*)mmcsd1->mapbase;
    mmcsd1->sdhci_data = RT_NULL;
    mmcsd1->sdhci_command = RT_NULL;
    mmcsd1->max_clk = 200000000;
    strncpy(host1->name, "sd", sizeof(host1->name) - 1);
    host1->ops = &ops;
    host1->freq_min = 400000;
    host1->freq_max = 50000000;
#ifdef RT_SDIO0_SD
    host1->valid_ocr = VDD_32_33 | VDD_33_34;
    host1->flags = MMCSD_BUSWIDTH_4 | MMCSD_MUTBLKWRITE | MMCSD_SUP_HIGHSPEED | MMCSD_SUP_SDIO_IRQ;
#else
    host1->valid_ocr = VDD_165_195;
    host1->flags = MMCSD_BUSWIDTH_8 | MMCSD_MUTBLKWRITE | MMCSD_SUP_HIGHSPEED | MMCSD_SUP_SDIO_IRQ;
#endif
    host1->max_seg_size = 512 * 512;
    host1->max_dma_segs = 1;
    host1->max_blk_size = 512;
    host1->max_blk_count = 4096;

    mmcsd1->host = host1;
    _mmcsd_clk_init(mmcsd1);

    _mmcsd_host_init(mmcsd1);
    host1->private_data = mmcsd1;

    if (init_kd_mmc_core(mmcsd1)) {
        rt_free(mmcsd1);
        goto err;
    }
    mmcsd_change(host1);
#endif
#ifdef RT_USING_SDIO1
    struct sdhci_host* mmcsd2;

    host2 = mmcsd_alloc_host();
    if (!host2) {
        goto err;
    }

    mmcsd2 = rt_malloc(sizeof(struct sdhci_host));
    if (!mmcsd2) {
        LOG_E("alloc mci failed\n");
        goto err;
    }

    rt_memset(mmcsd2, 0, sizeof(struct sdhci_host));
    mmcsd2->mapbase = (void*)rt_ioremap((void*)SDEMMC1_BASE, 0x1000);
    emmc1_base = (void*)mmcsd2->mapbase;
    mmcsd2->sdhci_data = RT_NULL;
    mmcsd2->sdhci_command = RT_NULL;
    mmcsd2->max_clk = 100000000;
    strncpy(host2->name, "sd", sizeof(host2->name) - 1);
    host2->ops = &ops;
    host2->freq_min = 375000;
    host2->freq_max = 50000000;
    host2->valid_ocr = VDD_32_33 | VDD_33_34;
    host2->flags = MMCSD_BUSWIDTH_4 | MMCSD_MUTBLKWRITE | MMCSD_SUP_HIGHSPEED | MMCSD_SUP_SDIO_IRQ;
    host2->max_seg_size = 512 * 512;
    host2->max_dma_segs = 1;
    host2->max_blk_size = 512;
    host2->max_blk_count = 4096;

    mmcsd2->host = host2;

    kd_sdhci_clk_reset();
    _mmcsd_clk_init(mmcsd2);
    _mmcsd_host_init(mmcsd2);

    host2->private_data = mmcsd2;

    if (init_kd_mmc_core(mmcsd2)) {
        rt_free(mmcsd2);
        goto err;
    }
    mmcsd_change(host2);
#endif

    mmcsd_mutex = rt_mutex_create("mmutex", RT_IPC_FLAG_FIFO);
    if (mmcsd_mutex == RT_NULL) {
        LOG_E("create mmcsd mutex failed.\n");
        goto err;
    }

    return 0;

err:
#ifdef RT_USING_SDIO0
    mmcsd_free_host(host1);
#endif
#ifdef RT_USING_SDIO1
    mmcsd_free_host(host2);
#endif
    return -RT_ENOMEM;
}

INIT_DEVICE_EXPORT(kd_sdhci_init);
#endif /*defined(RT_USING_SDIO0) || defined(RT_USING_SDIO1)*/

#endif /*defined(RT_USING_SDIO)*/
