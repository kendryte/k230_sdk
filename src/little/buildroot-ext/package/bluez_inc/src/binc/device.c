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

#include <gio/gio.h>
#include "logger.h"
#include "device.h"
#include "utility.h"
#include "service_internal.h"
#include "characteristic_internal.h"
#include "adapter.h"
#include "descriptor_internal.h"

static const char *const TAG = "Device";
static const char *const BLUEZ_DBUS = "org.bluez";
static const char *const INTERFACE_DEVICE = "org.bluez.Device1";

static const char *const DEVICE_METHOD_CONNECT = "Connect";
static const char *const DEVICE_METHOD_PAIR = "Pair";
static const char *const DEVICE_METHOD_DISCONNECT = "Disconnect";

static const char *const DEVICE_PROPERTY_ADDRESS = "Address";
static const char *const DEVICE_PROPERTY_ADDRESS_TYPE = "AddressType";
static const char *const DEVICE_PROPERTY_ALIAS = "Alias";
static const char *const DEVICE_PROPERTY_NAME = "Name";
static const char *const DEVICE_PROPERTY_PAIRED = "Paired";
static const char *const DEVICE_PROPERTY_RSSI = "RSSI";
static const char *const DEVICE_PROPERTY_UUIDS = "UUIDs";
static const char *const DEVICE_PROPERTY_MANUFACTURER_DATA = "ManufacturerData";
static const char *const DEVICE_PROPERTY_SERVICE_DATA = "ServiceData";
static const char *const DEVICE_PROPERTY_TRUSTED = "Trusted";
static const char *const DEVICE_PROPERTY_TXPOWER = "TxPower";
static const char *const DEVICE_PROPERTY_CONNECTED = "Connected";
static const char *const DEVICE_PROPERTY_SERVICES_RESOLVED = "ServicesResolved";

static const char *const INTERFACE_SERVICE = "org.bluez.GattService1";
static const char *const INTERFACE_CHARACTERISTIC = "org.bluez.GattCharacteristic1";
static const char *const INTERFACE_DESCRIPTOR = "org.bluez.GattDescriptor1";

static const char *connection_state_names[] = {
        [BINC_DISCONNECTED] = "DISCONNECTED",
        [BINC_CONNECTED] = "CONNECTED",
        [BINC_CONNECTING]  = "CONNECTING",
        [BINC_DISCONNECTING]  = "DISCONNECTING"
};

struct binc_device {
    GDBusConnection *connection; // Borrowed
    Adapter *adapter; // Borrowed
    const char *address; // Owned
    const char *address_type; // Owned
    const char *alias; // Owned
    ConnectionState connection_state;
    gboolean services_resolved;
    gboolean service_discovery_started;
    gboolean paired;
    BondingState bondingState;
    const char *path; // Owned
    const char *name; // Owned
    short rssi;
    gboolean trusted;
    short txpower;
    GHashTable *manufacturer_data; // Owned
    GHashTable *service_data; // Owned
    GList *uuids; // Owned
    guint mtu;

    guint device_prop_changed;
    ConnectionStateChangedCallback connection_state_callback;
    ServicesResolvedCallback services_resolved_callback;
    BondingStateChangedCallback bonding_state_callback;
    GHashTable *services; // Owned
    GList *services_list; // Owned
    GHashTable *characteristics; // Owned
    GHashTable *descriptors; // Owned
    gboolean is_central;

    OnReadCallback on_read_callback;
    OnWriteCallback on_write_callback;
    OnNotifyCallback on_notify_callback;
    OnNotifyingStateChangedCallback on_notify_state_callback;
    OnDescReadCallback on_read_desc_cb;
    OnDescWriteCallback on_write_desc_cb;
    void *user_data; // Borrowed
};


Device *binc_device_create(const char *path, Adapter *adapter) {
    g_assert(path != NULL);
    g_assert(strlen(path) > 0);
    g_assert(adapter != NULL);

    Device *device = g_new0(Device, 1);
    device->path = g_strdup(path);
    device->adapter = adapter;
    device->connection = binc_adapter_get_dbus_connection(adapter);
    device->bondingState = BINC_BOND_NONE;
    device->connection_state = BINC_DISCONNECTED;
    device->rssi = -255;
    device->txpower = -255;
    device->mtu = 23;
    device->user_data = NULL;
    return device;
}

static void binc_device_free_uuids(Device *device) {
    if (device->uuids != NULL) {
        g_list_free_full(device->uuids, g_free);
        device->uuids = NULL;
    }
}

static void byte_array_free(GByteArray *byteArray) { g_byte_array_free(byteArray, TRUE); }

static void binc_device_free_manufacturer_data(Device *device) {
    g_assert(device != NULL);

    if (device->manufacturer_data != NULL) {
        g_hash_table_destroy(device->manufacturer_data);
        device->manufacturer_data = NULL;
    }
}

static void binc_device_free_service_data(Device *device) {
    g_assert(device != NULL);

    if (device->service_data != NULL) {
        g_hash_table_destroy(device->service_data);
        device->service_data = NULL;
    }
}

void binc_device_free(Device *device) {
    g_assert(device != NULL);

    log_debug(TAG, "freeing %s", device->path);

    if (device->device_prop_changed != 0) {
        g_dbus_connection_signal_unsubscribe(device->connection, device->device_prop_changed);
        device->device_prop_changed = 0;
    }

    g_free((char *) device->path);
    device->path = NULL;
    g_free((char *) device->address_type);
    device->address_type = NULL;
    g_free((char *) device->address);
    device->address = NULL;
    g_free((char *) device->alias);
    device->alias = NULL;
    g_free((char *) device->name);
    device->name = NULL;

    if (device->descriptors != NULL) {
        g_hash_table_destroy(device->descriptors);
        device->descriptors = NULL;
    }

    if (device->characteristics != NULL) {
        g_hash_table_destroy(device->characteristics);
        device->characteristics = NULL;
    }

    if (device->services != NULL) {
        g_hash_table_destroy(device->services);
        device->services = NULL;
    }

    binc_device_free_manufacturer_data(device);
    binc_device_free_service_data(device);
    binc_device_free_uuids(device);

    if (device->services_list != NULL) {
        g_list_free(device->services_list);
        device->services_list = NULL;
    }

    device->connection = NULL;
    device->adapter = NULL;
    g_free(device);
}

char *binc_device_to_string(const Device *device) {
    g_assert(device != NULL);

    // First build up uuids string
    GString *uuids = g_string_new("[");
    if (g_list_length(device->uuids) > 0) {
        for (GList *iterator = device->uuids; iterator; iterator = iterator->next) {
            g_string_append_printf(uuids, "%s, ", (char *) iterator->data);
        }
        g_string_truncate(uuids, uuids->len - 2);
    }
    g_string_append(uuids, "]");

    // Build up manufacturer data string
    GString *manufacturer_data = g_string_new("[");
    if (device->manufacturer_data != NULL && g_hash_table_size(device->manufacturer_data) > 0) {
        GHashTableIter iter;
        int *key;
        gpointer value;
        g_hash_table_iter_init(&iter, device->manufacturer_data);
        while (g_hash_table_iter_next(&iter, (gpointer) &key, &value)) {
            GByteArray *byteArray = (GByteArray *) value;
            GString *byteArrayString = g_byte_array_as_hex(byteArray);
            gint keyInt = *key;
            g_string_append_printf(manufacturer_data, "%04X -> %s, ", keyInt, byteArrayString->str);
            g_string_free(byteArrayString, TRUE);
        }
        g_string_truncate(manufacturer_data, manufacturer_data->len - 2);
    }
    g_string_append(manufacturer_data, "]");

    // Build up service data string
    GString *service_data = g_string_new("[");
    if (device->service_data != NULL && g_hash_table_size(device->service_data) > 0) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, device->service_data);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            GByteArray *byteArray = (GByteArray *) value;
            GString *byteArrayString = g_byte_array_as_hex(byteArray);
            g_string_append_printf(service_data, "%s -> %s, ", (char *) key, byteArrayString->str);
            g_string_free(byteArrayString, TRUE);
        }
        g_string_truncate(service_data, service_data->len - 2);
    }
    g_string_append(service_data, "]");

    char *result = g_strdup_printf(
            "Device{name='%s', address='%s', address_type=%s, rssi=%d, uuids=%s, manufacturer_data=%s, service_data=%s, paired=%s, txpower=%d path='%s' }",
            device->name,
            device->address,
            device->address_type,
            device->rssi,
            uuids->str,
            manufacturer_data->str,
            service_data->str,
            device->paired ? "true" : "false",
            device->txpower,
            device->path
    );

    g_string_free(uuids, TRUE);
    g_string_free(manufacturer_data, TRUE);
    g_string_free(service_data, TRUE);
    return result;
}

static void
binc_on_characteristic_read(Device *device, Characteristic *characteristic, const GByteArray *byteArray, const GError *error) {
    if (device->on_read_callback != NULL) {
        device->on_read_callback(device, characteristic, byteArray, error);
    }
}

static void
binc_on_characteristic_write(Device *device, Characteristic *characteristic, const GByteArray *byteArray, const GError *error) {
    if (device->on_write_callback != NULL) {
        device->on_write_callback(device, characteristic, byteArray, error);
    }
}

static void binc_on_characteristic_notify(Device *device, Characteristic *characteristic, const GByteArray *byteArray) {
    if (device->on_notify_callback != NULL) {
        device->on_notify_callback(device, characteristic, byteArray);
    }
}

static void binc_on_characteristic_notification_state_changed(Device *device, Characteristic *characteristic, const GError *error) {
    if (device->on_notify_state_callback != NULL) {
        device->on_notify_state_callback(device, characteristic, error);
    }
}

static void binc_on_descriptor_read(Device *device, Descriptor *descriptor, const GByteArray *byteArray, const GError *error) {
    if (device->on_read_desc_cb != NULL) {
        device->on_read_desc_cb(device, descriptor, byteArray, error);
    }
}

static void binc_on_descriptor_write(Device *device, Descriptor *descriptor, const GByteArray *byteArray, const GError *error) {
    if (device->on_write_desc_cb != NULL) {
        device->on_write_desc_cb(device, descriptor, byteArray, error);
    }
}

static void binc_device_internal_set_conn_state(Device *device, ConnectionState state, GError *error) {
    ConnectionState old_state = device->connection_state;
    device->connection_state = state;
    if (device->connection_state_callback != NULL) {
        if (device->connection_state != old_state) {
            device->connection_state_callback(device, state, error);
        }
    }
}

static void binc_internal_extract_service(Device *device, const char *object_path, GVariant *properties) {
    g_assert(device != NULL);
    g_assert(object_path != NULL);
    g_assert(properties != NULL);

    char *uuid = NULL;
    const char *property_name;
    GVariantIter iter;
    GVariant *property_value;

    g_variant_iter_init(&iter, properties);
    while (g_variant_iter_loop(&iter, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, "UUID")) {
            uuid = g_strdup(g_variant_get_string(property_value, NULL));
        }
    }

    Service *service = binc_service_create(device, object_path, uuid);
    g_hash_table_insert(device->services, g_strdup(object_path), service);
    g_free(uuid);
}

static void binc_internal_extract_characteristic(Device *device, const char *object_path, GVariant *properties) {
    g_assert(device != NULL);
    g_assert(object_path != NULL);
    g_assert(properties != NULL);

    Characteristic *characteristic = binc_characteristic_create(device, object_path);
    binc_characteristic_set_read_cb(characteristic, &binc_on_characteristic_read);
    binc_characteristic_set_write_cb(characteristic, &binc_on_characteristic_write);
    binc_characteristic_set_notify_cb(characteristic, &binc_on_characteristic_notify);
    binc_characteristic_set_notifying_state_change_cb(characteristic,
                                                      &binc_on_characteristic_notification_state_changed);

    const char *property_name;
    GVariantIter iter;
    GVariant *property_value;

    g_variant_iter_init(&iter, properties);
    while (g_variant_iter_loop(&iter, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, "UUID")) {
            binc_characteristic_set_uuid(characteristic,
                                         g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "Service")) {
            binc_characteristic_set_service_path(characteristic,
                                                 g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "Flags")) {
            binc_characteristic_set_flags(characteristic,
                                          g_variant_string_array_to_list(property_value));
        } else if (g_str_equal(property_name, "Notifying")) {
            binc_characteristic_set_notifying(characteristic,
                                              g_variant_get_boolean(property_value));
        } else if (g_str_equal(property_name, "MTU")) {
            device->mtu = g_variant_get_uint16(property_value);
            binc_characteristic_set_mtu(characteristic, g_variant_get_uint16(property_value));
        }
    }

    // Get service and link the characteristic to the service
    Service *service = g_hash_table_lookup(device->services,
                                           binc_characteristic_get_service_path(characteristic));
    if (service != NULL) {
        binc_service_add_characteristic(service, characteristic);
        binc_characteristic_set_service(characteristic, service);
        g_hash_table_insert(device->characteristics, g_strdup(object_path), characteristic);

        char *charString = binc_characteristic_to_string(characteristic);
        log_debug(TAG, charString);
        g_free(charString);
    } else {
        log_error(TAG, "could not find service %s",
                  binc_characteristic_get_service_path(characteristic));
    }
}

static void binc_internal_extract_descriptor(Device *device, const char *object_path, GVariant *properties) {
    g_assert(device != NULL);
    g_assert(object_path != NULL);
    g_assert(properties != NULL);

    Descriptor *descriptor = binc_descriptor_create(device, object_path);
    binc_descriptor_set_read_cb(descriptor, &binc_on_descriptor_read);
    binc_descriptor_set_write_cb(descriptor, &binc_on_descriptor_write);

    const char *property_name;
    GVariantIter iter;
    GVariant *property_value;
    g_variant_iter_init(&iter, properties);
    while (g_variant_iter_loop(&iter, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, "UUID")) {
            binc_descriptor_set_uuid(descriptor, g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "Characteristic")) {
            binc_descriptor_set_char_path(descriptor,
                                          g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "Flags")) {
            binc_descriptor_set_flags(descriptor, g_variant_string_array_to_list(property_value));
        }
    }

    // Look up characteristic
    Characteristic *characteristic = g_hash_table_lookup(device->characteristics,
                                                         binc_descriptor_get_char_path(descriptor));
    if (characteristic != NULL) {
        binc_characteristic_add_descriptor(characteristic, descriptor);
        binc_descriptor_set_char(descriptor, characteristic);
        g_hash_table_insert(device->descriptors, g_strdup(object_path), descriptor);

        const char *descString = binc_descriptor_to_string(descriptor);
        log_debug(TAG, descString);
        g_free((char *) descString);
    } else {
        log_error(TAG, "could not find characteristic %s",
                  binc_descriptor_get_char_path(descriptor));
    }
}

static void binc_internal_collect_gatt_tree_cb(__attribute__((unused)) GObject *source_object,
                                               GAsyncResult *res,
                                               gpointer user_data) {

    GError *error = NULL;
    Device *device = (Device *) user_data;
    g_assert(device != NULL);

    GVariant *result = g_dbus_connection_call_finish(device->connection, res, &error);

    if (result == NULL) {
        log_error(TAG, "Unable to get result for GetManagedObjects");
        if (error != NULL) {
            log_error(TAG, "call failed (error %d: %s)", error->code, error->message);
            g_clear_error(&error);
            return;
        }
    }

    GVariantIter *iter;
    const char *object_path;
    GVariant *ifaces_and_properties;
    if (result) {
        if (device->services != NULL) {
            g_hash_table_destroy(device->services);
        }
        device->services = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                 g_free, (GDestroyNotify) binc_service_free);

        if (device->characteristics != NULL) {
            g_hash_table_destroy(device->characteristics);
        }
        device->characteristics = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                        g_free, (GDestroyNotify) binc_characteristic_free);

        if (device->descriptors != NULL) {
            g_hash_table_destroy(device->descriptors);
        }
        device->descriptors = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                    g_free, (GDestroyNotify) binc_descriptor_free);

        g_assert(g_str_equal(g_variant_get_type_string(result), "(a{oa{sa{sv}}})"));
        g_variant_get(result, "(a{oa{sa{sv}}})", &iter);
        while (g_variant_iter_loop(iter, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties)) {
            if (g_str_has_prefix(object_path, device->path)) {
                const char *interface_name;
                GVariant *properties;
                GVariantIter iter2;
                g_variant_iter_init(&iter2, ifaces_and_properties);
                while (g_variant_iter_loop(&iter2, "{&s@a{sv}}", &interface_name, &properties)) {
                    if (g_str_equal(interface_name, INTERFACE_SERVICE)) {
                        binc_internal_extract_service(device, object_path, properties);
                    } else if (g_str_equal(interface_name, INTERFACE_CHARACTERISTIC)) {
                        binc_internal_extract_characteristic(device, object_path, properties);
                    } else if (g_str_equal(interface_name, INTERFACE_DESCRIPTOR)) {
                        binc_internal_extract_descriptor(device, object_path, properties);

                    }
                }
            }
        }

        if (iter != NULL) {
            g_variant_iter_free(iter);
        }
        g_variant_unref(result);
    }

    if (device->services_list != NULL) {
        g_list_free(device->services_list);
    }
    device->services_list = g_hash_table_get_values(device->services);

    log_debug(TAG, "found %d services", g_list_length(device->services_list));
    if (device->services_resolved_callback != NULL) {
        device->services_resolved_callback(device);
    }
}

static void binc_collect_gatt_tree(Device *device) {
    g_assert(device != NULL);

    device->service_discovery_started = TRUE;
    g_dbus_connection_call(device->connection,
                           BLUEZ_DBUS,
                           "/",
                           "org.freedesktop.DBus.ObjectManager",
                           "GetManagedObjects",
                           NULL,
                           G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_collect_gatt_tree_cb,
                           device);
}

void binc_device_set_bonding_state_changed_cb(Device *device, BondingStateChangedCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);

    device->bonding_state_callback = callback;
}

void binc_device_set_bonding_state(Device *device, BondingState bonding_state) {
    g_assert(device != NULL);

    BondingState old_state = device->bondingState;
    device->bondingState = bonding_state;
    if (device->bonding_state_callback != NULL) {
        if (device->bondingState != old_state) {
            device->bonding_state_callback(device, device->bondingState, old_state, NULL);
        }
    }
}

static void binc_device_changed(__attribute__((unused)) GDBusConnection *conn,
                                __attribute__((unused)) const gchar *sender,
                                __attribute__((unused)) const gchar *path,
                                __attribute__((unused)) const gchar *interface,
                                __attribute__((unused)) const gchar *signal,
                                GVariant *params,
                                void *userdata) {

    GVariantIter *properties_changed = NULL;
    GVariantIter *properties_invalidated = NULL;
    const char *iface = NULL;
    const char *property_name = NULL;
    GVariant *property_value = NULL;

    Device *device = (Device *) userdata;
    g_assert(device != NULL);

    g_assert(g_str_equal(g_variant_get_type_string(params), "(sa{sv}as)"));
    g_variant_get(params, "(&sa{sv}as)", &iface, &properties_changed, &properties_invalidated);
    while (g_variant_iter_loop(properties_changed, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, DEVICE_PROPERTY_CONNECTED)) {
            binc_device_internal_set_conn_state(device, g_variant_get_boolean(property_value), NULL);
            if (device->connection_state == BINC_DISCONNECTED) {
                g_dbus_connection_signal_unsubscribe(device->connection, device->device_prop_changed);
                device->device_prop_changed = 0;
            }
        } else if (g_str_equal(property_name, DEVICE_PROPERTY_SERVICES_RESOLVED)) {
            device->services_resolved = g_variant_get_boolean(property_value);
            log_debug(TAG, "ServicesResolved %s", device->services_resolved ? "true" : "false");
            if (device->services_resolved == TRUE && device->bondingState != BINC_BONDING) {
                binc_collect_gatt_tree(device);
            }

            if (device->services_resolved == FALSE && device->connection_state == BINC_CONNECTED) {
                binc_device_internal_set_conn_state(device, BINC_DISCONNECTING, NULL);
            }
        } else if (g_str_equal(property_name, DEVICE_PROPERTY_PAIRED)) {
            device->paired = g_variant_get_boolean(property_value);
            log_debug(TAG, "Paired %s", device->paired ? "true" : "false");
            binc_device_set_bonding_state(device, device->paired ? BINC_BONDED : BINC_BOND_NONE);

            // If gatt-tree has not been built yet, start building it
            if (device->services == NULL && device->services_resolved && !device->service_discovery_started) {
                binc_collect_gatt_tree(device);
            }
        }
    }

    if (properties_changed != NULL)
        g_variant_iter_free(properties_changed);

    if (properties_invalidated != NULL)
        g_variant_iter_free(properties_invalidated);
}

static void binc_internal_device_connect_cb(__attribute__((unused)) GObject *source_object,
                                            GAsyncResult *res,
                                            gpointer user_data) {

    GError *error = NULL;
    Device *device = (Device *) user_data;
    g_assert(device != NULL);

    GVariant *value = g_dbus_connection_call_finish(device->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_error(TAG, "Connect failed (error %d: %s)", error->code, error->message);

        // Maybe don't do this because connection changes may com later? See A&D scale testing
        // Or send the current connection state?
        binc_device_internal_set_conn_state(device, BINC_DISCONNECTED, error);

        g_clear_error(&error);
    }
}

static void subscribe_prop_changed(Device *device) {
    if (device->device_prop_changed == 0) {
        device->device_prop_changed = g_dbus_connection_signal_subscribe(device->connection,
                                                                         BLUEZ_DBUS,
                                                                         "org.freedesktop.DBus.Properties",
                                                                         "PropertiesChanged",
                                                                         device->path,
                                                                         INTERFACE_DEVICE,
                                                                         G_DBUS_SIGNAL_FLAGS_NONE,
                                                                         binc_device_changed,
                                                                         device,
                                                                         NULL);
    }
}

void binc_device_connect(Device *device) {
    g_assert(device != NULL);
    g_assert(device->path != NULL);

    // Don't do anything if we are not disconnected
    if (device->connection_state != BINC_DISCONNECTED) return;

    log_debug(TAG, "Connecting to '%s' (%s) (%s)", device->name, device->address,
              device->paired ? "BINC_BONDED" : "BINC_BOND_NONE");

    binc_device_internal_set_conn_state(device, BINC_CONNECTING, NULL);
    subscribe_prop_changed(device);
    g_dbus_connection_call(device->connection,
                           BLUEZ_DBUS,
                           device->path,
                           INTERFACE_DEVICE,
                           DEVICE_METHOD_CONNECT,
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_device_connect_cb,
                           device);
}

static void binc_internal_device_pair_cb(__attribute__((unused)) GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data) {

    Device *device = (Device *) user_data;
    g_assert(device != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(device->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_error(TAG, "failed to call '%s' (error %d: %s)", DEVICE_METHOD_PAIR, error->code, error->message);
        binc_device_internal_set_conn_state(device, BINC_DISCONNECTED, error);
        g_clear_error(&error);
    }
}

void binc_device_pair(Device *device) {
    g_assert(device != NULL);
    g_assert(device->path != NULL);

    log_debug(TAG, "pairing device '%s'", device->address);

    if (device->connection_state == BINC_DISCONNECTING) {
        return;
    }

    if (device->connection_state == BINC_DISCONNECTED) {
        binc_device_internal_set_conn_state(device, BINC_CONNECTING, NULL);
    }

    subscribe_prop_changed(device);
    g_dbus_connection_call(device->connection,
                           BLUEZ_DBUS,
                           device->path,
                           INTERFACE_DEVICE,
                           DEVICE_METHOD_PAIR,
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_device_pair_cb,
                           device);
}

static void binc_internal_device_disconnect_cb(__attribute__((unused)) GObject *source_object,
                                               GAsyncResult *res,
                                               gpointer user_data) {

    Device *device = (Device *) user_data;
    g_assert(device != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(device->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_error(TAG, "failed to call '%s' (error %d: %s)", DEVICE_METHOD_DISCONNECT, error->code, error->message);
        binc_device_internal_set_conn_state(device, BINC_CONNECTED, error);
        g_clear_error(&error);
    }
}

void binc_device_disconnect(Device *device) {
    g_assert(device != NULL);
    g_assert(device->path != NULL);

    // Don't do anything if we are not connected
    if (device->connection_state != BINC_CONNECTED) return;

    log_debug(TAG, "Disconnecting '%s' (%s)", device->name, device->address);

    binc_device_internal_set_conn_state(device, BINC_DISCONNECTING, NULL);
    g_dbus_connection_call(device->connection,
                           BLUEZ_DBUS,
                           device->path,
                           INTERFACE_DEVICE,
                           DEVICE_METHOD_DISCONNECT,
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_device_disconnect_cb,
                           device);
}


void binc_device_set_connection_state_change_cb(Device *device, ConnectionStateChangedCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);

    device->connection_state_callback = callback;
}

GList *binc_device_get_services(const Device *device) {
    g_assert(device != NULL);
    return device->services_list;
}

void binc_device_set_services_resolved_cb(Device *device, ServicesResolvedCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);

    device->services_resolved_callback = callback;
}

Service *binc_device_get_service(const Device *device, const char *service_uuid) {
    g_assert(device != NULL);
    g_assert(service_uuid != NULL);
    g_assert(g_uuid_string_is_valid(service_uuid));

    if (device->services_list != NULL) {
        for (GList *iterator = device->services_list; iterator; iterator = iterator->next) {
            Service *service = (Service *) iterator->data;
            if (g_str_equal(service_uuid, binc_service_get_uuid(service))) {
                return service;
            }
        }
    }

    return NULL;
}

Characteristic *
binc_device_get_characteristic(const Device *device, const char *service_uuid, const char *characteristic_uuid) {
    g_assert(device != NULL);
    g_assert(service_uuid != NULL);
    g_assert(characteristic_uuid != NULL);
    g_assert(g_uuid_string_is_valid(service_uuid));
    g_assert(g_uuid_string_is_valid(characteristic_uuid));

    Service *service = binc_device_get_service(device, service_uuid);
    if (service != NULL) {
        return binc_service_get_characteristic(service, characteristic_uuid);
    }

    return NULL;
}

void binc_device_set_read_char_cb(Device *device, OnReadCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_read_callback = callback;
}

gboolean binc_device_read_char(const Device *device, const char *service_uuid, const char *characteristic_uuid) {
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic != NULL && binc_characteristic_supports_read(characteristic)) {
        binc_characteristic_read(characteristic);
        return TRUE;
    }
    return FALSE;
}

gboolean binc_device_read_desc(const Device *device, const char *service_uuid,
                               const char *characteristic_uuid, const char *desc_uuid) {
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));
    g_assert(is_valid_uuid(desc_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic == NULL) {
        return FALSE;
    }

    Descriptor *descriptor = binc_characteristic_get_descriptor(characteristic, desc_uuid);
    if (descriptor == NULL) {
        return FALSE;
    }

    binc_descriptor_read(descriptor);
    return TRUE;
}

gboolean binc_device_write_desc(const Device *device, const char *service_uuid,
                                const char *characteristic_uuid, const char *desc_uuid, const GByteArray *byteArray) {
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));
    g_assert(is_valid_uuid(desc_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic == NULL) {
        return FALSE;
    }

    Descriptor *descriptor = binc_characteristic_get_descriptor(characteristic, desc_uuid);
    if (descriptor == NULL) {
        return FALSE;
    }

    binc_descriptor_write(descriptor, byteArray);
    return TRUE;
}

void binc_device_set_write_char_cb(Device *device, OnWriteCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_write_callback = callback;
}

gboolean binc_device_write_char(const Device *device, const char *service_uuid, const char *characteristic_uuid,
                                const GByteArray *byteArray, WriteType writeType) {
    g_assert(device != NULL);
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic != NULL && binc_characteristic_supports_write(characteristic, writeType)) {
        binc_characteristic_write(characteristic, byteArray, writeType);
        return TRUE;
    }
    return FALSE;
}

void binc_device_set_notify_char_cb(Device *device, OnNotifyCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_notify_callback = callback;
}

void binc_device_set_notify_state_cb(Device *device, OnNotifyingStateChangedCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_notify_state_callback = callback;
}

gboolean binc_device_start_notify(const Device *device, const char *service_uuid, const char *characteristic_uuid) {
    g_assert(device != NULL);
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic != NULL && binc_characteristic_supports_notify(characteristic)) {
        binc_characteristic_start_notify(characteristic);
        return TRUE;
    }
    return FALSE;
}

gboolean binc_device_stop_notify(const Device *device, const char *service_uuid, const char *characteristic_uuid) {
    g_assert(device != NULL);
    g_assert(is_valid_uuid(service_uuid));
    g_assert(is_valid_uuid(characteristic_uuid));

    Characteristic *characteristic = binc_device_get_characteristic(device, service_uuid, characteristic_uuid);
    if (characteristic != NULL && binc_characteristic_supports_notify(characteristic) && binc_characteristic_is_notifying(characteristic)) {
        binc_characteristic_stop_notify(characteristic);
        return TRUE;
    }
    return FALSE;
}


void binc_device_set_read_desc_cb(Device *device, OnDescReadCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_read_desc_cb = callback;
}

void binc_device_set_write_desc_cb(Device *device, OnDescWriteCallback callback) {
    g_assert(device != NULL);
    g_assert(callback != NULL);
    device->on_write_desc_cb = callback;
}

ConnectionState binc_device_get_connection_state(const Device *device) {
    g_assert(device != NULL);
    return device->connection_state;
}

const char *binc_device_get_connection_state_name(const Device *device) {
    g_assert(device != NULL);
    return connection_state_names[device->connection_state];
}

const char *binc_device_get_address(const Device *device) {
    g_assert(device != NULL);
    return device->address;
}

void binc_device_set_address(Device *device, const char *address) {
    g_assert(device != NULL);
    g_assert(address != NULL);

    g_free((char *) device->address);
    device->address = g_strdup(address);
}

const char *binc_device_get_address_type(const Device *device) {
    g_assert(device != NULL);
    return device->address_type;
}

void binc_device_set_address_type(Device *device, const char *address_type) {
    g_assert(device != NULL);
    g_assert(address_type != NULL);

    g_free((char *) device->address_type);
    device->address_type = g_strdup(address_type);
}

const char *binc_device_get_alias(const Device *device) {
    g_assert(device != NULL);
    return device->alias;
}

void binc_device_set_alias(Device *device, const char *alias) {
    g_assert(device != NULL);
    g_assert(alias != NULL);

    g_free((char *) device->alias);
    device->alias = g_strdup(alias);
}

const char *binc_device_get_name(const Device *device) {
    g_assert(device != NULL);
    return device->name;
}

void binc_device_set_name(Device *device, const char *name) {
    g_assert(device != NULL);
    g_assert(name != NULL);
    g_assert(strlen(name) > 0);

    g_free((char *) device->name);
    device->name = g_strdup(name);
}

const char *binc_device_get_path(const Device *device) {
    g_assert(device != NULL);
    return device->path;
}

void binc_device_set_path(Device *device, const char *path) {
    g_assert(device != NULL);
    g_assert(path != NULL);

    g_free((char *) device->path);
    device->path = g_strdup(path);
}

gboolean binc_device_get_paired(const Device *device) {
    g_assert(device != NULL);
    return device->paired;
}

void binc_device_set_paired(Device *device, gboolean paired) {
    g_assert(device != NULL);
    device->paired = paired;
    binc_device_set_bonding_state(device, paired ? BINC_BONDED : BINC_BOND_NONE);
}

short binc_device_get_rssi(const Device *device) {
    g_assert(device != NULL);
    return device->rssi;
}

void binc_device_set_rssi(Device *device, short rssi) {
    g_assert(device != NULL);
    device->rssi = rssi;
}

gboolean binc_device_get_trusted(const Device *device) {
    g_assert(device != NULL);
    return device->trusted;
}

void binc_device_set_trusted(Device *device, gboolean trusted) {
    g_assert(device != NULL);
    device->trusted = trusted;
}

short binc_device_get_txpower(const Device *device) {
    g_assert(device != NULL);
    return device->txpower;
}

void binc_device_set_txpower(Device *device, short txpower) {
    g_assert(device != NULL);
    device->txpower = txpower;
}

GList *binc_device_get_uuids(const Device *device) {
    g_assert(device != NULL);
    return device->uuids;
}

void binc_device_set_uuids(Device *device, GList *uuids) {
    g_assert(device != NULL);

    binc_device_free_uuids(device);
    device->uuids = uuids;
}

GHashTable *binc_device_get_manufacturer_data(const Device *device) {
    g_assert(device != NULL);
    return device->manufacturer_data;
}

void binc_device_set_manufacturer_data(Device *device, GHashTable *manufacturer_data) {
    g_assert(device != NULL);

    binc_device_free_manufacturer_data(device);
    device->manufacturer_data = manufacturer_data;
}

GHashTable *binc_device_get_service_data(const Device *device) {
    g_assert(device != NULL);
    return device->service_data;
}

void binc_device_set_service_data(Device *device, GHashTable *service_data) {
    g_assert(device != NULL);

    binc_device_free_service_data(device);
    device->service_data = service_data;
}

void binc_device_set_is_central(Device *device, gboolean is_central) {
    g_assert(device != NULL);
    device->is_central = is_central;
}

gboolean binc_device_is_central(const Device *device) {
    g_assert(device != NULL);
    return device->is_central;
}

GDBusConnection *binc_device_get_dbus_connection(const Device *device) {
    g_assert(device != NULL);
    return device->connection;
}

BondingState binc_device_get_bonding_state(const Device *device) {
    g_assert(device != NULL);
    return device->bondingState;
}

Adapter *binc_device_get_adapter(const Device *device) {
    g_assert(device != NULL);
    return device->adapter;
}

guint binc_device_get_mtu(const Device *device) {
    g_assert(device != NULL);
    return device->mtu;
}

gboolean binc_device_has_service(const Device *device, const char *service_uuid) {
    g_assert(device != NULL);
    g_assert(g_uuid_string_is_valid(service_uuid));

    if (device->uuids != NULL && g_list_length(device->uuids) > 0) {
        for (GList *iterator = device->uuids; iterator; iterator = iterator->next) {
            if (g_str_equal(service_uuid, (char *) iterator->data)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void binc_internal_device_update_property(Device *device, const char *property_name, GVariant *property_value) {
    if (g_str_equal(property_name, DEVICE_PROPERTY_ADDRESS)) {
        binc_device_set_address(device, g_variant_get_string(property_value, NULL));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_ADDRESS_TYPE)) {
        binc_device_set_address_type(device, g_variant_get_string(property_value, NULL));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_ALIAS)) {
        binc_device_set_alias(device, g_variant_get_string(property_value, NULL));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_CONNECTED)) {
        binc_device_internal_set_conn_state(device, g_variant_get_boolean(property_value) ? BINC_CONNECTED : BINC_DISCONNECTED,
                                            NULL);
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_NAME)) {
        binc_device_set_name(device, g_variant_get_string(property_value, NULL));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_PAIRED)) {
        binc_device_set_paired(device, g_variant_get_boolean(property_value));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_RSSI)) {
        binc_device_set_rssi(device, g_variant_get_int16(property_value));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_TRUSTED)) {
        binc_device_set_trusted(device, g_variant_get_boolean(property_value));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_TXPOWER)) {
        binc_device_set_txpower(device, g_variant_get_int16(property_value));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_UUIDS)) {
        binc_device_set_uuids(device, g_variant_string_array_to_list(property_value));
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_MANUFACTURER_DATA)) {
        GVariantIter *iter;
        g_variant_get(property_value, "a{qv}", &iter);

        GVariant *array;
        guint16 key;
        GHashTable *manufacturer_data = g_hash_table_new_full(g_int_hash, g_int_equal,
                                                              g_free, (GDestroyNotify) byte_array_free);
        while (g_variant_iter_loop(iter, "{qv}", &key, &array)) {
            size_t data_length = 0;
            guint8 *data = (guint8 *) g_variant_get_fixed_array(array, &data_length, sizeof(guint8));
            GByteArray *byteArray = g_byte_array_sized_new(data_length);
            g_byte_array_append(byteArray, data, data_length);

            int *keyCopy = g_new0 (gint, 1);
            *keyCopy = key;

            g_hash_table_insert(manufacturer_data, keyCopy, byteArray);
        }
        binc_device_set_manufacturer_data(device, manufacturer_data);
        g_variant_iter_free(iter);
    } else if (g_str_equal(property_name, DEVICE_PROPERTY_SERVICE_DATA)) {
        GVariantIter *iter;
        g_variant_get(property_value, "a{sv}", &iter);

        GVariant *array;
        char *key;

        GHashTable *service_data = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                         g_free, (GDestroyNotify) byte_array_free);
        while (g_variant_iter_loop(iter, "{sv}", &key, &array)) {
            size_t data_length = 0;
            guint8 *data = (guint8 *) g_variant_get_fixed_array(array, &data_length, sizeof(guint8));
            GByteArray *byteArray = g_byte_array_sized_new(data_length);
            g_byte_array_append(byteArray, data, data_length);

            char *keyCopy = g_strdup(key);

            g_hash_table_insert(service_data, keyCopy, byteArray);
        }
        binc_device_set_service_data(device, service_data);
        g_variant_iter_free(iter);
    }
}

void binc_device_set_user_data(Device *device, void *user_data) {
    g_assert(device != NULL);
    device->user_data = user_data;
}

void *binc_device_get_user_data(const Device *device) {
    g_assert(device != NULL);
    return device->user_data;
}

