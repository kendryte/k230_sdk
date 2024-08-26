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
#ifndef BINC_DEVICE_H
#define BINC_DEVICE_H

#include <glib.h>
#include "forward_decl.h"
#include "characteristic.h"
#include "descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ConnectionState {
    BINC_DISCONNECTED = 0, BINC_CONNECTED = 1, BINC_CONNECTING = 2, BINC_DISCONNECTING = 3
} ConnectionState;

typedef enum BondingState {
    BINC_BOND_NONE = 0, BINC_BONDING = 1, BINC_BONDED = 2
} BondingState;

typedef void (*ConnectionStateChangedCallback)(Device *device, ConnectionState state, const GError *error);

typedef void (*ServicesResolvedCallback)(Device *device);

typedef void (*BondingStateChangedCallback)(Device *device, BondingState new_state, BondingState old_state,
                                            const GError *error);


/**
 * Connect to a device asynchronously
 *
 * When a connection is attempted, established or failed, the ConnectionStateChangedCallback is called. Times out in 25 seconds.
 *
 * @param device the device to connect to. Must be non-null and not already connected.
 */
void binc_device_connect(Device *device);

void binc_device_pair(Device *device);

void binc_device_disconnect(Device *device);

void binc_device_set_read_char_cb(Device *device, OnReadCallback callback);

gboolean binc_device_read_char(const Device *device, const char *service_uuid, const char *characteristic_uuid);

void binc_device_set_write_char_cb(Device *device, OnWriteCallback callback);

gboolean binc_device_write_char(const Device *device, const char *service_uuid,
                                const char *characteristic_uuid, const GByteArray *byteArray, WriteType writeType);

void binc_device_set_notify_char_cb(Device *device, OnNotifyCallback callback);

void binc_device_set_notify_state_cb(Device *device, OnNotifyingStateChangedCallback callback);

gboolean binc_device_start_notify(const Device *device, const char *service_uuid, const char *characteristic_uuid);

gboolean binc_device_stop_notify(const Device *device, const char *service_uuid, const char *characteristic_uuid);

gboolean binc_device_read_desc(const Device *device, const char *service_uuid,
                               const char *characteristic_uuid, const char *desc_uuid);

gboolean binc_device_write_desc(const Device *device, const char *service_uuid,
                                const char *characteristic_uuid, const char *desc_uuid, const GByteArray *byteArray);

void binc_device_set_read_desc_cb(Device *device, OnDescReadCallback callback);

void binc_device_set_write_desc_cb(Device *device, OnDescWriteCallback callback);

void binc_device_set_connection_state_change_cb(Device *device, ConnectionStateChangedCallback callback);

void binc_device_set_services_resolved_cb(Device *device, ServicesResolvedCallback callback);

void binc_device_set_bonding_state_changed_cb(Device *device, BondingStateChangedCallback callback);

gboolean binc_device_has_service(const Device *device, const char *service_uuid);

GList *binc_device_get_services(const Device *device);

Service *binc_device_get_service(const Device *device, const char *service_uuid);

Characteristic *binc_device_get_characteristic(const Device *device,
                                               const char *service_uuid, const char *characteristic_uuid);

ConnectionState binc_device_get_connection_state(const Device *device);

const char *binc_device_get_connection_state_name(const Device *device);

const char *binc_device_get_address(const Device *device);

const char *binc_device_get_address_type(const Device *device);

const char *binc_device_get_alias(const Device *device);

const char *binc_device_get_name(const Device *device);

const char *binc_device_get_path(const Device *device);

gboolean binc_device_get_paired(const Device *device);

short binc_device_get_rssi(const Device *device);

gboolean binc_device_get_trusted(const Device *device);

short binc_device_get_txpower(const Device *device);

GList *binc_device_get_uuids(const Device *device);

GHashTable *binc_device_get_manufacturer_data(const Device *device);

GHashTable *binc_device_get_service_data(const Device *device);

BondingState binc_device_get_bonding_state(const Device *device);

Adapter *binc_device_get_adapter(const Device *device);

guint binc_device_get_mtu(const Device *device);

gboolean binc_device_is_central(const Device *device);

char *binc_device_to_string(const Device *device);

void binc_device_set_user_data(Device *device, void *user_data);

void *binc_device_get_user_data(const Device *device);

#ifdef __cplusplus
}
#endif

#endif //BINC_DEVICE_H
