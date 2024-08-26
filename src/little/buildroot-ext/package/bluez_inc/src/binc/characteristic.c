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

#include "characteristic.h"
#include "logger.h"
#include "utility.h"
#include "device_internal.h"

static const char *const TAG = "Characteristic";
static const char *const INTERFACE_CHARACTERISTIC = "org.bluez.GattCharacteristic1";
static const char *const BLUEZ_DBUS = "org.bluez";

static const char *const CHARACTERISTIC_METHOD_READ_VALUE = "ReadValue";
static const char *const CHARACTERISTIC_METHOD_WRITE_VALUE = "WriteValue";
static const char *const CHARACTERISTIC_METHOD_STOP_NOTIFY = "StopNotify";
static const char *const CHARACTERISTIC_METHOD_START_NOTIFY = "StartNotify";
static const char *const CHARACTERISTIC_PROPERTY_NOTIFYING = "Notifying";
static const char *const CHARACTERISTIC_PROPERTY_VALUE = "Value";

typedef struct binc_write_data {
    GVariant *value;
    Characteristic *characteristic;
} WriteData;

struct binc_characteristic {
    Device *device; // Borrowed
    Service *service; // Borrowed
    GDBusConnection *connection; // Borrowed
    const char *path; // Owned
    const char *uuid; // Owned
    const char *service_path; // Owned
    gboolean notifying;
    GList *flags; // Owned
    guint properties;
    GList *descriptors; // Owned
    guint mtu;

    guint characteristic_prop_changed;
    OnNotifyingStateChangedCallback notify_state_callback;
    OnReadCallback on_read_callback;
    OnWriteCallback on_write_callback;
    OnNotifyCallback on_notify_callback;
};

Characteristic *binc_characteristic_create(Device *device, const char *path) {
    g_assert(device != NULL);
    g_assert(path != NULL);
    g_assert(strlen(path) > 0);

    Characteristic *characteristic = g_new0(Characteristic, 1);
    characteristic->device = device;
    characteristic->connection = binc_device_get_dbus_connection(device);
    characteristic->path = g_strdup(path);
    characteristic->mtu = 23;
    return characteristic;
}

void binc_characteristic_free(Characteristic *characteristic) {
    g_assert(characteristic != NULL);

    if (characteristic->characteristic_prop_changed != 0) {
        g_dbus_connection_signal_unsubscribe(characteristic->connection, characteristic->characteristic_prop_changed);
        characteristic->characteristic_prop_changed = 0;
    }

    if (characteristic->flags != NULL) {
        g_list_free_full(characteristic->flags, g_free);
        characteristic->flags = NULL;
    }

    if (characteristic->descriptors != NULL) {
        g_list_free(characteristic->descriptors);
        characteristic->descriptors = NULL;
    }

    g_free((char *) characteristic->uuid);
    characteristic->uuid = NULL;

    g_free((char *) characteristic->path);
    characteristic->path = NULL;

    g_free((char *) characteristic->service_path);
    characteristic->service_path = NULL;

    characteristic->device = NULL;
    characteristic->connection = NULL;
    characteristic->service = NULL;
    g_free(characteristic);
}

char *binc_characteristic_to_string(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);

    GString *flags = g_string_new("[");
    if (g_list_length(characteristic->flags) > 0) {
        for (GList *iterator = characteristic->flags; iterator; iterator = iterator->next) {
            g_string_append_printf(flags, "%s, ", (char *) iterator->data);
        }
        g_string_truncate(flags, flags->len - 2);
    }
    g_string_append(flags, "]");

    char *result = g_strdup_printf(
            "Characteristic{uuid='%s', flags='%s', properties=%d, service_uuid='%s, mtu=%d'}",
            characteristic->uuid,
            flags->str,
            characteristic->properties,
            binc_service_get_uuid(characteristic->service),
            characteristic->mtu);

    g_string_free(flags, TRUE);
    return result;
}

static void binc_internal_char_read_cb(__attribute__((unused)) GObject *source_object,
                                       GAsyncResult *res,
                                       gpointer user_data) {
    GError *error = NULL;
    GByteArray *byteArray = NULL;
    GVariant *innerArray = NULL;
    Characteristic *characteristic = (Characteristic *) user_data;
    g_assert(characteristic != NULL);

    GVariant *value = g_dbus_connection_call_finish(characteristic->connection, res, &error);
    if (value != NULL) {
        g_assert(g_str_equal(g_variant_get_type_string(value), "(ay)"));
        innerArray = g_variant_get_child_value(value, 0);
        byteArray = g_variant_get_byte_array(innerArray);
    }

    if (characteristic->on_read_callback != NULL) {
        characteristic->on_read_callback(characteristic->device, characteristic, byteArray, error);
    }

    if (byteArray != NULL) {
        g_byte_array_free(byteArray, FALSE);
    }

    if (innerArray != NULL) {
        g_variant_unref(innerArray);
    }

    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", CHARACTERISTIC_METHOD_READ_VALUE, error->code,
                  error->message);
        g_clear_error(&error);
    }
}

void binc_characteristic_read(Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    g_assert((characteristic->properties & GATT_CHR_PROP_READ) > 0);

    log_debug(TAG, "reading <%s>", characteristic->uuid);

    guint16 offset = 0;
    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(builder, "{sv}", "offset", g_variant_new_uint16(offset));
    GVariant *options = g_variant_builder_end(builder);
    g_variant_builder_unref(builder);

    g_dbus_connection_call(characteristic->connection,
                           BLUEZ_DBUS,
                           characteristic->path,
                           INTERFACE_CHARACTERISTIC,
                           CHARACTERISTIC_METHOD_READ_VALUE,
                           g_variant_new("(@a{sv})", options),
                           G_VARIANT_TYPE("(ay)"),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_char_read_cb,
                           characteristic);
}

static void binc_internal_char_write_cb(__attribute__((unused)) GObject *source_object,
                                        GAsyncResult *res,
                                        gpointer user_data) {
    WriteData *writeData = (WriteData*) user_data;
    Characteristic *characteristic = writeData->characteristic;
    g_assert(characteristic != NULL);

    GByteArray *byteArray = NULL;
    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(characteristic->connection, res, &error);

    if (writeData->value != NULL) {
        byteArray = g_variant_get_byte_array(writeData->value);
    }

    if (characteristic->on_write_callback != NULL) {
        characteristic->on_write_callback(characteristic->device, characteristic, byteArray, error);
    }

    if (byteArray != NULL) {
        g_byte_array_free(byteArray, FALSE);
    }
    g_variant_unref(writeData->value);
    g_free(writeData);

    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", CHARACTERISTIC_METHOD_WRITE_VALUE,
                  error->code, error->message);
        g_clear_error(&error);
    }
}

void binc_characteristic_write(Characteristic *characteristic, const GByteArray *byteArray, WriteType writeType) {
    g_assert(characteristic != NULL);
    g_assert(byteArray != NULL);
    g_assert(byteArray->len > 0);
    g_assert(binc_characteristic_supports_write(characteristic, writeType));

    GString *byteArrayStr = g_byte_array_as_hex(byteArray);
    log_debug(TAG, "writing <%s> to <%s>", byteArrayStr->str, characteristic->uuid);
    g_string_free(byteArrayStr, TRUE);

    GVariant *value = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data, byteArray->len, sizeof(guint8));

    WriteData *writeData = g_new0(WriteData, 1);
    writeData->value = g_variant_ref(value);
    writeData->characteristic = characteristic;

    guint16 offset = 0;
    const char *writeTypeString = writeType == WITH_RESPONSE ? "request" : "command";
    GVariantBuilder *optionsBuilder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(optionsBuilder, "{sv}", "offset", g_variant_new_uint16(offset));
    g_variant_builder_add(optionsBuilder, "{sv}", "type", g_variant_new_string(writeTypeString));
    GVariant *options = g_variant_builder_end(optionsBuilder);
    g_variant_builder_unref(optionsBuilder);

    g_dbus_connection_call(characteristic->connection,
                           BLUEZ_DBUS,
                           characteristic->path,
                           INTERFACE_CHARACTERISTIC,
                           CHARACTERISTIC_METHOD_WRITE_VALUE,
                           g_variant_new("(@ay@a{sv})", value, options),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_char_write_cb,
                           writeData);
}

static void binc_internal_signal_characteristic_changed(__attribute__((unused)) GDBusConnection *conn,
                                                        __attribute__((unused)) const gchar *sender,
                                                        __attribute__((unused)) const gchar *path,
                                                        __attribute__((unused)) const gchar *interface,
                                                        __attribute__((unused)) const gchar *signal,
                                                        GVariant *parameters,
                                                        void *user_data) {

    Characteristic *characteristic = (Characteristic *) user_data;
    g_assert(characteristic != NULL);

    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface = NULL;
    const char *property_name = NULL;
    GVariant *property_value = NULL;

    g_assert(g_str_equal(g_variant_get_type_string(parameters), "(sa{sv}as)"));
    g_variant_get(parameters, "(&sa{sv}as)", &iface, &properties, &unknown);
    while (g_variant_iter_loop(properties, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, CHARACTERISTIC_PROPERTY_NOTIFYING)) {
            characteristic->notifying = g_variant_get_boolean(property_value);
            log_debug(TAG, "notifying %s <%s>", characteristic->notifying ? "true" : "false", characteristic->uuid);

            if (characteristic->notify_state_callback != NULL) {
                characteristic->notify_state_callback(characteristic->device, characteristic, NULL);
            }

            if (characteristic->notifying == FALSE) {
                if (characteristic->characteristic_prop_changed != 0) {
                    g_dbus_connection_signal_unsubscribe(characteristic->connection,
                                                         characteristic->characteristic_prop_changed);
                    characteristic->characteristic_prop_changed = 0;
                }
            }
        } else if (g_str_equal(property_name, CHARACTERISTIC_PROPERTY_VALUE)) {
            GByteArray *byteArray = g_variant_get_byte_array(property_value);
            GString *result = g_byte_array_as_hex(byteArray);
            log_debug(TAG, "notification <%s> on <%s>", result->str, characteristic->uuid);
            g_string_free(result, TRUE);

            if (characteristic->on_notify_callback != NULL) {
                characteristic->on_notify_callback(characteristic->device, characteristic, byteArray);
            }
            g_byte_array_free(byteArray, FALSE);
        }
    }

    if (properties != NULL) {
        g_variant_iter_free(properties);
    }
    if (unknown != NULL) {
        g_variant_iter_free(unknown);
    }
}

static void binc_internal_char_start_notify_cb(__attribute__((unused)) GObject *source_object,
                                               GAsyncResult *res,
                                               gpointer user_data) {

    Characteristic *characteristic = (Characteristic *) user_data;
    g_assert(characteristic != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(characteristic->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", CHARACTERISTIC_METHOD_START_NOTIFY, error->code,
                  error->message);
        if (characteristic->notify_state_callback != NULL) {
            characteristic->notify_state_callback(characteristic->device, characteristic, error);
        }
        g_clear_error(&error);
    }
}

static void register_for_properties_changed_signal(Characteristic *characteristic) {
    if (characteristic->characteristic_prop_changed == 0) {
        characteristic->characteristic_prop_changed = g_dbus_connection_signal_subscribe(characteristic->connection,
                                                                                         BLUEZ_DBUS,
                                                                                         "org.freedesktop.DBus.Properties",
                                                                                         "PropertiesChanged",
                                                                                         characteristic->path,
                                                                                         INTERFACE_CHARACTERISTIC,
                                                                                         G_DBUS_SIGNAL_FLAGS_NONE,
                                                                                         binc_internal_signal_characteristic_changed,
                                                                                         characteristic,
                                                                                         NULL);
    }
}

void binc_characteristic_start_notify(Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    g_assert(binc_characteristic_supports_notify(characteristic));

    log_debug(TAG, "start notify for <%s>", characteristic->uuid);
    register_for_properties_changed_signal(characteristic);

    g_dbus_connection_call(characteristic->connection,
                           BLUEZ_DBUS,
                           characteristic->path,
                           INTERFACE_CHARACTERISTIC,
                           CHARACTERISTIC_METHOD_START_NOTIFY,
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_char_start_notify_cb,
                           characteristic);
}

static void binc_internal_char_stop_notify_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    Characteristic *characteristic = (Characteristic *) user_data;
    g_assert(characteristic != NULL);

    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(characteristic->connection, res, &error);
    if (value != NULL) {
        g_variant_unref(value);
    }

    if (error != NULL) {
        log_debug(TAG, "failed to call '%s' (error %d: %s)", CHARACTERISTIC_METHOD_STOP_NOTIFY, error->code,
                  error->message);
        if (characteristic->notify_state_callback != NULL) {
            characteristic->notify_state_callback(characteristic->device, characteristic, error);
        }
        g_clear_error(&error);
    }
}

void binc_characteristic_stop_notify(Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    g_assert((characteristic->properties & GATT_CHR_PROP_INDICATE) > 0 ||
             (characteristic->properties & GATT_CHR_PROP_NOTIFY) > 0);

    g_dbus_connection_call(characteristic->connection,
                           BLUEZ_DBUS,
                           characteristic->path,
                           INTERFACE_CHARACTERISTIC,
                           CHARACTERISTIC_METHOD_STOP_NOTIFY,
                           NULL,
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_char_stop_notify_cb,
                           characteristic);
}

void binc_characteristic_set_read_cb(Characteristic *characteristic, OnReadCallback callback) {
    g_assert(characteristic != NULL);
    g_assert(callback != NULL);
    characteristic->on_read_callback = callback;
}

void binc_characteristic_set_write_cb(Characteristic *characteristic, OnWriteCallback callback) {
    g_assert(characteristic != NULL);
    g_assert(callback != NULL);
    characteristic->on_write_callback = callback;
}

void binc_characteristic_set_notify_cb(Characteristic *characteristic, OnNotifyCallback callback) {
    g_assert(characteristic != NULL);
    g_assert(callback != NULL);
    characteristic->on_notify_callback = callback;
}

void binc_characteristic_set_notifying_state_change_cb(Characteristic *characteristic,
                                                       OnNotifyingStateChangedCallback callback) {
    g_assert(characteristic != NULL);
    g_assert(callback != NULL);

    characteristic->notify_state_callback = callback;
}

void binc_characteristic_set_notifying(Characteristic *characteristic, gboolean notifying) {
    g_assert(characteristic != NULL);
    characteristic->notifying = notifying;

    if(characteristic->notifying) {
        register_for_properties_changed_signal(characteristic);
    }
}

const char *binc_characteristic_get_uuid(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->uuid;
}

void binc_characteristic_set_uuid(Characteristic *characteristic, const char *uuid) {
    g_assert(characteristic != NULL);
    g_assert(uuid != NULL);

    g_free((char *) characteristic->uuid);
    characteristic->uuid = g_strdup(uuid);
}

void binc_characteristic_set_mtu(Characteristic *characteristic, guint mtu) {
    g_assert(characteristic != NULL);
    characteristic->mtu = mtu;
}

Device *binc_characteristic_get_device(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->device;
}

Service *binc_characteristic_get_service(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->service;
}

void binc_characteristic_set_service(Characteristic *characteristic, Service *service) {
    g_assert(characteristic != NULL);
    g_assert(service != NULL);
    characteristic->service = service;
}

const char *binc_characteristic_get_service_path(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->service_path;
}

void binc_characteristic_set_service_path(Characteristic *characteristic, const char *service_path) {
    g_assert(characteristic != NULL);
    g_assert(service_path != NULL);

    if (characteristic->service_path != NULL) {
        g_free((char *) characteristic->service_path);
    }
    characteristic->service_path = g_strdup(service_path);
}

GList *binc_characteristic_get_flags(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->flags;
}

static guint binc_characteristic_flags_to_int(GList *flags) {
    guint result = 0;
    if (g_list_length(flags) > 0) {
        for (GList *iterator = flags; iterator; iterator = iterator->next) {
            char *property = (char *) iterator->data;
            if (g_str_equal(property, "broadcast")) {
                result += GATT_CHR_PROP_BROADCAST;
            } else if (g_str_equal(property, "read")) {
                result += GATT_CHR_PROP_READ;
            } else if (g_str_equal(property, "write-without-response")) {
                result += GATT_CHR_PROP_WRITE_WITHOUT_RESP;
            } else if (g_str_equal(property, "write")) {
                result += GATT_CHR_PROP_WRITE;
            } else if (g_str_equal(property, "notify")) {
                result += GATT_CHR_PROP_NOTIFY;
            } else if (g_str_equal(property, "indicate")) {
                result += GATT_CHR_PROP_INDICATE;
            } else if (g_str_equal(property, "authenticated-signed-writes")) {
                result += GATT_CHR_PROP_AUTH;
            } else if (g_str_equal(property, "encrypt-read")) {
                result += GATT_CHR_PROP_ENCRYPT_READ;
            } else if (g_str_equal(property, "encrypt-write")) {
                result += GATT_CHR_PROP_ENCRYPT_WRITE;
            } else if (g_str_equal(property, "encrypt-notify")) {
                result += GATT_CHR_PROP_ENCRYPT_NOTIFY;
            } else if (g_str_equal(property, "encrypt-indicate")) {
                result += GATT_CHR_PROP_ENCRYPT_INDICATE;
            }
        }
    }
    return result;
}

void binc_characteristic_set_flags(Characteristic *characteristic, GList *flags) {
    g_assert(characteristic != NULL);
    g_assert(flags != NULL);

    if (characteristic->flags != NULL) {
        g_list_free_full(characteristic->flags, g_free);
    }
    characteristic->flags = flags;
    characteristic->properties = binc_characteristic_flags_to_int(flags);
}

guint binc_characteristic_get_properties(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->properties;
}

gboolean binc_characteristic_is_notifying(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->notifying;
}

gboolean binc_characteristic_supports_write(const Characteristic *characteristic, WriteType writeType) {
    if (writeType == WITH_RESPONSE) {
        return (characteristic->properties & GATT_CHR_PROP_WRITE) > 0;
    } else {
        return (characteristic->properties & GATT_CHR_PROP_WRITE_WITHOUT_RESP) > 0;
    }
}

gboolean binc_characteristic_supports_read(const Characteristic *characteristic) {
    return (characteristic->properties & GATT_CHR_PROP_READ) > 0;
}

gboolean binc_characteristic_supports_notify(const Characteristic *characteristic) {
    return ((characteristic->properties & GATT_CHR_PROP_INDICATE) > 0 ||
            (characteristic->properties & GATT_CHR_PROP_NOTIFY) > 0);
}

void binc_characteristic_add_descriptor(Characteristic *characteristic, Descriptor *descriptor) {
    g_assert(characteristic != NULL);
    g_assert(descriptor != NULL);

    characteristic->descriptors = g_list_append(characteristic->descriptors, descriptor);
}

Descriptor *binc_characteristic_get_descriptor(const Characteristic *characteristic, const char* desc_uuid) {
    g_assert(characteristic != NULL);
    g_assert(is_valid_uuid(desc_uuid));

    if (characteristic->descriptors != NULL) {
        for (GList *iterator = characteristic->descriptors; iterator; iterator = iterator->next) {
            Descriptor *descriptor = (Descriptor *) iterator->data;
            if (g_str_equal(desc_uuid, binc_descriptor_get_uuid(descriptor))) {
                return descriptor;
            }
        }
    }
    return NULL;
}

GList *binc_characteristic_get_descriptors(const Characteristic *characteristic) {
    g_assert(characteristic != NULL);
    return characteristic->descriptors;
}
