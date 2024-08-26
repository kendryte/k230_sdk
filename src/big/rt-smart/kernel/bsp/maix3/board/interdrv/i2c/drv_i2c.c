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
#include "dfs_poll.h"
#include "mmu.h"
#include "ioremap.h"
#include "drv_i2c.h"
#include <dfs_file.h>
#include <dfs_posix.h>

#include "tick.h"

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

enum i2c_slave_event {
	I2C_SLAVE_READ_REQUESTED,
	I2C_SLAVE_WRITE_REQUESTED,
	I2C_SLAVE_READ_PROCESSED,
	I2C_SLAVE_WRITE_RECEIVED,
	I2C_SLAVE_STOP,
};

struct chip_i2c_bus;

typedef void(*i2c_slave_cb)(void* ctx, enum i2c_slave_event event, rt_uint8_t* val);

struct chip_i2c_bus
{
    struct rt_i2c_bus_device parent;
    struct dw_i2c i2c;

    rt_bool_t slave;
    rt_uint8_t slave_address;
    i2c_slave_cb slave_callback;
    void* slave_callback_ctx;
    rt_uint32_t slave_status;

    struct rt_i2c_msg *msg;
    rt_uint32_t msg_cnt;
    volatile rt_uint32_t msg_ptr;
    volatile rt_uint32_t dptr;
    char *device_name;

    int irq;
};

static uint64_t i2c_perf_get_times(void)
{
    uint64_t cnt;
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(cnt));
    return cnt;
}

static void i2c_delay_us(uint64_t us)
{
    uint64_t delay = (TIMER_CLK_FREQ / 1000000) * us;
    volatile uint64_t cur_time = i2c_perf_get_times();
    while(1)
    {
        if((i2c_perf_get_times() - cur_time ) >= delay)
            break;
    }
}

#ifdef RT_USING_I2C_SLAVE_EEPROM
// simulate block device
struct i2c_slave_eeprom {
    struct rt_device device;
    volatile int poll_event;
    rt_uint8_t* buffer;
    rt_uint8_t ptr;
    rt_bool_t flag_recv_ptr;
};

static rt_uint8_t eeprom_buffer[256];
static struct i2c_slave_eeprom eeprom = {
    .buffer = eeprom_buffer,
};

static int eeprom_open(struct dfs_fd *file) {
    file->fnode->size = sizeof(eeprom_buffer);
    return 0;
}

static int eeprom_close(struct dfs_fd *file) {
    return 0;
}

static int eeprom_read(struct dfs_fd *file, void *buffer, size_t size) {
    if (file->pos >= file->fnode->size) {
        return 0;
    }
    unsigned lack = file->fnode->size - file->pos;
    if (size > lack) {
        size = lack;
    }
    memcpy(buffer, eeprom_buffer + file->pos, size);
    file->pos += size;
    return size;
}

static int eeprom_write(struct dfs_fd *file, const void *buffer, size_t size) {
    if (file->pos >= file->fnode->size) {
        return 0;
    }
    unsigned lack = file->fnode->size - file->pos;
    if (size > lack) {
        size = lack;
    }
    memcpy(eeprom_buffer + file->pos, buffer, size);
    file->pos += size;
    return size;
}

static int eeprom_lseek(struct dfs_fd *file, off_t offset) {
    if (offset < file->fnode->size) {
        return offset;
    } else {
        return -1;
    }
}

static int eeprom_poll(struct dfs_fd *file, struct rt_pollreq *req) {
    rt_device_t device = file->fnode->data;
    rt_poll_add(&device->wait_queue, req);
    int mask = eeprom.poll_event;
    eeprom.poll_event = 0;
    return mask;
}

static const struct dfs_file_ops eeprom_fops = {
    .open = eeprom_open,
    .close = eeprom_close,
    .read = eeprom_read,
    .write = eeprom_write,
    .poll = eeprom_poll,
    .lseek = eeprom_lseek,
};

int i2c_slave_eeprom_init(void) {
    int ret = 0;
    // or you can use rt_malloc to allocate
    rt_device_t device = (rt_device_t)&eeprom;
    ret = rt_device_register(device, "slave-eeprom", RT_DEVICE_FLAG_RDWR);
    if (ret) {
        rt_kprintf("%s: rt_device_register error %d\n", __func__, ret);
        return -1;
    }
    rt_wqueue_init(&device->wait_queue);
    device->fops = &eeprom_fops;
    device->user_data = device;
    return 0;
}
INIT_BOARD_EXPORT(i2c_slave_eeprom_init);

static void i2c_slave_eeprom_callback(void* ctx, enum i2c_slave_event event, rt_uint8_t* val) {
    struct i2c_slave_eeprom* eeprom = ctx;

    switch (event) {
	case I2C_SLAVE_WRITE_RECEIVED:
        if (eeprom->flag_recv_ptr) {
            // write data
            eeprom->buffer[eeprom->ptr++] = *val;
            eeprom->poll_event |= POLLIN;
            rt_wqueue_wakeup(&eeprom->device.wait_queue, (void*)POLLIN);
        } else {
            // recv addr byte
            eeprom->flag_recv_ptr = RT_TRUE;
            eeprom->ptr = *val;
        }
		break;

	case I2C_SLAVE_READ_PROCESSED:
		/* The previous byte made it to the bus, get next one */
		eeprom->ptr++;
	case I2C_SLAVE_READ_REQUESTED:
		*val = eeprom->buffer[eeprom->ptr];
		/*
		 * Do not increment buffer_idx here, because we don't know if
		 * this byte will be actually used. Read Linux I2C slave docs
		 * for details.
		 */
		break;

	case I2C_SLAVE_STOP:
	case I2C_SLAVE_WRITE_REQUESTED:
		eeprom->ptr = 0;
        eeprom->flag_recv_ptr = RT_FALSE;
		break;

	default:
		break;
	}
}
#endif

void xxd(int argc, char* argv[]) {
    if (argc != 2) {
        rt_kprintf("Usage: xxd file\n");
        return;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        rt_kprintf("Can't open %s\n", argv[1]);
        return;
    }
    char buffer[16];
    int size = 0;
    do {
        size = read(fd, buffer, sizeof(buffer));
        for (unsigned i = 0; i < size; i++) {
            rt_kprintf("%02x ", buffer[i]);
        }
        rt_kprintf("\n");
    } while (size != 0);
    close(fd);
    return;
}
MSH_CMD_EXPORT(xxd, make a hexdump or do the reverse.)

static struct chip_i2c_bus i2c_buses[] =
{
#ifdef RT_USING_I2C0
    {
        .device_name = "i2c0",
        .i2c = {
            .regs = (struct i2c_regs *)0x91405000U,
        },
        .i2c.scl_sda_cfg = {0},
        #ifdef RT_USING_I2C0_SLAVE
        #ifndef RT_USING_I2C_SLAVE_EEPROM
        #error RT_USING_I2C_SLAVE_EEPROM is required
        #endif
        .slave = RT_TRUE,
        .slave_address = 0x20,
        .slave_callback = i2c_slave_eeprom_callback,
        .slave_callback_ctx = &eeprom,
        #else
        .slave = RT_FALSE,
        #endif
        .irq = 21,
    },
#endif
#ifdef RT_USING_I2C1
    {
        .device_name = "i2c1",
        .i2c = {
            .regs = (struct i2c_regs *)0x91406000U,
        },
        .i2c.scl_sda_cfg = {0},
        #ifdef RT_USING_I2C1_SLAVE
        #ifndef RT_USING_I2C_SLAVE_EEPROM
        #error RT_USING_I2C_SLAVE_EEPROM is required
        #endif
        .slave = RT_TRUE,
        .slave_address = 0x21,
        .slave_callback = i2c_slave_eeprom_callback,
        .slave_callback_ctx = &eeprom,
        #else
        .slave = RT_FALSE,
        #endif
        .irq = 22,
    },
#endif
#ifdef RT_USING_I2C2
    {
        .device_name = "i2c2",
        .i2c = {
            .regs = (struct i2c_regs *)0x91407000U,
        },
        .i2c.scl_sda_cfg = {0},
        #ifdef RT_USING_I2C2_SLAVE
        #ifndef RT_USING_I2C_SLAVE_EEPROM
        #error RT_USING_I2C_SLAVE_EEPROM is required
        #endif
        .slave = RT_TRUE,
        .slave_address = 0x22,
        .slave_callback = i2c_slave_eeprom_callback,
        .slave_callback_ctx = &eeprom,
        #else
        .slave = RT_FALSE,
        #endif
        .irq = 23,
    },
#endif
#ifdef RT_USING_I2C3
    {
        .device_name = "i2c3",
        .i2c = {
            .regs = (struct i2c_regs *)0x91408000U,
        },
        .i2c.scl_sda_cfg = {0},
        #ifdef RT_USING_I2C3_SLAVE
        #ifndef RT_USING_I2C_SLAVE_EEPROM
        #error RT_USING_I2C_SLAVE_EEPROM is required
        #endif
        .slave = RT_TRUE,
        .slave_address = 0x23,
        .slave_callback = i2c_slave_eeprom_callback,
        .slave_callback_ctx = &eeprom,
        #else
        .slave = RT_FALSE,
        #endif
        .irq = 24,
    },
#endif
#ifdef RT_USING_I2C4
    {
        .device_name = "i2c4",
        .i2c = {
            .regs = (struct i2c_regs *)0x91409000U,
        },
        .i2c.scl_sda_cfg = {0},
        #ifdef RT_USING_I2C4_SLAVE
        #ifndef RT_USING_I2C_SLAVE_EEPROM
        #error RT_USING_I2C_SLAVE_EEPROM is required
        #endif
        .slave = RT_TRUE,
        .slave_address = 0x24,
        .slave_callback = i2c_slave_eeprom_callback,
        .slave_callback_ctx = &eeprom,
        #else
        .slave = RT_FALSE,
        #endif
        .irq = 25,
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
        cntl |= IC_CON_SPD_HS;
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
    int cut = 0;

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

            while( (readl(&i2c_base->ic_status) & IC_STATUS_TFE) != IC_STATUS_TFE)
            {
                cut = cut + 1;
                i2c_delay_us(100);
                if(cut > 100)
                    break;
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
    int cut = 0;
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

    while( (readl(&i2c_base->ic_status) & IC_STATUS_TFE) != IC_STATUS_TFE)
    {
        cut = cut + 1;
        i2c_delay_us(100);
        if(cut > 100)
            return -1;
    }


    // if(readl(&i2c_base->ic_tx_abrt_source) != 0){
    //     if(readl(&i2c_base->ic_raw_intr_stat)&IC_TX_ABRT){
    //         LOG_E("i2c tx abort\n");
    //         return 1;
    //     }
    // }

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

static rt_uint32_t i2c_dw_read_clear_intrbits_slave(struct i2c_regs *i2c_base) {
    rt_uint32_t dummy;
    rt_uint32_t stat = readl(&i2c_base->ic_intr_stat);

    if (stat & DW_IC_INTR_TX_ABRT) {
        // error, read reason
        dummy = readl(&i2c_base->ic_tx_abrt_source);
        rt_kprintf("tx_abrt_source: %08x\n", dummy);
        dummy = readl(&i2c_base->ic_clr_tx_abrt);
    }
	if (stat & DW_IC_INTR_RX_UNDER)
        dummy = readl(&i2c_base->ic_clr_rx_under);
	if (stat & DW_IC_INTR_RX_OVER)
        dummy = readl(&i2c_base->ic_clr_rx_over);
	if (stat & DW_IC_INTR_TX_OVER)
        dummy = readl(&i2c_base->ic_clr_tx_over);
	if (stat & DW_IC_INTR_RX_DONE)
        dummy = readl(&i2c_base->ic_clr_rx_done);
	if (stat & DW_IC_INTR_ACTIVITY)
        dummy = readl(&i2c_base->ic_clr_activity);
	if (stat & DW_IC_INTR_STOP_DET)
        dummy = readl(&i2c_base->ic_clr_stop_det);
	if (stat & DW_IC_INTR_START_DET)
        dummy = readl(&i2c_base->ic_clr_start_det);
	if (stat & DW_IC_INTR_GEN_CALL)
        dummy = readl(&i2c_base->ic_clr_gen_call);

	return stat;
}

#define STATUS_ACTIVE				BIT(0)
#define STATUS_WRITE_IN_PROGRESS		BIT(1)
#define STATUS_READ_IN_PROGRESS			BIT(2)

static void dw_i2c_slave_isr(int irq, void *param) {
    struct chip_i2c_bus* bus = param;
    struct i2c_regs *i2c_base = bus->i2c.regs;
    rt_uint32_t raw_stat, stat, enabled, tmp;
	rt_uint8_t val = 0, slave_activity;

    enabled = readl(&i2c_base->ic_enable);
    raw_stat = readl(&i2c_base->ic_raw_intr_stat);
    tmp = readl(&i2c_base->ic_status);
	slave_activity = ((tmp & DW_IC_STATUS_SLAVE_ACTIVITY) >> 6);

	if (!enabled || !(raw_stat & ~DW_IC_INTR_ACTIVITY))
		return;

	stat = i2c_dw_read_clear_intrbits_slave(i2c_base);

    // rt_kprintf("i2c_slave_isr enabled(%u) slave_activity(%u) raw_stat(%08x) stat(%08x)\n", enabled, slave_activity, raw_stat, stat);

	if (stat & DW_IC_INTR_RX_FULL) {
		if (!(bus->slave_status & STATUS_WRITE_IN_PROGRESS)) {
			bus->slave_status |= STATUS_WRITE_IN_PROGRESS;
			bus->slave_status &= ~STATUS_READ_IN_PROGRESS;
            bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_WRITE_REQUESTED, &val);
		}

		do {
            tmp = readl(&i2c_base->ic_cmd_data);
			if (tmp & DW_IC_DATA_CMD_FIRST_DATA_BYTE)
                bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_WRITE_REQUESTED, &val);
			val = tmp;
            bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_WRITE_RECEIVED, &val);
            tmp = readl(&i2c_base->ic_status);
		} while (tmp & DW_IC_STATUS_RFNE);
	}

	if (stat & DW_IC_INTR_RD_REQ) {
		if (slave_activity) {
            tmp = readl(&i2c_base->ic_clr_rd_req);

			if (!(bus->slave_status & STATUS_READ_IN_PROGRESS)) {
                bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_READ_REQUESTED, &val);
				bus->slave_status |= STATUS_READ_IN_PROGRESS;
				bus->slave_status &= ~STATUS_WRITE_IN_PROGRESS;
			} else {
                bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_READ_PROCESSED, &val);
			}
            writel(val, &i2c_base->ic_cmd_data);
		}
	}

	if (stat & DW_IC_INTR_STOP_DET)
        bus->slave_callback(bus->slave_callback_ctx, I2C_SLAVE_STOP, &val);
}

static void dw_i2c_slave_init(struct chip_i2c_bus* bus) {
    struct i2c_regs *i2c_base = bus->i2c.regs;
    rt_uint8_t slave_address = bus->slave_address;
    int irq = bus->irq;

    dw_i2c_enable(i2c_base, 0);
    bus->slave_status &= ~STATUS_ACTIVE;

    if (!bus->slave_callback) {
        // error
        rt_kprintf("i2c device %s has no slave callback\n", bus->device_name);
        return;
    }

    bus->slave_status = 0;
    writel(0, &i2c_base->ic_tx_tl);
    writel(0, &i2c_base->ic_rx_tl);
    writel(
        DW_IC_CON_RX_FIFO_FULL_HLD_CTRL |
        DW_IC_CON_RESTART_EN |
        DW_IC_CON_STOP_DET_IFADDRESSED,
        &i2c_base->ic_con
    );
    writel(DW_IC_INTR_SLAVE_MASK, &i2c_base->ic_intr_mask);
    writel(slave_address, &i2c_base->ic_sar);
    rt_hw_interrupt_install(irq, dw_i2c_slave_isr, bus, bus->device_name);

    bus->slave_status |= STATUS_ACTIVE;
    dw_i2c_enable(i2c_base, 1);

    rt_hw_interrupt_umask(irq);
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
    if (bus->slave) {
        return -1;
    }

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

        if (i2c_buses[i].slave) {
            dw_i2c_slave_init(&i2c_buses[i]);
        } else {
            dw_i2c_init((struct i2c_regs *)i2c_buses[i].i2c.regs);
            designware_i2c_set_bus_speed(&i2c_buses[i], I2C_FAST_SPEED);

            /* pinctrl configuration */
            /* wait for next version */
            /* register i2c device */
            rt_i2c_bus_device_register(&i2c_buses[i].parent, i2c_buses[i].device_name);
        }
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