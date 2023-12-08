// #include "autoconf.h"

#include "dfs_file.h"
#include "rtdef.h"
#include "rtthread.h"
#include "rthw.h"

#include "cache.h"
#include "ioremap.h"
#include "riscv_io.h"

#include "usb_config.h"
#include "core/usbd_core.h"
#include "usb_dc.h"
#include <string.h>
#include <unistd.h>

uint32_t SystemCoreClock = 32000000 * 2U;   // for cherryusb dwc2 use
uintptr_t g_usb_otg_base = (uintptr_t)0x91540000UL;

static void dcd_int_handler_wrap(int irq, void *param)
{
    (void)irq;
    (void)param;

#if CONFIG_ENABLE_USB_HOST
    extern void USBH_IRQHandler(void);
    USBH_IRQHandler();
#elif CONFIG_ENABLE_USB_DEVICE
    extern void USBD_IRQHandler(void);
    USBD_IRQHandler();
#endif
}

#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define DEV_NAME "ttyUSB1"

struct rt_device g_cdc_rt_device;
struct rt_semaphore cdc_read_sem, cdc_write_sem;
extern volatile char ep_tx_busy_flag;

static int cdc_open(struct dfs_fd *fd) {
    return 0;
}

static int cdc_close(struct dfs_fd *fd) {
    return 0;
}

volatile size_t actual_bytes = 0;

static int cdc_read(struct dfs_fd *fd, void *buf, size_t count) {
    while(rt_sem_take(&cdc_read_sem, 0) == RT_EOK);
    usbd_ep_start_read(CDC_OUT_EP, buf, count);
    rt_sem_take(&cdc_read_sem, RT_WAITING_FOREVER);
    return actual_bytes;
}

static int cdc_write(struct dfs_fd *fd, const void *buf, size_t count) {
    if (count == 0) {
        return 0;
    }
    // TODO: async
    rt_sem_take(&cdc_write_sem, RT_WAITING_FOREVER);
    usbd_ep_start_write(CDC_IN_EP, buf, count);
    return count;
}

static int cdc_poll(struct dfs_fd *fd, struct rt_pollreq *req) {
    return 0;
}

static struct dfs_file_ops cdc_ops = {
    .open = cdc_open,
    .close = cdc_close,
    .read = cdc_read,
    .write = cdc_write,
    .poll = cdc_poll
};

volatile rt_device_t sdcard_device = RT_NULL;

void usb_dc_low_level_init(void)
{
    rt_device_t device = &g_cdc_rt_device;
    uint32_t *hs_reg = (uint32_t *)rt_ioremap((void *)(0x91585000 + 0x9C), 0x1000);
    *hs_reg = 0;
    rt_iounmap(hs_reg);

    rt_hw_interrupt_install(174, dcd_int_handler_wrap, NULL, "usb");
    rt_hw_interrupt_umask(174);

    // CDC uart
    rt_err_t err = rt_device_register(device, DEV_NAME, RT_DEVICE_OFLAG_RDWR);
    if (err != RT_EOK) {
        rt_kprintf("[usb] register device error\n");
        return;
    }
    device->fops = &cdc_ops;
    rt_sem_init(&cdc_read_sem, "cdc/read", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&cdc_write_sem, "cdc/write", 1, RT_IPC_FLAG_FIFO);

#define MEMORY_MSC 1
#if !MEMORY_MSC
    // wait sdcard ready
    //while (rt_device_find("sd") == RT_NULL) {
    //    rt_thread_delay(10);
    //}
    sdcard_device = rt_device_find("sd");
    if (sdcard_device == RT_NULL) {
        rt_kprintf("[usb] find sdcard error\n");
        goto skipsdcard;
    }
    if (rt_device_open(sdcard_device, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
        rt_kprintf("[usb] open sdcard error\n");
        goto skipsdcard;
    }
    struct rt_device_blk_geometry geometry;
    if (rt_device_control(sdcard_device, RT_DEVICE_CTRL_BLK_GETGEOME, &geometry) != RT_EOK) {
        rt_kprintf("[usb] sdcard RT_DEVICE_CTRL_BLK_GETGEOME error\n");
        rt_device_close(sdcard_device);
        goto skipsdcard;
    }
    rt_kprintf("[usb] sdcard sector: %u, bytes/sector: %u, block size: %u\n",
        geometry.sector_count,
        geometry.bytes_per_sector,
        geometry.block_size
    );
skipsdcard:
#endif
}

#if !MEMORY_MSC
#define BLOCK_NUM 1024
#define BLOCK_SIZE 512
static uint8_t memory_msc[BLOCK_NUM][BLOCK_SIZE];

void usbd_msc_get_cap(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    struct rt_device_blk_geometry geometry;
    if (sdcard_device == RT_NULL) {
        *block_num = 0;
        *block_size = 0;
    }
    else if (rt_device_control(sdcard_device, RT_DEVICE_CTRL_BLK_GETGEOME, &geometry) == RT_EOK) {
        // should be block_size?
        *block_num = geometry.sector_count;
        *block_size = geometry.bytes_per_sector;
    } else {
        *block_num = 0;
        *block_size = 0;
    }
    rt_kprintf("[usb] MSC block num: %u, block size: %u\n", *block_num, *block_size);
}

int usbd_msc_sector_read(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    if (sdcard_device == RT_NULL) {
        return RT_EOK;
    }
    return rt_device_read(sdcard_device, sector, buffer, length);
}

int usbd_msc_sector_write(uint32_t sector, uint8_t *buffer, uint32_t length)
{
    if (sdcard_device == RT_NULL) {
        return RT_EOK;
    }
    return rt_device_write(sdcard_device, sector, buffer, length);
}
#endif

void usb_dc_low_level_deinit(void)
{
    rt_hw_interrupt_mask(174);
    rt_sem_detach(&cdc_read_sem);
    rt_sem_detach(&cdc_write_sem);
}

void usb_hc_low_level_init(void)
{
    usb_dc_low_level_init();
}

int usb_init(void)
{
#if CONFIG_ENABLE_SUPPORT_NONCACHE
    extern char _cherryusb_no_cache_start;
    extern char _cherryusb_no_cache_end;

    rt_hw_cpu_dcache_invalidate(&_cherryusb_no_cache_start, &_cherryusb_no_cache_end - &_cherryusb_no_cache_start);
    rt_hw_cpu_dcache_clean_flush(&_cherryusb_no_cache_start, &_cherryusb_no_cache_end - &_cherryusb_no_cache_start);

    char *p = &_cherryusb_no_cache_start;
    for (size_t i = 0; i < (&_cherryusb_no_cache_end - &_cherryusb_no_cache_start); i++)
    {
        p[i] = 0x0;
    }

    rt_kprintf("set %p - %p no cache\n", &_cherryusb_no_cache_start, &_cherryusb_no_cache_end);
#endif

    g_usb_otg_base = (uintptr_t)rt_ioremap_nocache((void *)0x91540000UL, 0x10000);

    rt_kprintf("usb_init end\n");
}
