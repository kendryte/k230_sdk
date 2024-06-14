/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

//#define DEBUG
//#define VERBOSE_DEBUG

#include <linux/bitmap.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include "virt-dma.h"

#define PDMA_MAX_LINE_SIZE      0x3FFFFFFF

/* interrupt mask */
#define PDONE_INT               0x00000001
#define PITEM_INT               0x00000100
#define PPAUSE_INT              0x00010000
#define PTOUT_INT               0x01000000

/* llt structure */
typedef struct pdma_llt
{
    u32 line_size : 30;
    u32 pause : 1;
    u32 node_intr : 1;
    u32 src_addr;
    u32 dst_addr;
    u32 next_llt_addr;
} pdma_llt_t;

/* register structure */
typedef struct pdma_ch_cfg
{
    u32 ch_src_type : 1;
    u32 ch_dev_hsize : 2;
    u32 reserved0 : 1;
    u32 ch_dat_endian : 2;
    u32 reserved1 : 2;
    u32 ch_dev_blen : 4;
    u32 ch_priority : 4;
    u32 ch_dev_tout : 12;
    u32 reserved2 : 4;
} pdma_ch_cfg_t;


/**
 * @param TX: means transmitting data from system memory
 *            to external peripheral devices
 * @param RX: means receiving data from peripheral devices
 *            to system memory
 */
typedef enum pdma_rxtx
{
    TX = 0,
    RX = 1,
} pdma_rxtx_e;

typedef enum pdma_ch
{
    PDMA_CH_0 = 0,
    PDMA_CH_1 = 1,
    PDMA_CH_2 = 2,
    PDMA_CH_3 = 3,
    PDMA_CH_4 = 4,
    PDMA_CH_5 = 5,
    PDMA_CH_6 = 6,
    PDMA_CH_7 = 7,
    PDMA_CH_MAX,
} pdma_ch_e;


typedef enum pdma_burst_len
{
    PBURST_LEN_1 = 0,
    PBURST_LEN_2 = 1,
    PBURST_LEN_3 = 2,
    PBURST_LEN_4 = 3,
    PBURST_LEN_5 = 4,
    PBURST_LEN_6 = 5,
    PBURST_LEN_7 = 6,
    PBURST_LEN_8 = 7,
    PBURST_LEN_9 = 8,
    PBURST_LEN_10 = 9,
    PBURST_LEN_11 = 10,
    PBURST_LEN_12 = 11,
    PBURST_LEN_13 = 12,
    PBURST_LEN_14 = 13,
    PBURST_LEN_15 = 14,
    PBURST_LEN_16 = 15,
} pdma_burst_len_e;


/* Global register offsets */
#define PDMA_CH_EN		0x0
#define PDMA_INT_MASK           0x4
#define PDMA_INT_STAT		0x8

/* Channel register offsets */
#define CH_CTL			0x0
#define CH_STAT			0x4
#define CH_CFG			0x8
#define CH_LLT_SADDR		0xc


#define CH_OFF			0x20
#define CH0_BASE		0x20

#define CH_NUM			8

#define MAX_LINE_SIZE		0xffff
#define MAX_LINE_NUM		0x3ff
#define RECT_MAX_TRANS_LEN		(MAX_LINE_SIZE*MAX_LINE_NUM)


#define CH_STAT_BUSY		BIT(0)
#define CH_STAT_PAUSE		BIT(1)


typedef enum ch_peri_dev_sel {
    UART0_TX = 0,
    UART0_RX = 1,
    UART1_TX = 2,
    UART1_RX = 3,
    UART2_TX = 4,
    UART2_RX = 5,
    UART3_TX = 6,
    UART3_RX = 7,
    UART4_TX = 8,
    UART4_RX = 9,
    I2C0_TX = 10,
    I2C0_RX = 11,
    I2C1_TX = 12,
    I2C1_RX = 13,
    I2C2_TX = 14,
    I2C2_RX = 15,
    I2C3_TX = 16,
    I2C3_RX = 17,
    I2C4_TX = 18,
    I2C4_RX = 19,
    AUDIO_TX = 20,
    AUDIO_RX = 21,
    JAMLINK0_TX = 22,
    JAMLINK0_RX = 23,
    JAMLINK1_TX = 24,
    JAMLINK1_RX = 25,
    JAMLINK2_TX = 26,
    JAMLINK2_RX = 27,
    JAMLINK3_TX = 28,
    JAMLINK3_RX = 29,
    ADC0 = 30,
    ADC1 = 31,
    ADC2 = 32,
    PDM_IN = 33,
    PERI_DEV_SEL_MAX,
} ch_peri_dev_sel_t;

/* usr pdma structure */
typedef struct usr_pdma_cfg
{
    pdma_ch_e ch;
    ch_peri_dev_sel_t device;
    dma_addr_t src_addr;
    dma_addr_t dst_addr;
    u32 line_size;
    pdma_ch_cfg_t pdma_ch_cfg;
} usr_pdma_cfg_t;

typedef enum peridma_mode {
    LINE_MODE,
    RECT_MODE,
    DMA_MODE_MAX
} peridma_mode_t;

typedef enum ch_src_type {
    SRC_TYPE_MEM,
    SRC_TYPE_DEV,
    SRC_TYPE_MAX
} ch_src_type_t;

typedef enum ch_dat_endian {
    DAT_ENDIAN_DEF,
    DAT_ENDIAN_TWO_BYTE,
    DAT_ENDIAN_FOUR_BYTE,
    DAT_ENDIAN_EIGHT_BYTE,
    DAT_ENDIAN_MAX
} ch_dat_endian_t;

typedef enum ch_dev_hsize {
    DEV_HSIZE_ONE_BYTE,
    DEV_HSIZE_TWO_BYTE,
    DEV_HSIZE_FOUR_BYTE,
    DEV_HSIZE_INVALID
} ch_dev_hsize_t;

/** DMA Driver **/
struct k230_peridma_hwdesc {
    usr_pdma_cfg_t              usr_pdma_chn_cfg;
    bool                        last;
    struct list_head		list;
};

struct k230_peridma_desc {
    struct virt_dma_desc            vd;
    struct k230_peridma_vchan	*vchan;
    struct list_head                xfer_list;
    struct list_head                completed_list;
    struct k230_peridma_hwdesc	*hwdesc;
    atomic_t			hwdesc_remain;
    bool				next;
    struct k230_peridma_hwdesc *cyclic;
    u32 remain;
    u32 buf_len;
};


struct k230_peridma_vchan {
        struct k230_peridma_dev		*pdev;
	struct virt_dma_chan		vchan;
	struct dma_slave_config		cfg;
	struct k230_peridma_desc	*desc;
	struct k230_peridma_hwdesc	*cur_hwdesc;
	atomic_t			descs_allocated;
	struct k230_peridma_pchan	*pchan;
	enum   dma_transfer_direction 	dir;
	dma_addr_t			dev_addr;
	u32				priority;
	u32				dev_tout;
	ch_dat_endian_t			dat_endian;
	ch_peri_dev_sel_t		dev_sel;
	enum dma_status			status;
        bool                            is_paused;

};

struct k230_peridma_pchan {
        struct k230_peridma_dev         *pdev;
        /* Register base of channel */
        void __iomem                    *ch_regs;
        u8                              id;
        struct k230_peridma_vchan	*vchan;
        bool                            is_paused;

};

struct k230_peridma_dev {
	DECLARE_BITMAP(pchans_used, CH_NUM);
        struct device			*dev;
	struct dma_device		slave;
	struct k230_peridma_vchan	*vchan;
	struct k230_peridma_pchan	*pchan;
        u32				nr_channels;
        u32				nr_requests;
	void __iomem			*base;
	struct clk			*clk;
	int				irq;
	spinlock_t			lock;
};


typedef struct pdma_chn_llt
{
        dma_addr_t llt_list_p;
        pdma_llt_t *llt_list_v;
        bool        use;
}pdma_chn_llt_t;
static pdma_chn_llt_t g_pdma_llt_list[CH_NUM];

static void free_pchan(struct k230_peridma_dev *pdev, struct k230_peridma_pchan *pchan);

static struct k230_peridma_dev *to_k230_peridma_dev(struct dma_device *dev)
{
	return container_of(dev, struct k230_peridma_dev, slave);
}

static struct k230_peridma_vchan *to_k230_peridma_vchan(struct dma_chan *chan)
{
        return container_of(chan, struct k230_peridma_vchan, vchan.chan);
}

static struct device *vchan2dev(struct k230_peridma_vchan *vchan)
{
	return &vchan->vchan.chan.dev->device;
}

static inline struct k230_peridma_desc *vd_to_k230_peridma_desc(struct virt_dma_desc *vd)
{
        return container_of(vd, struct k230_peridma_desc, vd);
}

static inline struct k230_peridma_vchan *vc_to_k230_peridma_vchan(struct virt_dma_chan *vc)
{
        return container_of(vc, struct k230_peridma_vchan, vchan);
}

static inline struct k230_peridma_vchan *dchan_to_k230_peridma_vchan(struct dma_chan *dchan)
{
        return vc_to_k230_peridma_vchan(to_virt_chan(dchan));
}

static inline const char *peridma_vchan_name(struct k230_peridma_vchan *vchan)
{
        return dma_chan_name(&vchan->vchan.chan);
}

static inline bool peridma_pchan_is_busy(struct k230_peridma_pchan *pchan)
{
        u32 val;

        val = ioread32(pchan->ch_regs + CH_STAT);

        return (val & CH_STAT_BUSY);
}

static inline bool peridma_pchan_is_idle(struct k230_peridma_pchan *pchan)
{
        return true;
        //return !peridma_pchan_is_busy(pchan);
}


static inline bool peridma_pchan_is_paused(struct k230_peridma_pchan *pchan)
{
        u32 val;

        val = ioread32(pchan->ch_regs + CH_STAT);

        return (val & CH_STAT_PAUSE);
}

static void k230_peridma_free_chan_resources(struct dma_chan *chan)
{
	struct k230_peridma_vchan *pvchan = to_k230_peridma_vchan(chan);

	vchan_free_chan_resources(&pvchan->vchan);
}

static inline void peridma_pchan_irq_clear(struct k230_peridma_pchan *pchan)
{
        struct k230_peridma_dev *pdev = pchan->pdev;
        u32 reg;
        u32 ch_id = pchan->id;
        u32 clear = PDONE_INT | PITEM_INT | PPAUSE_INT | PTOUT_INT;

        reg = ioread32(pdev->base + PDMA_INT_STAT);
        reg |= (clear << ch_id);
        iowrite32(reg, pdev->base + PDMA_INT_STAT);
}

static inline void peridma_pchan_irq_enable(struct k230_peridma_pchan *pchan)
{
	struct k230_peridma_dev *pdev = pchan->pdev;
        u32 reg;
	u32 ch_id = pchan->id;

        reg = ioread32(pdev->base + PDMA_CH_EN);
        reg |= (1 << ch_id);
        iowrite32(reg, pdev->base + PDMA_CH_EN);
}

static inline void peridma_pchan_irq_disable(struct k230_peridma_pchan *pchan)
{
	struct k230_peridma_dev *pdev = pchan->pdev;
        u32 reg;
	u32 ch_id = pchan->id;

        reg = ioread32(pdev->base + PDMA_CH_EN);
        reg &= (~(1 << ch_id));
        iowrite32(reg, pdev->base + PDMA_CH_EN);
}

static inline void peridma_pchan_start(struct k230_peridma_pchan *pchan)
{
        u32 reg = 0;
        reg = 0x1;
        iowrite32(reg, pchan->ch_regs + CH_CTL);
}

static inline void peridma_pchan_stop(struct k230_peridma_pchan *pchan)
{
        u32 reg = 0;
        reg = 0x2;
        iowrite32(reg, pchan->ch_regs + CH_CTL);
}

static inline void peridma_pchan_pause(struct k230_peridma_pchan *pchan)
{
        return ;
}

static inline void peridma_pchan_resume(struct k230_peridma_pchan *pchan)
{
        u32 reg = 0;
        reg = 0x4;
        iowrite32(reg, pchan->ch_regs + CH_CTL);
}

static void k230_peridma_hw_init(struct k230_peridma_dev *pdev)
{
        u32 i;

        iowrite32(0, pdev->base + PDMA_CH_EN);
        iowrite32(0, pdev->base + PDMA_INT_MASK);
        iowrite32(0, pdev->base + PDMA_INT_STAT);

        for (i = 0; i < pdev->nr_channels; i++) {
                peridma_pchan_stop(&pdev->pchan[i]);
        }
}

static void peridma_desc_put(struct k230_peridma_desc *desc)
{
        struct k230_peridma_vchan *vchan = desc->vchan;
        unsigned int descs_put = 1;
        struct k230_peridma_hwdesc *hwdesc, *tmp;

//	vchan->desc = NULL;

        list_for_each_entry_safe(hwdesc, tmp, &desc->xfer_list, list)
                kfree(hwdesc);
        if(!desc->cyclic)
        {
            list_for_each_entry_safe(hwdesc, tmp, &desc->completed_list, list)
                    kfree(hwdesc);
        }

	kfree(desc);

        atomic_sub(descs_put, &vchan->descs_allocated);
        dev_vdbg(vchan2dev(vchan), ": %d descs put, %d still allocated\n",
                descs_put,
                atomic_read(&vchan->descs_allocated));
}

static void vchan_desc_put(struct virt_dma_desc *vdesc)
{
        peridma_desc_put(vd_to_k230_peridma_desc(vdesc));
}

static ch_dev_hsize_t slave_buswidth_to_hsize(enum dma_slave_buswidth buswidth)
{
    ch_dev_hsize_t hsize = DEV_HSIZE_INVALID;

    switch (buswidth) {
    case DMA_SLAVE_BUSWIDTH_1_BYTE:
        hsize = DEV_HSIZE_ONE_BYTE;
        break;

    case DMA_SLAVE_BUSWIDTH_2_BYTES:
        hsize = DEV_HSIZE_TWO_BYTE;
        break;

    case DMA_SLAVE_BUSWIDTH_4_BYTES:
        hsize = DEV_HSIZE_FOUR_BYTE;
        break;

    default:
        hsize = DEV_HSIZE_INVALID;
        break;
    }

    return hsize;
}

static struct k230_peridma_hwdesc *generate_hwdesc(
	struct k230_peridma_vchan *vchan, dma_addr_t src, dma_addr_t dest,
	size_t len, struct dma_slave_config *sconfig, enum dma_transfer_direction dir)
{
	struct k230_peridma_hwdesc *hwdesc = NULL;
	ch_dev_hsize_t hsize = DEV_HSIZE_INVALID;

	hwdesc = kzalloc(sizeof(*hwdesc), GFP_NOWAIT);
	if(!hwdesc){
		dev_err(vchan2dev(vchan), "alloc hwdesc failure");
		return NULL;
	}

        if (dir == DMA_MEM_TO_DEV) {
		hsize = slave_buswidth_to_hsize(sconfig->dst_addr_width);
                //printk("=======DMA_MEM_TO_DEV hsize:%d\n",hsize);
                if(hsize == DEV_HSIZE_INVALID)
			goto err_exit;
		hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_src_type = TX;
                hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_dev_hsize = hsize;
        } else if(dir == DMA_DEV_TO_MEM){
                hsize = slave_buswidth_to_hsize(sconfig->src_addr_width);
                //printk("=======DMA_DEV_TO_MEM hsize:%d\n",hsize);
                if(hsize == DEV_HSIZE_INVALID)
                        goto err_exit;
                hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_src_type = RX;
                hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_dev_hsize = hsize;
        }
        hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_dat_endian = 0;
        hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_dev_blen = PBURST_LEN_8;
        hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_priority = vchan->priority;
        hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg.ch_dev_tout = vchan->dev_tout;

        hwdesc->usr_pdma_chn_cfg.src_addr  = src;
	hwdesc->usr_pdma_chn_cfg.dst_addr = dest;
        hwdesc->usr_pdma_chn_cfg.device = vchan->dev_sel;
        hwdesc->usr_pdma_chn_cfg.line_size = len;

        //printk("=======dev_sel:%d,hsize:%d,src:0x%llx,dst:0x%llx,priority:%d,size:%d,tout:0x%x\n",vchan->dev_sel,hsize,src,dest,vchan->priority,len,vchan->dev_tout);

	//dev_vdbg(vchan2dev(vchan), "generate hwdesc(%px): ctl=%x, cfg0=%x, cfg1=%x, dev_tout=%x, dev_sel=%x, src_addr=%x, dst_addr=%x \n", hwdesc, hwdesc->ctl, hwdesc->cfg0, hwdesc->cfg1, hwdesc->dev_tout, hwdesc->dev_sel, hwdesc->src_addr, hwdesc->dst_addr);

        return hwdesc;

err_exit:
    kfree(hwdesc);
    return NULL;

}

static struct dma_async_tx_descriptor *k230_peridma_prep_slave_sg(
	struct dma_chan *chan, struct scatterlist *sgl, unsigned int sglen,
	enum dma_transfer_direction dir, unsigned long flags, void *context)
{
        //printk("$$$$$$$$$$$k230_peridma_prep_slave_sg\n");
        struct k230_peridma_desc *desc = NULL;
	struct k230_peridma_hwdesc *hwdesc = NULL;
	struct k230_peridma_hwdesc *tmp = NULL;
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(chan);
	struct dma_slave_config *sconfig = &vchan->cfg;
	struct scatterlist *sg;
        size_t xfer_size, avail, total = 0;
	dma_addr_t addr, src = 0, dst = 0;
	int i = 0;

        dev_vdbg(vchan2dev(vchan), ": prep_slave_sg: sgl: %px sglen: %zd flags: %#lx",
                sgl, sglen, flags);
//        printk("k230_peridma_prep_slave_sg: sgl: %px sglen: %zd flags: %#lx",
//                sgl, sglen, flags);

	desc = kzalloc(sizeof(struct k230_peridma_desc), GFP_NOWAIT);

	if(!desc)
		return NULL;

	desc->vchan = vchan;
//	vchan->desc = desc;
	INIT_LIST_HEAD(&desc->xfer_list);
	INIT_LIST_HEAD(&desc->completed_list);
//	atomic_set(&desc->hwdesc_remain, 0);
	atomic_inc(&vchan->descs_allocated);
//	desc->next = false;


	for_each_sg(sgl, sg, sglen, i) {
		addr = sg_dma_address(sg);
		avail = sg_dma_len(sg);

		do {
			xfer_size = min_t(size_t, avail, MAX_LINE_SIZE);

			if (dir == DMA_MEM_TO_DEV) {
                                src = addr;
                                dst = sconfig->dst_addr;
                        } else if (dir == DMA_DEV_TO_MEM) {
                                src = sconfig->src_addr;
                                dst = addr;
                        }

			dev_vdbg(vchan2dev(vchan), "dma_async_tx_descriptor: src=%x, dst=%x, xfer_size=%x \n", src, dst, xfer_size);

			hwdesc = generate_hwdesc(vchan, src, dst, xfer_size, sconfig, dir);
			if(hwdesc)
				list_add_tail(&hwdesc->list, &desc->xfer_list);
			else {
				list_for_each_entry_safe(hwdesc, tmp, &desc->xfer_list, list)
					kfree(hwdesc);

				kfree(desc);
				dev_err(vchan2dev(vchan), "dma_async_tx_descriptor: generate hwdesc failure \n");
				return NULL;
			}
//			atomic_inc(&desc->hwdesc_remain);

			addr += xfer_size;
			avail -= xfer_size;

		}while(avail);

	}

	dev_vdbg(vchan2dev(vchan), "dma_async_tx_descriptor: prepare desc(%px) \n", desc);

        return vchan_tx_prep(&vchan->vchan, &desc->vd, flags);
}

static struct dma_async_tx_descriptor *k230_peridma_prep_dma_cyclic(
	struct dma_chan *chan, dma_addr_t buf_addr, size_t buf_len,
	size_t period_len, enum dma_transfer_direction dir,
	unsigned long flags)
{
    //printk("$$$$$$$$$$$k230_peridma_prep_dma_cyclic buf_addr:0x%llx,buf_len:%d,period_len:%d,dir:%d,num_periods:%d\n",\
    buf_addr,buf_len,period_len,dir,buf_len / period_len);
    struct k230_peridma_desc *desc = NULL;
    struct k230_peridma_hwdesc *hwdesc = NULL;
    struct k230_peridma_hwdesc *tmp = NULL;
    struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(chan);
    struct dma_slave_config *sconfig = &vchan->cfg;
    struct scatterlist *sg;
    size_t xfer_size, avail, total = 0;
    u32 num_periods;
    dma_addr_t addr, src = 0, dst = 0;
    int i = 0;

    desc = kzalloc(sizeof(struct k230_peridma_desc), GFP_NOWAIT);

    if(!desc)
        return NULL;

    desc->vchan = vchan;
    desc->buf_len = buf_len;
    desc->remain = buf_len;
    INIT_LIST_HEAD(&desc->xfer_list);

    atomic_inc(&vchan->descs_allocated);

    num_periods = buf_len / period_len;
    for(i = 0; i < num_periods; i++)
    {
        addr = buf_addr + period_len*i;
        avail = period_len;

        do {
            xfer_size = min_t(size_t, avail, MAX_LINE_SIZE);

            if (dir == DMA_MEM_TO_DEV) {
                src = addr;
                dst = sconfig->dst_addr;
            } else if (dir == DMA_DEV_TO_MEM) {
                src = sconfig->src_addr;
                dst = addr;
            }

            dev_vdbg(vchan2dev(vchan),  "dma_async_tx_descriptor: src=%x, dst=%x, xfer_size=%x \n", src, dst, xfer_size);

            hwdesc = generate_hwdesc(vchan, src, dst, xfer_size, sconfig, dir);
            if(hwdesc)
            {
                list_add_tail(&hwdesc->list, &desc->xfer_list);
                if(desc->cyclic == NULL)
                    desc->cyclic = hwdesc;
            }
            else {
                list_for_each_entry_safe(hwdesc, tmp, &desc->xfer_list, list)
                kfree(hwdesc);

                kfree(desc);
                dev_vdbg(vchan2dev(vchan), "dma_async_tx_descriptor: generate hwdesc failure \n");
                return NULL;
            }

            addr += xfer_size;
            avail -= xfer_size;
            if(avail == 0)
                hwdesc->last = true;
        } while(avail);
    }

    return vchan_tx_prep(&vchan->vchan, &desc->vd, flags);
}

static int k230_peridma_terminate_all(struct dma_chan *dchan)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);
        unsigned long flags;
        LIST_HEAD(head);

        spin_lock_irqsave(&vchan->vchan.lock, flags);

	if(vchan->pchan){
		dev_vdbg(vchan2dev(vchan), "k230_peridma_terminate_all: release pchan %x \n", vchan->pchan->id);
		peridma_pchan_irq_disable(vchan->pchan);
		peridma_pchan_stop(vchan->pchan);
		free_pchan(vchan->pdev,vchan->pchan);
		vchan->pchan = NULL;
	} else
		dev_vdbg(vchan2dev(vchan), "k230_peridma_terminate_all: vchan->pchan is NULL \n");

	if(vchan->desc){
//		vchan_terminate_vdesc(&vchan->desc->vd);
		vchan->desc = NULL;
	}

	vchan->is_paused = false;

        vchan_get_all_descriptors(&vchan->vchan, &head);

        /*
         * As vchan_dma_desc_free_list can access to desc_allocated list
         * we need to call it in vc.lock context.
         */
        vchan_dma_desc_free_list(&vchan->vchan, &head);

        spin_unlock_irqrestore(&vchan->vchan.lock, flags);

        dev_vdbg(vchan2dev(vchan), "terminated \n");

        return 0;
}

// 无论是cycle还是sg类型的buffer，
// 从desc的第一个hwdesc节点开始遍历到最后一个hwdesc，得到剩余下的需要传输数据长度
static size_t k230_peridma_desc_residue(struct k230_peridma_vchan *chan,
				     struct k230_peridma_desc *desc)
{
	struct k230_peridma_hwdesc *hwdesc = NULL;
	u32 residue = 0;
        residue = desc->remain;

	return residue;
}

static enum dma_status k230_peridma_tx_status(struct dma_chan *dchan,
					   dma_cookie_t cookie,
					   struct dma_tx_state *txstate)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);
        enum dma_status ret;
        struct virt_dma_desc *vdesc;
        unsigned long flags;
	    u32 residue = 0;

        ret = dma_cookie_status(dchan, cookie, txstate);
        if (ret == DMA_COMPLETE)
		    return ret;

        spin_lock_irqsave(&vchan->vchan.lock, flags);
        ret = dma_cookie_status(dchan, cookie, txstate);
        if (ret != DMA_COMPLETE)
        {
            vdesc = vchan_find_desc(&vchan->vchan, cookie);
            if (vdesc)
            {
                residue = k230_peridma_desc_residue(vchan,
                                vd_to_k230_peridma_desc(vdesc));
            }
        }
        spin_unlock_irqrestore(&vchan->vchan.lock, flags);
        dma_set_residue(txstate, residue);

        if (vchan->is_paused && ret == DMA_IN_PROGRESS)
            ret = DMA_PAUSED;

        return ret;
}


static void free_pchan(struct k230_peridma_dev *pdev, struct k230_peridma_pchan *pchan)
{
        unsigned long flags;
        int nr = pchan - pdev->pchan;
	struct k230_peridma_vchan *vchan;

        spin_lock_irqsave(&pdev->lock, flags);

	vchan = pchan->vchan;
	vchan->pchan = NULL;
        pchan->vchan = NULL;
        clear_bit(nr, pdev->pchans_used);

        spin_unlock_irqrestore(&pdev->lock, flags);

}

static struct k230_peridma_pchan *find_phy_chan(struct k230_peridma_dev *pdev, struct k230_peridma_vchan *vchan)
{
        struct k230_peridma_pchan *pchan = NULL, *pchans = pdev->pchan;
        unsigned long flags;
        int i=0, ch_num;

        spin_lock_irqsave(&pdev->lock, flags);
        for_each_clear_bit_from(i, pdev->pchans_used, CH_NUM) {
                pchan = &pchans[i];
                pchan->vchan = vchan;
                pdev->pchan[i].vchan = vchan;
                set_bit(i, pdev->pchans_used);
                break;
        }
        spin_unlock_irqrestore(&pdev->lock, flags);

        return pchan;
}

static u32* pdma_llt_cal(struct device *dev,struct k230_peridma_hwdesc *hwdesc,u8 chn)
{
        int i;
        u32 list_num;
        pdma_llt_t *llt_list;
        dma_addr_t llt_list_hard;

        list_num = (hwdesc->usr_pdma_chn_cfg.line_size - 1) / PDMA_MAX_LINE_SIZE + 1;

        llt_list = g_pdma_llt_list[chn].llt_list_v;
        g_pdma_llt_list[chn].use = true;

        for (i = 0; i < list_num; i++) {
                llt_list[i].src_addr = ((u32)(uintptr_t)hwdesc->usr_pdma_chn_cfg.src_addr + PDMA_MAX_LINE_SIZE*i);
                llt_list[i].dst_addr = ((u32)(uintptr_t)hwdesc->usr_pdma_chn_cfg.dst_addr + PDMA_MAX_LINE_SIZE*i);

                //printk("===pdma_llt_cal src_addr:0x%llx,dst_addr:0x%llx,list_num:%d\n",llt_list[i].src_addr,llt_list[i].dst_addr,list_num);
                if (i == list_num -1) {
                        llt_list[i].line_size = hwdesc->usr_pdma_chn_cfg.line_size % PDMA_MAX_LINE_SIZE;
                        llt_list[i].next_llt_addr = 0;
                } else {
                        llt_list[i].line_size = PDMA_MAX_LINE_SIZE;
                        llt_list[i].next_llt_addr = (u32)(uintptr_t)(&llt_list[i+1]);
                }
        }

        return (u32 *)g_pdma_llt_list[chn].llt_list_p;
}

static void configure_pchan(struct k230_peridma_pchan *pchan, struct k230_peridma_hwdesc *hwdesc)
{
	struct k230_peridma_dev *pdev = pchan->pdev;
	unsigned long flags;

        if (unlikely(!peridma_pchan_is_idle(pchan))) {
                dev_err(vchan2dev(pchan->vchan), "pchan %d is non-idle!\n",
                        pchan->id);

                return;
        }

        hwdesc->usr_pdma_chn_cfg.ch = (pdma_ch_e)pchan->id;

        u32 ch_cfg = *(u32*)&hwdesc->usr_pdma_chn_cfg.pdma_ch_cfg;
        iowrite32(ch_cfg, pchan->ch_regs+CH_CFG);
        iowrite32(hwdesc->usr_pdma_chn_cfg.device, pdev->base + 0x120 + pchan->id*4);
        iowrite32((u32)(uintptr_t)pdma_llt_cal(pdev->dev,hwdesc,pchan->id),pchan->ch_regs+CH_LLT_SADDR);
}

static void peridma_vchan_start_first_queued(struct k230_peridma_vchan *vchan)
{
        struct k230_peridma_desc *desc;
        struct virt_dma_desc *vd;
        struct k230_peridma_dev *pdev = vchan->pdev;
        struct k230_peridma_pchan *pchan = NULL;
        struct k230_peridma_hwdesc *hwdesc = NULL;
	unsigned long flags;


        if(vchan->desc) {
                dev_err(vchan2dev(vchan), "%s already processing something \n", peridma_vchan_name(vchan));
		dump_stack();
                return;
        }

        if(vchan->pchan) {
		pchan = vchan->pchan;
                dev_vdbg(vchan2dev(vchan), "%s already allocated phy channel \n", peridma_vchan_name(vchan));
        } else {
                pchan = find_phy_chan(pdev, vchan);
                if(pchan)
                {
                        vchan->pchan = pchan;
                        //printk("============:vchan name:%s allocated phy channel:%d\n",peridma_vchan_name(vchan),vchan->pchan->id);
                }
                else
                        return;
        }



        vd = vchan_next_desc(&vchan->vchan);
        if (!vd) {
		dev_vdbg(vchan2dev(vchan), "peridma_vchan_start_first_queued: no next vd found \n");
                goto freepchan;
	}

        desc = vd_to_k230_peridma_desc(vd);
        dev_vdbg(vchan2dev(vchan), "peridma_vchan_start_first_queued: desc %px \n", desc);

	if(!list_empty(&desc->xfer_list)){
        	hwdesc = list_first_entry(&desc->xfer_list, struct k230_peridma_hwdesc, list);
//		list_del(&hwdesc->list);
//		atomic_sub(1, &desc->hwdesc_remain);

		//dev_vdbg(vchan2dev(vchan), "peridma_vchan_start_first_queued: hwdesc(%px) \n", hwdesc);
		//dev_vdbg(vchan2dev(vchan), " ctl=%x, cfg0=%x, cfg1=%x, dev_tout=%x, dev_sel=%x, src_addr=%x, dst_addr=%x \n", hwdesc->ctl, hwdesc->cfg0, hwdesc->cfg1, hwdesc->dev_tout, hwdesc->dev_sel, hwdesc->src_addr, hwdesc->dst_addr);
                vchan->desc = desc;
                desc->hwdesc = hwdesc;

		peridma_pchan_irq_clear(vchan->pchan);
                configure_pchan(vchan->pchan, desc->hwdesc);
                peridma_pchan_irq_enable(vchan->pchan);
                peridma_pchan_start(vchan->pchan);


                return;
        }


freepchan:
//	dev_vdbg(vchan2dev(vchan), "peridma_vchan_start_first_queued: free_pchan \n");
//        free_pchan(pdev, pchan);
        return;

}


static void peridma_vchan_xfer_complete(struct k230_peridma_vchan *vchan)
{
        struct virt_dma_desc *vd;
        unsigned long flags;

//        spin_lock_irqsave(&vchan->vchan.lock, flags);

        if (unlikely(!peridma_pchan_is_idle(vchan->pchan))) {
                dev_err(vchan2dev(vchan), "BUG: %s caught chx_tr_done, but channel not idle!\n",
                        peridma_vchan_name(vchan));
                peridma_pchan_stop(vchan->pchan);
        }

        /* The completed descriptor currently is in the head of vc list */
        vd = vchan_next_desc(&vchan->vchan);

        /* Remove the completed descriptor from issued list before completing */
        list_del(&vd->node);
        vchan_cookie_complete(vd);

        /* Submit queued descriptors after processing the completed ones */
        peridma_vchan_start_first_queued(vchan);

//        spin_unlock_irqrestore(&vchan->vchan.lock, flags);
}


static void k230_peridma_issue_pending(struct dma_chan *dchan)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);
        unsigned long flags;

        spin_lock_irqsave(&vchan->vchan.lock, flags);

        if (vchan_issue_pending(&vchan->vchan)){
	        dev_vdbg(vchan2dev(vchan), "k230_peridma_issue_pending: 1 \n");
                peridma_vchan_start_first_queued(vchan);
	}
	dev_vdbg(vchan2dev(vchan), "k230_peridma_issue_pending: 2 \n");

        spin_unlock_irqrestore(&vchan->vchan.lock, flags);
}

static void _pdma_chn_state_clear(pdma_ch_e pdma_chn, u32* int_state)
{
    if (*int_state & (1 << pdma_chn))
    {
        *int_state |= (PDONE_INT << pdma_chn);

        //free 链表
    }
    if (*int_state & (1 << (8 + pdma_chn)))
    {
        *int_state |= (PITEM_INT << pdma_chn);
    }
    if (*int_state & (1 << (16 + pdma_chn)))
    {
        *int_state |= (PPAUSE_INT << pdma_chn);
    }
    if (*int_state & (1 << (24 + pdma_chn)))
    {
        *int_state |= (PTOUT_INT << pdma_chn);
    }
}

static irqreturn_t k230_peridma_interrupt(int irq, void *dev_id)
{

    struct k230_peridma_dev *priv = dev_id;
    struct k230_peridma_vchan *vchan;
    struct k230_peridma_desc *desc = NULL;
    struct k230_peridma_hwdesc *hwdesc = NULL;
    int i;
    u32 ch_id;

    u32 int_stat = 0;
    int_stat = ioread32(priv->base + PDMA_INT_STAT);
    for (i = 0;i < CH_NUM;i ++)
    {
        if ((int_stat >> i) & 0x1)
        {
                _pdma_chn_state_clear(i, &int_stat);
                iowrite32(int_stat, priv->base + PDMA_INT_STAT);
                g_pdma_llt_list[i].use = false;
        }
        else
        {
                continue;
        }
        ch_id = i;
        vchan = priv->pchan[ch_id].vchan;
        if (vchan == NULL)
        {
                return IRQ_HANDLED;
        }
        spin_lock(&vchan->vchan.lock);
        //dev_vdbg(vchan2dev(vchan), "k230_peridma_interrupt: process Transfer done irq of phy ch%d, int_status(%x) \n", ch_id, int_stat0);
        desc = vchan->desc;
        //printk("========desc->cyclic:0x%llx\n",desc->cyclic);
        if(desc->cyclic)
        {
            hwdesc = desc->cyclic;
            //dev_vdbg(vchan2dev(vchan),"hwdesc(%px): last=%d ctl=%x, cfg0=%x, \n cfg1=%x, dev_tout=%x, dev_sel=%x, src_addr=%x, dst_addr=%x \n", hwdesc, hwdesc->last, hwdesc->ctl, hwdesc->cfg0, hwdesc->cfg1, hwdesc->dev_tout, hwdesc->dev_sel, hwdesc->src_addr, hwdesc->dst_addr);
            desc->remain -= hwdesc->usr_pdma_chn_cfg.line_size;
            if(desc->remain == 0)
                desc->remain = desc->buf_len;
            if(hwdesc->last)
                vchan_cyclic_callback(&vchan->desc->vd);

            if(desc->cyclic->list.next == &(desc->xfer_list))
                desc->cyclic = list_entry(desc->xfer_list.next, typeof(*(desc->cyclic)), list);
            else
                desc->cyclic = list_next_entry(desc->hwdesc, list);

            hwdesc = desc->cyclic;
            desc->hwdesc = hwdesc;
            configure_pchan(vchan->pchan, hwdesc);
            peridma_pchan_start(vchan->pchan);
        }
        else
        {
            list_del(&desc->hwdesc->list);
            list_add_tail(&desc->hwdesc->list, &desc->completed_list);

            if(!list_empty(&desc->xfer_list)){
                hwdesc = list_first_entry(&desc->xfer_list, struct k230_peridma_hwdesc, list);
                //		        list_del(&hwdesc->list);
                //		        atomic_sub(1, &desc->hwdesc_remain);

                //dev_vdbg(vchan2dev(vchan), "hwdesc(%px): ctl=%x, cfg0=%x, cfg1=%x, dev_tout=%x, dev_sel=%x, src_addr=%x, dst_addr=%x \n", hwdesc, hwdesc->ctl, hwdesc->cfg0, hwdesc->cfg1, hwdesc->dev_tout, hwdesc->dev_sel, hwdesc->src_addr, hwdesc->dst_addr);
                desc->hwdesc = hwdesc;
                //			desc->next = true;
                configure_pchan(vchan->pchan, hwdesc);
                peridma_pchan_start(vchan->pchan);
            } else	{
                vchan->desc = NULL;
                peridma_vchan_xfer_complete(vchan);
            }
        }
        spin_unlock(&vchan->vchan.lock);
    }

    return IRQ_HANDLED;
}

static int k230_peridma_vchan_pause(struct dma_chan *dchan)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);
        unsigned long flags;
        unsigned int timeout = 20; /* timeout iterations */
        u32 val;

	if(!vchan->pchan){
		dev_err(vchan2dev(vchan), "k230_peridma_vchan_pause: vchan->pchan is NULL \n");
		dump_stack();
		return -EBUSY;
	}

        spin_lock_irqsave(&vchan->vchan.lock, flags);

        if (!peridma_pchan_is_busy(vchan->pchan)) {
                dev_err(vchan2dev(vchan), "%s is non-busy!\n",
                        peridma_vchan_name(vchan));

		spin_unlock_irqrestore(&vchan->vchan.lock, flags);

                return -EBUSY;
        }

	peridma_pchan_pause(vchan->pchan);

        do  {
                if (peridma_pchan_is_paused(vchan->pchan))
                        break;

                udelay(2);
        } while (--timeout);

        peridma_pchan_irq_disable(vchan->pchan);

        vchan->is_paused = true;

        spin_unlock_irqrestore(&vchan->vchan.lock, flags);

        return timeout ? 0 : -EAGAIN;
}

static int k230_peridma_vchan_resume(struct dma_chan *dchan)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);
        unsigned long flags;

        spin_lock_irqsave(&vchan->vchan.lock, flags);

        if (vchan->is_paused){
		peridma_pchan_irq_enable(vchan->pchan);
                peridma_pchan_resume(vchan->pchan);
	}

	vchan->is_paused = false;

        spin_unlock_irqrestore(&vchan->vchan.lock, flags);

        return 0;
}


static void k230_peridma_vchan_free_chan_resources(struct dma_chan *dchan)
{
        struct k230_peridma_vchan *vchan = dchan_to_k230_peridma_vchan(dchan);

        if(vchan->pchan) {
            /* ASSERT: channel is idle */
            if (!peridma_pchan_is_idle(vchan->pchan)) {
                    dev_err(vchan2dev(vchan), "%s is non-idle!\n",
                        peridma_vchan_name(vchan));
            }

        peridma_pchan_stop(vchan->pchan);
        peridma_pchan_irq_disable(vchan->pchan);
        }


        vchan_free_chan_resources(&vchan->vchan);

        dev_vdbg(vchan2dev(vchan),
                 "%s: free resources, descriptor still allocated: %u\n",
                 peridma_vchan_name(vchan), atomic_read(&vchan->descs_allocated));

}

static int k230_peridma_config(struct dma_chan *chan,
                            struct dma_slave_config *config)
{
        //printk("$$$$$$$$$$$k230_peridma_config,src_addr:0x%llx,dst_addr:0x%llx,src_maxburst:%d,dst_maxburst:%d,src_addr_width:%d,dst_addr_width:%d,direction:%d\n" \
        ,config->src_addr,config->dst_addr,config->src_maxburst,config->dst_maxburst,config->src_addr_width,config->dst_addr_width,config->direction);
        struct k230_peridma_vchan *vchan = to_k230_peridma_vchan(chan);

        memcpy(&vchan->cfg, config, sizeof(*config));

        return 0;
}


static int parse_device_properties(struct k230_peridma_dev *pdev)
{
        struct device *dev = pdev->dev;
        u32 tmp;
        int ret;

        ret = device_property_read_u32(dev, "dma-channels", &tmp);
        if (ret)
                return ret;

        if (tmp == 0 || tmp > CH_NUM)
                return -EINVAL;

        pdev->nr_channels = tmp;

        ret = device_property_read_u32(dev, "dma-requests", &tmp);
        if (ret)
                return ret;

        if (tmp == 0 || tmp > 35)
                return -EINVAL;

        pdev->nr_requests = tmp;

        return 0;
}

static struct dma_chan *k230_pdma_of_xlate(struct of_phandle_args *dma_spec,
                                                struct of_dma *ofdma)
{
        //printk("==========k230_pdma_of_xlate before\n");
        struct k230_peridma_dev *priv = ofdma->of_dma_data;
        struct k230_peridma_vchan *vchan;
        struct dma_chan *chan;
        unsigned int priority = dma_spec->args[0];
        unsigned int dev_tout = dma_spec->args[1];
        unsigned int dat_endian = dma_spec->args[2];
        unsigned int dev_sel = dma_spec->args[3];

	dev_vdbg(priv->dev, "k230_pdma_of_xlate: priority=%x, dev_tout=%x, dat_endian=%x, dev_sel=%x \n", priority, dev_tout, dat_endian, dev_sel);

        if (dev_sel > (priv->nr_requests -1))
                return NULL;

        chan = dma_get_slave_channel(&(priv->vchan[dev_sel].vchan.chan));

        if(!chan)
                return NULL;

        vchan = to_k230_peridma_vchan(chan);

        vchan->priority = priority;
        vchan->dev_tout = dev_tout;
        vchan->dat_endian = dat_endian;
        vchan->dev_sel = dev_sel;

	dev_vdbg(priv->dev, "k230_pdma_of_xlate: get vchan %s \n", peridma_vchan_name(vchan));
        return chan;
}

static int k230_peridma_probe(struct platform_device *pdev)
{
	struct k230_peridma_dev *priv;
	struct resource *res;
	int i, j, ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

        priv->dev = &pdev->dev;
        priv->slave.dev = &pdev->dev;

	priv->irq = platform_get_irq(pdev, 0);
	if (priv->irq < 0) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		return priv->irq;
	}


	priv->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(priv->clk)) {
		dev_err(&pdev->dev, "No clock specified\n");
		return PTR_ERR(priv->clk);
	}

        ret = parse_device_properties(priv);
        if (ret)
                return ret;

        //printk("=====k230_peridma_probe irq:%d,dma_channels:%d,requests:%d\n",priv->irq,priv->nr_channels,priv->nr_requests);

	/*  Initialize physical channels  */
        priv->pchan = devm_kcalloc(priv->dev, priv->nr_channels,
                                sizeof(struct k230_peridma_pchan), GFP_KERNEL);
        if (!priv->pchan)
                return -ENOMEM;

        for (i = 0; i < priv->nr_channels; i++) {
                struct k230_peridma_pchan *pchan = &priv->pchan[i];

                pchan->pdev = priv;
                pchan->id = i;
                pchan->ch_regs = priv->base + CH0_BASE + i * CH_OFF;
        }

        /*  Initialize virtual channels  */
        priv->vchan = devm_kcalloc(priv->dev, priv->nr_requests,
                                sizeof(struct k230_peridma_vchan), GFP_KERNEL);
        if (!priv->vchan)
                return -ENOMEM;

        INIT_LIST_HEAD(&priv->slave.channels);
        for (i = 0; i < priv->nr_requests; i++) {
                struct k230_peridma_vchan *vchan = &priv->vchan[i];

                vchan->pdev = priv;
		vchan->desc = NULL;
                atomic_set(&vchan->descs_allocated, 0);

                vchan->vchan.desc_free = vchan_desc_put;
                vchan_init(&vchan->vchan, &priv->slave);
        }

	platform_set_drvdata(pdev, priv);
	spin_lock_init(&priv->lock);

	dma_cap_zero(priv->slave.cap_mask);
	dma_cap_set(DMA_SLAVE, priv->slave.cap_mask);

//	priv->slave.chancnt			= priv->nr_channels;
//        priv->slave.src_addr_widths = DMA_SLAVE_BUSWIDTH_8_BYTES;
//        priv->slave.dst_addr_widths = DMA_SLAVE_BUSWIDTH_8_BYTES;
//        priv->slave.directions = BIT(DMA_MEM_TO_MEM);
//        priv->slave.residue_granularity = DMA_RESIDUE_GRANULARITY_DESCRIPTOR;

        priv->slave.directions                  = BIT(DMA_DEV_TO_MEM) |
                                                  BIT(DMA_MEM_TO_DEV);
	priv->slave.residue_granularity         = DMA_RESIDUE_GRANULARITY_BURST;
        priv->slave.src_addr_widths             = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
                                                  BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
                                                  BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);
        priv->slave.dst_addr_widths             = BIT(DMA_SLAVE_BUSWIDTH_1_BYTE) |
                                                  BIT(DMA_SLAVE_BUSWIDTH_2_BYTES) |
                                                  BIT(DMA_SLAVE_BUSWIDTH_4_BYTES);

	priv->slave.device_tx_status		= k230_peridma_tx_status;
	priv->slave.device_issue_pending	= k230_peridma_issue_pending;
	priv->slave.device_terminate_all	= k230_peridma_terminate_all;
        priv->slave.device_pause 		= k230_peridma_vchan_pause;
        priv->slave.device_resume 		= k230_peridma_vchan_resume;

//        priv->slave.device_alloc_chan_resources	= k230_peridma_vchan_alloc_chan_resources;
        priv->slave.device_free_chan_resources	= k230_peridma_vchan_free_chan_resources;

	priv->slave.device_prep_slave_sg	= k230_peridma_prep_slave_sg;
    priv->slave.device_prep_dma_cyclic	= k230_peridma_prep_dma_cyclic;
	priv->slave.device_config		= k230_peridma_config;

	priv->slave.dev = &pdev->dev;

	ret = clk_prepare_enable(priv->clk);
	if (ret) {
		dev_err(&pdev->dev, "Couldn't enable the clock\n");
		return ret;
	}

	/*
	 * Initialize peridma controller
	 */

	k230_peridma_hw_init(priv);

	ret = devm_request_irq(&pdev->dev, priv->irq, k230_peridma_interrupt,
			       0, dev_name(&pdev->dev), priv);
	if (ret) {
		dev_err(&pdev->dev, "Cannot request IRQ\n");
		goto err_clk_disable;
	}

	ret = dma_async_device_register(&priv->slave);
	if (ret) {
		dev_warn(&pdev->dev, "Failed to register DMA engine device\n");
		goto err_clk_disable;
	}

        ret = of_dma_controller_register(pdev->dev.of_node, k230_pdma_of_xlate,
                                         priv);
        if (ret) {
                dev_err(&pdev->dev, "of_dma_controller_register failed\n");
                goto err_clk_disable;
        }

        for (i =0;i < CH_NUM;i ++)
        {
                g_pdma_llt_list[i].llt_list_v = dma_alloc_coherent(&pdev->dev,sizeof(pdma_llt_t),&g_pdma_llt_list[i].llt_list_p,GFP_KERNEL);
                g_pdma_llt_list[i].use = false;
                //printk("===========g_pdma_llt_list[%d].llt_list_p:0x%llx\n",i,g_pdma_llt_list[i].llt_list_p);
        }

	dev_vdbg(&pdev->dev, "Successfully probed K230 peridma controller, register mapped at %px(PHY %px) \n", priv->base ,res->start);

	return 0;

err_clk_disable:
	clk_disable_unprepare(priv->clk);
	return ret;
}

static int k230_peridma_remove(struct platform_device *pdev)
{
        int i =0;
	struct k230_peridma_dev *priv = platform_get_drvdata(pdev);

        for (i =0;i < CH_NUM;i ++)
        {
                dma_free_coherent(&pdev->dev,sizeof(pdma_llt_t),g_pdma_llt_list[i].llt_list_v,g_pdma_llt_list[i].llt_list_p);
                g_pdma_llt_list[i].llt_list_v = NULL;
        }

	/* Disable IRQ so no more work is scheduled */
	disable_irq(priv->irq);

	of_dma_controller_free(pdev->dev.of_node);
	dma_async_device_unregister(&priv->slave);

	clk_disable_unprepare(priv->clk);

	return 0;
}

static const struct of_device_id k230_peridma_match[] = {
	{ .compatible = "canaan,k230-pdma" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, k230_peridma_match);

static struct platform_driver k230_peridma_driver = {
	.probe	= k230_peridma_probe,
	.remove	= k230_peridma_remove,
	.driver	= {
		.name		= "k230-peridma",
		.of_match_table	= k230_peridma_match,
	},
};

module_platform_driver(k230_peridma_driver);

MODULE_DESCRIPTION("K230 peridma driver");
MODULE_LICENSE("GPL");
