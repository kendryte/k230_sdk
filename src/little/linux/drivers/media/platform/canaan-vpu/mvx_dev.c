/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include <linux/clk.h>
#include "mvx_bitops.h"
#include "mvx_dev.h"
#include "mvx_hwreg.h"
#include "mvx_if.h"
#include "mvx_scheduler.h"
#include "mvx_session.h"
#include "mvx_log_group.h"
#include "mvx_pm_runtime.h"

/****************************************************************************
 * Defines
 ****************************************************************************/

/**
 * Name of the MVx dev device.
 */
#define MVX_DEV_NAME    "amvx_dev"

#define MVX_PCI_VENDOR 0x13b5
#define MVX_PCI_DEVICE 0x0001

/****************************************************************************
 * Types
 ****************************************************************************/

/**
 * struct mvx_dev_ctx - Private context for the MVx dev device.
 */
struct mvx_dev_ctx {
    struct device *dev;
    struct clk* clk;
    struct mvx_if_ops *if_ops;
    struct mvx_client_ops client_ops;
    struct mvx_hwreg hwreg;
    struct mvx_sched scheduler;
    unsigned int irq;
    struct workqueue_struct *work_queue;
    struct work_struct work;
    unsigned long irqve;
    struct dentry *dentry;
};

/**
 * struct mvx_client_session - Device session.
 *
 * When the if module registers a session this structure is returned.
 */
struct mvx_client_session {
    struct mvx_dev_ctx *ctx;
    struct mvx_sched_session session;
};

/****************************************************************************
 * Static variables and functions
 ****************************************************************************/

static struct mvx_dev_ctx *client_ops_to_ctx(struct mvx_client_ops *client)
{
    return container_of(client, struct mvx_dev_ctx, client_ops);
}

static void get_hw_ver(struct mvx_client_ops *client,
               struct mvx_hw_ver *hw_ver)
{
    struct mvx_dev_ctx *ctx = client_ops_to_ctx(client);

    int ret = 0;

    ret = mvx_pm_runtime_get_sync(ctx->dev);
    if (ret < 0) {
        hw_ver->id = MVE_Unknown;
        return;
    }

    hw_ver->id = mvx_hwreg_get_hw_id(&ctx->hwreg, &hw_ver->revision,
                     &hw_ver->patch);

    ret = mvx_pm_runtime_put_sync(ctx->dev);
    if (ret < 0)
        hw_ver->id = MVE_Unknown;
}

static void get_formats(struct mvx_client_ops *client,
            enum mvx_direction direction,
            uint64_t *formats)
{
    struct mvx_dev_ctx *ctx = client_ops_to_ctx(client);
    uint32_t fuses;

    int ret = 0;

    *formats = 0;

    ret = mvx_pm_runtime_get_sync(ctx->dev);
    if (ret < 0)
        return;

    ctx->hwreg.ops.get_formats(direction, formats);

    /* Remove formats based on fuses. */
    fuses = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_FUSE);

    if (fuses & MVX_HWREG_FUSE_DISABLE_AFBC) {
        mvx_clear_bit(MVX_FORMAT_YUV420_AFBC_8, formats);
        mvx_clear_bit(MVX_FORMAT_YUV420_AFBC_10, formats);
        mvx_clear_bit(MVX_FORMAT_YUV422_AFBC_8, formats);
        mvx_clear_bit(MVX_FORMAT_YUV422_AFBC_10, formats);
    }

    if (fuses & MVX_HWREG_FUSE_DISABLE_REAL)
        mvx_clear_bit(MVX_FORMAT_RV, formats);

    if (fuses & MVX_HWREG_FUSE_DISABLE_VPX) {
        mvx_clear_bit(MVX_FORMAT_VP8, formats);
        mvx_clear_bit(MVX_FORMAT_VP9, formats);
    }

    if (fuses & MVX_HWREG_FUSE_DISABLE_HEVC)
        mvx_clear_bit(MVX_FORMAT_HEVC, formats);

    ret = mvx_pm_runtime_put_sync(ctx->dev);
    if (ret < 0)
        *formats = 0;
}

static unsigned int get_ncores(struct mvx_client_ops *client)
{
    struct mvx_dev_ctx *ctx = client_ops_to_ctx(client);
    uint32_t ncores;

    int ret = 0;

    ret = mvx_pm_runtime_get_sync(ctx->dev);
    if (ret < 0)
        return 0;

    ncores = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_NCORES);

    ret = mvx_pm_runtime_put_sync(ctx->dev);
    if (ret < 0)
        return 0;

    return ncores;
}

static struct mvx_client_session *register_session(
    struct mvx_client_ops *client,
    struct mvx_if_session *isession)
{
    struct mvx_dev_ctx *ctx = client_ops_to_ctx(client);
    struct mvx_client_session *csession;
    int ret;

    csession = devm_kzalloc(ctx->dev, sizeof(*csession), GFP_KERNEL);
    if (csession == NULL)
        return ERR_PTR(-ENOMEM);

    csession->ctx = ctx;

    ret = mvx_pm_runtime_get_sync(ctx->dev);
    if (ret < 0)
        goto free_session;

    ret = mvx_sched_session_construct(&csession->session, isession);
    if (ret != 0)
        goto runtime_put;

    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO,
              "Register client session. csession=0x%p, isession=0x%p.",
              csession, isession);

    return csession;

runtime_put:
    mvx_pm_runtime_put_sync(csession->ctx->dev);
free_session:
    devm_kfree(ctx->dev, csession);

    return ERR_PTR(ret);
}

static void unregister_session(struct mvx_client_session *csession)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO,
              "%p Unregister client session. csession=0x%p.",
              mvx_if_session_to_session(csession->session.isession),
              csession);

    mvx_sched_terminate(&csession->ctx->scheduler, &csession->session);
    mvx_sched_session_destruct(&csession->session);

    mvx_pm_runtime_put_sync(csession->ctx->dev);

    devm_kfree(csession->ctx->dev, csession);
}

static int switch_in(struct mvx_client_session *csession)
{
    struct mvx_dev_ctx *ctx = csession->ctx;
    int ret;

    ret = mvx_sched_switch_in(&ctx->scheduler, &csession->session);

    return ret;
}

static int send_irq(struct mvx_client_session *csession)
{
    struct mvx_dev_ctx *ctx = csession->ctx;
    int ret;

    ret = mvx_sched_send_irq(&ctx->scheduler, &csession->session);

    return ret;
}

static int flush_mmu(struct mvx_client_session *csession)
{
    struct mvx_dev_ctx *ctx = csession->ctx;
    int ret;

    ret = mvx_sched_flush_mmu(&ctx->scheduler, &csession->session);

    return ret;
}

static void print_debug(struct mvx_client_session *csession)
{
    struct mvx_dev_ctx *ctx = csession->ctx;

    mvx_sched_print_debug(&ctx->scheduler, &csession->session);
}

static struct mvx_dev_ctx *work_to_ctx(struct work_struct *work)
{
    return container_of(work, struct mvx_dev_ctx, work);
}

/**
 * irq_bottom() - Handle IRQ bottom.
 * @work:    Work struct that is part of the context structure.
 *
 * This function is called from a work queue and id doing the actual work of
 * handling the interrupt.
 */
static void irq_bottom(struct work_struct *work)
{
    struct mvx_dev_ctx *ctx = work_to_ctx(work);
    uint32_t nlsid;
    uint32_t i;

    nlsid = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_NLSID);
    for (i = 0; i < nlsid; i++)
        if (test_and_clear_bit(i, &ctx->irqve))
            mvx_sched_handle_irq(&ctx->scheduler, i);
}

/**
 * irq_top() - Handle IRQ top.
 * @irq:    IRQ number.
 * @dev_id:    Pointer to context.
 *
 * This function is called in interrupt context. It should be short and must not
 * block.
 *
 * Return: IRQ status if the IRQ was handled or not.
 */
static irqreturn_t irq_top(int irq,
               void *dev_id)
{
    struct mvx_dev_ctx *ctx = dev_id;
    uint32_t nlsid;
    uint32_t irqve;
    int ret = IRQ_NONE;

    nlsid = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_NLSID);
    irqve = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_IRQVE);
    while (nlsid-- > 0)
        if ((irqve >> nlsid) & 0x1) {
            mvx_hwreg_write_lsid(&ctx->hwreg,
                         nlsid,
                         MVX_HWREG_LIRQVE,
                         0);
            set_bit(nlsid, &ctx->irqve);
            ret = IRQ_HANDLED;
        }

    queue_work(ctx->work_queue, &ctx->work);

    return ret;
}

static int mvx_dev_probe(struct device *dev,
             struct resource *iores,
             struct resource *irqres)
{
    struct mvx_dev_ctx *ctx;
    int ret;

    /* Create device context and store pointer in device private data. */
    ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
    if (ctx == NULL)
        return -EINVAL;

    ctx->dev = dev;

    dev_set_drvdata(dev, ctx);

    ret = mvx_pm_runtime_get_sync(ctx->dev);
    if (ret < 0)
        goto free_ctx;

#ifdef ENABLE_PM_CLK
    ctx->clk = devm_clk_get(dev, "vpu");
    clk_prepare_enable(ctx->clk);
#endif

    /* Setup client ops callbacks. */
    ctx->client_ops.get_hw_ver = get_hw_ver;
    ctx->client_ops.get_formats = get_formats;
    ctx->client_ops.get_ncores = get_ncores;
    ctx->client_ops.register_session = register_session;
    ctx->client_ops.unregister_session = unregister_session;
    ctx->client_ops.switch_in = switch_in;
    ctx->client_ops.send_irq = send_irq;
    ctx->client_ops.flush_mmu = flush_mmu;
    ctx->client_ops.print_debug = print_debug;

    /* Create if context. */
    ctx->if_ops = mvx_if_create(dev, &ctx->client_ops, ctx);
    if (IS_ERR(ctx->if_ops))
        goto runtime_put;

    /* Create debugfs entry */
    if (IS_ENABLED(CONFIG_DEBUG_FS)) {
        char name[20];

        scnprintf(name, sizeof(name), "%s%u", MVX_DEV_NAME, dev->id);
        ctx->dentry = debugfs_create_dir(name, NULL);
        if (IS_ERR_OR_NULL(ctx->dentry)) {
            ret = -EINVAL;
            goto destroy_if;
        }
    }

    /* Construct hw register context. */
    ret = mvx_hwreg_construct(&ctx->hwreg, dev, iores, ctx->dentry);
    if (ret != 0)
        goto destruct_dentry;

    ret = mvx_sched_construct(&ctx->scheduler, dev, ctx->if_ops,
                  &ctx->hwreg, ctx->dentry);
    if (ret != 0)
        goto destruct_hwreg;

    /* Create work queue for IRQ handler. */
    ctx->work_queue = alloc_workqueue(dev_name(dev), WQ_UNBOUND, 1);
    if (ctx->work_queue == NULL) {
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_ERROR,
                  "Failed to create work queue.");
        ret = -EINVAL;
        goto destruct_sched;
    }

    INIT_WORK(&ctx->work, irq_bottom);

    /* Request IRQ handler. */
    ctx->irq = irqres->start;
    ret = request_irq(ctx->irq, irq_top,
              IRQF_SHARED | (irqres->flags & IRQF_TRIGGER_MASK),
              dev_name(dev), ctx);
    if (ret != 0) {
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_ERROR,
                  "Failed to request IRQ. irq=%u, ret=%d.",
                  ctx->irq,
                  ret);
        goto workqueue_destroy;
    }

    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_WARNING,
              "Linlon v%x identified. cores=%u, nlsid=%u, id=%u.",
              mvx_hwreg_get_hw_id(&ctx->hwreg, NULL, NULL),
              mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_NCORES),
              mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_NLSID),
              dev->id);

    ret = mvx_pm_runtime_put_sync(ctx->dev);
    if (ret < 0)
        goto irq_free;

    return 0;

irq_free:
    free_irq(ctx->irq, ctx);

workqueue_destroy:
    destroy_workqueue(ctx->work_queue);

destruct_sched:
    mvx_sched_destruct(&ctx->scheduler);

destruct_hwreg:
    mvx_hwreg_destruct(&ctx->hwreg);

destruct_dentry:
    if (IS_ENABLED(CONFIG_DEBUG_FS))
        debugfs_remove_recursive(ctx->dentry);

destroy_if:
    mvx_if_destroy(ctx->if_ops);

runtime_put:
    pm_runtime_put_sync(ctx->dev);

free_ctx:
    devm_kfree(dev, ctx);

    return ret;
}

static int mvx_dev_remove(struct mvx_dev_ctx *ctx)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "remove");

    mvx_if_destroy(ctx->if_ops);
    free_irq(ctx->irq, ctx);
    destroy_workqueue(ctx->work_queue);
    mvx_sched_destruct(&ctx->scheduler);
    mvx_hwreg_destruct(&ctx->hwreg);
    dev_set_drvdata(ctx->dev, NULL);

    if (IS_ENABLED(CONFIG_DEBUG_FS))
        debugfs_remove_recursive(ctx->dentry);
#ifdef ENABLE_PM_CLK
    clk_disable_unprepare(ctx->clk);
#endif
    devm_kfree(ctx->dev, ctx);

    return 0;
}

/****************************************************************************
 * Platform driver
 ****************************************************************************/

static int mvx_pdev_probe(struct platform_device *pdev)
{
    struct resource iores;
    struct resource irqres;
    unsigned int irq;
    int ret;

    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "probe");

    /* Get resource. */
    ret = of_address_to_resource(pdev->dev.of_node, 0, &iores);
    if (ret != 0) {
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_ERROR,
                  "Failed to get address of resource. ret=%d.",
                  ret);
        return ret;
    }

    /* Get IRQ resource. */
    irq = of_irq_to_resource(pdev->dev.of_node, 0, &irqres);
    if (irq == 0) {
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_ERROR,
                  "Failed to get IRQ resource.");
        return -EINVAL;
    }

#ifdef ENABLE_PM_CLK
    pm_runtime_enable(&pdev->dev);
#endif

    ret = mvx_dev_probe(&pdev->dev, &iores, &irqres);

#ifdef ENABLE_PM_CLK
    if (ret != 0)
        pm_runtime_disable(&pdev->dev);
#endif
    return ret;
}

static int mvx_pdev_remove(struct platform_device *pdev)
{
    struct mvx_dev_ctx *ctx = platform_get_drvdata(pdev);
    int ret;

    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "pdev remove");

    ret = mvx_dev_remove(ctx);

#ifdef ENABLE_PM_CLK
    pm_runtime_disable(&pdev->dev);
#endif
    return ret;
}

#ifdef CONFIG_PM

static int mvx_pm_suspend(struct device *dev)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "mvx_pm_suspend");
    return 0;
}

static int mvx_pm_resume(struct device *dev)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "mvx_pm_resume");
    return 0;
}

static int mvx_pm_runtime_suspend(struct device *dev)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "mvx_pm_runtime_suspend");
    return 0;
}

static int mvx_pm_runtime_resume(struct device *dev)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "mvx_pm_runtime_resume");
    return 0;
}

static int mvx_pm_runtime_idle(struct device *dev)
{
    MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_INFO, "mvx_pm_runtime_idle");
    return 0;
}

static const struct dev_pm_ops mvx_dev_pm_ops = {
    .suspend         = mvx_pm_suspend,
    .resume          = mvx_pm_resume,
    .runtime_suspend = mvx_pm_runtime_suspend,
    .runtime_resume  = mvx_pm_runtime_resume,
    .runtime_idle    = mvx_pm_runtime_idle,
};
#endif /* CONFIG_PM */

static const struct of_device_id mvx_dev_match_table[] = {
    { .compatible = "arm,mali-mve"  },
    { .compatible = "arm,mali-v500" },
    { .compatible = "arm,mali-v550" },
    { .compatible = "arm,mali-v61"  },
    { .compatible = "armChina,linlon-v5"  },
    { .compatible = "armChina,linlon-v6"  },
    { .compatible = "armChina,linlon-v7"  },
    { .compatible = "armChina,linlon-v8"  },
    { { 0 } }
};

static struct platform_driver mvx_dev_driver = {
    .probe          = mvx_pdev_probe,
    .remove         = mvx_pdev_remove,
    .driver         = {
    .name           = MVX_DEV_NAME,
    .owner          = THIS_MODULE,
    .of_match_table = mvx_dev_match_table,
#ifdef CONFIG_PM
    .pm             = &mvx_dev_pm_ops
#endif /* CONFIG_PM */
    }
};

/****************************************************************************
 * PCI driver
 ****************************************************************************/

/* LCOV_EXCL_START */
static int mvx_pci_probe(struct pci_dev *pdev,
             const struct pci_device_id *id)
{
    static unsigned int dev_id;
    struct resource irqres = {
        .start = pdev->irq,
        .end   = pdev->irq,
        .flags = 0
    };
    pdev->dev.id = dev_id++;
    return mvx_dev_probe(&pdev->dev, &pdev->resource[1], &irqres);
}

static void mvx_pci_remove(struct pci_dev *pdev)
{
    struct mvx_dev_ctx *ctx = pci_get_drvdata(pdev);

    mvx_dev_remove(ctx);
}

static struct pci_device_id mvx_pci_device_id[] = {
    { PCI_DEVICE(MVX_PCI_VENDOR,
             MVX_PCI_DEVICE) },
    { 0, }
};

MODULE_DEVICE_TABLE(pci, mvx_pci_device_id);

static struct pci_driver mvx_pci_driver = {
    .name     = MVX_DEV_NAME,
    .id_table = mvx_pci_device_id,
    .probe    = mvx_pci_probe,
    .remove   = mvx_pci_remove
};
/* LCOV_EXCL_STOP */

/****************************************************************************
 * Exported variables and functions
 ****************************************************************************/

int mvx_dev_init(void)
{
    int ret;

    ret = platform_driver_register(&mvx_dev_driver);
    if (ret != 0) {
        pr_err("mvx_dev: Failed to register driver.\n");
        return ret;
    }

    /* LCOV_EXCL_START */
    ret = pci_register_driver(&mvx_pci_driver);
    if (ret != 0) {
        pr_err("mvx_dev: Failed to register PCI driver.\n");
        goto unregister_driver;
    }

    /* LCOV_EXCL_STOP */

    return 0;

unregister_driver:
    platform_driver_unregister(&mvx_dev_driver); /* LCOV_EXCL_LINE */

    return ret;
}

void mvx_dev_exit(void)
{
    pci_unregister_driver(&mvx_pci_driver); /* LCOV_EXCL_LINE */
    platform_driver_unregister(&mvx_dev_driver);
}
