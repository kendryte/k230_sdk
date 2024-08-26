#include "rtthread.h"
#include "drivers/sdio.h"
#include "drivers/mmcsd_card.h"
#include "drivers/mmcsd_core.h"
#include "customer_rtos_service.h"
#include "card.h"
#include "wifi_io.h"

#define ADDR_MASK 0x10000
#define LOCAL_ADDR_MASK 0x00000

extern struct rt_sdio_function* rtt_sdio_func;
static void (*wifi_irq_handler)(struct sdio_func *);

static int rtt_sdio_bus_probe(void)
{
    return 0;
}
static int rtt_sdio_bus_remove(void)
{
    return 0;
}

static int rtt_sdio_enable_func(struct sdio_func* func)
{
    return sdio_enable_func(rtt_sdio_func);
}

static int rtt_sdio_disable_func(struct sdio_func* func)
{
    return sdio_disable_func(rtt_sdio_func);
}

static void rtt_sdio_irq_handler(struct rt_sdio_function* rt_func)
{
    struct sdio_func* func = sdio_get_drvdata(rt_func);
    wifi_irq_handler(func);
}

static int rtt_sdio_claim_irq(struct sdio_func* func, void (*handler)(struct sdio_func*))
{
    wifi_irq_handler = handler;
    int ret = sdio_attach_irq(rtt_sdio_func, rtt_sdio_irq_handler);
    rtt_sdio_func->card->host->ops->enable_sdio_irq(rtt_sdio_func->card->host, 1);
    return ret;
}

static int rtt_sdio_release_irq(struct sdio_func* func)
{
    wifi_irq_handler = NULL;
    return sdio_detach_irq(rtt_sdio_func);
}

static void rtt_sdio_claim_host(struct sdio_func* func)
{
    mmcsd_host_lock(rtt_sdio_func->card->host);
}

static void rtt_sdio_release_host(struct sdio_func* func)
{
    mmcsd_host_unlock(rtt_sdio_func->card->host);
}

static unsigned char rtt_sdio_readb(struct sdio_func* func, unsigned int addr, int* err_ret)
{
    return sdio_io_readb(rtt_sdio_func, addr, err_ret);
}

static unsigned short rtt_sdio_readw(struct sdio_func* func, unsigned int addr, int* err_ret)
{
    unsigned short data;
    int ret = sdio_io_rw_extended_block(rtt_sdio_func, 0, addr, 1, (rt_uint8_t*)&data, sizeof(unsigned short));
    if (err_ret)
        *err_ret = ret;
    return data;
}

static unsigned int rtt_sdio_readl(struct sdio_func* func, unsigned int addr, int* err_ret)
{
    unsigned int data;
    int ret = sdio_io_rw_extended_block(rtt_sdio_func, 0, addr, 1, (rt_uint8_t*)&data, sizeof(unsigned int));
    if (err_ret)
        *err_ret = ret;
    return data;
}

static void rtt_sdio_writeb(struct sdio_func* func, unsigned char b, unsigned int addr, int* err_ret)
{
    int ret = sdio_io_writeb(rtt_sdio_func, addr, b);
    if (err_ret)
        *err_ret = ret;
}

static void rtt_sdio_writew(struct sdio_func* func, unsigned short b, unsigned int addr, int* err_ret)
{
    int ret = sdio_io_rw_extended_block(rtt_sdio_func, 1, addr, 1, (rt_uint8_t*)&b, sizeof(unsigned short));
    if (err_ret)
        *err_ret = ret;
}

static void rtt_sdio_writel(struct sdio_func* func, unsigned int b, unsigned int addr, int* err_ret)
{
    int ret = sdio_io_rw_extended_block(rtt_sdio_func, 1, addr, 1, (rt_uint8_t*)&b, sizeof(unsigned int));
    if (err_ret)
        *err_ret = ret;
}

static int rtt_sdio_memcpy_fromio(struct sdio_func* func, void* dst, unsigned int addr, int count)
{
    return sdio_io_rw_extended_block(rtt_sdio_func, 0, addr, 1, dst, count);
}

static int rtt_sdio_memcpy_toio(struct sdio_func* func, unsigned int addr, void* src, int count)
{
    return sdio_io_rw_extended_block(rtt_sdio_func, 1, addr, 1, src, count);
}

int wifi_read(struct sdio_func* func, u32 addr, u32 cnt, void* pdata)
{
    rtt_sdio_claim_host(func);
    int err = rtt_sdio_memcpy_fromio(func, pdata, addr, cnt);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL(%d)! ADDR=%#x Size=%d\n", __func__, err, addr, cnt);
    return err;
}

int wifi_write(struct sdio_func* func, u32 addr, u32 cnt, void* pdata)
{
    rtt_sdio_claim_host(func);
    int err = rtt_sdio_memcpy_toio(func, addr, pdata, cnt);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL(%d)! ADDR=%#x Size=%d\n", __func__, err, addr, cnt);
    return err;
}

u8 wifi_readb(struct sdio_func* func, u32 addr)
{
    int err;
    u8 v;

    rtt_sdio_claim_host(func);
    v = rtt_sdio_readb(func, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);
    return v;
}

u16 wifi_readw(struct sdio_func* func, u32 addr)
{
    int err;
    u16 v;

    rtt_sdio_claim_host(func);
    v = rtt_sdio_readw(func, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);
    return v;
}

u32 wifi_readl(struct sdio_func* func, u32 addr)
{
    int err;
    u32 v;

    rtt_sdio_claim_host(func);
    v = rtt_sdio_readl(func, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);
    return v;
}

void wifi_writeb(struct sdio_func* func, u32 addr, u8 v)
{
    int err;

    rtt_sdio_claim_host(func);
    rtt_sdio_writeb(func, v, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr, v);
}

void wifi_writew(struct sdio_func* func, u32 addr, u16 v)
{
    int err;

    rtt_sdio_claim_host(func);
    rtt_sdio_writew(func, v, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x val=0x%04x\n", __func__, err, addr, v);
}

void wifi_writel(struct sdio_func* func, u32 addr, u32 v)
{
    int err;

    rtt_sdio_claim_host(func);
    rtt_sdio_writel(func, v, ADDR_MASK | addr, &err);
    rtt_sdio_release_host(func);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x val=0x%08x\n", __func__, err, addr, v);
}

u8 wifi_readb_local(struct sdio_func* func, u32 addr)
{
    int err;
    u8 v;

    v = rtt_sdio_readb(func, LOCAL_ADDR_MASK | addr, &err);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);

    return v;
}

void wifi_writeb_local(struct sdio_func* func, u32 addr, u8 val)
{
    int err;

    rtt_sdio_writeb(func, val, LOCAL_ADDR_MASK | addr, &err);
    if (err)
        DBG_INFO("%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr, val);
}

extern int rtw_fake_driver_probe(struct sdio_func* func);
void wifi_fake_driver_probe_rtlwifi(struct sdio_func* func)
{
    rtw_fake_driver_probe(func);
}

SDIO_BUS_OPS rtw_sdio_bus_ops = {
    rtt_sdio_bus_probe,
    rtt_sdio_bus_remove,
    rtt_sdio_enable_func,
    rtt_sdio_disable_func,
    NULL,
    NULL,
    rtt_sdio_claim_irq,
    rtt_sdio_release_irq,
    rtt_sdio_claim_host,
    rtt_sdio_release_host,
    rtt_sdio_readb,
    rtt_sdio_readw,
    rtt_sdio_readl,
    rtt_sdio_writeb,
    rtt_sdio_writew,
    rtt_sdio_writel,
    rtt_sdio_memcpy_fromio,
    rtt_sdio_memcpy_toio,
};
