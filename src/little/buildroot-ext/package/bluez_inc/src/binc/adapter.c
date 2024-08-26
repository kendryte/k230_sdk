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

#include "adapter.h"
#include "device.h"
#include "device_internal.h"
#include "logger.h"
#include "utility.h"
#include "advertisement.h"
#include "application.h"

static const char *const TAG = "Adapter";
static const char *const BLUEZ_DBUS = "org.bluez";
static const char *const INTERFACE_ADAPTER = "org.bluez.Adapter1";
static const char *const INTERFACE_DEVICE = "org.bluez.Device1";
static const char *const INTERFACE_OBJECT_MANAGER = "org.freedesktop.DBus.ObjectManager";
static const char *const INTERFACE_GATT_MANAGER = "org.bluez.GattManager1";
static const char *const INTERFACE_PROPERTIES = "org.freedesktop.DBus.Properties";

static const char *const METHOD_START_DISCOVERY = "StartDiscovery";
static const char *const METHOD_STOP_DISCOVERY = "StopDiscovery";
static const char *const METHOD_REMOVE_DEVICE = "RemoveDevice";
static const char *const METHOD_SET_DISCOVERY_FILTER = "SetDiscoveryFilter";

static const char *const ADAPTER_PROPERTY_POWERED = "Powered";
static const char *const ADAPTER_PROPERTY_DISCOVERING = "Discovering";
static const char *const ADAPTER_PROPERTY_ADDRESS = "Address";
static const char *const ADAPTER_PROPERTY_DISCOVERABLE = "Discoverable";

static const char *const DEVICE_PROPERTY_RSSI = "RSSI";
static const char *const DEVICE_PROPERTY_UUIDS = "UUIDs";
static const char *const DEVICE_PROPERTY_MANUFACTURER_DATA = "ManufacturerData";
static const char *const DEVICE_PROPERTY_SERVICE_DATA = "ServiceData";

static const char *const SIGNAL_PROPERTIES_CHANGED = "PropertiesChanged";

static const guint MAC_ADDRESS_LENGTH = 17;

static const char *discovery_state_names[] = {
        [BINC_DISCOVERY_STOPPED] = "stopped",
        [BINC_DISCOVERY_STARTED] = "started",
        [BINC_DISCOVERY_STARTING]  = "starting",
        [BINC_DISCOVERY_STOPPING]  = "stopping"
};

typedef struct binc_discovery_filter {
    short rssi;
    GPtrArray *services;
    const char *pattern;
} DiscoveryFilter;

struct binc_adapter {
    const char *path; // Owned
    const char *address; // Owned
    gboolean powered;
    gboolean discoverable;
    gboolean discovering;
    DiscoveryState discovery_state;
    DiscoveryFilter discovery_filter;

    GDBusConnection *connection;  // Borrowed
    guint device_prop_changed;
    guint adapter_prop_changed;
    guint iface_added;
    guint iface_removed;

    AdapterDiscoveryResultCallback discoveryResultCallback;
    AdapterDiscoveryStateChangeCallback discoveryStateCallback;
    AdapterPoweredStateChangeCallback poweredStateCallback;
    RemoteCentralConnectionStateCallback centralStateCallback;
    void *user_data; // Borrowed
    GHashTable *devices_cache; // Owned

    Advertisement *advertisement; // Borrowed
};

static void remove_signal_subscribers(Adapter *adapter) {
    g_assert(adapter != NULL);

    g_dbus_connection_signal_unsubscribe(adapter->connection, adapter->device_prop_changed);
    adapter->device_prop_changed = 0;
    g_dbus_connection_signal_unsubscribe(adapter->connection, adapter->adapter_prop_changed);
    adapter->device_prop_changed = 0;
    g_dbus_connection_signal_unsubscribe(adapter->connection, adapter->iface_added);
    adapter->iface_added = 0;
    g_dbus_connection_signal_unsubscribe(adapter->connection, adapter->iface_removed);
    adapter->iface_removed = 0;
}

static void free_discovery_filter(Adapter *adapter) {
    g_assert(adapter != NULL);

    for (guint i = 0; i < adapter->discovery_filter.services->len; i++) {
        char *uuid_filter = g_ptr_array_index(adapter->discovery_filter.services, i);
        g_free(uuid_filter);
    }
    g_ptr_array_free(adapter->discovery_filter.services, TRUE);
    adapter->discovery_filter.services = NULL;

    g_free((char *) adapter->discovery_filter.pattern);
    adapter->discovery_filter.pattern = NULL;
}

void binc_adapter_free(Adapter *adapter) {
    g_assert(adapter != NULL);

    remove_signal_subscribers(adapter);

    if (adapter->discovery_filter.services != NULL) {
        free_discovery_filter(adapter);
        adapter->discovery_filter.services = NULL;
    }

    if (adapter->devices_cache != NULL) {
        g_hash_table_destroy(adapter->devices_cache);
        adapter->devices_cache = NULL;
    }

    g_free((char *) adapter->path);
    adapter->path = NULL;

    g_free((char *) adapter->address);
    adapter->address = NULL;

    adapter->connection = NULL;
    g_free(adapter);
}

static void binc_internal_adapter_call_method_cb(__attribute__((unused)) GObject *source_object,
                                                 GAsyncResult *res,
                                                 gpointer user_data) {
    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to call adapter method (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    }
}

static void binc_internal_adapter_call_method(Adapter *adapter, const char *method, GVariant *parameters) {
    g_assert(adapter != NULL);
    g_assert(method != NULL);

    g_dbus_connection_call(adapter->connection,
                           BLUEZ_DBUS,
                           adapter->path,
                           INTERFACE_ADAPTER,
                           method,
                           parameters,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_adapter_call_method_cb,
                           adapter);
}

static void binc_internal_set_discovery_state(Adapter *adapter, DiscoveryState discovery_state) {
    g_assert(adapter != NULL);
    if (adapter->discovery_state == discovery_state) return;

    adapter->discovery_state = discovery_state;
    if (adapter->discoveryStateCallback != NULL) {
        adapter->discoveryStateCallback(adapter, adapter->discovery_state, NULL);
    }
}

static void binc_internal_adapter_changed(__attribute__((unused)) GDBusConnection *conn,
                                          __attribute__((unused)) const gchar *sender,
                                          __attribute__((unused)) const gchar *path,
                                          __attribute__((unused)) const gchar *interface,
                                          __attribute__((unused)) const gchar *signal,
                                          GVariant *parameters,
                                          void *user_data) {

    GVariantIter *properties_changed = NULL;
    GVariantIter *properties_invalidated = NULL;
    const char *iface = NULL;
    const char *property_name = NULL;
    GVariant *property_value = NULL;

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    g_assert(g_str_equal(g_variant_get_type_string(parameters), "(sa{sv}as)"));
    g_variant_get(parameters, "(&sa{sv}as)", &iface, &properties_changed, &properties_invalidated);
    while (g_variant_iter_loop(properties_changed, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, ADAPTER_PROPERTY_POWERED)) {
            adapter->powered = g_variant_get_boolean(property_value);
            if (adapter->poweredStateCallback != NULL) {
                adapter->poweredStateCallback(adapter, adapter->powered);
            }
        } else if (g_str_equal(property_name, ADAPTER_PROPERTY_DISCOVERING)) {
            adapter->discovering = g_variant_get_boolean(property_value);

            // It could be that some other app is causing discovery to be stopped, e.g. power off
            if (adapter->discovering == FALSE) {
                // Update discovery state to reflect discovery state
                binc_internal_set_discovery_state(adapter, BINC_DISCOVERY_STOPPED);
            }
        } else if (g_str_equal(property_name, ADAPTER_PROPERTY_DISCOVERABLE)) {
            adapter->discoverable = g_variant_get_boolean(property_value);
        }
    }

    if (properties_changed != NULL)
        g_variant_iter_free(properties_changed);

    if (properties_invalidated != NULL)
        g_variant_iter_free(properties_invalidated);
}

static gboolean matches_discovery_filter(Adapter *adapter, Device *device) {
    g_assert(adapter != NULL);
    g_assert(device != NULL);

    if (binc_device_get_rssi(device) < adapter->discovery_filter.rssi) return FALSE;

    const char *pattern = adapter->discovery_filter.pattern;
    if (pattern != NULL) {
        if (!(g_str_has_prefix(binc_device_get_name(device), pattern) ||
              g_str_has_prefix(binc_device_get_address(device), pattern)))
            return FALSE;
    }

    GPtrArray *services_filter = adapter->discovery_filter.services;
    if (services_filter != NULL) {
        guint count = services_filter->len;
        if (count == 0) return TRUE;

        for (guint i = 0; i < count; i++) {
            const char *uuid_filter = g_ptr_array_index(services_filter, i);
            if (binc_device_has_service(device, uuid_filter)) {
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

static void deliver_discovery_result(Adapter *adapter, Device *device) {
    g_assert(adapter != NULL);
    g_assert(device != NULL);

    if (binc_device_get_connection_state(device) == BINC_DISCONNECTED) {
        // Double check if the device matches the discovery filter
        if (!matches_discovery_filter(adapter, device)) return;

        if (adapter->discoveryResultCallback != NULL) {
            adapter->discoveryResultCallback(adapter, device);
        }
    }
}

static void binc_internal_device_disappeared(__attribute__((unused)) GDBusConnection *conn,
                                             __attribute__((unused)) const gchar *sender_name,
                                             __attribute__((unused)) const gchar *object_path,
                                             __attribute__((unused)) const gchar *interface,
                                             __attribute__((unused)) const gchar *signal_name,
                                             GVariant *parameters,
                                             gpointer user_data) {

    GVariantIter *interfaces = NULL;
    const char *object = NULL;
    const char *interface_name = NULL;

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    g_assert(g_str_equal(g_variant_get_type_string(parameters), "(oas)"));
    g_variant_get(parameters, "(&oas)", &object, &interfaces);
    while (g_variant_iter_loop(interfaces, "s", &interface_name)) {
        if (g_str_equal(interface_name, INTERFACE_DEVICE)) {
            log_debug(TAG, "Device %s removed", object);
            if (g_hash_table_lookup(adapter->devices_cache, object) != NULL) {
                g_hash_table_remove(adapter->devices_cache, object);
            }
        }
    }

    if (interfaces != NULL)
        g_variant_iter_free(interfaces);
}

static void binc_internal_device_appeared(__attribute__((unused)) GDBusConnection *conn,
                                          __attribute__((unused)) const gchar *sender_name,
                                          __attribute__((unused)) const gchar *object_path,
                                          __attribute__((unused)) const gchar *interface,
                                          __attribute__((unused)) const gchar *signal_name,
                                          GVariant *parameters,
                                          gpointer user_data) {

    GVariantIter *interfaces = NULL;
    const char *object = NULL;
    const char *interface_name = NULL;
    GVariant *properties = NULL;

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    g_assert(g_str_equal(g_variant_get_type_string(parameters), "(oa{sa{sv}})"));
    g_variant_get(parameters, "(&oa{sa{sv}})", &object, &interfaces);
    while (g_variant_iter_loop(interfaces, "{&s@a{sv}}", &interface_name, &properties)) {
        if (g_str_equal(interface_name, INTERFACE_DEVICE)) {
            Device *device = binc_device_create(object, adapter);

            char *property_name = NULL;
            GVariantIter iter;
            GVariant *property_value = NULL;
            g_variant_iter_init(&iter, properties);
            while (g_variant_iter_loop(&iter, "{&sv}", &property_name, &property_value)) {
                binc_internal_device_update_property(device, property_name, property_value);
            }

            g_hash_table_insert(adapter->devices_cache,
                                g_strdup(binc_device_get_path(device)),
                                device);

            if (adapter->discovery_state == BINC_DISCOVERY_STARTED && binc_device_get_connection_state(device) == BINC_DISCONNECTED) {
                deliver_discovery_result(adapter, device);
            }

            if (binc_device_get_connection_state(device) == BINC_CONNECTED &&
                binc_device_get_rssi(device) == -255 &&
                binc_device_get_uuids(device) == NULL) {
                binc_device_set_is_central(device, TRUE);
                if (adapter->centralStateCallback != NULL) {
                    adapter->centralStateCallback(adapter, device);
                }
            }
        }
    }

    if (interfaces != NULL)
        g_variant_iter_free(interfaces);
}

static void binc_internal_device_getall_properties_cb(__attribute__((unused)) GObject *source_object,
                                                      GAsyncResult *res,
                                                      gpointer user_data) {

    Device *device = (Device *) user_data;
    g_assert(device != NULL);

    GError *error = NULL;
    GVariant *result = g_dbus_connection_call_finish(binc_device_get_dbus_connection(device), res, &error);

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", "GetAll", error->code, error->message);
        g_clear_error(&error);
    }

    if (result != NULL) {
        GVariantIter *iter = NULL;
        const char *property_name = NULL;
        GVariant *property_value = NULL;

        g_assert(g_str_equal(g_variant_get_type_string(result), "(a{sv})"));
        g_variant_get(result, "(a{sv})", &iter);
        while (g_variant_iter_loop(iter, "{&sv}", &property_name, &property_value)) {
            binc_internal_device_update_property(device, property_name, property_value);
        }

        if (iter != NULL) {
            g_variant_iter_free(iter);
        }
        g_variant_unref(result);
    }
}

static void binc_internal_device_getall_properties(Adapter *adapter, Device *device) {
    g_dbus_connection_call(adapter->connection,
                           BLUEZ_DBUS,
                           binc_device_get_path(device),
                           INTERFACE_PROPERTIES,
                           "GetAll",
                           g_variant_new("(s)", INTERFACE_DEVICE),
                           G_VARIANT_TYPE("(a{sv})"),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_device_getall_properties_cb,
                           device);
}


static void binc_internal_device_changed(__attribute__((unused)) GDBusConnection *conn,
                                         __attribute__((unused)) const gchar *sender,
                                         const gchar *path,
                                         __attribute__((unused)) const gchar *interface,
                                         __attribute__((unused)) const gchar *signal,
                                         GVariant *parameters,
                                         void *user_data) {

    GVariantIter *properties_changed = NULL;
    GVariantIter *properties_invalidated = NULL;
    const char *iface = NULL;
    const char *property_name = NULL;
    GVariant *property_value = NULL;

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    Device *device = g_hash_table_lookup(adapter->devices_cache, path);
    if (device == NULL) {
        device = binc_device_create(path, adapter);
        g_hash_table_insert(adapter->devices_cache, g_strdup(binc_device_get_path(device)), device);
        binc_internal_device_getall_properties(adapter, device);
    } else {
        gboolean isDiscoveryResult = FALSE;
        ConnectionState oldState = binc_device_get_connection_state(device);
        g_assert(g_str_equal(g_variant_get_type_string(parameters), "(sa{sv}as)"));
        g_variant_get(parameters, "(&sa{sv}as)", &iface, &properties_changed, &properties_invalidated);
        while (g_variant_iter_loop(properties_changed, "{&sv}", &property_name, &property_value)) {
            binc_internal_device_update_property(device, property_name, property_value);
            if (g_str_equal(property_name, DEVICE_PROPERTY_RSSI) ||
                g_str_equal(property_name, DEVICE_PROPERTY_MANUFACTURER_DATA) ||
                g_str_equal(property_name, DEVICE_PROPERTY_SERVICE_DATA)) {
                isDiscoveryResult = TRUE;
            }
        }
        if (adapter->discovery_state == BINC_DISCOVERY_STARTED && isDiscoveryResult) {
            deliver_discovery_result(adapter, device);
        }

        if (binc_device_get_bonding_state(device) == BINC_BONDED && binc_device_get_rssi(device) == -255) {
            binc_device_set_is_central(device, TRUE);
        }

        if (binc_device_is_central(device)) {
            ConnectionState newState = binc_device_get_connection_state(device);
            if (oldState != newState) {
                if (adapter->centralStateCallback != NULL) {
                    adapter->centralStateCallback(adapter, device);
                }
            }
        }
    }

    if (properties_changed != NULL)
        g_variant_iter_free(properties_changed);

    if (properties_invalidated != NULL)
        g_variant_iter_free(properties_invalidated);
}

static void setup_signal_subscribers(Adapter *adapter) {
    adapter->device_prop_changed = g_dbus_connection_signal_subscribe(adapter->connection,
                                                                      BLUEZ_DBUS,
                                                                      INTERFACE_PROPERTIES,
                                                                      SIGNAL_PROPERTIES_CHANGED,
                                                                      NULL,
                                                                      INTERFACE_DEVICE,
                                                                      G_DBUS_SIGNAL_FLAGS_NONE,
                                                                      binc_internal_device_changed,
                                                                      adapter,
                                                                      NULL);

    adapter->adapter_prop_changed = g_dbus_connection_signal_subscribe(adapter->connection,
                                                                       BLUEZ_DBUS,
                                                                       INTERFACE_PROPERTIES,
                                                                       SIGNAL_PROPERTIES_CHANGED,
                                                                       adapter->path,
                                                                       INTERFACE_ADAPTER,
                                                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                                                       binc_internal_adapter_changed,
                                                                       adapter,
                                                                       NULL);

    adapter->iface_added = g_dbus_connection_signal_subscribe(adapter->connection,
                                                              BLUEZ_DBUS,
                                                              INTERFACE_OBJECT_MANAGER,
                                                              "InterfacesAdded",
                                                              NULL,
                                                              NULL,
                                                              G_DBUS_SIGNAL_FLAGS_NONE,
                                                              binc_internal_device_appeared,
                                                              adapter,
                                                              NULL);

    adapter->iface_removed = g_dbus_connection_signal_subscribe(adapter->connection,
                                                                BLUEZ_DBUS,
                                                                INTERFACE_OBJECT_MANAGER,
                                                                "InterfacesRemoved",
                                                                NULL,
                                                                NULL,
                                                                G_DBUS_SIGNAL_FLAGS_NONE,
                                                                binc_internal_device_disappeared,
                                                                adapter,
                                                                NULL);
}

const char *binc_adapter_get_name(const Adapter *adapter) {
    g_assert(adapter != NULL);
    g_assert(adapter->path != NULL);
    return strrchr(adapter->path, '/') + 1;
}

const char *binc_adapter_get_address(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->address;
}

static Adapter *binc_adapter_create(GDBusConnection *connection, const char *path) {
    g_assert(connection != NULL);
    g_assert(path != NULL);
    g_assert(strlen(path) > 0);

    Adapter *adapter = g_new0(Adapter, 1);
    adapter->connection = connection;
    adapter->path = g_strdup(path);
    adapter->discovery_filter.rssi = -255;
    adapter->devices_cache = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                   g_free, (GDestroyNotify) binc_device_free);
    adapter->user_data = NULL;
    setup_signal_subscribers(adapter);
    return adapter;
}

static Adapter *binc_internal_get_adapter_by_path(GPtrArray *adapters, const char *path) {
    g_assert(adapters != NULL);
    g_assert(path != NULL);
    g_assert(strlen(path) > 0);

    for (guint i = 0; i < adapters->len; i++) {
        Adapter *adapter = g_ptr_array_index(adapters, i);
        const char *adapter_path = binc_adapter_get_path(adapter);
        if (g_str_has_prefix(path, adapter_path)) {
            return adapter;
        }
    }
    return NULL;
}

GPtrArray *binc_adapter_find_all(GDBusConnection *dbusConnection) {
    g_assert(dbusConnection != NULL);

    GPtrArray *binc_adapters = g_ptr_array_new();
    log_debug(TAG, "finding adapters");

    GError *error = NULL;
    GVariant *result = g_dbus_connection_call_sync(dbusConnection,
                                                   BLUEZ_DBUS,
                                                   "/",
                                                   INTERFACE_OBJECT_MANAGER,
                                                   "GetManagedObjects",
                                                   NULL,
                                                   G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
                                                   G_DBUS_CALL_FLAGS_NONE,
                                                   -1,
                                                   NULL,
                                                   &error);

    if (result) {
        GVariantIter *iter;
        const char *object_path;
        GVariant *ifaces_and_properties;

        g_assert(g_str_equal(g_variant_get_type_string(result), "(a{oa{sa{sv}}})"));
        g_variant_get(result, "(a{oa{sa{sv}}})", &iter);
        while (g_variant_iter_loop(iter, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties)) {
            const char *interface_name;
            GVariant *properties;
            GVariantIter iter2;

            g_variant_iter_init(&iter2, ifaces_and_properties);
            while (g_variant_iter_loop(&iter2, "{&s@a{sv}}", &interface_name, &properties)) {
                if (g_str_equal(interface_name, INTERFACE_ADAPTER)) {
                    Adapter *adapter = binc_adapter_create(dbusConnection, object_path);
                    char *property_name;
                    GVariantIter iter3;
                    GVariant *property_value;
                    g_variant_iter_init(&iter3, properties);
                    while (g_variant_iter_loop(&iter3, "{&sv}", &property_name, &property_value)) {
                        if (g_str_equal(property_name, ADAPTER_PROPERTY_ADDRESS)) {
                            adapter->address = g_strdup(g_variant_get_string(property_value, NULL));
                        } else if (g_str_equal(property_name, ADAPTER_PROPERTY_POWERED)) {
                            adapter->powered = g_variant_get_boolean(property_value);
                        } else if (g_str_equal(property_name, ADAPTER_PROPERTY_DISCOVERING)) {
                            adapter->discovering = g_variant_get_boolean(property_value);
                        } else if (g_str_equal(property_name, ADAPTER_PROPERTY_DISCOVERABLE)) {
                            adapter->discoverable = g_variant_get_boolean(property_value);
                        }
                    }
                    g_ptr_array_add(binc_adapters, adapter);
                } else if (g_str_equal(interface_name, INTERFACE_DEVICE)) {
                    Adapter *adapter = binc_internal_get_adapter_by_path(binc_adapters, object_path);
                    Device *device = binc_device_create(object_path, adapter);
                    g_hash_table_insert(adapter->devices_cache, g_strdup(binc_device_get_path(device)), device);

                    char *property_name;
                    GVariantIter iter4;
                    GVariant *property_value;
                    g_variant_iter_init(&iter4, properties);
                    while (g_variant_iter_loop(&iter4, "{&sv}", &property_name, &property_value)) {
                        binc_internal_device_update_property(device, property_name, property_value);
                    }
                    log_debug(TAG, "found device %s '%s'", object_path, binc_device_get_name(device));
                }
            }
        }

        if (iter != NULL) {
            g_variant_iter_free(iter);
        }
        g_variant_unref(result);
    }

    if (error != NULL) {
        log_error(TAG, "Error GetManagedObjects: %s", error->message);
        g_clear_error(&error);
    }

    log_debug(TAG, "found %d adapter%s", binc_adapters->len, binc_adapters->len > 1 ? "s" : "");
    return binc_adapters;
}

Adapter *binc_adapter_get_default(GDBusConnection *dbusConnection) {
    g_assert(dbusConnection != NULL);

    Adapter *adapter = NULL;
    GPtrArray *adapters = binc_adapter_find_all(dbusConnection);
    if (adapters->len > 0) {
        // Choose the first one in the array, typically the 'hciX' with the highest X
        adapter = g_ptr_array_index(adapters, 0);

        // Free any other adapters we are not going to use
        for (guint i = 1; i < adapters->len; i++) {
            binc_adapter_free(g_ptr_array_index(adapters, i));
        }
        g_ptr_array_free(adapters, TRUE);
    }
    return adapter;
}

Adapter *binc_adapter_get(GDBusConnection *dbusConnection, const char *name) {
    g_assert(dbusConnection != NULL);
    g_assert(name != NULL && strlen(name) > 0);

    Adapter *result = NULL;
    GPtrArray *adapters = binc_adapter_find_all(dbusConnection);
    if (adapters->len > 0) {
        for (guint i = 0; i < adapters->len; i++) {
            Adapter *adapter = g_ptr_array_index(adapters, i);
            if (g_str_equal(binc_adapter_get_name(adapter), name)) {
                result = adapter;
            } else {
                binc_adapter_free(g_ptr_array_index(adapters, i));
            }
        }
        g_ptr_array_free(adapters, TRUE);
    }
    return result;
}

static void binc_internal_start_discovery_cb(__attribute__((unused)) GObject *source_object,
                                             GAsyncResult *res,
                                             gpointer user_data) {

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", METHOD_START_DISCOVERY, error->code, error->message);
        adapter->discovery_state = BINC_DISCOVERY_STOPPED;
        if (adapter->discoveryStateCallback != NULL) {
            adapter->discoveryStateCallback(adapter, adapter->discovery_state, error);
        }
        g_clear_error(&error);
    } else {
        binc_internal_set_discovery_state(adapter, BINC_DISCOVERY_STARTED);
    }

    if (value != NULL) {
        g_variant_unref(value);
    }
}

void binc_adapter_start_discovery(Adapter *adapter) {
    g_assert (adapter != NULL);

    if (adapter->discovery_state == BINC_DISCOVERY_STOPPED) {
        binc_internal_set_discovery_state(adapter, BINC_DISCOVERY_STARTING);
        g_dbus_connection_call(adapter->connection,
                               BLUEZ_DBUS,
                               adapter->path,
                               INTERFACE_ADAPTER,
                               METHOD_START_DISCOVERY,
                               NULL,
                               NULL,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               (GAsyncReadyCallback) binc_internal_start_discovery_cb,
                               adapter);
    }
}

static void binc_internal_stop_discovery_cb(__attribute__((unused)) GObject *source_object,
                                            GAsyncResult *res,
                                            gpointer user_data) {
    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", METHOD_STOP_DISCOVERY, error->code, error->message);
        if (adapter->discoveryStateCallback != NULL) {
            adapter->discoveryStateCallback(adapter, adapter->discovery_state, error);
        }
        g_clear_error(&error);
    } else {
        binc_internal_set_discovery_state(adapter, BINC_DISCOVERY_STOPPED);
    }

    if (value != NULL)
        g_variant_unref(value);
}

void binc_adapter_stop_discovery(Adapter *adapter) {
    g_assert (adapter != NULL);

    if (adapter->discovery_state == BINC_DISCOVERY_STARTED) {
        binc_internal_set_discovery_state(adapter, BINC_DISCOVERY_STOPPING);
        g_dbus_connection_call(adapter->connection,
                               BLUEZ_DBUS,
                               adapter->path,
                               INTERFACE_ADAPTER,
                               METHOD_STOP_DISCOVERY,
                               NULL,
                               NULL,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               (GAsyncReadyCallback) binc_internal_stop_discovery_cb,
                               adapter);
    }
}

void binc_adapter_remove_device(Adapter *adapter, Device *device) {
    g_assert(device != NULL);
    g_assert (adapter != NULL);

    log_debug(TAG, "removing %s (%s)", binc_device_get_name(device), binc_device_get_address(device));
    binc_internal_adapter_call_method(adapter, METHOD_REMOVE_DEVICE,
                                      g_variant_new("(o)", binc_device_get_path(device)));
}

GList *binc_adapter_get_devices(const Adapter *adapter) {
    g_assert (adapter != NULL);
    return g_hash_table_get_values(adapter->devices_cache);
}

GList *binc_adapter_get_connected_devices(const Adapter *adapter) {
    g_assert (adapter != NULL);

    GList *all_devices = binc_adapter_get_devices(adapter);
    if (g_list_length(all_devices) <= 0)
        return all_devices;

    GList *result = NULL;
    for (GList *iterator = all_devices; iterator; iterator = iterator->next) {
        Device *device = (Device *) iterator->data;
        if (binc_device_get_connection_state(device) == BINC_CONNECTED) {
            result = g_list_append(result, device);
        }
    }

    g_list_free(all_devices);
    return result;
}

void binc_adapter_set_discovery_filter(Adapter *adapter, short rssi_threshold, const GPtrArray *service_uuids,
                                       const char *pattern) {
    g_assert(adapter != NULL);
    g_assert(rssi_threshold >= -127);
    g_assert(rssi_threshold <= 20);

    // Setup discovery filter so we can double-check the results later
    if (adapter->discovery_filter.services != NULL) {
        free_discovery_filter(adapter);
    }
    adapter->discovery_filter.services = g_ptr_array_new();
    adapter->discovery_filter.rssi = rssi_threshold;
    adapter->discovery_filter.pattern = g_strdup(pattern);

    GVariantBuilder *arguments = g_variant_builder_new(G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(arguments, "{sv}", "Transport", g_variant_new_string("le"));
    g_variant_builder_add(arguments, "{sv}", DEVICE_PROPERTY_RSSI, g_variant_new_int16(rssi_threshold));
    g_variant_builder_add(arguments, "{sv}", "DuplicateData", g_variant_new_boolean(TRUE));

    if (pattern != NULL) {
        g_variant_builder_add(arguments, "{sv}", "Pattern", g_variant_new_string(pattern));
    }

    if (service_uuids != NULL && service_uuids->len > 0) {
        GVariantBuilder *uuids = g_variant_builder_new(G_VARIANT_TYPE_STRING_ARRAY);
        for (guint i = 0; i < service_uuids->len; i++) {
            char *uuid = g_ptr_array_index(service_uuids, i);
            g_assert(g_uuid_string_is_valid(uuid));
            g_variant_builder_add(uuids, "s", uuid);
            g_ptr_array_add(adapter->discovery_filter.services, g_strdup(uuid));
        }
        g_variant_builder_add(arguments, "{sv}", DEVICE_PROPERTY_UUIDS, g_variant_builder_end(uuids));
        g_variant_builder_unref(uuids);
    }

    GVariant *filter = g_variant_builder_end(arguments);
    g_variant_builder_unref(arguments);
    binc_internal_adapter_call_method(adapter, METHOD_SET_DISCOVERY_FILTER, g_variant_new_tuple(&filter, 1));
}

static void binc_internal_set_property_cb(__attribute__((unused)) GObject *source_object,
                                          GAsyncResult *res,
                                          gpointer user_data) {
    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to set adapter property (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    }
}

static void adapter_set_property(Adapter *adapter, const char *property, GVariant *value) {
    g_assert(adapter != NULL);
    g_assert(property != NULL);
    g_assert(value != NULL);

    g_dbus_connection_call(adapter->connection,
                           BLUEZ_DBUS,
                           adapter->path,
                           INTERFACE_PROPERTIES,
                           "Set",
                           g_variant_new("(ssv)", INTERFACE_ADAPTER, property, value),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_set_property_cb,
                           adapter);
}

void binc_adapter_power_on(Adapter *adapter) {
    g_assert(adapter != NULL);

    adapter_set_property(adapter, ADAPTER_PROPERTY_POWERED, g_variant_new("b", TRUE));
}

void binc_adapter_power_off(Adapter *adapter) {
    g_assert(adapter != NULL);

    adapter_set_property(adapter, ADAPTER_PROPERTY_POWERED, g_variant_new("b", FALSE));
}

void binc_adapter_discoverable_on(Adapter *adapter) {
    g_assert(adapter != NULL);

    adapter_set_property(adapter, ADAPTER_PROPERTY_DISCOVERABLE, g_variant_new("b", TRUE));
}

void binc_adapter_discoverable_off(Adapter *adapter) {
    g_assert(adapter != NULL);

    adapter_set_property(adapter, ADAPTER_PROPERTY_DISCOVERABLE, g_variant_new("b", FALSE));
}


void binc_adapter_set_discovery_cb(Adapter *adapter, AdapterDiscoveryResultCallback callback) {
    g_assert(adapter != NULL);
    g_assert(callback != NULL);

    adapter->discoveryResultCallback = callback;
}

void binc_adapter_set_discovery_state_cb(Adapter *adapter, AdapterDiscoveryStateChangeCallback callback) {
    g_assert(adapter != NULL);
    g_assert(callback != NULL);

    adapter->discoveryStateCallback = callback;
}

void binc_adapter_set_powered_state_cb(Adapter *adapter, AdapterPoweredStateChangeCallback callback) {
    g_assert(adapter != NULL);
    g_assert(callback != NULL);

    adapter->poweredStateCallback = callback;
}

const char *binc_adapter_get_path(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->path;
}

DiscoveryState binc_adapter_get_discovery_state(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->discovery_state;
}

gboolean binc_adapter_get_powered_state(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->powered;
}

gboolean binc_adapter_is_discoverable(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->discoverable;
}

Device *binc_adapter_get_device_by_path(const Adapter *adapter, const char *path) {
    g_assert(adapter != NULL);
    return g_hash_table_lookup(adapter->devices_cache, path);
}

Device *binc_adapter_get_device_by_address(const Adapter *adapter, const char *address) {
    g_assert(adapter != NULL);
    g_assert(address != NULL);
    g_assert(strlen(address) == MAC_ADDRESS_LENGTH);

    char *path = g_strdup_printf("%s/dev_%s", adapter->path, address);
    path = replace_char(path, ':', '_');
    Device *device = g_hash_table_lookup(adapter->devices_cache, path);
    g_free(path);
    return device;
}

GDBusConnection *binc_adapter_get_dbus_connection(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->connection;
}

const char *binc_adapter_get_discovery_state_name(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return discovery_state_names[adapter->discovery_state];
}

static void binc_internal_start_advertising_cb(__attribute__((unused)) GObject *source_object,
                                               GAsyncResult *res,
                                               gpointer user_data) {
    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to register advertisement (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    } else {
        log_debug(TAG, "started advertising (%s)", adapter->address);
    }
}

void binc_adapter_start_advertising(Adapter *adapter, Advertisement *advertisement) {
    g_assert(adapter != NULL);
    g_assert(advertisement != NULL);

    adapter->advertisement = advertisement;
    binc_advertisement_register(advertisement, adapter);

    g_dbus_connection_call(binc_adapter_get_dbus_connection(adapter),
                           "org.bluez",
                           adapter->path,
                           "org.bluez.LEAdvertisingManager1",
                           "RegisterAdvertisement",
                           g_variant_new("(oa{sv})", binc_advertisement_get_path(advertisement), NULL),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_start_advertising_cb, adapter);
}

static void binc_internal_stop_advertising_cb(__attribute__((unused)) GObject *source_object,
                                              GAsyncResult *res,
                                              gpointer user_data) {

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to unregister advertisement (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    } else {
        binc_advertisement_unregister(adapter->advertisement, adapter);
        log_debug(TAG, "stopped advertising");
    }

}

void binc_adapter_stop_advertising(Adapter *adapter, Advertisement *advertisement) {
    g_assert(adapter != NULL);
    g_assert(advertisement != NULL);

    g_dbus_connection_call(binc_adapter_get_dbus_connection(adapter),
                           "org.bluez",
                           adapter->path,
                           "org.bluez.LEAdvertisingManager1",
                           "UnregisterAdvertisement",
                           g_variant_new("(o)", binc_advertisement_get_path(advertisement)),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_stop_advertising_cb, adapter);
}

static void binc_internal_register_appl_cb(__attribute__((unused)) GObject *source_object,
                                           GAsyncResult *res,
                                           gpointer user_data) {
    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to register application (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    } else {
        log_debug(TAG, "successfully registered application");
    }
}

void binc_adapter_register_application(Adapter *adapter, Application *application) {
    g_assert(adapter != NULL);
    g_assert(application != NULL);

    g_dbus_connection_call(binc_adapter_get_dbus_connection(adapter),
                           BLUEZ_DBUS,
                           adapter->path,
                           INTERFACE_GATT_MANAGER,
                           "RegisterApplication",
                           g_variant_new("(oa{sv})", binc_application_get_path(application), NULL),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_register_appl_cb, adapter);

}

static void binc_internal_unregister_appl_cb(__attribute__((unused)) GObject *source_object,
                                             GAsyncResult *res,
                                             gpointer user_data) {

    Adapter *adapter = (Adapter *) user_data;
    g_assert(adapter != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(adapter->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to unregister application (error %d: %s)", error->code, error->message);
        g_clear_error(&error);
    } else {
        log_debug(TAG, "successfully unregistered application");
    }
}

void binc_adapter_unregister_application(Adapter *adapter, Application *application) {
    g_assert(adapter != NULL);
    g_assert(application != NULL);

    g_dbus_connection_call(binc_adapter_get_dbus_connection(adapter),
                           BLUEZ_DBUS,
                           adapter->path,
                           INTERFACE_GATT_MANAGER,
                           "UnregisterApplication",
                           g_variant_new("(o)", binc_application_get_path(application)),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_unregister_appl_cb, adapter);

}

void binc_adapter_set_remote_central_cb(Adapter *adapter, RemoteCentralConnectionStateCallback callback) {
    g_assert(adapter != NULL);
    adapter->centralStateCallback = callback;
}

void binc_adapter_set_user_data(Adapter *adapter, void *user_data) {
    g_assert(adapter != NULL);
    adapter->user_data = user_data;
}

void *binc_adapter_get_user_data(const Adapter *adapter) {
    g_assert(adapter != NULL);
    return adapter->user_data;
}
