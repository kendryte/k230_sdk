/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _CANAAN_DSI_H_
#define _CANAAN_DSI_H_

#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>
#include <drm/drm_mipi_dsi.h>

struct canaan_dsi {
	struct drm_connector	connector;
	struct drm_encoder	encoder;
	struct mipi_dsi_host	host;

	struct clk		*bus_clk;
	struct clk		*mod_clk;
	struct regmap		*regs;
	struct regulator	*regulator;
	struct reset_control	*reset;
	struct phy		*dphy;

	struct device		*dev;
	struct mipi_dsi_device	*device;
	struct drm_device	*drm;
	struct drm_panel	*panel;
};

static inline struct canaan_dsi *host_to_canaan_dsi(struct mipi_dsi_host *host)
{
	return container_of(host, struct canaan_dsi, host);
};

static inline struct canaan_dsi *connector_to_canaan_dsi(struct drm_connector *connector)
{
	return container_of(connector, struct canaan_dsi, connector);
};

static inline struct canaan_dsi *encoder_to_canaan_dsi(const struct drm_encoder *encoder)
{
	return container_of(encoder, struct canaan_dsi, encoder);
};


#endif /* _CANAAN_DSI_H_ */
