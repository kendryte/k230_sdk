/*
 * Copyright (c) 2011-2022, Shanghai Real-Thread Electronic Technology Co.,Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-01     SummerGift   add i2c driver
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <riscv_io.h>
#include <stdlib.h>
#include "mmu.h"
#include "ioremap.h"
#include "drv_i2c.h"

#define DRV_DEBUG
#include <rtdbg.h>

struct dw_scl_sda_cfg
{
    rt_uint32_t ss_hcnt;
    rt_uint32_t fs_hcnt;
    rt_uint32_t ss_lcnt;
    rt_uint32_t fs_lcnt;
    rt_uint32_t sda_hold;
};

struct dw_i2c
{
    struct i2c_regs *regs;
    struct dw_scl_sda_cfg scl_sda_cfg;

    /* enum chip_functions pin_function */
    rt_uint8_t pin_group_scl_function;
    rt_uint8_t pin_group_sda_function;
    rt_uint8_t pin_group_scl;
    rt_uint8_t pin_group_sda;
};

struct chip_i2c_bus
{
    struct rt_i2c_bus_device parent;
    struct dw_i2c i2c;

    struct rt_i2c_msg *msg;
    rt_uint32_t msg_cnt;
    volatile rt_uint32_t msg_ptr;
    volatile rt_uint32_t dptr;
    char *device_name;
};

static struct chip_i2c_bus i2c_buses[] =
{
#ifdef RT_USING_I2C0
    {
        .device_name = "i2c0",
        .i2c = {
            .regs = (struct i2c_regs *)0x91405000U,
        },
        .i2c.scl_sda_cfg = {0}
    },
#endif
#ifdef RT_USING_I2C1
    {
        .device_name = "i2c1",
        .i2c = {
            .regs = (struct i2c_regs *)0x91406000U,
        },
        .i2c.scl_sda_cfg = {0}
    },
#endif
#ifdef RT_USING_I2C2
    {
        .device_name = "i2c2",
        .i2c = {
            .regs = (struct i2c_regs *)0x91407000U,
        },
        .i2c.scl_sda_cfg = {0}
    },
#endif
#ifdef RT_USING_I2C3
    {
        .device_name = "i2c3",
        .i2c = {
            .regs = (struct i2c_regs *)0x91408000U,
        },
        .i2c.scl_sda_cfg = {0}
    },
#endif
#ifdef RT_USING_I2C4
    {
        .device_name = "i2c4",
        .i2c = {
            .regs = (struct i2c_regs *)0x91409000U,
        },
        .i2c.scl_sda_cfg = {0}
    },
#endif
};

static rt_size_t get_timer(rt_size_t base)
{
    return rt_tick_get() - base ;
}

static void dw_i2c_enable(struct i2c_regs *i2c_base, rt_bool_t enable)
{
    rt_uint32_t ena = enable ? IC_ENABLE_0B : 0;
    int timeout = 100;

    do
    {
        writel(ena, &i2c_base->ic_enable);
        if ((readl(&i2c_base->ic_enable_status) & IC_ENABLE_0B) == ena)
            return;

        /*
         * Wait 10 times the signaling period of the highest I2C
         * transfer supported by the driver (for 400KHz this is
         * 25us) as described in the DesignWare I2C databook.
         */
        // udelay(25);
    } while (timeout--);

    LOG_D("timeout in %sabling I2C adapter\n", enable ? "en" : "dis");
}

/*
 * i2c_set_bus_speed - Set the i2c speed
 * @speed:  required i2c speed
 *
 * Set the i2c speed.
 */
static unsigned int __dw_i2c_set_bus_speed(struct i2c_regs *i2c_base,
        struct dw_scl_sda_cfg *scl_sda_cfg,
        unsigned int speed)
{
    unsigned int cntl;
    unsigned int hcnt, lcnt;
    int i2c_spd;

    if (speed >= I2C_MAX_SPEED)
        i2c_spd = IC_SPEED_MODE_MAX;
    else if (speed >= I2C_FAST_SPEED)
        i2c_spd = IC_SPEED_MODE_FAST;
    else
        i2c_spd = IC_SPEED_MODE_STANDARD;

    /* to set speed cltr must be disabled */
    dw_i2c_enable(i2c_base, 0);

    cntl = (readl(&i2c_base->ic_con) & (~IC_CON_SPD_MSK));

    switch (i2c_spd)
    {
    case IC_SPEED_MODE_MAX:
        cntl |= IC_CON_SPD_SS;
        if (scl_sda_cfg)
        {
            hcnt = scl_sda_cfg->fs_hcnt;
            lcnt = scl_sda_cfg->fs_lcnt;
        }
        else
        {
            hcnt = (IC_CLK * MIN_HS_SCL_HIGHTIME) / NANO_TO_MICRO;
            lcnt = (IC_CLK * MIN_HS_SCL_LOWTIME) / NANO_TO_MICRO;
        }
        writel(hcnt, &i2c_base->ic_hs_scl_hcnt);
        writel(lcnt, &i2c_base->ic_hs_scl_lcnt);
        break;

    case IC_SPEED_MODE_STANDARD:
        cntl |= IC_CON_SPD_SS;
        if (scl_sda_cfg)
        {
            hcnt = scl_sda_cfg->ss_hcnt;
            lcnt = scl_sda_cfg->ss_lcnt;
        }
        else
        {
            hcnt = (IC_CLK * MIN_SS_SCL_HIGHTIME) / NANO_TO_MICRO;
            lcnt = (IC_CLK * MIN_SS_SCL_LOWTIME) / NANO_TO_MICRO;
        }
        writel(hcnt, &i2c_base->ic_ss_scl_hcnt);
        writel(lcnt, &i2c_base->ic_ss_scl_lcnt);
        break;

    case IC_SPEED_MODE_FAST:
    default:
        cntl |= IC_CON_SPD_FS;
        if (scl_sda_cfg)
        {
            hcnt = scl_sda_cfg->fs_hcnt;
            lcnt = scl_sda_cfg->fs_lcnt;
        }
        else
        {
            hcnt = (IC_CLK * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO;
            lcnt = (IC_CLK * MIN_FS_SCL_LOWTIME) / NANO_TO_MICRO;
        }

        writel(hcnt, &i2c_base->ic_fs_scl_hcnt);
        writel(lcnt, &i2c_base->ic_fs_scl_lcnt);
        break;
    }

    writel(cntl, &i2c_base->ic_con);

    /* Configure SDA Hold Time if required */
    if (scl_sda_cfg)
        writel(scl_sda_cfg->sda_hold, &i2c_base->ic_sda_hold);

    /* Enable back i2c now speed set */
    dw_i2c_enable(i2c_base, 1);

    return 0;
}

/*
 * i2c_setaddress - Sets the target slave address
 * @i2c_addr:   target i2c address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(struct i2c_regs *i2c_base, unsigned int i2c_addr)
{
    /* Disable i2c */
    dw_i2c_enable(i2c_base, 0);

    writel(i2c_addr, &i2c_base->ic_tar);

    /* Enable i2c */
    dw_i2c_enable(i2c_base, 1);
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo(struct i2c_regs *i2c_base)
{
    while (readl(&i2c_base->ic_status) & IC_STATUS_RFNE)
        readl(&i2c_base->ic_cmd_data);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
static int i2c_wait_for_bb(struct i2c_regs *i2c_base)
{
    unsigned long start_time_bb = get_timer(0);

    while ((readl(&i2c_base->ic_status) & IC_STATUS_MA) ||
           !(readl(&i2c_base->ic_status) & IC_STATUS_TFE))
    {

        /* Evaluate timeout */
        if (get_timer(start_time_bb) > (unsigned long)(I2C_BYTE_TO_BB))
            return 1;
    }

    return 0;
}

static int i2c_xfer_init(struct i2c_regs *i2c_base, uchar chip, uint addr,
                         int alen)
{
    if (i2c_wait_for_bb(i2c_base))
        return 1;

    i2c_setaddress(i2c_base, chip);
    while (alen)
    {
        alen--;
        /* high byte address going out first */
        writel((addr >> (alen * 8)) & 0xff,
               &i2c_base->ic_cmd_data);
    }
    return 0;
}

static int i2c_xfer_finish(struct i2c_regs *i2c_base)
{
    ulong start_stop_det = get_timer(0);

    while (1)
    {

        if ((readl(&i2c_base->ic_raw_intr_stat) & IC_STOP_DET))
        {
            readl(&i2c_base->ic_clr_stop_det);
            break;
        }
        else if (get_timer(start_stop_det) > I2C_STOPDET_TO)
        {
            LOG_D("get_timer(start_stop_det) > I2C_STOPDET_TO");
            break;
        }
    }

    if (i2c_wait_for_bb(i2c_base))
    {
        LOG_D("Timed out waiting for bus\n");
        return 1;
    }

    i2c_flush_rxfifo(i2c_base);

    return 0;
}

/*
 * i2c_read - Read from i2c memory
 * @chip:   target i2c address
 * @addr:   address to read from
 * @alen:
 * @buffer: buffer for read data
 * @len:    no of bytes to be read
 *
 * Read from i2c memory.
 */
static int __dw_i2c_read(struct i2c_regs *i2c_base, rt_uint8_t dev, uint addr,
                         int alen, rt_uint8_t *buffer, int len, uint flags)
{
    unsigned long start_time_rx;
    unsigned int active = 0;
    int need_restart = 0;

    if (flags & I2C_M_RESTART)
        need_restart = BIT(10);
    else if (i2c_xfer_init(i2c_base, dev, addr, alen) != 0)
    {
        return -RT_EBUSY;
    }

    start_time_rx = get_timer(0);

    while (len)
    {
        if (!active)
        {
            /*
             * Avoid writing to ic_cmd_data multiple times
             * in case this loop spins too quickly and the
             * ic_status RFNE bit isn't set after the first
             * write. Subsequent writes to ic_cmd_data can
             * trigger spurious i2c transfer.
             */
            if (len == 1)
            {
                LOG_D("set IC_STOP");
                writel(IC_CMD | IC_STOP | need_restart, &i2c_base->ic_cmd_data);
            }
            else
            {
                writel(IC_CMD | need_restart, &i2c_base->ic_cmd_data);
            }
            active = 1;
            need_restart = 0;
        }

        if (readl(&i2c_base->ic_status) & IC_STATUS_RFNE)
        {
            *buffer++ = (uchar)readl(&i2c_base->ic_cmd_data);
            len--;
            start_time_rx = get_timer(0);
            active = 0;
        }
        else if (get_timer(start_time_rx) > I2C_BYTE_TO)
        {
            LOG_D("__dw_i2c_read timeout \r\n");
            return 1;
        }
    }

    return i2c_xfer_finish(i2c_base);
}

/*
 * i2c_write - Write to i2c memory
 * @chip:   target i2c address
 * @addr:   address to read from
 * @alen:
 * @buffer: buffer for read data
 * @len:    no of bytes to be read
 *
 * Write to i2c memory.
 */
static int __dw_i2c_write(struct i2c_regs *i2c_base, rt_uint8_t dev, uint addr,
                          int alen, rt_uint8_t *buffer, int len, uint flags)
{
    int nb = len;
    unsigned long start_time_tx;

    if (i2c_xfer_init(i2c_base, dev, addr, alen))
    {
        LOG_D("i2c set addr failed");
        return 1;
    }

    start_time_tx = get_timer(0);
    while (len)
    {
        if (readl(&i2c_base->ic_status) & IC_STATUS_TFNF)
        {
            if ((--len == 0) && (flags & I2C_M_STOP))
            {
                writel(*buffer | IC_STOP,
                       &i2c_base->ic_cmd_data);
            }
            else
            {
                writel(*buffer, &i2c_base->ic_cmd_data);
            }
            buffer++;
            start_time_tx = get_timer(0);

        }
        else if (get_timer(start_time_tx) > (nb * I2C_BYTE_TO))
        {
            LOG_D("Timed out. i2c write Failed\n");
            return 1;
        }
    }

    return 0;
}

/*
 * dw_i2c_init - Init function
 *
 * Initialization function.
 */
static void dw_i2c_init(struct i2c_regs *i2c_base)
{
    /* Disable i2c */
    dw_i2c_enable(i2c_base, 0);

    writel(IC_CON_SD | IC_CON_RE | IC_CON_SPD_FS | IC_CON_MM,
           &i2c_base->ic_con);
    writel(IC_RX_TL, &i2c_base->ic_rx_tl);
    writel(IC_TX_TL, &i2c_base->ic_tx_tl);
    writel(IC_STOP_DET, &i2c_base->ic_intr_mask);

    /* Enable i2c */
    dw_i2c_enable(i2c_base, 1);
}

static int designware_i2c_xfer(struct chip_i2c_bus *bus, struct rt_i2c_msg *msg,
                               int nmsgs)
{
    int ret;
    int send_mesgs = 0;

    send_mesgs = nmsgs;

    /* get device from bus */
    struct dw_i2c *i2c = &(bus->i2c);

    msg->flags |= I2C_M_START;
    for (; nmsgs > 0; nmsgs--, msg++)
    {
        if (nmsgs > 1)
            msg->flags &= ~I2C_M_STOP;
        else
            msg->flags |= I2C_M_STOP;
        if (!(msg->flags & I2C_M_START))
            msg->flags |= I2C_M_RESTART;
        if (msg->flags & I2C_M_RD)
        {
            ret = __dw_i2c_read(i2c->regs, msg->addr, 0, 0,
                                msg->buf, msg->len, msg->flags);
        }
        else
        {
            ret = __dw_i2c_write(i2c->regs, msg->addr, 0, 0,
                                 msg->buf, msg->len, msg->flags);
        }
        if (ret)
        {
            LOG_D("i2c_xfer: error sending");
            return -RT_EIO;
        }
    }

    return send_mesgs;
}

static int designware_i2c_set_bus_speed(struct chip_i2c_bus *bus, unsigned int speed)
{
    struct dw_i2c *i2c = &bus->i2c;

    return __dw_i2c_set_bus_speed(i2c->regs, RT_NULL, speed);
}

static rt_size_t chip_i2c_mst_xfer(struct rt_i2c_bus_device *bus,
                                 struct rt_i2c_msg msgs[],
                                 rt_uint32_t num)
{
    return designware_i2c_xfer((struct chip_i2c_bus *)bus, msgs, num);
}

static int dw_i2c_control(struct chip_i2c_bus *bus,                        
                        rt_uint32_t               cmd,
                        rt_uint32_t               arg)
{
    rt_uint32_t bus_clock;
    rt_err_t ret;
    
    RT_ASSERT(bus != RT_NULL);

    switch (cmd)
    {
    /* set 10-bit addr mode */
    case RT_I2C_DEV_CTRL_10BIT:
        // bus->flags |= RT_I2C_ADDR_10BIT;
        break;
    case RT_I2C_DEV_CTRL_TIMEOUT:
        // bus->timeout = *(rt_uint32_t *)arg;
        break;
    case RT_I2C_DEV_CTRL_CLK:
        bus_clock = arg;
        ret = designware_i2c_set_bus_speed(bus, bus_clock);
        if (ret < 0)
        {
            return -RT_EIO;
        }
        break;
    default:
        break;
    }   
    
    return RT_EOK;
}                               

static rt_err_t chip_i2c_control(struct rt_i2c_bus_device *bus,
                        rt_uint32_t               cmd,
                        rt_uint32_t               arg)
{
    return dw_i2c_control((struct chip_i2c_bus *)bus, cmd, arg);
}                    

static const struct rt_i2c_bus_device_ops chip_i2c_ops =
{
    .master_xfer = chip_i2c_mst_xfer,
    .i2c_bus_control = chip_i2c_control,
};

rt_uint32_t i2c_dw_readl(struct i2c_regs *i2c_base, rt_uint32_t offset)
{
    return readl((char *)i2c_base + offset);
}

void i2c_dw_writel(const struct i2c_regs *i2c_base, rt_uint32_t val, rt_uint32_t offset)
{
    writel(val, (char *)i2c_base + offset);
}

void i2c_dw_disable_int(struct i2c_regs *i2c_base)
{
    i2c_dw_writel(i2c_base, DW_IC_INTR_MASK, 0);
}

int rt_hw_i2c_init(void)
{
    int i;

    for (i = 0; i < sizeof(i2c_buses) / sizeof(i2c_buses[0]); i++)
    {
        i2c_buses[i].i2c.regs = (struct i2c_regs *)rt_ioremap((void *)i2c_buses[i].i2c.regs, 0x10000);
        i2c_buses[i].parent.ops = &chip_i2c_ops;

        dw_i2c_init((struct i2c_regs *)i2c_buses[i].i2c.regs);
        designware_i2c_set_bus_speed(&i2c_buses[i], I2C_FAST_SPEED);

        /* pinctrl configuration */
        /* wait for next version */
        /* register i2c device */
        rt_i2c_bus_device_register(&i2c_buses[i].parent, i2c_buses[i].device_name);
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);

#ifdef RT_USING_I2C_DUMP
void i2c_reg_show(int argc, char *argv[])
{
    if(argc < 2) {
        rt_kprintf("please input i2c num.\n");
        return;
    }
    struct i2c_regs *i2c_base = i2c_buses[atoi(argv[1])].i2c.regs;

    rt_kprintf("base: \t\t0x%08x \r\n", i2c_base);
    rt_kprintf("CON: \t\t0x%08x \r\n", i2c_dw_readl(i2c_base, DW_IC_CON));
    rt_kprintf("TAR: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_TAR));
    rt_kprintf("SAR: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_SAR));
    rt_kprintf("DATA_CMD: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_DATA_CMD));
    rt_kprintf("DW_IC_SS_SCL_HCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_SS_SCL_HCNT));
    rt_kprintf("DW_IC_SS_SCL_LCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_SS_SCL_LCNT));
    rt_kprintf("DW_IC_FS_SCL_HCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_FS_SCL_HCNT));
    rt_kprintf("DW_IC_FS_SCL_LCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_FS_SCL_LCNT));
    rt_kprintf("DW_IC_HS_SCL_HCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_HS_SCL_HCNT));
    rt_kprintf("DW_IC_HS_SCL_LCNT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_HS_SCL_LCNT));
    rt_kprintf("INTR_STAT: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_INTR_STAT));
    rt_kprintf("INTR_MASK: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_INTR_MASK));
    rt_kprintf("DW_IC_ENABLE: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_ENABLE));
    rt_kprintf("RX_TL: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_RX_TL));
    rt_kprintf("TX_TL: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_TX_TL));
    rt_kprintf("STATUS: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_STATUS));
    rt_kprintf("TXFLR: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_TXFLR));
    rt_kprintf("RXFLR: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_RXFLR));
    rt_kprintf("DMA_CR: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_DMA_CR));
    rt_kprintf("DMA_TDLR: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_DMA_TDLR));
    rt_kprintf("DMA_RDLR: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_DMA_RDLR));
    rt_kprintf("DEVICE_ID: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_DEVICE_ID));
    rt_kprintf("PARAM: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_COMP_PARAM_1));
    rt_kprintf("VERSION: \t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_COMP_VERSION));
    rt_kprintf("TYPE: \t\t0x%08x\n", i2c_dw_readl(i2c_base, DW_IC_COMP_TYPE));
}
MSH_CMD_EXPORT(i2c_reg_show, run i2c_reg_show);
#endif