// SPDX-License-Identifier: GPL-2.0

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sizes.h>

#include <linux/delay.h>

#include "sdhci-pltfm.h"

#define DWC_MSHC_PTR_VENDOR1 0x500
#define SDHCI_VENDER_AT_CTRL_REG (DWC_MSHC_PTR_VENDOR1 + 0x40)
#define SDHCI_VENDER_AT_STAT_REG (DWC_MSHC_PTR_VENDOR1 + 0x44)
#define SDHCI_TUNE_CLK_STOP_EN_MASK BIT(16)
#define SDHCI_TUNE_SWIN_TH_VAL_LSB (24)
#define SDHCI_TUNE_SWIN_TH_VAL_MASK (0xFF)

#define MSHC_CTRL_R (DWC_MSHC_PTR_VENDOR1 + 0x08) //16bit

#define EMMC_CTRL_R (DWC_MSHC_PTR_VENDOR1 + 0x2c) //16bit
#define CARD_IS_EMMC 0x0 //1bit


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
	 (1 << RXSEL_LSB))
	//  (0 << RXSEL_LSB))

#define DWC_MSHC_PHY_PAD_EMMC_DAT                                              \
	((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (1 << WEAKPULL_EN_LSB) |  \
	 (1 << RXSEL_LSB))
#define DWC_MSHC_PHY_PAD_EMMC_STB                                              \
	((2 << TXSLEW_N_LSB) | (2 << TXSLEW_P_LSB) | (2 << WEAKPULL_EN_LSB) |  \
	 (1 << RXSEL_LSB))

/* DWCMSHC specific Mode Select value */
#define DWCMSHC_CTRL_HS400		0x7

#define BOUNDARY_OK(addr, len) \
	((addr | (SZ_128M - 1)) == ((addr + len - 1) | (SZ_128M - 1)))

struct dwcmshc_priv {
	struct clk	*bus_clk;
	bool is_emmc_card;
    u32 tx_delay_line;
    u32 rx_delay_line;
    u8 mshc_ctrl_r;
    bool io_fixed_1v8;
    void __iomem *hs_regs;
    bool have_phy;
};

static void dwcmshc_phy_1_8v_init(struct sdhci_host *host)
{
    sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_CMDPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_DATPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_CLK, DWC_MSHC_CLKPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_STB, DWC_MSHC_STBPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_EMMC_DAT, DWC_MSHC_RSTNPAD_CNFG);

}

static void dwcmshc_phy_3_3v_init(struct sdhci_host *host)
{
    sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_CMDPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_DATPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_CLK, DWC_MSHC_CLKPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_STB, DWC_MSHC_STBPAD_CNFG);
    sdhci_writew(host, DWC_MSHC_PHY_PAD_SD_DAT, DWC_MSHC_RSTNPAD_CNFG);

}

static void dwcmshc_phy_delay_config(struct sdhci_host *host)
{
    struct sdhci_pltfm_host *pltfm_host;
	struct dwcmshc_priv *priv;
    pltfm_host = sdhci_priv(host);
	priv = sdhci_pltfm_priv(pltfm_host);

	sdhci_writeb(host, 1, DWC_MSHC_COMMDL_CNFG);
    if (priv->tx_delay_line > 256) {
        pr_info("%s: tx_delay_line err\n", mmc_hostname(host->mmc));
    } else if (priv->tx_delay_line > 128) {
        sdhci_writeb(host, 0x1, DWC_MSHC_SDCLKDL_CNFG);
        sdhci_writeb(host, priv->tx_delay_line - 128, DWC_MSHC_SDCLKDL_DC);
    } else {
        sdhci_writeb(host, 0x0, DWC_MSHC_SDCLKDL_CNFG);
        sdhci_writeb(host, priv->tx_delay_line, DWC_MSHC_SDCLKDL_DC);
    }

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
    struct sdhci_pltfm_host *pltfm_host;
	struct dwcmshc_priv *priv;
    pltfm_host = sdhci_priv(host);
	priv = sdhci_pltfm_priv(pltfm_host);
	/* reset phy */
	sdhci_writew(host, 0, DWC_MSHC_PHY_CNFG);

	/* Disable the clock */
	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

    if (priv->io_fixed_1v8) {
        u32 data = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        data |= SDHCI_CTRL_VDD_180;
        sdhci_writew(host, data, SDHCI_HOST_CONTROL2);
        dwcmshc_phy_1_8v_init(host);
    } else {
        dwcmshc_phy_3_3v_init(host);
    }

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

		usleep_range(10, 15);
	}

	reg = PAD_SN_DEFAULT | PAD_SP_DEFAULT;
	sdhci_writel(host, reg, DWC_MSHC_PHY_CNFG);

	/* de-assert the phy */
	reg |= PHY_RSTN;
	sdhci_writel(host, reg, DWC_MSHC_PHY_CNFG);

	return 0;
}

static void dwcmshc_sdhci_reset(struct sdhci_host *host, u8 mask)
{
	struct sdhci_pltfm_host *pltfm_host;
	struct dwcmshc_priv *priv;
	u8 emmc_ctl;

	pltfm_host = sdhci_priv(host);
	priv = sdhci_pltfm_priv(pltfm_host);

	/*host reset*/
	sdhci_reset(host, mask);
    if(mask == SDHCI_RESET_ALL) {
        emmc_ctl = sdhci_readw(host, EMMC_CTRL_R);

        if (priv->is_emmc_card) {
            emmc_ctl |= (1 << CARD_IS_EMMC);
        } else {
            emmc_ctl &=~(1 << CARD_IS_EMMC);
        }
        sdhci_writeb(host, emmc_ctl, EMMC_CTRL_R);
        if(priv->have_phy) {
            dwcmshc_phy_init(host);
        } else {
            sdhci_writeb(host, priv->mshc_ctrl_r, MSHC_CTRL_R);
        }
    }
}

/*
 * If DMA addr spans 128MB boundary, we split the DMA transfer into two
 * so that each DMA transfer doesn't exceed the boundary.
 */
static void dwcmshc_adma_write_desc(struct sdhci_host *host, void **desc,
				    dma_addr_t addr, int len, unsigned int cmd)
{
	int tmplen, offset;

	if (likely(!len || BOUNDARY_OK(addr, len))) {
		sdhci_adma_write_desc(host, desc, addr, len, cmd);
		return;
	}

	offset = addr & (SZ_128M - 1);
	tmplen = SZ_128M - offset;
	sdhci_adma_write_desc(host, desc, addr, tmplen, cmd);

	addr += tmplen;
	len -= tmplen;
	sdhci_adma_write_desc(host, desc, addr, len, cmd);
}

static void dwcmshc_sdhci_set_uhs_signaling(struct sdhci_host *host, unsigned timing)
{
	u16 ctrl_2;

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	if ((timing == MMC_TIMING_MMC_HS200) ||
		(timing == MMC_TIMING_UHS_SDR104)) {
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
	}
	else if (timing == MMC_TIMING_UHS_SDR12)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
	else if ((timing == MMC_TIMING_UHS_SDR25) ||
		 (timing == MMC_TIMING_MMC_HS))
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
	else if (timing == MMC_TIMING_UHS_SDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
	else if ((timing == MMC_TIMING_UHS_DDR50) ||
		(timing == MMC_TIMING_MMC_DDR52))
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
	else if (timing == MMC_TIMING_MMC_HS400) {
		ctrl_2 |= DWCMSHC_CTRL_HS400; /* Non-standard */
	}

	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
}

static const struct sdhci_ops sdhci_dwcmshc_kendryte_ops = {
	.set_clock		= sdhci_set_clock,
	.set_bus_width		= sdhci_set_bus_width,
	.set_uhs_signaling	= dwcmshc_sdhci_set_uhs_signaling,
	.get_max_clock		= sdhci_pltfm_clk_get_max_clock,
	.reset			= dwcmshc_sdhci_reset,
	.adma_write_desc	= dwcmshc_adma_write_desc,
	.voltage_switch     = dwcmshc_phy_1_8v_init,
};

static const struct sdhci_pltfm_data sdhci_dwcmshc_kendryte_pdata = {
	.ops = &sdhci_dwcmshc_kendryte_ops,
	.quirks = SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN,
};

/* This api is for wifi driver rescan the sdio device,
 * ugly but it is needed */
static unsigned int slot_index = 0;
static struct mmc_host *__mmc__host[3] = {NULL};

int plat_sdio_rescan(int slot)
{
	struct mmc_host *mmc = __mmc__host[slot];

	if (mmc == NULL) {
			pr_err("invalid mmc, please check the argument\n");
			return -EINVAL;
	}

	mmc_detect_change(mmc, 0);
	return 0;
}
EXPORT_SYMBOL(plat_sdio_rescan);

static int dwcmshc_probe(struct platform_device *pdev)
{
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_host *host;
	struct dwcmshc_priv *priv;
	int err;
	u32 extra;
	unsigned int data;
	unsigned int hi_sys_config_addr = 0x91585000;

	host = sdhci_pltfm_init(pdev, &sdhci_dwcmshc_kendryte_pdata,
				sizeof(struct dwcmshc_priv));
	if (IS_ERR(host))
		return PTR_ERR(host);

	/*
	 * extra adma table cnt for cross 128M boundary handling.
	 */
	extra = DIV_ROUND_UP_ULL(dma_get_required_mask(&pdev->dev), SZ_128M);
	if (extra > SDHCI_MAX_SEGS)
		extra = SDHCI_MAX_SEGS;
	host->adma_table_cnt += extra;

	pltfm_host = sdhci_priv(host);
	priv = sdhci_pltfm_priv(pltfm_host);
    priv->hs_regs = ioremap(hi_sys_config_addr,0x400);

    if(memcmp(host->hw_name,"91581000",8) == 0) {
        priv->have_phy = 0;
        data = readl(priv->hs_regs + 8);
        data |= 1<<2;
        writel(data, priv->hs_regs + 8);
    } else {
        priv->have_phy = 1;
        data = readl(priv->hs_regs + 0);
        data |= 1<<6 | 1<<4;
        writel(data, priv->hs_regs + 0);
    }

    if (device_property_present(&pdev->dev, "is_emmc")) {
        priv->is_emmc_card = 1;
    } else {
        priv->is_emmc_card = 0;
    }

    if(priv->have_phy) {
        if (device_property_present(&pdev->dev, "io_fixed_1v8")) {
            priv->io_fixed_1v8 = 1;
        } else {
            priv->io_fixed_1v8 = 0;
        }
        err = device_property_read_u32(&pdev->dev, "tx_delay_line", &priv->tx_delay_line);
        if(err)
            priv->tx_delay_line = 0;

        err = device_property_read_u32(&pdev->dev, "rx_delay_line", &priv->rx_delay_line);
        if(err)
            priv->rx_delay_line = 0;

    } else {
        /*sdio:(fpga board) Launches CMD/DATA with respect to positive edge of cclk_tx */
        err = device_property_read_u8(&pdev->dev, "mshc_ctrl_r", &priv->mshc_ctrl_r);
        if(err)
            priv->mshc_ctrl_r = 0;
    }

    if (priv->io_fixed_1v8) {
		host->flags &= ~SDHCI_SIGNALING_330;
	}

	pltfm_host->clk = devm_clk_get(&pdev->dev, "core");
	if (IS_ERR(pltfm_host->clk)) {
		err = PTR_ERR(pltfm_host->clk);
		dev_err(&pdev->dev, "failed to get core clk: %d\n", err);
		goto free_pltfm;
	}
	err = clk_prepare_enable(pltfm_host->clk);
	if (err)
		goto free_pltfm;

	priv->bus_clk = devm_clk_get(&pdev->dev, "bus");
	if (!IS_ERR(priv->bus_clk))
		clk_prepare_enable(priv->bus_clk);

    /* new fix: storage mmc host to array */
	__mmc__host[slot_index++] = host->mmc;

	err = mmc_of_parse(host->mmc);
	if (err)
		goto err_clk;

	sdhci_get_of_property(pdev);

	err = sdhci_add_host(host);
	if (err)
		goto err_clk;

	return 0;

err_clk:
	clk_disable_unprepare(pltfm_host->clk);
	clk_disable_unprepare(priv->bus_clk);
free_pltfm:
	sdhci_pltfm_free(pdev);
	return err;
}

static int dwcmshc_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);

	sdhci_remove_host(host, 0);

	clk_disable_unprepare(pltfm_host->clk);
	clk_disable_unprepare(priv->bus_clk);

    iounmap(priv->hs_regs);

	sdhci_pltfm_free(pdev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dwcmshc_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	clk_disable_unprepare(pltfm_host->clk);
	if (!IS_ERR(priv->bus_clk))
		clk_disable_unprepare(priv->bus_clk);

	return ret;
}

static int dwcmshc_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = clk_prepare_enable(pltfm_host->clk);
	if (ret)
		return ret;

	if (!IS_ERR(priv->bus_clk)) {
		ret = clk_prepare_enable(priv->bus_clk);
		if (ret)
			return ret;
	}

	return sdhci_resume_host(host);
}
#endif

static SIMPLE_DEV_PM_OPS(dwcmshc_pmops, dwcmshc_suspend, dwcmshc_resume);

static const struct of_device_id sdhci_dwcmshc_kendryte_dt_ids[] = {
	{ .compatible = "kendryte,k230-dw-mshc" },
	{}
};
MODULE_DEVICE_TABLE(of, sdhci_dwcmshc_kendryte_dt_ids);

static struct platform_driver sdhci_dwcmshc_kendryte_driver = {
	.driver	= {
		.name	= "sdhci-dwcmshc-kendryte",
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
		.of_match_table = sdhci_dwcmshc_kendryte_dt_ids,
		.pm = &dwcmshc_pmops,
	},
	.probe	= dwcmshc_probe,
	.remove	= dwcmshc_remove,
};
module_platform_driver(sdhci_dwcmshc_kendryte_driver);

MODULE_DESCRIPTION("SDHCI platform driver for Synopsys DWC MSHC kendryte");
MODULE_LICENSE("GPL v2");
