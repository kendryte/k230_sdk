/* 
 * Canaan thermal sensor driver
 * 
 */

#include <linux/clk.h>
#include <linux/cpu_cooling.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/types.h>
#include <linux/canaan-hardlock.h>


#define TS_CONFIG           0x00
#define TS_DATA             0x04
#define TS_POWERDOWN        0x22
#define TS_POWERON          0x23

// #define _debug_print

static int hardlock;
static bool request_status = false;

struct canaan_thermal_data {
    struct thermal_zone_device *tz;
    void __iomem *base;
};


static int canaan_get_temp(struct thermal_zone_device *tz, int *temp)
{
    struct canaan_thermal_data *data = tz->devdata;
    u32 val = 0;

    while(hardlock_lock(hardlock));
    iowrite32(TS_POWERDOWN, data->base + TS_CONFIG);
    iowrite32(TS_POWERON, data->base + TS_CONFIG);
    msleep(20);
    hardlock_unlock(hardlock);

    while(1)
    {
        val = ioread32(data->base + TS_DATA);
        // msleep(2600);

        if(val >> 12)
        {
        #ifdef _debug_print
            printk("val: 0x%x\n", val);
        #else
            *temp = val;
            break;
        #endif
        }
    }

    return 0;
}


static struct thermal_zone_device_ops canaan_tz_ops = {
    .get_temp = canaan_get_temp,
};

static const struct of_device_id of_canaan_thermal_match[] = {
    { .compatible = "canaan,k230-tsensor"},
    { /* end */ }
};
MODULE_DEVICE_TABLE(of, of_canaan_thermal_match);

static int canaan_thermal_probe(struct platform_device *pdev)
{
    struct canaan_thermal_data *data;
    struct resource *res;
    int ret;

#ifdef _debug_print
    printk("[TS]: %s %d\n", __func__, __LINE__);
#endif

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if(!data)
        return -ENOMEM;
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    data->base = devm_ioremap_resource(&pdev->dev, res);
    if(IS_ERR(data->base))
        return PTR_ERR(data->base);

    platform_set_drvdata(pdev, data);

    if (!request_status) {
		if(of_property_read_u32(pdev->dev.of_node, "hardlock", &hardlock))
        {
            dev_err(&pdev->dev, "fail to parse hardlock num\n");
            return -EINVAL;
        }
		if (request_lock(hardlock))
		{
			dev_err(&pdev->dev, "request hardlock %d failed!\n", hardlock);
			hardlock = -1;
		}
		request_status = true;
		dev_err(&pdev->dev, "request hardlock %d success!\n", hardlock);
	}

    data->tz = thermal_zone_device_register("canaan_thermal_zone", 
                                0, 0, data,
                                &canaan_tz_ops, NULL,
                                0, 0);
    
    if(IS_ERR(data->tz))
    {
        ret = PTR_ERR(data->tz);
        dev_err(&pdev->dev, 
            "failed to register thermal zone device %d\n", ret);
        return ret;
    }

    while(hardlock_lock(hardlock));
    iowrite32(TS_POWERDOWN, data->base + TS_CONFIG);
    iowrite32(TS_POWERON, data->base + TS_CONFIG);
    msleep(20);
    hardlock_unlock(hardlock);

#ifdef _debug_print
    printk("[TS]: %s %d\n", __func__, __LINE__);
#endif

    return 0;
}

static int canaan_thermal_remove(struct platform_device *pdev)
{
    struct canaan_thermal_data *data = platform_get_drvdata(pdev);

    thermal_zone_device_unregister(data->tz);
    
    return 0;
}

static struct platform_driver canaan_thermal = {
    .driver = {
        .name = "canaan_thermal",
        .of_match_table = of_canaan_thermal_match,
    },
    .probe = canaan_thermal_probe,
    .remove = canaan_thermal_remove,
};
module_platform_driver(canaan_thermal);

MODULE_DESCRIPTION("Thermal driver for canaan k230 Soc");
MODULE_LICENSE("GPL v2");

