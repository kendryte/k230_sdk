// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 IBM Corp.
 * Eddie James <eajames@linux.ibm.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/delay.h>

struct snps_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};
#define DWC_MSHC_PTR_VENDOR1 0x500
#define SDHCI_VENDER_AT_CTRL_REG (DWC_MSHC_PTR_VENDOR1 + 0x40)
#define SDHCI_VENDER_AT_STAT_REG (DWC_MSHC_PTR_VENDOR1 + 0x44)
#define SDHCI_TUNE_CLK_STOP_EN_MASK BIT(16)
#define SDHCI_TUNE_SWIN_TH_VAL_LSB (24)
#define SDHCI_TUNE_SWIN_TH_VAL_MASK (0xFF)

#define DWC_MSHC_PTR_PHY_REGS 0x300
#define DWC_MSHC_PHY_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x0)
#define PAD_SN_LSB 20
#define PAD_SN_MASK 0xF
#define PAD_SN_DEFAULT ((0x8 & PAD_SN_MASK) << PAD_SN_LSB)
#define PAD_SP_LSB 16
#define PAD_SP_MASK 0xF
#define PAD_SP_DEFAULT ((0x9 & PAD_SP_MASK) << PAD_SP_LSB)
#define PHY_PWRGOOD BIT(1)
#define PHY_RSTN BIT(0)

#define DWC_MSHC_CMDPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x4)
#define DWC_MSHC_DATPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x6)
#define DWC_MSHC_CLKPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x8)
#define DWC_MSHC_STBPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xA)
#define DWC_MSHC_RSTNPAD_CNFG (DWC_MSHC_PTR_PHY_REGS + 0xC)
#define TXSLEW_N_LSB 9
#define TXSLEW_N_MASK 0xF
#define TXSLEW_P_LSB 5
#define TXSLEW_P_MASK 0xF
#define WEAKPULL_EN_LSB 3
#define WEAKPULL_EN_MASK 0x3
#define RXSEL_LSB 0
#define RXSEL_MASK 0x3

#define DWC_MSHC_COMMDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1C)
#define DWC_MSHC_SDCLKDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x1D)
#define DWC_MSHC_SDCLKDL_DC (DWC_MSHC_PTR_PHY_REGS + 0x1E)
#define DWC_MSHC_SMPLDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x20)
#define DWC_MSHC_ATDL_CNFG (DWC_MSHC_PTR_PHY_REGS + 0x21)
#define DWC_MSHC_DLL_CNFG2_R (DWC_MSHC_PTR_PHY_REGS + 0x26)

#define DWC_MSHC_PHY_PAD_SD_CLK                                                \
	((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |  \
	 (2 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_DAT                                                \
	((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |  \
	 (2 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_SD_STB                                                \
	((1 << TXSLEW_N_LSB) | (3 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |  \
	 (2 << RXSEL_LSB))

#define DWC_MSHC_PHY_PAD_EMMC_CLK                                              \
	((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (0 << WEAKPULL_EN_LSB) |  \
	 (0 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_DAT                                              \
	((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |  \
	 (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_STB                                              \
	((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |  \
	 (1 << RXSEL_LSB))


static void dwcmshc_phy_pad_config(struct sdhci_host *host)
{
	u16 clk_ctrl;

	/* Disable the card clock */
	clk_ctrl = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk_ctrl &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk_ctrl, SDHCI_CLOCK_CONTROL);

	if (!dev_read_bool(host->mmc->dev, "1-8-v"))
	{
		sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_CMDPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_DATPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_CLK, DWC_MSHC_CLKPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_STB, DWC_MSHC_STBPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_RSTNPAD_CNFG);
	} else {
		sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_CMDPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_DATPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_CLK, DWC_MSHC_CLKPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_STB, DWC_MSHC_STBPAD_CNFG);
		sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_RSTNPAD_CNFG);
	}

	return;
}

static void dwcmshc_phy_delay_config(struct sdhci_host *host)
{
    u32 val;
    int ret;
    u32 tx_delay_line = 0x40;
    u32 rx_delay_line = 0xd;
    u32 extdly_en = 0;
    u32 cckdl_dc = 0x40;
    ret = dev_read_u32(host->mmc->dev, "tx_delay_line", &tx_delay_line);

    if(tx_delay_line > 255) {
        printf("tx_delay_line err\n");
    }
    else if (tx_delay_line >= 128) {
        extdly_en = 1;
        cckdl_dc = tx_delay_line - 128;
    } else {
        extdly_en = 0;
        cckdl_dc = tx_delay_line;
    }
    /* disable delay line */
	sdhci_writeb(host, 1<<4 | extdly_en, DWC_MSHC_SDCLKDL_CNFG);

	/* set delay line */
	sdhci_writeb(host, cckdl_dc, DWC_MSHC_SDCLKDL_DC);

	/* enable delay lane */
	val = sdhci_readb(host, DWC_MSHC_SDCLKDL_CNFG);
	val &= ~(1<<4);
	sdhci_writeb(host, val, DWC_MSHC_SDCLKDL_CNFG);
    
    dev_read_u32(host->mmc->dev, "rx_delay_line", &rx_delay_line);
    sdhci_writeb(host, rx_delay_line, DWC_MSHC_SMPLDL_CNFG);

	sdhci_writeb(host, 0xc, DWC_MSHC_ATDL_CNFG);
	sdhci_writel(host, (sdhci_readl(host, SDHCI_VENDER_AT_CTRL_REG) | \
	  BIT(16) | BIT(17) | BIT(19) | BIT(20)), SDHCI_VENDER_AT_CTRL_REG);
    sdhci_writel(host,0x0,SDHCI_VENDER_AT_STAT_REG);
	return;
}

static int dwcmshc_phy_init(struct sdhci_host *host)
{
	u32 reg;
	
	unsigned int timeout = 15000;

	/* reset phy */
	sdhci_writew(host, 0, DWC_MSHC_PHY_CNFG);

	/* Disable the clock */
	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	dwcmshc_phy_pad_config(host);
	dwcmshc_phy_delay_config(host);

	/* Wait max 150 ms */
	while (1) {
		reg = sdhci_readl(host, DWC_MSHC_PHY_CNFG);
		if (reg & PHY_PWRGOOD)
			break;
		if (!timeout) {
			return -1;
		}
		timeout--;

		udelay(10);
	}

	reg = PAD_SN_DEFAULT | PAD_SP_DEFAULT;
	sdhci_writel(host, reg, DWC_MSHC_PHY_CNFG);

	/* de-assert the phy */
	reg |= PHY_RSTN;
	sdhci_writel(host, reg, DWC_MSHC_PHY_CNFG);

	return 0;
}

struct sdhci_ops k230_host_opt={
	.deferred_probe = dwcmshc_phy_init,
};

static int snps_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct snps_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
    struct mmc_config *cfg = &plat->cfg;
	u32 max_clk;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);

	max_clk = clk_get_rate(&clk);
	if (IS_ERR_VALUE(max_clk)) {
		ret = max_clk;
		goto err;
	}

	host->max_clk = max_clk;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(cfg, host, cfg->f_max, 0);
	if (ret)
		goto err;
	
	if(dev_read_bool(dev, "have-emmc-phy")){
		host->ops= &k230_host_opt;
	}

	ret = sdhci_probe(dev);
	if (ret)
		goto err;

	return 0;

err:
	clk_disable(&clk);
	clk_free(&clk);
	return ret;
}

static int snps_sdhci_bind(struct udevice *dev)
{
	struct snps_sdhci_plat *plat = dev_get_plat(dev);

	mmc_of_parse(dev, &plat->cfg);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id snps_sdhci_ids[] = {
	{ .compatible = "snps,dwcmshc-sdhci" },
	{ .compatible = "snps,dwcmshc-sdhci-k230" },
	{ }
};

U_BOOT_DRIVER(snps_sdhci_drv) = {
	.name           = "snps_sdhci",
	.id             = UCLASS_MMC,
	.of_match       = snps_sdhci_ids,
	.ops            = &sdhci_ops,
	.bind           = snps_sdhci_bind,
	.probe          = snps_sdhci_probe,
	.priv_auto = sizeof(struct sdhci_host),
	.plat_auto = sizeof(struct snps_sdhci_plat),
};
