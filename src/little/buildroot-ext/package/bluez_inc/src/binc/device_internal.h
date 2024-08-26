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

#ifndef BINC_DEVICE_INTERNAL_H
#define BINC_DEVICE_INTERNAL_H

#include "device.h"

Device *binc_device_create(const char *path, Adapter *adapter);

void binc_device_free(Device *device);

GDBusConnection *binc_device_get_dbus_connection(const Device *device);

void binc_device_set_address(Device *device, const char *address);

void binc_device_set_address_type(Device *device, const char *address_type);

void binc_device_set_alias(Device *device, const char *alias);

void binc_device_set_adapter_path(Device *device, const char *adapter_path);

void binc_device_set_name(Device *device, const char *name);

void binc_device_set_path(Device *device, const char *path);

void binc_device_set_paired(Device *device, gboolean paired);

void binc_device_set_rssi(Device *device, short rssi);

void binc_device_set_trusted(Device *device, gboolean trusted);

void binc_device_set_txpower(Device *device, short txpower);

void binc_device_set_uuids(Device *device, GList *uuids);

void binc_device_set_manufacturer_data(Device *device, GHashTable *manufacturer_data);

void binc_device_set_service_data(Device *device, GHashTable *service_data);

void binc_device_set_bonding_state(Device *device, BondingState bonding_state);

void binc_device_set_is_central(Device *device, gboolean is_central);

void binc_internal_device_update_property(Device *device, const char *property_name, GVariant *property_value);

#endif //BINC_DEVICE_INTERNAL_H
