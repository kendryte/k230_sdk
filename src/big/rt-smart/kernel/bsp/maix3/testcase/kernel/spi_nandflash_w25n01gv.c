#include <rtthread.h>
#include <rtdevice.h>
#include <rtdef.h>
#include <drivers/spi.h>
#include "sysctl_rst.h"
#include "drv_spi.h"
#include <stdint.h>
#include "utest.h"
#define BIT0            (1 << 0)
#define BIT1            (1 << 1)
#define BIT2            (1 << 2)
#define BIT3            (1 << 3)
#define BIT4            (1 << 4)
#define BIT5            (1 << 5)
#define BIT6            (1 << 6)
#define BIT7            (1 << 7)
#define BIT8            (1 << 8)
#define BIT9            (1 << 9)
#define BIT10           (1 << 10)
#define BIT11           (1 << 11)
#define BIT12 	        (1 << 12)
#define BIT13 	        (1 << 13)
#define BIT14 	        (1 << 14)
#define BIT15 	        (1 << 15)
#define BIT16 	        (1 << 16)
#define BIT17 	        (1 << 17)
#define BIT18 	        (1 << 18)
#define BIT19 	        (1 << 19)
#define BIT20 	        (1 << 20)
#define BIT21 	        (1 << 21)
#define BIT22 	        (1 << 22)
#define BIT23 	        (1 << 23)
#define BIT24 	        (1 << 24)
#define BIT25 	        (1 << 25)
#define BIT26 	        (1 << 26)
#define BIT27 	        (1 << 27)
#define BIT28 	        (1 << 28)
#define BIT29 	        (1 << 29)
#define BIT30 	        (1 << 30)
#define BIT31 	        (1 << 31)


#define DEF_MID             0x00
#define GD_MID              0xC8
#define MACRONIX_MID        0xC2
#define MICRON_MID          0x2C
#define TOSHIBA_MID         0x98
#define WINBOND_MID         0xEF
#define PARAGON_MID         0xA1
#define XTX_MID             0x0B

#define PAGE_256B     0
#define PAGE_512B     1
#define PAGE_1KB      2
#define PAGE_2KB      3
#define PAGE_4KB      4

#define OTP_VALID	BIT0
#define FLASH_QUAD	BIT1
#define FLASH_OCTAL	BIT2
#define ADDR_2B	    BIT3
#define ADDR_3B	    BIT4
#define DUMMY_1B	BIT5
#define DUMMY_2B	BIT6


#define READ_JEDEC_ID           0x9f
#define NOR_ENABLE_RESET        0x66
#define NOR_RESET               0x99
#define NAND_RESET              0xFF
#define NAND_GET_FEATURE        0x0F
#define NAND_SET_FEATURE        0x1F
#define READ_TO_CACHE           0x13
#define READ_FROM_CACHE         0x03
#define READ_FROM_CACHE_FAST    0x0B
#define READ_FROM_CACHE_X4      0x6B
#define READ_FROM_CACHE_X8      0x8B
#define NAND_WRITE_EN           0x06
#define NAND_WRITE_DIS          0x04
#define NAND_PROG_LOAD                  0x02
#define NAND_PROG_LOAD_X4               0x32
#define NAND_PROG_LOAD_RANDOM           0x84
#define NAND_PROG_LOAD_RANDOM_X4        0x34
#define NAND_PROG_LOAD_X8               0x82
#define NAND_PROG_EXEC                  0x10
#define NAND_BLOCK_ERASE                0xD8


#define FLASH_INFO(mid_, did0_, did1_, page_shift_, baud_, rx_delay_, flag_)    \
    {                                               \
        .mid = mid_,                                \
        .did[0] = did0_,                            \
        .did[1] = did1_,                            \
        .page_shift = page_shift_,                  \
        .baud = baud_,                              \
        .rx_delay = rx_delay_,                      \
        .flag = flag_,                              \
    }


typedef struct flash_info
{
    uint8_t mid;
    uint8_t did[2];
    // 有的nandflash 一页2048bytes，最新的有一页4096bytes的。
    // 暂时使用其中3bit来表示，0-256B 1-512 2-1024 3-2048 4-4096
    // 剩余5bit保留
    uint8_t page_shift;
    uint32_t baud;
    uint8_t rx_delay;
    uint8_t flag;
} flash_info_t;




#define SPI_BAUDR           (25*1000*1000) // * MHz

flash_info_t *current_flash_info;
flash_info_t default_norflash_info = FLASH_INFO(DEF_MID, 0x00, 0x00, PAGE_256B, SPI_BAUDR, 5, ADDR_3B|DUMMY_1B);
flash_info_t default_nandflash_info = FLASH_INFO(DEF_MID, 0x00, 0x00, PAGE_2KB, SPI_BAUDR, 2, ADDR_2B|DUMMY_1B);

typedef enum {
    DWENUM_SPI_TXRX = 0,
    DWENUM_SPI_TX   = 1,
    DWENUM_SPI_RX   = 2,
    DWENUM_SPI_EEPROM = 3
} DWENUM_SPI_TRANSFER_MODE;


#define SPI_BUS_NAME                "spi1"
#define SPI_FLASH_DEVICE_NAME       "w25n01gv"

static struct rt_spi_device spi_dev_nand;           /* SPI设备对象 */
extern void rt_thread_exit(void);
static uint8_t *data_p;

static uint8_t addr_byte = 2;
static uint8_t dummy_byte = 1;
int test_len = 2 * 1024 * 1024;
uint8_t *write_buf;
uint8_t *read_buf;


static void data_prepare(void)
{
    write_buf = (uint8_t *)rt_malloc(test_len);
    read_buf = (uint8_t *)rt_malloc(test_len) ;
#ifndef BUILD_FE
    for (int i = 0; i < test_len ; i++)
    {
        write_buf[i] = i % 256;
        read_buf[i] = 0;
    }
#endif
}

static void data_destroy(void)
{
    rt_free(write_buf);
    rt_free(read_buf);
}

static rt_err_t check_data(uint8_t *buf1, uint8_t *buf2, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        if (buf1[i] != buf2[i])
        {
            LOG_E("error: buf1[%d] != buf2[%d], %d != %d\n", i, i, buf1[i], buf2[i]);
            return -RT_ERROR;
        }
    }
    return RT_EOK;
}

static void nandflash_wait_idle(void)
{
    uint8_t cmd[2] = {0x0f, 0xc0};
    uint8_t status;
    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_EEPROM,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;

    while(1)
    {
        rt_spi_send_then_recv(spi_dev_w25n, cmd, 2, &status, 1);

        if((status & (1<<0)) == 0)
        {
            return;
        }
        else if(status & (1<<2))
        {
            LOG_D("nand is in erase progress\n");
        }
        else if(status & (1<<3))
        {
            LOG_D("nand is in write progress\n");
        } else if(status & (3<<4) == (2<<4))
        {
            LOG_D("nand is in ecc progress\n");
        }
    }
}

static void nandflash_write_enable(void)
{
    uint8_t cmd[] = { NAND_WRITE_EN };

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;
    rt_spi_send(spi_dev_w25n, cmd, 1);
}

static void nandflash_block_erase(uint32_t block_addr)
{
    nandflash_write_enable();
    uint8_t cmd[4];

    cmd[0] = NAND_BLOCK_ERASE;
    cmd[1] = (block_addr>>16) & 0xff;
    cmd[2] = (block_addr>>8) & 0xff;
    cmd[3] = block_addr & 0xff;

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;
    rt_spi_send(spi_dev_w25n, cmd, 4);

    nandflash_wait_idle();
}

void nandflash_erase(uint32_t addr, size_t len)
{
    uint32_t pagesize = 256<<current_flash_info->page_shift;
    uint32_t blockpage = 64;
    uint32_t col_addr = addr % pagesize;
    uint32_t ra_addr = addr / pagesize;
    size_t erase_len = len;
    nandflash_block_erase(ra_addr);
    while(len)
    {
        erase_len = (len < pagesize - col_addr) ? len : (pagesize - col_addr);
        len -= erase_len;
        col_addr = 0;
        ra_addr ++;
        if(ra_addr % blockpage == 0)
        {
            nandflash_block_erase(ra_addr);
        }
    }
}

static void nandflash_less_page_write(uint32_t col_addr, uint32_t ra_addr, uint8_t *data, uint32_t len)
{
    uint8_t *cmd = (uint8_t *)rt_malloc(test_len + 4);
    int i = 0;

    nandflash_write_enable();

    cmd[0] = NAND_PROG_LOAD_RANDOM;

    for(i=1; i<=addr_byte; i++)
    {
        cmd[i] = (col_addr>>((addr_byte-i)*8)) & 0xff; // 先发送地址的高位
    }

    rt_memcpy(cmd+i, data, len);

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;
    rt_spi_send(spi_dev_w25n, cmd, 1 + addr_byte + len);

    cmd[0] = NAND_PROG_EXEC;
    cmd[1] = (ra_addr >> 16) & 0xff;
    cmd[2] = (ra_addr >> 8) & 0xff;
    cmd[3] = ra_addr & 0xff;

    spi_dev_w25n->bus->owner = RT_NULL;
    rt_spi_send(spi_dev_w25n, cmd, 1 + 3);

    nandflash_wait_idle();

    rt_free(cmd);
}

static void nandflash_write(uint32_t addr, uint8_t *data, size_t len) //地址需要pagebytes对齐
{
    uint32_t pagesize = 256<<current_flash_info->page_shift;
    uint32_t col_addr = addr % pagesize;
    uint32_t ra_addr = addr / pagesize;
    size_t write_len = len;

    while(len)
    {
        write_len = (len < pagesize - col_addr) ? len : (pagesize - col_addr);
        nandflash_less_page_write(col_addr, ra_addr, data, write_len);
        len -= write_len;
        data += write_len;
        col_addr = 0;
        ra_addr ++;
    }
}

static void nandflash_less_page_read(uint32_t col_addr, uint32_t ra_addr, uint8_t *data, uint32_t len)
{
    uint8_t cmd[5];

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;

    cmd[0] = READ_TO_CACHE;
    cmd[1] = (ra_addr >> 16) & 0xff;
    cmd[2] = (ra_addr >> 8) & 0xff;
    cmd[3] = ra_addr & 0xff;
    rt_spi_send(spi_dev_w25n, cmd, 4);

    nandflash_wait_idle();

    cfg.mode = DWENUM_SPI_EEPROM;
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;

    cmd[0] = READ_FROM_CACHE_FAST;
    int i = 1;
    for(i=1; i<=addr_byte; i++)
    {
        cmd[i] = (col_addr>>((addr_byte-i)*8)) & 0xff; // column address[15:0]
    }
    cmd[i] = 0; // 8 dummy clocks
    rt_spi_send_then_recv(spi_dev_w25n, cmd, i+1, data, len);
}

static void nandflash_read(uint32_t addr, uint8_t *data, size_t len)
{
    uint32_t pagesize = 256<<current_flash_info->page_shift;
    uint32_t col_addr = addr % pagesize;
    uint32_t ra_addr = addr / pagesize;
    size_t read_len = len;
    while(len)
    {
        read_len = (len < pagesize - col_addr) ? len : (pagesize - col_addr);
        nandflash_less_page_read(col_addr, ra_addr, data, read_len);
        len -= read_len;
        data += read_len;
        col_addr = 0;
        ra_addr ++;
    }
}

static void nandflash_unlock_all_block(void)
{
    uint8_t cmd[4];
    cmd[0] = NAND_SET_FEATURE;
    cmd[1] = 0xA0;
    cmd[2] = 0x00;

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;
    rt_spi_send(spi_dev_w25n, cmd, 3);
}

static void nandflash_reset(void)
{
    uint8_t cmd[1];
    cmd[0] = NAND_RESET;

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_TX,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;

    rt_spi_send(spi_dev_w25n, cmd, 1);
    nandflash_wait_idle();
}

static void nandflash_detect(void)
{
    uint8_t cmd[4]; // 1 dummy
    uint8_t id_table[4];

    cmd[0] = READ_JEDEC_ID;
    cmd[1] = 0x0;

    struct rt_spi_device *spi_dev_w25n;
    struct rt_spi_configuration cfg = {
        .mode = DWENUM_SPI_EEPROM,
        .data_width = 8,
        .max_hz = SPI_BAUDR,
    };
    spi_dev_w25n = (struct rt_spi_device *)rt_device_find(SPI_FLASH_DEVICE_NAME);
    spi_dev_w25n->config = cfg;
    spi_dev_w25n->bus->owner = RT_NULL;

    rt_spi_send_then_recv(spi_dev_w25n, cmd, 2, id_table, 3);

    LOG_W("read ID: [ 0x%X 0x%X 0x%X] \n", id_table[0], id_table[1], id_table[2]);
}


typedef struct mux_config {
    rt_uint32_t st : 1;
	/*!< Schmitt trigger. */
	rt_uint32_t ds : 4;
	/*!< Driving selector. */
	rt_uint32_t pd : 1;
	/*!< Pull down enable. 0 for nothing, 1 for pull down. */
	rt_uint32_t pu : 1;
	/*!< Pull up enable. 0 for nothing, 1 for pull up. */
	rt_uint32_t oe_en : 1;
	/*!< Static output enable. */
	rt_uint32_t ie_en : 1;
	/*!< Static output enable. */
	rt_uint32_t msc : 1;
	/*!< msc control bit. */
	rt_uint32_t sl : 1;
	/*!< Slew rate control enable. */
	/*!< IO config setting. */
	rt_uint32_t io_sel : 3;
	/*!< set io function mode. */
	rt_uint32_t resv0 : 17;
	/*!< Reserved bits. */
	rt_uint32_t pad_di : 1;
	/*!< Read current IO's data input. */
}mux_config_t;

#define MUXPIN_NUM_IO    (64)
typedef struct muxpin_ {
	mux_config_t io[MUXPIN_NUM_IO];
	/*!< FPIOA GPIO multiplexer io array */
}muxpin_t;

/* IOMUX */
#define IOMUX_BASE_ADDR 0x91105000
#define IOMUX_REG_SIZE 0x1000
volatile muxpin_t *const muxpin = (volatile muxpin_t *)IOMUX_BASE_ADDR;

/**
 * @brief set io configuretion
 * 
 * @param io_num  0 ~ 63
 * @param config  be careful
 * @return int 
 */
static rt_base_t muxpin_set_config(rt_base_t io_num, mux_config_t config)
{

#if 1
    muxpin->io[io_num].st       = config.st;
    muxpin->io[io_num].ds       = config.ds;
    muxpin->io[io_num].pd       = config.pd;
    muxpin->io[io_num].pu       = config.pu;
    muxpin->io[io_num].oe_en    = config.oe_en;
    muxpin->io[io_num].ie_en    = config.ie_en;
    muxpin->io[io_num].msc      = config.msc;
    muxpin->io[io_num].sl       = config.sl;
    muxpin->io[io_num].io_sel   = config.io_sel;
    muxpin->io[io_num].pad_di   = config.pad_di;
#else
    muxpin->io[io_num] = config;
#endif
    return 0;
}

static void spi_set_gpio(void)
{
    mux_config_t cs_pin = {.st = 1, .ds = 0x7, .pd = 0, .pu = 1, .oe_en = 1,
								 .ie_en = 0, .msc = 1, .sl = 0, .io_sel = 3};
	mux_config_t clk_pin = {.st = 1, .ds = 0x7, .pd = 0, .pu = 0, .oe_en = 1, 
								.ie_en = 0, .msc = 1, .sl = 0, .io_sel = 3};
	mux_config_t data0_pin = {.st = 1, .ds = 0x7, .pd = 0, .pu = 0, .oe_en = 1, 
								.ie_en = 1, .msc = 1, .sl = 0, .io_sel = 3};
    mux_config_t data1_pin = {.st = 1, .ds = 0x7, .pd = 0, .pu = 0, .oe_en = 1, 
								.ie_en = 1, .msc = 1, .sl = 0, .io_sel = 3};
    muxpin_set_config(14,cs_pin);
    muxpin_set_config(15,clk_pin);
    muxpin_set_config(16,data0_pin);
    muxpin_set_config(17,data1_pin);

}

static void nandflash_init(void)
{
    int index = 1;
    int i;
    int ret;
    uint32_t addr = 0;

    spi_set_gpio();

    rt_memcpy(current_flash_info, &default_nandflash_info, sizeof(flash_info_t));

    data_prepare();
    nandflash_reset();

    nandflash_detect();

    nandflash_unlock_all_block();

    for (addr = 0x100000; addr < 0xA00000; addr+=test_len) {

        nandflash_erase(addr, test_len);

        nandflash_write(addr, write_buf, test_len);

        nandflash_read(addr, read_buf, test_len);

        ret = check_data(write_buf, read_buf, test_len);
        uassert_int_equal(ret, RT_EOK);

        rt_thread_mdelay(10);
    }
    data_destroy();
}

static int rt_hw_w25n01gv_init(void)
{
    rt_err_t res;

    res = rt_spi_bus_attach_device(&spi_dev_nand, SPI_FLASH_DEVICE_NAME, SPI_BUS_NAME, (void*)&spi_dev_nand);
    if (res != RT_EOK)
    {
        LOG_E("rt_spi_bus_attach_device() failed!\n");
        return res;
    }
#ifndef RT_FASTBOOT
    LOG_D("%s attach to bus of %s\n", SPI_FLASH_DEVICE_NAME, SPI_BUS_NAME);
#endif
    return RT_EOK;
}
INIT_ENV_EXPORT(rt_hw_w25n01gv_init);

void nandflash_test()
{
    nandflash_init();
}

static void spi_testcase(void)
{
    UTEST_UNIT_RUN(nandflash_test);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

UTEST_TC_EXPORT(spi_testcase, "testcases.kernel.spi_nand_flash", utest_tc_init, utest_tc_cleanup, 10);


