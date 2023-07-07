// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022, Canaan Bright Sight Co., Ltd
 *
 * All enquiries to https://www.canaan-creative.com/
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>


struct canaan_panel_instr {
	char cmd;
	char data;
};

struct canaan_panel_desc {
	const struct canaan_panel_instr *init;
	const size_t init_length;
	//const struct drm_display_mode *mode;
};

struct canaan_panel {
	struct drm_panel	panel;
	struct mipi_dsi_device	*dsi;
	const struct canaan_panel_desc	*desc;

	struct regulator	*power;
	struct gpio_desc	*reset;
	struct videomode vm;
	u32 width_mm;
	u32 height_mm;
};


static const struct canaan_panel_instr hx8399_init[] = {
	
};


static inline struct canaan_panel *panel_to_canaan_panel(struct drm_panel *panel)
{
	return container_of(panel, struct canaan_panel, panel);
}

static int canaan_panel_prepare(struct drm_panel *panel)
{
	return 0;
}

static int canaan_panel_enable(struct drm_panel *panel)
{
	return 0;
}

static int canaan_panel_disable(struct drm_panel *panel)
{
	return 0;
}

static int canaan_panel_unprepare(struct drm_panel *panel)
{
	return 0;
}

static int canaan_panel_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct canaan_panel *ctx = panel_to_canaan_panel(panel);
	struct drm_display_mode *mode;

	mode = drm_mode_create(connector->dev);
	if (!mode) {
		dev_err(panel->dev, "failed to create a new display mode\n");
		return 0;
	}

	drm_display_mode_from_videomode(&ctx->vm, mode);
	mode->width_mm = ctx->width_mm;
	mode->height_mm = ctx->height_mm;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs canaan_panel_funcs = {
	.prepare	= canaan_panel_prepare,
	.unprepare	= canaan_panel_unprepare,
	.enable		= canaan_panel_enable,
	.disable	= canaan_panel_disable,
	.get_modes	= canaan_panel_get_modes,
};

static int canaan_panel_parse_dt(struct canaan_panel *ctx)
{
	struct device *dev = &ctx->dsi->dev;
	struct device_node *np = dev->of_node;
	int ret;

	ret = of_get_videomode(np, &ctx->vm, 0);
	if (ret < 0)
		return ret;

	of_property_read_u32(np, "panel-width-mm", &ctx->width_mm);
	of_property_read_u32(np, "panel-height-mm", &ctx->height_mm);

	return 0;
}

static int canaan_panel_dsi_probe(struct mipi_dsi_device *dsi)
{
	struct canaan_panel *ctx;
	int ret;

	ctx = devm_kzalloc(&dsi->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;
	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dsi = dsi;
	ctx->desc = of_device_get_match_data(&dsi->dev);

	ret = canaan_panel_parse_dt(ctx);
	if (ret < 0)
		return ret;

	// Panel Device Tree Read..

	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_SYNC_PULSE;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->lanes = 4;

	drm_panel_init(&ctx->panel, &dsi->dev, &canaan_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret) {
		drm_panel_remove(&ctx->panel);
	}

	return ret;
}

static int canaan_panel_dsi_remove(struct mipi_dsi_device *dsi)
{
	struct canaan_panel *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct canaan_panel_desc hx8399_desc = {
	// .init = hx8399_init,
	// .init_length = ARRAY_SIZE(hx8399_init),
	// .mode = &hx8399_default_mode,
};


static const struct of_device_id canaan_panel_of_match[] = {
	{ .compatible = "canaan,hx8399", .data = &hx8399_desc },
	{ }
};
MODULE_DEVICE_TABLE(of, canaan_panel_of_match);

static struct mipi_dsi_driver canaan_panel_driver = {
	.probe		= canaan_panel_dsi_probe,
	.remove		= canaan_panel_dsi_remove,
	.driver = {
		.name		= "canaan-panel-dsi",
		.of_match_table	= canaan_panel_of_match,
	},
};
module_mipi_dsi_driver(canaan_panel_driver);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Canaan K230 Panel Driver");
MODULE_LICENSE("GPL");
