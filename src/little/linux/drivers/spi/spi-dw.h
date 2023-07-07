/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DW_SPI_HEADER_H
#define DW_SPI_HEADER_H

#include <linux/bits.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/irqreturn.h>
#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/spi/spi-mem.h>

/* Register offsets */
#define DW_SPI_CTRLR0			0x00
#define DW_SPI_CTRLR1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFTLR			0x18
#define DW_SPI_RXFTLR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_AXIAWLEN			0x50
#define DW_SPI_AXIARLEN			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60
#define DW_SPI_RX_SAMPLE_DLY		0xf0
#define DW_SPI_CS_OVERRIDE		0xf4
#define DW_SPI_SPI_CTRLR0		0xf4
#define DW_SPI_DDR_DRIVE_EDGE	0xf8

#define DW_SPI_SPIDR			0x120
#define DW_SPI_SPIAR			0x124
#define DW_SPI_AXIAR0			0x128
#define DW_SPI_AXIAR1			0x12c
#define DW_SPI_AXIECR			0x130
#define DW_SPI_DONECR			0x134

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET			0

#define SPI_FRF_OFFSET			4
#define SPI_FRF_SPI			0x0
#define SPI_FRF_SSP			0x1
#define SPI_FRF_MICROWIRE		0x2
#define SPI_FRF_RESV			0x3

#define SPI_MODE_OFFSET			6
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7

#define SPI_TMOD_OFFSET			8
#define SPI_TMOD_MASK			(0x3 << SPI_TMOD_OFFSET)
#define	SPI_TMOD_TR			0x0		/* xmit & recv */
#define SPI_TMOD_TO			0x1		/* xmit only */
#define SPI_TMOD_RO			0x2		/* recv only */
#define SPI_TMOD_EPROMREAD		0x3		/* eeprom read mode */

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12

/* Bit fields in CTRLR0 based on DWC_ssi_databook.pdf v1.01a */
#define DWC_SSI_CTRLR0_SRL_OFFSET	13
#define DWC_SSI_CTRLR0_TMOD_OFFSET	10
#define DWC_SSI_CTRLR0_TMOD_MASK	GENMASK(11, 10)
#define DWC_SSI_CTRLR0_SCPOL_OFFSET	9
#define DWC_SSI_CTRLR0_SCPH_OFFSET	8
#define DWC_SSI_CTRLR0_FRF_OFFSET	6
#define DWC_SSI_CTRLR0_DFS_OFFSET	0

/*
 * For Keem Bay, CTRLR0[31] is used to select controller mode.
 * 0: SSI is slave
 * 1: SSI is master
 */
#define DWC_SSI_CTRLR0_KEEMBAY_MST	BIT(31)

/* Bit fields in CTRLR0 for enhanced SPI */
#define DW_HSSI_CTRLR0_SPI_FRF_MASK		GENMASK(23, 22)
#define DW_SPI_CTRLR0_SPI_FRF_STD_SPI		0x0
#define DW_SSI_CTRLR0_SPI_FRF_DUAL_SPI		0x1
#define DW_SSI_CTRLR0_SPI_FRF_QUAD_SPI		0x2
#define DW_SSI_CTRLR0_SPI_FRF_OCT_SPI		0x3
/* Bit fields in CTRLR1 */
#define SPI_NDF_MASK			GENMASK(15, 0)

#define DW_SPI_SPI_CTRLR0_CLK_STRETCH_EN	BIT(30)
#define DW_SPI_SPI_CTRLR0_WAIT_CYCLE_MASK	GENMASK(15, 11)
#define DW_SPI_SPI_CTRLR0_INST_L_MASK		GENMASK(9, 8)
#define DW_SPI_SPI_CTRLR0_INST_L_INST_L0	0x0
#define DW_SPI_SPI_CTRLR0_INST_L_INST_L8	0x2
#define DW_SPI_SPI_CTRLR0_INST_L_INST_L16	0x3
#define DW_SPI_SPI_CTRLR0_ADDR_L_MASK		GENMASK(5, 2)
#define DW_HSSI_SPI_CTRLR0_ADDR_L32			0x8
#define DW_SPI_SPI_CTRLR0_TRANS_TYPE_MASK		GENMASK(1, 0)
#define DW_SPI_SPI_CTRLR0_TRANS_TYPE_TT0	0x0
#define DW_SPI_SPI_CTRLR0_TRANS_TYPE_TT1	0x1
#define DW_SPI_SPI_CTRLR0_TRANS_TYPE_TT2	0x2

/* Bit fields in SR, 7 bits */
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT			(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)

/* Bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI			(1 << 0)
#define SPI_INT_TXOI			(1 << 1)
#define SPI_INT_RXUI			(1 << 2)
#define SPI_INT_RXOI			(1 << 3)
#define SPI_INT_RXFI			(1 << 4)
#define SPI_INT_MSTI			(1 << 5)
#define SPI_INT_DONE			(1 << 11)

/* Bit fields in DMACR */
#define SPI_DMA_RDMAE			(1 << 0)
#define SPI_DMA_TDMAE			(1 << 1)

#define SPI_WAIT_RETRIES		5
#define SPI_BUF_SIZE \
	(sizeof_field(struct spi_mem_op, cmd.opcode) + \
	 sizeof_field(struct spi_mem_op, addr.val) + 256)
#define SPI_GET_BYTE(_val, _idx) \
	((_val) >> (BITS_PER_BYTE * (_idx)) & 0xff)

enum dw_ssi_type {
	SSI_MOTO_SPI = 0,
	SSI_TI_SSP,
	SSI_NS_MICROWIRE,
};

/* DW SPI capabilities */
#define DW_SPI_CAP_CS_OVERRIDE		BIT(0)
#define DW_SPI_CAP_KEEMBAY_MST		BIT(1)
#define DW_SPI_CAP_DWC_SSI		BIT(2)
#define DW_SPI_CAP_EXT_SPI BIT(2)

typedef enum {
    BYTE_1 = 0,
    BYTE_2 = 1,
    BYTE_4 = 2,
    BYTE_8 = 3
} ATW;

typedef struct _spi_txftlr_t
{
    //When the number of transmit FIFO entries is less than orequal to this value, the transmit FIFO empty interrupt is triggered.
    uint32_t tft:8;
    uint32_t rsvd0:8;
    //In Internal DMA mode, this field sets the minimum amount of data frames present in the FIFO after which DWC_ssi starts the transfer.
    uint32_t txfthr:11;
    uint32_t rsvd1:5;
} __attribute__((packed, aligned(4))) spi_txftlr_t;

typedef union _spi_txftlr_u
{
    spi_txftlr_t txftlr;
    uint32_t data;
} spi_txftlr_u;

typedef struct _spi_axiarlen_t
{
    uint32_t rsvd0:8;
    uint32_t arlen:8;
    uint32_t rsvd1:16;
} __attribute__((packed, aligned(4))) spi_axiarlen_t;

typedef union _spi_axiarlen_u
{
    spi_axiarlen_t axiarlen;
    uint32_t data;
} spi_axiarlen_u;

typedef struct _spi_axiawlen_t
{
    uint32_t rsvd0:8;
    uint32_t awlen:8;
    uint32_t rsvd1:16;
} __attribute__((packed, aligned(4))) spi_axiawlen_t;

typedef union _spi_axiawlen_u
{
    spi_axiawlen_t axiawlen;
    uint32_t data;
} spi_axiawlen_u;

typedef struct _spi_dmacr_t
{
    uint32_t rsvd0:2;
    uint32_t idmae:1;
    uint32_t atw:2;
    uint32_t rsvd1:1;
    uint32_t ainc:1;
    uint32_t rsvd2:1;
    uint32_t acache:4;
    uint32_t aprot:3;
    uint32_t aid:4;
    uint32_t rsvd3:13;
} __attribute__((packed, aligned(4))) spi_dmacr_t;

typedef union _spi_dmacr_u
{
    spi_dmacr_t dmacr;
    uint32_t data;
} spi_dmacr_u;

/* Slave spi_transfer/spi_mem_op related */
struct dw_spi_cfg {
	u8 tmode;
	u8 dfs;
	u32 ndf;
	u32 freq;
	u8 spi_frf;
	u8 trans_t;
	u8 inst_l;
	u8 addr_l;
	u8 wait_c;
	u8 dtr;
};

struct dw_spi;
struct dw_spi_dma_ops {
	int (*dma_init)(struct device *dev, struct dw_spi *dws);
	void (*dma_exit)(struct dw_spi *dws);
	int (*dma_setup)(struct dw_spi *dws, struct spi_transfer *xfer);
	bool (*can_dma)(struct spi_controller *master, struct spi_device *spi,
			struct spi_transfer *xfer);
	int (*dma_transfer)(struct dw_spi *dws, struct spi_transfer *xfer);
	void (*dma_stop)(struct dw_spi *dws);
};

struct dw_spi {
	struct spi_controller	*master;

	void __iomem		*regs;
	unsigned long		paddr;
	int			irq;
	int         txe_irq;
	int			rxf_irq;
	int			done_irq;
	u32			fifo_len;	/* depth of the FIFO buffer */
	u32			max_mem_freq;	/* max mem-ops bus freq */
	u32			max_freq;	/* max bus freq supported */

	u32			caps;		/* DW SPI capabilities */

	u32			reg_io_width;	/* DR I/O width in bytes */
	u16			bus_num;
	u16			num_cs;		/* supported slave numbers */
	void (*set_cs)(struct spi_device *spi, bool enable);

	/* Current message transfer state info */
	void			*tx;
	unsigned int		tx_len;
	void			*rx;
	unsigned int		rx_len;
	u8			buf[SPI_BUF_SIZE];
	int			dma_mapped;
	u8			n_bytes;	/* current is a 1/2 bytes op */
	irqreturn_t		(*transfer_handler)(struct dw_spi *dws);
	u32			current_freq;	/* frequency in hz */
	u32			cur_rx_sample_dly;
	u32			def_rx_sample_dly_ns;

	/* Custom memory operations */
	struct spi_controller_mem_ops mem_ops;

	/* DMA info */
	struct dma_chan		*txchan;
	u32			txburst;
	struct dma_chan		*rxchan;
	u32			rxburst;
	u32			dma_sg_burst;
	unsigned long		dma_chan_busy;
	dma_addr_t		dma_addr; /* phy address of the Data register */
	const struct dw_spi_dma_ops *dma_ops;
	struct completion	dma_completion;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
	struct debugfs_regset32 regset;
#endif
};

static inline u32 dw_readl(struct dw_spi *dws, u32 offset)
{
	return __raw_readl(dws->regs + offset);
}

static inline void dw_writel(struct dw_spi *dws, u32 offset, u32 val)
{
	__raw_writel(val, dws->regs + offset);
}

static inline u32 dw_read_io_reg(struct dw_spi *dws, u32 offset)
{
	switch (dws->reg_io_width) {
	case 2:
		return readw_relaxed(dws->regs + offset);
	case 4:
	default:
		return readl_relaxed(dws->regs + offset);
	}
}

static inline void dw_write_io_reg(struct dw_spi *dws, u32 offset, u32 val)
{
	switch (dws->reg_io_width) {
	case 2:
		writew_relaxed(val, dws->regs + offset);
		break;
	case 4:
	default:
		writel_relaxed(val, dws->regs + offset);
		break;
	}
}

static inline void spi_enable_chip(struct dw_spi *dws, int enable)
{
	dw_writel(dws, DW_SPI_SSIENR, (enable ? 1 : 0));
}

static inline void spi_set_clk(struct dw_spi *dws, u16 div)
{
	dw_writel(dws, DW_SPI_BAUDR, div);
}

/* Disable IRQ bits */
static inline void spi_mask_intr(struct dw_spi *dws, u32 mask)
{
	u32 new_mask;

	new_mask = dw_readl(dws, DW_SPI_IMR) & ~mask;
	dw_writel(dws, DW_SPI_IMR, new_mask);
}

/* Enable IRQ bits */
static inline void spi_umask_intr(struct dw_spi *dws, u32 mask)
{
	u32 new_mask;

	new_mask = dw_readl(dws, DW_SPI_IMR) | mask;
	dw_writel(dws, DW_SPI_IMR, new_mask);
}

/*
 * This disables the SPI controller, interrupts, clears the interrupts status
 * and CS, then re-enables the controller back. Transmit and receive FIFO
 * buffers are cleared when the device is disabled.
 */
static inline void spi_reset_chip(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
	spi_mask_intr(dws, 0xff);
	dw_readl(dws, DW_SPI_ICR);
	dw_writel(dws, DW_SPI_SER, 0);
	spi_enable_chip(dws, 1);
}

static inline void spi_shutdown_chip(struct dw_spi *dws)
{
	spi_enable_chip(dws, 0);
	spi_set_clk(dws, 0);
}

extern void dw_spi_set_cs(struct spi_device *spi, bool enable);
extern void dw_spi_update_config(struct dw_spi *dws, struct spi_device *spi,
				 struct dw_spi_cfg *cfg);
extern int dw_spi_check_status(struct dw_spi *dws, bool raw);
extern int dw_spi_add_host(struct device *dev, struct dw_spi *dws);
extern void dw_spi_remove_host(struct dw_spi *dws);
extern int dw_spi_suspend_host(struct dw_spi *dws);
extern int dw_spi_resume_host(struct dw_spi *dws);

#ifdef CONFIG_SPI_DW_DMA

extern void dw_spi_dma_setup_mfld(struct dw_spi *dws);
extern void dw_spi_dma_setup_generic(struct dw_spi *dws);

#else

static inline void dw_spi_dma_setup_mfld(struct dw_spi *dws) {}
static inline void dw_spi_dma_setup_generic(struct dw_spi *dws) {}

#endif /* !CONFIG_SPI_DW_DMA */

#endif /* DW_SPI_HEADER_H */
