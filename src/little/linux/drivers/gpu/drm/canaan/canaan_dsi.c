/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/clk.h>
#include <linux/component.h>
#include <linux/crc-ccitt.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy-mipi-dphy.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/slab.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_print.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include "canaan_dsi.h"
#include <video/mipi_display.h>


static void canaan_dsi_inst_abort(struct canaan_dsi *dsi)
{
	// TODO
}

static int canaan_dsi_inst_wait_for_completion(struct canaan_dsi *dsi)
{
	// TODO
	return 0;
}
static int canaan_dsi_dcs_write_short(struct canaan_dsi *dsi,
				     const struct mipi_dsi_msg *msg)
{
	// TODO
	return msg->tx_len;
}

static int canaan_dsi_dcs_write_long(struct canaan_dsi *dsi,
				    const struct mipi_dsi_msg *msg)
{
	// TODO
	return msg->tx_len;
}

static int canaan_dsi_dcs_read(struct canaan_dsi *dsi,
			      const struct mipi_dsi_msg *msg)
{
	// TODO
	return 1;
}

static void canaan_dsi_encoder_enable(struct drm_encoder *encoder)
{
	struct canaan_dsi *dsi = encoder_to_canaan_dsi(encoder);
	int err;

	DRM_DEBUG_DRIVER("Enabling DSI output\n");

	/*
	 * Enable the DSI block.
	 */

	if (dsi->panel)
		drm_panel_prepare(dsi->panel);

	/*
	 * FIXME: This should be moved after the switch to HS mode.
	 *
	 * Unfortunately, once in HS mode, it seems like we're not
	 * able to send DCS commands anymore, which would prevent any
	 * panel to send any DCS command as part as their enable
	 * method, which is quite common.
	 *
	 * I haven't seen any artifact due to that sub-optimal
	 * ordering on the panels I've tested it with, so I guess this
	 * will do for now, until that IP is better understood.
	 */
	if (dsi->panel)
		drm_panel_enable(dsi->panel);

}

static void canaan_dsi_encoder_disable(struct drm_encoder *encoder)
{
	struct canaan_dsi *dsi = encoder_to_canaan_dsi(encoder);

	DRM_DEBUG_DRIVER("Disabling DSI output\n");

	if (dsi->panel) {
		drm_panel_disable(dsi->panel);
		drm_panel_unprepare(dsi->panel);
	}
}

static int canaan_dsi_get_modes(struct drm_connector *connector)
{
	struct canaan_dsi *dsi = connector_to_canaan_dsi(connector);

	return drm_panel_get_modes(dsi->panel, connector);
}

static const struct drm_connector_helper_funcs canaan_dsi_connector_helper_funcs = {
	.get_modes	= canaan_dsi_get_modes,
};

static enum drm_connector_status
canaan_dsi_connector_detect(struct drm_connector *connector, bool force)
{
	struct canaan_dsi *dsi = connector_to_canaan_dsi(connector);

	return dsi->panel ? connector_status_connected :
			    connector_status_disconnected;
}

static const struct drm_connector_funcs canaan_dsi_connector_funcs = {
	.detect			= canaan_dsi_connector_detect,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= drm_connector_cleanup,
	.reset			= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
};

static const struct drm_encoder_helper_funcs canaan_dsi_enc_helper_funcs = {
	.disable	= canaan_dsi_encoder_disable,
	.enable		= canaan_dsi_encoder_enable,
};

static int canaan_dsi_attach(struct mipi_dsi_host *host,
			    struct mipi_dsi_device *device)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);
	struct drm_panel *panel = of_drm_find_panel(device->dev.of_node);

	if (IS_ERR(panel))
		return PTR_ERR(panel);
	else
		dsi->connector.status = connector_status_connected;

	dsi->panel = panel;
	dsi->device = device;

	dev_info(host->dev, "Attached device %s\n", device->name);

	return 0;
}

static int canaan_dsi_detach(struct mipi_dsi_host *host,
			    struct mipi_dsi_device *device)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);

	dsi->panel = NULL;
	dsi->device = NULL;

	return 0;
}

static ssize_t canaan_dsi_transfer(struct mipi_dsi_host *host,
				  const struct mipi_dsi_msg *msg)
{
	struct canaan_dsi *dsi = host_to_canaan_dsi(host);
	int ret;

	ret = canaan_dsi_inst_wait_for_completion(dsi);
	if (ret < 0)
		canaan_dsi_inst_abort(dsi);

	switch (msg->type) {
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
		ret = canaan_dsi_dcs_write_short(dsi, msg);
		break;

	case MIPI_DSI_DCS_LONG_WRITE:
		ret = canaan_dsi_dcs_write_long(dsi, msg);
		break;

	case MIPI_DSI_DCS_READ:
		if (msg->rx_len == 1) {
			ret = canaan_dsi_dcs_read(dsi, msg);
			break;
		}
		fallthrough;

	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct mipi_dsi_host_ops canaan_dsi_host_ops = {
	.attach		= canaan_dsi_attach,
	.detach		= canaan_dsi_detach,
	.transfer	= canaan_dsi_transfer,
};


static int canaan_dsi_bind(struct device *dev, struct device *master,
			 void *data)
{
	struct drm_device *drm = data;
	struct canaan_dsi *dsi = dev_get_drvdata(dev);
	int ret;

	drm_encoder_helper_add(&dsi->encoder,
			       &canaan_dsi_enc_helper_funcs);
	ret = drm_simple_encoder_init(drm, &dsi->encoder,
				      DRM_MODE_ENCODER_DSI);
	if (ret) {
		dev_err(dsi->dev, "Couldn't initialise the DSI encoder\n");
		return ret;
	}
	dsi->encoder.possible_crtcs = BIT(0);

	drm_connector_helper_add(&dsi->connector,
				 &canaan_dsi_connector_helper_funcs);
	ret = drm_connector_init(drm, &dsi->connector,
				 &canaan_dsi_connector_funcs,
				 DRM_MODE_CONNECTOR_DSI);
	if (ret) {
		dev_err(dsi->dev,
			"Couldn't initialise the DSI connector\n");
		goto err_cleanup_connector;
	}

	drm_connector_attach_encoder(&dsi->connector, &dsi->encoder);

	dsi->drm = drm;

	return 0;

err_cleanup_connector:
	drm_encoder_cleanup(&dsi->encoder);
	return ret;
}

static void canaan_dsi_unbind(struct device *dev, struct device *master,
			    void *data)
{
	struct canaan_dsi *dsi = dev_get_drvdata(dev);

	dsi->drm = NULL;
}

static const struct component_ops canaan_dsi_ops = {
	.bind	= canaan_dsi_bind,
	.unbind	= canaan_dsi_unbind,
};

static int canaan_dsi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const char *bus_clk_name = NULL;
	struct canaan_dsi *dsi;
	struct resource *res;
	void __iomem *base;
	int ret;

	dsi = devm_kzalloc(dev, sizeof(*dsi), GFP_KERNEL);
	if (!dsi)
		return -ENOMEM;
	dev_set_drvdata(dev, dsi);
	dsi->dev = dev;
	dsi->host.ops = &canaan_dsi_host_ops;
	dsi->host.dev = dev;

	// DSI Device Tree Read..

	ret = mipi_dsi_host_register(&dsi->host);
	if (ret) {
		dev_err(dev, "Couldn't register MIPI-DSI host\n");
		goto err_unprotect_clk;
	}

	ret = component_add(&pdev->dev, &canaan_dsi_ops);
	if (ret) {
		dev_err(dev, "Couldn't register our component\n");
		goto err_remove_dsi_host;
	}

	return 0;

err_remove_dsi_host:
	mipi_dsi_host_unregister(&dsi->host);
err_unprotect_clk:
	clk_rate_exclusive_put(dsi->mod_clk);
err_attach_clk:
	if (!IS_ERR(dsi->bus_clk))
		regmap_mmio_detach_clk(dsi->regs);
	return ret;
}

static int canaan_dsi_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id canaan_dsi_of_table[] = {
	{ .compatible = "canaan,k230-mipi-dsi" },
	{ }
};
MODULE_DEVICE_TABLE(of, canaan_dsi_of_table);

struct platform_driver canaan_dsi_driver = {
	.probe		= canaan_dsi_probe,
	.remove		= canaan_dsi_remove,
	.driver		= {
		.name		= "canaan-mipi-dsi",
		.of_match_table	= canaan_dsi_of_table,
	},
};


MODULE_AUTHOR("");
MODULE_DESCRIPTION("Canaan K230 DSI Driver");
MODULE_LICENSE("GPL");
