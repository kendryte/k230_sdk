/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/component.h>
#include <linux/of_graph.h>
#include <linux/of_platform.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_of.h>
#include <drm/drm_irq.h>
#include <drm/drm_vblank.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_print.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_probe_helper.h>

#include "canaan_drv.h"
#include "canaan_vo.h"

static int canaan_drm_open(struct inode *inode, struct file *filp);
static int canaan_drm_release(struct inode *inode, struct file *filp);

static const struct file_operations canaan_drm_fops = {
    .owner		= THIS_MODULE,
    .open		= canaan_drm_open,
    .release	= canaan_drm_release,
    .unlocked_ioctl	= drm_ioctl,
    .compat_ioctl	= drm_compat_ioctl,
    .poll		= drm_poll,
    .read		= drm_read,
    .llseek		= noop_llseek,
    .mmap		= drm_gem_cma_mmap,
    DRM_GEM_CMA_UNMAPPED_AREA_FOPS
};

static struct drm_driver canaan_drm_driver = {
    .driver_features    = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
    DRM_GEM_CMA_DRIVER_OPS_VMAP,
    .fops               = &canaan_drm_fops,
    .name               = "canaan-drm",
    .desc               = "Canaan K230 DRM driver",
    .date               = "20230501",
    .major              = 1,
    .minor              = 0,
};

static const struct drm_mode_config_funcs canaan_drm_mode_config_funcs = {
    .fb_create = drm_gem_fb_create,
    .atomic_check = drm_atomic_helper_check,
    .atomic_commit = drm_atomic_helper_commit,
};

static const struct drm_mode_config_helper_funcs canaan_drm_mode_config_helpers = {
    .atomic_commit_tail = drm_atomic_helper_commit_tail_rpm,
};

static struct device *disp_dev;

static int canaan_drm_open(struct inode *inode, struct file *filp)
{
    int ret;

    pm_runtime_get_sync(disp_dev);
    ret = drm_open(inode, filp);

    return ret;
}

static int canaan_drm_release(struct inode *inode, struct file *filp)
{
    int ret;

    ret = drm_release(inode, filp);
    pm_runtime_put_sync(disp_dev);

    return ret;
}

static int canaan_drm_bind(struct device *dev)
{
    int ret = 0;
    struct drm_device *drm_dev;

    drm_dev = drm_dev_alloc(&canaan_drm_driver, dev);
    if (IS_ERR(drm_dev))
        return PTR_ERR(drm_dev);
    dev_set_drvdata(dev, drm_dev);

    drm_mode_config_init(drm_dev);
    drm_dev->mode_config.min_width = 16;
    drm_dev->mode_config.min_height = 16;
    drm_dev->mode_config.max_width = 4096;
    drm_dev->mode_config.max_height = 4096;
    drm_dev->mode_config.normalize_zpos = true;
    drm_dev->mode_config.funcs = &canaan_drm_mode_config_funcs;
    drm_dev->mode_config.helper_private = &canaan_drm_mode_config_helpers;

    ret = component_bind_all(dev, drm_dev);
    if (ret) {
        DRM_DEV_ERROR(dev, "Failed to bind all components \n");
        goto cleanup_mode_config;
    }

    ret = drm_vblank_init(drm_dev, drm_dev->mode_config.num_crtc);
    if (ret) {
        DRM_DEV_ERROR(dev, "Failed to init vblank \n");
        goto unbind_all;
    }

    drm_dev->irq_enabled = true;
    drm_mode_config_reset(drm_dev);
    drm_kms_helper_poll_init(drm_dev);

    ret = drm_dev_register(drm_dev, 0);
    if (ret) {
        DRM_DEV_ERROR(dev, "Failed to register drm device \n");
        goto finish_poll;
    }

    drm_fbdev_generic_setup(drm_dev, 32);
    DRM_DEV_INFO(dev, "Canaan K230 DRM driver register successfully \n");

    return 0;

finish_poll:
    drm_kms_helper_poll_fini(drm_dev);
unbind_all:
    component_unbind_all(dev, drm_dev);
cleanup_mode_config:
    drm_mode_config_cleanup(drm_dev);
free_drm:
    dev_set_drvdata(dev, NULL);
    drm_dev_put(drm_dev);
    return ret;
}

static void canaan_drm_unbind(struct device *dev)
{
    struct drm_device *drm_dev = dev_get_drvdata(dev);

    drm_dev_unregister(drm_dev);
    drm_kms_helper_poll_fini(drm_dev);
    drm_atomic_helper_shutdown(drm_dev);
    component_unbind_all(dev, drm_dev);
    drm_mode_config_cleanup(drm_dev);
    dev_set_drvdata(dev, NULL);
    drm_dev_put(drm_dev);
}

static const struct component_master_ops canaan_drm_master_ops = {
    .bind   = canaan_drm_bind,
    .unbind = canaan_drm_unbind,
};

static struct platform_driver *const component_drivers[] = {
    &canaan_vo_driver,
    &canaan_dsi_driver,
};

static int compare_dev(struct device *dev, void *data)
{
    return dev == (struct device *)data;
}

static struct component_match *canaan_drm_match_add(struct device *dev)
{
    int i = 0;
    struct component_match *match = NULL;

    for (i = 0; i < ARRAY_SIZE(component_drivers); i++) {
        struct device_driver *driver = &component_drivers[i]->driver;
        struct device *p = NULL, *d;

        while ((d = platform_find_device_by_driver(p, driver))) {
            put_device(p);
            component_match_add(dev, &match, compare_dev, d);
            p = d;
        }
        put_device(p);
    }

    return match ?: ERR_PTR(-ENODEV);
}

static int canaan_drm_platform_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct component_match *match = NULL;

    disp_dev = dev;
    pm_runtime_enable(disp_dev);
    match = canaan_drm_match_add(dev);
    if (IS_ERR(match))
        return PTR_ERR(match);

    return component_master_add_with_match(dev, &canaan_drm_master_ops, match);
}

static int canaan_drm_platform_remove(struct platform_device *pdev)
{
    component_master_del(&pdev->dev, &canaan_drm_master_ops);

    return 0;
}

static const struct of_device_id canaan_drm_of_table[] = {
	{ .compatible = "canaan,display-subsystem", },
	{},
};
MODULE_DEVICE_TABLE(of, canaan_drm_of_table);

static struct platform_driver canaan_drm_platform_driver = {
	.probe = canaan_drm_platform_probe,
	.remove = canaan_drm_platform_remove,
	.driver = {
		.name = "canaan-drm",
		.of_match_table = canaan_drm_of_table,
	},
};

static int __init canaan_drm_init(void)
{
    int ret = 0;

    ret = platform_register_drivers(component_drivers,
                            ARRAY_SIZE(component_drivers));
    if (ret)
        return ret;

    ret = platform_driver_register(&canaan_drm_platform_driver);
    if (ret)
        platform_unregister_drivers(component_drivers,
                            ARRAY_SIZE(component_drivers));

    return ret;
}

static void __exit canaan_drm_exit(void)
{
    platform_unregister_drivers(component_drivers,
                        ARRAY_SIZE(component_drivers));
    platform_driver_unregister(&canaan_drm_platform_driver);
}

module_init(canaan_drm_init);
module_exit(canaan_drm_exit);

MODULE_DESCRIPTION("Canaan K230 DRM driver");
MODULE_LICENSE("GPL v2");
