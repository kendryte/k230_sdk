/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/nvmem-provider.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>


#define OTP_BYPASS_ADDR_P           0x91102040
#define OTP_REG_OFFSET              0x0
#define OTP_USER_START_OFFSET       0x0

typedef enum
{
    OTP_BYPASS_STATUS                   = 0,
    OTP_MAX_STATUS                      = 1,
} OTP_STATUS_E;

struct otp_priv {
    struct device *dev;
    void __iomem *base;
    struct nvmem_config *config;
};


/**
 * @brief After the OTP is powered on, the driver can read the sysctl register SOC_BOOT_CTL 
 *        to determine whether the OTP has been bypassed.
 * 
 * @return true 
 * @return false 
 */
static bool sysctl_boot_get_otp_bypass(void)
{
     void __iomem *OTP_BYPASS_ADDR_V;
     OTP_BYPASS_ADDR_V = ioremap(OTP_BYPASS_ADDR_P, 4);
     if(readl(OTP_BYPASS_ADDR_V) & 0x10)
        return true;
    else
        return false;
}

static bool otp_get_status(OTP_STATUS_E eStatus)
{
    if(OTP_BYPASS_STATUS == eStatus)
        return sysctl_boot_get_otp_bypass();
    else
        return false;
}

/**
 * @brief OTP read operation, can only read specific areas, including production information, reserved user space
 * 
 * @param context 
 * @param offset 
 * @param val 
 * @param bytes 
 * @return int 
 */
static int k230_otp_read(void *context, unsigned int offset,
    void *val, size_t bytes)
{
    struct otp_priv *priv = context;
    uint32_t *outbuf = (uint32_t *)val;
    uint32_t *buf;
    uint32_t WORD_SIZE = priv->config->word_size;
    uint32_t wlen = bytes / WORD_SIZE;
    uint32_t word;
    uint32_t i = 0;

    if(true == otp_get_status(OTP_BYPASS_STATUS))
        return -1;

    if(wlen > 0)
    {
        for(i=0; i<wlen; i++)
            outbuf[i] = readl(priv->base + OTP_REG_OFFSET + offset + i*WORD_SIZE);
    }

    if(bytes % WORD_SIZE != 0)
    {
        outbuf += wlen * WORD_SIZE;
        word = readl(priv->base + OTP_REG_OFFSET + offset + wlen);
        memcpy(outbuf, &word, bytes % WORD_SIZE);
    }

    return 0;
}

#if 0
/**
 * @brief Dangerous operation, once using the OTP's API to writes otp space, it cannot be recovered.
 * 
 * @param context 
 * @param offset 
 * @param val 
 * @param bytes 
 * @return int 
 */
static int k230_otp_write(void *context, unsigned int offset,
				 void *val, size_t bytes)
{
    struct otp_priv *priv = context;
    uint32_t *inbuf = (uint32_t *)val;
    uint32_t WORD_SIZE = priv->config->word_size;
    uint32_t wlen = bytes / WORD_SIZE;
    uint32_t i = 0;

    if(true == otp_get_status(OTP_BYPASS_STATUS))
        return -1;

    while(wlen--)
    {
        writel(inbuf[i], priv->base + OTP_USER_START_OFFSET + offset + i);
        i += 4;
    }

    return 0;
}
#endif

static struct nvmem_config kendryte_otp_nvmem_config = {
    .name = "kendryte_otp",
    .owner = THIS_MODULE,
    .read_only = true,
    .word_size = 4,
    .reg_read = k230_otp_read,
    // .reg_write = k230_otp_write,
    .size = 0x300,
};


static const struct of_device_id kendryte_otp_dt_ids[] = {
    { .compatible = "canaan,k230-otp" },
    { },
};
MODULE_DEVICE_TABLE(of, kendryte_otp_dt_ids);

static int kendryte_otp_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct resource *res;
    struct otp_priv *priv;
    struct nvmem_device *nvmem;

    priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
    if(!priv)
        return -ENOMEM;
    
    priv->dev = dev;

    /* Get OTP base address register from DTS. */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    priv->base = devm_ioremap_resource(dev, res);
    if(IS_ERR(priv->base))
        return PTR_ERR(priv->base);

    kendryte_otp_nvmem_config.dev = dev;
    kendryte_otp_nvmem_config.priv = priv;
    priv->config = &kendryte_otp_nvmem_config;
    nvmem = devm_nvmem_register(dev, &kendryte_otp_nvmem_config);

    return PTR_ERR_OR_ZERO(nvmem);
}

static struct platform_driver kendryte_otp_driver = {
    .probe = kendryte_otp_probe,
    .driver = {
        .name = "kendryte_otp",
        .of_match_table = kendryte_otp_dt_ids,
    },
};
module_platform_driver(kendryte_otp_driver);

MODULE_DESCRIPTION("kendryte k230 otp driver");
MODULE_LICENSE("GPL v2");
