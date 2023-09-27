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
#include <drm/drm_fourcc.h>
#include <drm/drm_print.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_plane.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_probe_helper.h>

#include "canaan_vo.h"
#include "canaan_vo_regs.h"
#include "canaan_crtc.h"
#include "canaan_plane.h"

static void canaan_vo_update_video(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_display_mode *adj_mode)
{
    uint32_t reg_val = 0x00;
    struct drm_plane_state *plane_state = canaan_plane->base.state;
    struct drm_framebuffer *fb = plane_state->fb;
    struct drm_gem_cma_object *cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
    struct canaan_plane_config *config = canaan_plane->config;
    uint32_t plane_offset = config->plane_offset;
    uint32_t plane_enable_bit = config->plane_enable_bit;
    uint32_t xctl_reg_offset = config->xctl_reg_offset;
    uint32_t yctl_reg_offset = config->yctl_reg_offset;
    uint32_t actual_w, start_w, offset_w;
    uint32_t actual_h, start_h, offset_h;
    uint32_t y_addr, uv_addr;
    uint32_t stride = 0x00;
    uint32_t disp_en = 0x00;

    switch (fb->format->format) {
    case DRM_FORMAT_NV12:
        reg_val = 1 + (1 << 8) + (1 << 12) + (3 << 16) + (0 << 28);
        break;
    case DRM_FORMAT_NV21:
        reg_val = 1 + (1 << 8) + (1 << 12) + (2 << 16) + (0 << 28);
        break;
    case DRM_FORMAT_NV16:
        reg_val = 1 + (1 << 4) + (1 << 12) + (3 << 16) + (0 << 28);
        break;
    case DRM_FORMAT_NV61:
        reg_val = 1 + (1 << 4) + (1 << 12) + (2 << 16) + (0 << 28);
        break;
    default:
        DRM_DEV_ERROR(vo->dev, "Invalid pixel format %d\n", fb->format->format);
        return;
    }
    writel(reg_val, vo->reg_base + plane_offset + VO_LAYER2_3_CTL_REG_OFFSET);

    actual_w = plane_state->src_w >> 16;
    actual_h = plane_state->src_h >> 16;
    reg_val = (actual_w - 1) | (actual_h << 16);
    writel(reg_val, vo->reg_base + plane_offset + VO_LAYER2_3_ACT_SIZE_REG_OFFSET);

    offset_w = plane_state->crtc_x;
    start_w = adj_mode->crtc_htotal - adj_mode->crtc_hsync_start + 5 + 1;
    reg_val = ((start_w + offset_w + actual_w -1) << 16) + (start_w + offset_w);
    writel(reg_val, vo->reg_base + xctl_reg_offset);

    offset_h = plane_state->crtc_y;
    start_h = adj_mode->crtc_vtotal - adj_mode->crtc_vsync_end + 1;
    reg_val = ((start_h + offset_h + actual_h -1) << 16) + (start_h + offset_h);
    writel(reg_val, vo->reg_base + yctl_reg_offset);

    y_addr = cma_obj->paddr;
    writel(y_addr, vo->reg_base + plane_offset + VO_LAYER2_3_Y_ADDR0_REG_OFFSET);
    writel(y_addr, vo->reg_base + plane_offset + VO_LAYER2_3_Y_ADDR1_REG_OFFSET);

    uv_addr = cma_obj->paddr + fb->offsets[1];
    writel(uv_addr, vo->reg_base + plane_offset + VO_LAYER2_3_UV_ADDR0_REG_OFFSET);
    writel(uv_addr, vo->reg_base + plane_offset + VO_LAYER2_3_UV_ADDR1_REG_OFFSET);

    stride = (actual_w/8 - 1) | (actual_h << 16);
    writel(stride, vo->reg_base + plane_offset + VO_LAYER2_3_STRIDE_REG_OFFSET);

    writel(0x00, vo->reg_base + plane_offset + VO_LAYER2_3_IMG_IN_OFFSET_REG_OFFSET);
    writel(0x0f, vo->reg_base + plane_offset + VO_LAYER2_3_BLENTH_REG_OFFSET);
    writel(0x11, vo->reg_base + plane_offset + VO_LAYER2_3_ADDR_SEL_MODE_REG_OFFSET);

    disp_en = readl(vo->reg_base + VO_DISP_ENABLE);
    disp_en |= 1 << plane_enable_bit;
    writel(disp_en, vo->reg_base + VO_DISP_ENABLE);

    DRM_DEBUG_DRIVER("VIDEO_CTL_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_LAYER2_3_CTL_REG_OFFSET));
    DRM_DEBUG_DRIVER("VIDEO_ACT_SIZE_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_LAYER2_3_ACT_SIZE_REG_OFFSET));
    DRM_DEBUG_DRIVER("VIDEO_Y_ADDR0_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_LAYER2_3_Y_ADDR0_REG_OFFSET));
    DRM_DEBUG_DRIVER("VIDEO_UV_ADDR0_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_LAYER2_3_UV_ADDR0_REG_OFFSET));
    DRM_DEBUG_DRIVER("VIDEO_STRIDE_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_LAYER2_3_STRIDE_REG_OFFSET));
}

static void canaan_vo_update_osd(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_display_mode *adj_mode)
{
    uint32_t reg_val = 0x00;
    struct drm_plane_state *plane_state = canaan_plane->base.state;
    struct drm_framebuffer *fb = plane_state->fb;
    struct drm_gem_cma_object *cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
    struct canaan_plane_config *config = canaan_plane->config;
    uint32_t plane_offset = config->plane_offset;
    uint32_t plane_enable_bit = config->plane_enable_bit;
    uint32_t xctl_reg_offset = config->xctl_reg_offset;
    uint32_t yctl_reg_offset = config->yctl_reg_offset;
    uint32_t actual_w, start_w, offset_w;
    uint32_t actual_h, start_h, offset_h;
    dma_addr_t paddr = 0x00;
    uint32_t stride = 0x00;
    uint32_t disp_en = 0x00;

    switch (fb->format->format) {
    case DRM_FORMAT_ARGB8888:
        reg_val = 0x53;
        break;
    case DRM_FORMAT_ARGB4444:
        reg_val = 0x54;
        break;
    case DRM_FORMAT_ARGB1555:
        reg_val = 0x55;
        break;
    case DRM_FORMAT_RGB888:
        reg_val = 0x00;
        break;
    case DRM_FORMAT_RGB565:
        reg_val = 0x02;
        break;
    default:
        DRM_DEV_ERROR(vo->dev, "Invalid pixel format %d\n", fb->format->format);
        return;
    }
    writel(reg_val, vo->reg_base + plane_offset + VO_OSD0_7_INFO_REG_OFFSET);

    actual_w = plane_state->src_w >> 16;
    actual_h = plane_state->src_h >> 16;
    reg_val = actual_w | actual_h << 16;
    writel(reg_val, vo->reg_base + plane_offset + VO_OSD0_7_SIZE_REG_OFFSET);

    offset_w = plane_state->crtc_x;
    start_w = adj_mode->crtc_htotal - adj_mode->crtc_hsync_start + 5 + 1;
    reg_val = ((start_w + offset_w + actual_w -1) << 16) + (start_w + offset_w);
    writel(reg_val, vo->reg_base + xctl_reg_offset);

    offset_h = plane_state->crtc_y;
    start_h = adj_mode->crtc_vtotal - adj_mode->crtc_vsync_end + 1;
    reg_val = ((start_h + offset_h + actual_h -1) << 16) + (start_h + offset_h);
    writel(reg_val, vo->reg_base + yctl_reg_offset);

    paddr = cma_obj->paddr;
    writel(paddr, vo->reg_base + plane_offset + VO_OSD0_7_VLU_ADDR0_REG_OFFSET);
    writel(paddr, vo->reg_base + plane_offset + VO_OSD0_7_ALP_ADDR0_REG_OFFSET);
    writel(paddr, vo->reg_base + plane_offset + VO_OSD0_7_VLU_ADDR1_REG_OFFSET);
    writel(paddr, vo->reg_base + plane_offset + VO_OSD0_7_ALP_ADDR1_REG_OFFSET);

    stride = fb->pitches[0]/8;
    writel(stride, vo->reg_base + plane_offset + VO_OSD0_7_STRIDE_REG_OFFSET);

    writel(0x4F, vo->reg_base + plane_offset + VO_OSD0_7_DMA_CTRL_REG_OFFSET);
    writel(0x100, vo->reg_base + plane_offset + VO_OSD0_7_ADDR_SEL_MODE_REG_OFFSET);

    disp_en = readl(vo->reg_base + VO_DISP_ENABLE);
    disp_en |= 1 << plane_enable_bit;
    writel(disp_en, vo->reg_base + VO_DISP_ENABLE);

    DRM_DEBUG_DRIVER("OSD_INFO_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_OSD0_7_INFO_REG_OFFSET));
    DRM_DEBUG_DRIVER("OSD_SIZE_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_OSD0_7_SIZE_REG_OFFSET));
    DRM_DEBUG_DRIVER("OSD_VLU_ADDR0_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_OSD0_7_VLU_ADDR0_REG_OFFSET));
    DRM_DEBUG_DRIVER("OSD_ALP_ADDR0_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_OSD0_7_ALP_ADDR0_REG_OFFSET));
    DRM_DEBUG_DRIVER("OSD_STRIDE_REG: 0x%02x\n", readl(vo->reg_base + plane_offset + VO_OSD0_7_STRIDE_REG_OFFSET));
}

int canaan_vo_check_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_plane_state *plane_state)
{
    int ret = 0;
    struct drm_crtc_state *crtc_state = NULL;

    crtc_state = drm_atomic_get_crtc_state(plane_state->state, plane_state->crtc);
    if (IS_ERR(crtc_state)) {
        DRM_DEV_ERROR(vo->dev, "Failed to get crtc_state \n");
        return PTR_ERR(crtc_state);
    }

    ret = drm_atomic_helper_check_plane_state(plane_state, crtc_state,
                                        DRM_PLANE_HELPER_NO_SCALING,
                                        DRM_PLANE_HELPER_NO_SCALING,
                                        true, true);
    if (ret) {
        DRM_DEV_ERROR(vo->dev, "Failed to check plane_state \n");
        return ret;
    }

    return 0;
}

void canaan_vo_update_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane,
                    struct drm_plane_state *plane_state)
{
    struct drm_crtc_state *crtc_state = plane_state->crtc->state;
    struct drm_display_mode *adj_mode = &crtc_state->adjusted_mode;

    if (vo->canaan_plane[0]->id == canaan_plane->id) {
        canaan_vo_update_video(vo, canaan_plane, adj_mode);
    } else {
        canaan_vo_update_osd(vo, canaan_plane, adj_mode);
    }
}

void canaan_vo_disable_plane(struct canaan_vo *vo, struct canaan_plane *canaan_plane)
{
    struct canaan_plane_config *config = canaan_plane->config;
    uint32_t plane_enable_bit = config->plane_enable_bit;
    uint32_t disp_en = 0x00;

    disp_en = readl(vo->reg_base + VO_DISP_ENABLE);
    disp_en &= ~(1 << plane_enable_bit);
    writel(disp_en, vo->reg_base + VO_DISP_ENABLE);
}

static irqreturn_t canaan_vo_irq_handler(int irq, void *dev_id)
{
    struct canaan_vo *vo = dev_id;
    struct canaan_crtc *canaan_crtc = vo->canaan_crtc;

    if (atomic_read(&vo->vsync_enabled)) {
        drm_crtc_handle_vblank(&canaan_crtc->base);
    }

    return IRQ_HANDLED;
}

int canaan_vo_enable_vblank(struct canaan_vo *vo)
{
    atomic_set(&vo->vsync_enabled, 1);

    return 0;
}

void canaan_vo_disable_vblank(struct canaan_vo *vo)
{
    atomic_set(&vo->vsync_enabled, 0);
}

void canaan_vo_enable_crtc(struct canaan_vo *vo, struct canaan_crtc *canaan_crtc)
{

}

void canaan_vo_disable_crtc(struct canaan_vo *vo, struct canaan_crtc *canaan_crtc)
{

}

void canaan_vo_flush_config(struct canaan_vo *vo)
{
    writel(0x11, vo->reg_base + VO_REG_LOAD_CTL);
}

static const uint32_t video_plane_formats[] = {
    DRM_FORMAT_NV12,
    DRM_FORMAT_NV21,
    DRM_FORMAT_NV16,
    DRM_FORMAT_NV61,
};

static const uint32_t osd_plane_formats[] = {
    DRM_FORMAT_ARGB8888,
    DRM_FORMAT_ARGB4444,
    DRM_FORMAT_ARGB1555,
    DRM_FORMAT_RGB888,
    DRM_FORMAT_RGB565,
};

static const struct canaan_plane_config canaan_plane_configurations[CANAAN_PLANE_NUMBER] = {
    {
        .id = 0,
        .name = "video_3",
        .formats = video_plane_formats,
        .num_formats = ARRAY_SIZE(video_plane_formats),
        .plane_type = DRM_PLANE_TYPE_OVERLAY,
        .plane_offset = VO_LAYER3_OFFSET,
        .plane_enable_bit = 3,
        .xctl_reg_offset = VO_DISP_LAYER3_XCTL,
        .yctl_reg_offset = VO_DISP_LAYER3_YCTL,
    },
    {
        .id = 1,
        .name = "OSD4",
        .formats = osd_plane_formats,
        .num_formats = ARRAY_SIZE(osd_plane_formats),
        .plane_type = DRM_PLANE_TYPE_PRIMARY,
        .plane_offset = VO_OSD4_OFFSET,
        .plane_enable_bit = 8,
        .xctl_reg_offset = VO_DISP_OSD4_XCTL,
        .yctl_reg_offset = VO_DISP_OSD4_YCTL,
    },
    {
        .id = 2,
        .name = "OSD5",
        .formats = osd_plane_formats,
        .num_formats = ARRAY_SIZE(osd_plane_formats),
        .plane_type = DRM_PLANE_TYPE_CURSOR,
        .plane_offset = VO_OSD5_OFFSET,
        .plane_enable_bit = 9,
        .xctl_reg_offset = VO_DISP_OSD5_XCTL,
        .yctl_reg_offset = VO_DISP_OSD5_YCTL,
    },
    {
        .id = 3,
        .name = "OSD6",
        .formats = osd_plane_formats,
        .num_formats = ARRAY_SIZE(osd_plane_formats),
        .plane_type = DRM_PLANE_TYPE_OVERLAY,
        .plane_offset = VO_OSD6_OFFSET,
        .plane_enable_bit = 10,
        .xctl_reg_offset = VO_DISP_OSD6_XCTL,
        .yctl_reg_offset = VO_DISP_OSD6_YCTL,
    },
    {
        .id = 4,
        .name = "OSD7",
        .formats = osd_plane_formats,
        .num_formats = ARRAY_SIZE(osd_plane_formats),
        .plane_type = DRM_PLANE_TYPE_OVERLAY,
        .plane_offset = VO_OSD7_OFFSET,
        .plane_enable_bit = 11,
        .xctl_reg_offset = VO_DISP_OSD7_XCTL,
        .yctl_reg_offset = VO_DISP_OSD7_YCTL,
    },
};

static int canaan_vo_bind(struct device *dev, struct device *master, void *data)
{
    int i = 0;
    int ret = 0;
    struct canaan_vo *vo = NULL;
    struct drm_device *drm_dev = data;
    struct platform_device *pdev = to_platform_device(dev);
    struct resource *res = NULL;
    struct canaan_plane_config *config;

    vo = devm_kzalloc(dev, sizeof(*vo), GFP_KERNEL);
    if (!vo)
        return -ENOMEM;
    vo->dev     = dev;
    vo->drm_dev = drm_dev;
    dev_set_drvdata(dev, vo);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        DRM_DEV_ERROR(dev, "Failed to get register resource \n");
        return -ENXIO;
    }

    vo->reg_base = devm_ioremap_resource(dev, res);
    if (IS_ERR(vo->reg_base)) {
        DRM_DEV_ERROR(dev, "Failed to map register resource \n");
        return PTR_ERR(vo->reg_base);
    }

    atomic_set(&vo->vsync_enabled, 0);
    vo->irq = platform_get_irq(pdev, 0);
    if (vo->irq < 0) {
        DRM_DEV_ERROR(dev, "Failed to get vsync irq \n");
        return vo->irq;
    }

    ret = devm_request_irq(dev, vo->irq, canaan_vo_irq_handler,
                    0, dev_name(dev), vo);
    if (ret) {
        DRM_DEV_ERROR(dev, "Failed to request vsync irq \n");
        return ret;
    }

    for (i = 0; i < CANAAN_PLANE_NUMBER; i ++) {
        config = &canaan_plane_configurations[i];
        config->possible_crtcs = 1 << drm_dev->mode_config.num_crtc;
        vo->canaan_plane[i] = canaan_plane_create(drm_dev, config, vo);
        if (IS_ERR(vo->canaan_plane[i])) {
            DRM_DEV_ERROR(dev, "Failed to create canaan_plane \n");
            return PTR_ERR(vo->canaan_plane[i]);
        }
    }

    struct drm_plane *primary   = &vo->canaan_plane[1]->base;
    struct drm_plane *cursor    = &vo->canaan_plane[2]->base;
    vo->canaan_crtc = canaan_crtc_create(drm_dev, primary, cursor, vo);
    if (IS_ERR(vo->canaan_crtc)) {
        DRM_DEV_ERROR(dev, "Failed to create canaan_crtc \n");
        return PTR_ERR(vo->canaan_crtc);
    }

    return 0;
}

static void canaan_vo_unbind(struct device *dev, struct device *master, void *data)
{

}

static const struct component_ops canaan_vo_component_ops = {
    .bind   = canaan_vo_bind,
    .unbind = canaan_vo_unbind,
};

static int canaan_vo_probe(struct platform_device *pdev)
{
    return component_add(&pdev->dev, &canaan_vo_component_ops);
}

static int canaan_vo_remove(struct platform_device *pdev)
{
    component_del(&pdev->dev, &canaan_vo_component_ops);

    return 0;
}

static const struct of_device_id canaan_vo_of_table[] = {
	{ .compatible = "canaan,k230-vo", },
	{},
};
MODULE_DEVICE_TABLE(of, canaan_vo_dt_match);

struct platform_driver canaan_vo_driver = {
	.probe = canaan_vo_probe,
	.remove = canaan_vo_remove,
	.driver = {
		.name = "canaan-vo",
		.of_match_table = of_match_ptr(canaan_vo_of_table),
	},
};

MODULE_DESCRIPTION("Canaan K230 VO Controller");
MODULE_LICENSE("GPL v2");
