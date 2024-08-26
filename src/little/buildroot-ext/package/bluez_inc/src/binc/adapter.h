/*
 *   Copyright (c) 2022 Martijn van Welie
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 *
 */

#ifndef BINC_ADAPTER_H
#define BINC_ADAPTER_H

#include <gio/gio.h>
#include "forward_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DiscoveryState {
    BINC_DISCOVERY_STOPPED = 0, BINC_DISCOVERY_STARTED = 1, BINC_DISCOVERY_STARTING = 2, BINC_DISCOVERY_STOPPING = 3
} DiscoveryState;

typedef void (*AdapterDiscoveryResultCallback)(Adapter *adapter, Device *device);

typedef void (*AdapterDiscoveryStateChangeCallback)(Adapter *adapter, DiscoveryState state, const GError *error);

typedef void (*AdapterPoweredStateChangeCallback)(Adapter *adapter, gboolean state);

typedef void (*RemoteCentralConnectionStateCallback)(Adapter *adapter, Device *device);


Adapter *binc_adapter_get_default(GDBusConnection *dbusConnection);

Adapter *binc_adapter_get(GDBusConnection *dbusConnection, const char *name);

GPtrArray *binc_adapter_find_all(GDBusConnection *dbusConnection);

void binc_adapter_free(Adapter *adapter);

void binc_adapter_start_discovery(Adapter *adapter);

void binc_adapter_stop_discovery(Adapter *adapter);

void binc_adapter_set_discovery_filter(Adapter *adapter, short rssi_threshold, const GPtrArray *service_uuids, const char *pattern);

void binc_adapter_remove_device(Adapter *adapter, Device *device);

GList *binc_adapter_get_devices(const Adapter *adapter);

GList *binc_adapter_get_connected_devices(const Adapter *adapter);

Device *binc_adapter_get_device_by_path(const Adapter *adapter, const char *path); // make this internal

Device *binc_adapter_get_device_by_address(const Adapter *adapter, const char *address);

void binc_adapter_power_on(Adapter *adapter);

void binc_adapter_power_off(Adapter *adapter);

void binc_adapter_discoverable_on(Adapter *adapter);

void binc_adapter_discoverable_off(Adapter *adapter);

const char *binc_adapter_get_path(const Adapter *adapter);

const char *binc_adapter_get_name(const Adapter *adapter);

const char *binc_adapter_get_address(const Adapter *adapter);

DiscoveryState binc_adapter_get_discovery_state(const Adapter *adapter);

const char *binc_adapter_get_discovery_state_name(const Adapter *adapter);

gboolean binc_adapter_get_powered_state(const Adapter *adapter);

void binc_adapter_set_discovery_cb(Adapter *adapter, AdapterDiscoveryResultCallback callback);

void binc_adapter_set_discovery_state_cb(Adapter *adapter, AdapterDiscoveryStateChangeCallback callback);

void binc_adapter_set_powered_state_cb(Adapter *adapter, AdapterPoweredStateChangeCallback callback);

GDBusConnection *binc_adapter_get_dbus_connection(const Adapter *adapter);

gboolean binc_adapter_is_discoverable(const Adapter *adapter);

void binc_adapter_start_advertising(Adapter *adapter, Advertisement *advertisement);

void binc_adapter_stop_advertising(Adapter *adapter, Advertisement *advertisement);

void binc_adapter_register_application(Adapter *adapter, Application *application);

void binc_adapter_unregister_application(Adapter *adapter, Application *application);

void binc_adapter_set_remote_central_cb(Adapter *adapter, RemoteCentralConnectionStateCallback callback);

void binc_adapter_set_user_data(Adapter *adapter, void *user_data);

void *binc_adapter_get_user_data(const Adapter *adapter);

#ifdef __cplusplus
}
#endif

#endif //BINC_ADAPTER_H
