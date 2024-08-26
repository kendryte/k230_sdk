/*
 * Copyright (c) 2011-2022, Shanghai Real-Thread Electronic Technology Co.,Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-01     SummerGift   add i2c driver
 */

#ifndef __DW_I2C_H_
#define __DW_I2C_H_

typedef unsigned char     uchar;
#define BIT(x)              (1<<(x))
/*
 * Registers offset
 */
#define DW_IC_CON           0x00u
#define DW_IC_TAR           0x04u
#define DW_IC_SAR           0x08u
#define DW_IC_HS_MADDR      0x0cu
#define DW_IC_DATA_CMD      0x10u
#define DW_IC_SS_SCL_HCNT   0x14u
#define DW_IC_SS_SCL_LCNT   0x18u
#define DW_IC_FS_SCL_HCNT   0x1cu
#define DW_IC_FS_SCL_LCNT   0x20u
#define DW_IC_HS_SCL_HCNT   0x24u
#define DW_IC_HS_SCL_LCNT   0x28u
#define DW_IC_INTR_STAT     0x2cu
#define DW_IC_INTR_MASK     0x30u
#define DW_IC_RAW_INTR_STAT 0x34u
#define DW_IC_RX_TL         0x38u
#define DW_IC_TX_TL         0x3cu
#define DW_IC_CLR_INTR      0x40u
#define DW_IC_CLR_RX_UNDER  0x44u
#define DW_IC_CLR_RX_OVER   0x48u
#define DW_IC_CLR_TX_OVER   0x4cu
#define DW_IC_CLR_RD_REQ    0x50u
#define DW_IC_CLR_TX_ABRT   0x54u
#define DW_IC_CLR_RX_DONE   0x58u
#define DW_IC_CLR_ACTIVITY  0x5cu
#define DW_IC_CLR_STOP_DET  0x60u
#define DW_IC_CLR_START_DET 0x64u
#define DW_IC_CLR_GEN_CALL  0x68u
#define DW_IC_ENABLE        0x6cu
#define DW_IC_STATUS        0x70u
#define DW_IC_TXFLR         0x74u
#define DW_IC_RXFLR         0x78u
#define DW_IC_SDA_HOLD      0x7cu
#define DW_IC_TX_ABRT_SOURCE            0x80u
#define DW_IC_SLV_DATA_NACK_ONLY        0x84u
#define DW_IC_DMA_CR        0x88u
#define DW_IC_DMA_TDLR      0x8cu
#define DW_IC_DMA_RDLR      0x90u
#define DW_IC_SDA_SETUP     0x94u
#define DW_IC_ACK_GENERAL_CALL          0x98u
#define DW_IC_ENABLE_STATUS 0x9cu
#define DW_IC_FS_SPKLEN     0xa0u
#define DW_IC_HS_SPKLEN     0xa4u
#define DW_IC_CLR_RESTART_DET           0xa8u
#define DW_IC_SCL_STUCK_AT_LOW_TIMEOUT  0xacu
#define DW_IC_SDA_STUCK_AT_LOW_TIMEOUT  0xb0u
#define DW_IC_CLR_SCL_STUCK_DET         0xb4u
#define DW_IC_DEVICE_ID     0xb8u
#define DW_IC_SMBUS_CLK_LOW_SEXT    0xbcu
#define DW_IC_SMBUS_CLK_LOW_MEXT    0xc0u
#define DW_IC_SMBUS_THIGH_MAX_IDLE_COUTN    0xc4u
#define DW_IC_SMBUS_INTR_STAT       0xc8u
#define DW_IC_SMBUS_INTR_MASK       0xccu
#define DW_IC_SMBUS_RAW_INTR_STAT   0xd0u
#define DW_IC_CLR_SMBUS_INTR        0xd4u
#define DW_IC_OPTIONAL_SAR          0xd8u
#define DW_IC_SMBUS_UDID_LSB        0xdcu
#define DW_IC_SMBUS_UDID_WORD0      0xdcu
#define DW_IC_SMBUS_UDID_WORD1      0xe0u
#define DW_IC_SMBUS_UDID_WORD2      0xe4u
#define DW_IC_SMBUS_UDID_WORD3      0xe8u
#define DW_IC_RESERVE               0xecu
#define DW_IC_REG_TIMEOUT_RST       0xf0u
#define DW_IC_COMP_PARAM_1          0xf4u
#define DW_IC_COMP_VERSION          0xf8u
#define DW_IC_SDA_HOLD_MIN_VERS     0x3131312Au
#define DW_IC_COMP_TYPE             0xfcu
#define DW_IC_COMP_TYPE_VALUE       0x44570140u
#define DW_IC_COMP_TYPE_MASK        0xffffu

#define DW_IC_CON_MASTER            0x1u
#define DW_IC_CON_SPEED_STD         0x2u
#define DW_IC_CON_SPEED_FAST        0x4u
#define DW_IC_CON_SPEED_HIGH        0x6u
#define DW_IC_CON_SPEED_MASK        0x6u
#define DW_IC_CON_10BITADDR_SLAVE   0x8u
#define DW_IC_CON_10BITADDR_MASTER  0x10u
#define DW_IC_CON_RESTART_EN        0x20u
#define DW_IC_CON_SLAVE_DISABLE     0x40u
#define DW_IC_CON_STOP_DET_IFADDRESSED  0x80u
#define DW_IC_CON_TX_EMPTY_CTRL         0x100u
#define DW_IC_CON_RX_FIFO_FULL_HLD_CTRL 0x200u

#define DW_IC_TAR_10BITADDR_MASTER  BIT(12)
#define DW_IC_DATA_CMD_READ         BIT(8)
#define DW_IC_DATA_CMD_STOP         BIT(9)
#define DW_IC_DATA_CMD_RESTART      BIT(10)
#define DW_IC_DATA_CMD_FIRST_DATA_BYTE		BIT(11)

#define DW_IC_INTR_RX_UNDER         0x001u
#define DW_IC_INTR_RX_OVER          0x002u
#define DW_IC_INTR_RX_FULL          0x004u
#define DW_IC_INTR_TX_OVER          0x008u
#define DW_IC_INTR_TX_EMPTY         0x010u
#define DW_IC_INTR_RD_REQ           0x020u
#define DW_IC_INTR_TX_ABRT          0x040u
#define DW_IC_INTR_RX_DONE          0x080u
#define DW_IC_INTR_ACTIVITY         0x100u
#define DW_IC_INTR_STOP_DET         0x200u
#define DW_IC_INTR_START_DET        0x400u
#define DW_IC_INTR_GEN_CALL         0x800u
#define DW_IC_INTR_RESTART_DET      0x1000u
#define DW_IC_INTR_DEFAULT_MASK     (DW_IC_INTR_RX_FULL | DW_IC_INTR_TX_ABRT | DW_IC_INTR_STOP_DET)
#define DW_IC_INTR_MASTER_MASK      (DW_IC_INTR_DEFAULT_MASK | DW_IC_INTR_TX_EMPTY)
#define DW_IC_INTR_SLAVE_MASK       (DW_IC_INTR_DEFAULT_MASK | DW_IC_INTR_RX_UNDER | DW_IC_INTR_RD_REQ)

#define DW_IC_STATUS_ACTIVITY       0x1u
#define DW_IC_STATUS_TFE                BIT(2)
#define DW_IC_STATUS_RFNE			BIT(3)
#define DW_IC_STATUS_MASTER_ACTIVITY    BIT(5)
#define DW_IC_STATUS_SLAVE_ACTIVITY     BIT(6)
#define DW_IC_STATUS_SLAVE_ACTIVITY_SHIFT  (6)

#define DW_IC_SDA_HOLD_RX_SHIFT     16
#define DW_IC_SDA_HOLD_RX_MASK      GENMASK(23, DW_IC_SDA_HOLD_RX_SHIFT)

#define DW_IC_ENABLE_STATUS_ENABLE      BIT(0)

#define DW_IC_COMP_PARAM_1_SPEED_MODE_HIGH  (BIT(2) | BIT(3))
#define DW_IC_COMP_PARAM_1_SPEED_MODE_MASK  GENMASK(3, 2)

#define DW_IC_ERR_TX_ABRT   0x1u

struct i2c_regs
{
    rt_uint32_t ic_con;             /* 0x00 */
    rt_uint32_t ic_tar;             /* 0x04 */
    rt_uint32_t ic_sar;             /* 0x08 */
    rt_uint32_t ic_hs_maddr;        /* 0x0c */
    rt_uint32_t ic_cmd_data;        /* 0x10 */
    rt_uint32_t ic_ss_scl_hcnt;     /* 0x14 */
    rt_uint32_t ic_ss_scl_lcnt;     /* 0x18 */
    rt_uint32_t ic_fs_scl_hcnt;     /* 0x1c */
    rt_uint32_t ic_fs_scl_lcnt;     /* 0x20 */
    rt_uint32_t ic_hs_scl_hcnt;     /* 0x24 */
    rt_uint32_t ic_hs_scl_lcnt;     /* 0x28 */
    rt_uint32_t ic_intr_stat;       /* 0x2c */
    rt_uint32_t ic_intr_mask;       /* 0x30 */
    rt_uint32_t ic_raw_intr_stat;   /* 0x34 */
    rt_uint32_t ic_rx_tl;           /* 0x38 */
    rt_uint32_t ic_tx_tl;           /* 0x3c */
    rt_uint32_t ic_clr_intr;        /* 0x40 */
    rt_uint32_t ic_clr_rx_under;    /* 0x44 */
    rt_uint32_t ic_clr_rx_over;     /* 0x48 */
    rt_uint32_t ic_clr_tx_over;     /* 0x4c */
    rt_uint32_t ic_clr_rd_req;      /* 0x50 */
    rt_uint32_t ic_clr_tx_abrt;     /* 0x54 */
    rt_uint32_t ic_clr_rx_done;     /* 0x58 */
    rt_uint32_t ic_clr_activity;    /* 0x5c */
    rt_uint32_t ic_clr_stop_det;    /* 0x60 */
    rt_uint32_t ic_clr_start_det;   /* 0x64 */
    rt_uint32_t ic_clr_gen_call;    /* 0x68 */
    rt_uint32_t ic_enable;          /* 0x6c */
    rt_uint32_t ic_status;          /* 0x70 */
    rt_uint32_t ic_txflr;           /* 0x74 */
    rt_uint32_t ic_rxflr;           /* 0x78 */
    rt_uint32_t ic_sda_hold;        /* 0x7c */
    rt_uint32_t ic_tx_abrt_source;  /* 0x80 */
    rt_uint8_t  res1[0x18];         /* 0x84 */
    rt_uint32_t ic_enable_status;   /* 0x9c */
};

#if !defined(IC_CLK)
#define IC_CLK          100
#endif
#define NANO_TO_MICRO   1000

/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME     4000
#define MIN_SS_SCL_LOWTIME      4700
#define MIN_FS_SCL_HIGHTIME     600
#define MIN_FS_SCL_LOWTIME      1300
#define MIN_HS_SCL_HIGHTIME     60
#define MIN_HS_SCL_LOWTIME      160

/* Worst case timeout for 1 byte is kept as 2ms */

#define CONFIG_SYS_HZ       RT_TICK_PER_SECOND
#define I2C_BYTE_TO         (CONFIG_SYS_HZ/250)
#define I2C_STOPDET_TO      (CONFIG_SYS_HZ/250)
#define I2C_BYTE_TO_BB      (I2C_BYTE_TO * 16)

/* i2c control register definitions */
#define IC_CON_SD               0x0040
#define IC_CON_RE               0x0020
#define IC_CON_10BITADDRMASTER  0x0010
#define IC_CON_10BITADDR_SLAVE  0x0008
#define IC_CON_SPD_MSK          0x0006
#define IC_CON_SPD_SS           0x0002
#define IC_CON_SPD_FS           0x0004
#define IC_CON_SPD_HS           0x0006
#define IC_CON_MM               0x0001

/* i2c target address register definitions */
#define TAR_ADDR        0x0050

/* i2c slave address register definitions */
#define IC_SLAVE_ADDR   0x0002

/* i2c data buffer and command register definitions */
#define IC_CMD          0x0100
#define IC_STOP         0x0200

/* i2c interrupt status register definitions */
#define IC_GEN_CALL     0x0800
#define IC_START_DET    0x0400
#define IC_STOP_DET     0x0200
#define IC_ACTIVITY     0x0100
#define IC_RX_DONE      0x0080
#define IC_TX_ABRT      0x0040
#define IC_RD_REQ       0x0020
#define IC_TX_EMPTY     0x0010
#define IC_TX_OVER      0x0008
#define IC_RX_FULL      0x0004
#define IC_RX_OVER      0x0002
#define IC_RX_UNDER     0x0001

/* fifo threshold register definitions */
#define IC_TL0          0x00
#define IC_TL1          0x01
#define IC_TL2          0x02
#define IC_TL3          0x03
#define IC_TL4          0x04
#define IC_TL5          0x05
#define IC_TL6          0x06
#define IC_TL7          0x07
#define IC_RX_TL        IC_TL0
#define IC_TX_TL        IC_TL0

/* i2c enable register definitions */
#define IC_ENABLE_0B    0x0001

/* i2c status register  definitions */
#define IC_STATUS_SA    0x0040
#define IC_STATUS_MA    0x0020
#define IC_STATUS_RFF   0x0010
#define IC_STATUS_RFNE  0x0008
#define IC_STATUS_TFE   0x0004
#define IC_STATUS_TFNF  0x0002
#define IC_STATUS_ACT   0x0001

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD    1
#define IC_SPEED_MODE_FAST      2
#define IC_SPEED_MODE_MAX       3

#define I2C_MAX_SPEED           3400000
#define I2C_FAST_SPEED          400000
#define I2C_STANDARD_SPEED      100000

struct i2c_adapter
{
    void (*init)(struct i2c_adapter *adap, int speed, int slaveaddr);
    int (*probe)(struct i2c_adapter *adap, rt_uint8_t chip);
    int (*read)(struct i2c_adapter *adap, rt_uint8_t chip,
                rt_uint32_t addr, int alen, rt_uint8_t *buffer, int len);
    int (*write)(struct i2c_adapter *adap, rt_uint8_t chip,
                 rt_uint32_t addr, int alen, rt_uint8_t *buffer, int len);
    rt_uint32_t (*set_bus_speed)(struct i2c_adapter *adap, rt_uint32_t speed);
    int speed;
    int waitdelay;
    int slaveaddr;
    int init_done;
    int hwadapnr;
    char *name;
};

/*
 * Not all of these flags are implemented in the U-Boot API
 */
enum dm_i2c_msg_flags
{
    I2C_M_TEN           = 0x0010, /* ten-bit chip address */
    I2C_M_RD            = 0x0001, /* read data, from slave to master */
    I2C_M_STOP          = 0x8000, /* send stop after this message */
    I2C_M_NOSTART       = 0x4000, /* no start before this message */
    I2C_M_REV_DIR_ADDR  = 0x2000, /* invert polarity of R/W bit */
    I2C_M_IGNORE_NAK    = 0x1000, /* continue after NAK */
    I2C_M_NO_RD_ACK     = 0x0800, /* skip the Ack bit on reads */
    I2C_M_RECV_LEN      = 0x0400, /* length is first received byte */
    I2C_M_RESTART       = 0x0002, /* restart before this message */
    I2C_M_START         = 0x0004, /* start before this message */
};

/**
 * struct i2c_msg - an I2C message
 *
 * @addr: Slave address
 * @flags: Flags (see enum dm_i2c_msg_flags)
 * @len: Length of buffer in bytes, may be 0 for a probe
 * @buf: Buffer to send/receive, or NULL if no data
 */
struct i2c_msg
{
    rt_uint32_t addr;
    rt_uint32_t flags;
    rt_uint32_t len;
    rt_uint8_t *buf;
};

/**
 * struct i2c_msg_list - a list of I2C messages
 *
 * This is called i2c_rdwr_ioctl_data in Linux but the name does not seem
 * appropriate in U-Boot.
 *
 * @msg: Pointer to i2c_msg array
 * @nmsgs: Number of elements in the array
 */
struct i2c_msg_list
{
    struct i2c_msg *msgs;
    rt_uint32_t nmsgs;
};

#endif /* __DW_I2C_H_ */
