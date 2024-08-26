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

/*
 * Open issues:
 *
 * - We never get 'flags' for a descriptor. So not exposing flags-related methods yet
 */

#include "descriptor.h"
#include "device_internal.h"
#include "utility.h"
#include "logger.h"

static const char *const TAG = "Descriptor";

static const char *const BLUEZ_DBUS = "org.bluez";
static const char *const INTERFACE_DESCRIPTOR = "org.bluez.GattDescriptor1";
static const char *const DESCRIPTOR_METHOD_READ_VALUE = "ReadValue";
static const char *const DESCRIPTOR_METHOD_WRITE_VALUE = "WriteValue";

struct binc_descriptor {
    Device *device; // Borrowed
    Characteristic *characteristic; // Borrowed
    GDBusConnection *connection; // Borrowed
    const char *path; // Owned
    const char *char_path; // Owned
    const char *uuid; // Owned
    GList *flags; // Owned

    OnDescReadCallback on_read_cb;
    OnDescWriteCallback on_write_cb;
};

Descriptor *binc_descriptor_create(Device *device, const char *path) {
    g_assert(device != NULL);
    g_assert(path != NULL);
    g_assert(strlen(path) > 0);

    Descriptor *descriptor = g_new0(Descriptor, 1);
    descriptor->device = device;
    descriptor->connection = binc_device_get_dbus_connection(device);
    descriptor->path = g_strdup(path);
    return descriptor;
}

void binc_descriptor_free(Descriptor *descriptor) {
    g_assert(descriptor != NULL);

    if (descriptor->flags != NULL) {
        g_list_free_full(descriptor->flags, g_free);
        descriptor->flags = NULL;
    }

    g_free((char *) descriptor->uuid);
    descriptor->uuid = NULL;
    g_free((char *) descriptor->path);
    descriptor->path = NULL;
    g_free((char *) descriptor->char_path);
    descriptor->char_path = NULL;

    descriptor->characteristic = NULL;
    descriptor->device = NULL;
    descriptor->connection = NULL;
    g_free(descriptor);
}

const char *binc_descriptor_to_string(const Descriptor *descriptor) {
    g_assert(descriptor != NULL);

    GString *flags = g_string_new("[");
    if (g_list_length(descriptor->flags) > 0) {
        for (GList *iterator = descriptor->flags; iterator; iterator = iterator->next) {
            g_string_append_printf(flags, "%s, ", (char *) iterator->data);
        }
        g_string_truncate(flags, flags->len - 2);
    }
    g_string_append(flags, "]");

    char *result = g_strdup_printf(
            "Descriptor{uuid='%s', flags='%s', properties=%d, char_uuid='%s'}",
            descriptor->uuid,
            flags->str,
            0,
            binc_characteristic_get_uuid(descriptor->characteristic));

    g_string_free(flags, TRUE);
    return result;
}

void binc_descriptor_set_uuid(Descriptor *descriptor, const char *uuid) {
    g_assert(descriptor != NULL);
    g_assert(is_valid_uuid(uuid));

    g_free((char *) descriptor->uuid);
    descriptor->uuid = g_strdup(uuid);
}

void binc_descriptor_set_char_path(Descriptor *descriptor, const char *path) {
    g_assert(descriptor != NULL);
    g_assert(path != NULL);

    g_free((char *) descriptor->char_path);
    descriptor->char_path = g_strdup(path);
}

const char *binc_descriptor_get_char_path(const Descriptor *descriptor) {
    g_assert(descriptor != NULL);
    return descriptor->char_path;
}

const char *binc_descriptor_get_uuid(const Descriptor *descriptor) {
    g_assert(descriptor != NULL);
    return descriptor->uuid;
}

void binc_descriptor_set_char(Descriptor *descriptor, Characteristic *characteristic) {
    g_assert(descriptor != NULL);
    g_assert(characteristic != NULL);

    descriptor->characteristic = characteristic;
}

void binc_descriptor_set_flags(Descriptor *descriptor, GList *flags) {
    g_assert(descriptor != NULL);
    g_assert(flags != NULL);

    if (descriptor->flags != NULL) {
        g_list_free_full(descriptor->flags, g_free);
    }
    descriptor->flags = flags;
}

static void binc_internal_descriptor_read_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    GError *error = NULL;
    GByteArray *byteArray = NULL;
    GVariant *innerArray = NULL;
    Descriptor *descriptor = (Descriptor *) user_data;
    g_assert(descriptor != NULL);

    GVariant *value = g_dbus_connection_call_finish(descriptor->connection, res, &error);
    if (value != NULL) {
        g_assert(g_str_equal(g_variant_get_type_string(value), "(ay)"));
        innerArray = g_variant_get_child_value(value, 0);
        byteArray = g_variant_get_byte_array(innerArray);
    }

    if (descriptor->on_read_cb != NULL) {
        descriptor->on_read_cb(descriptor->device, descriptor, byteArray, error);
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
        log_debug(TAG, "failed to call '%s' (error %d: %s)", DESCRIPTOR_METHOD_READ_VALUE, error->code,
                  error->message);
        g_clear_error(&error);
    }
}

void binc_descriptor_read(Descriptor *descriptor) {
    g_assert(descriptor != NULL);

    log_debug(TAG, "reading <%s>", descriptor->uuid);

    guint16 offset = 0;
    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(builder, "{sv}", "offset", g_variant_new_uint16(offset));
    GVariant *options = g_variant_builder_end(builder);
    g_variant_builder_unref(builder);

    g_dbus_connection_call(descriptor->connection,
                           BLUEZ_DBUS,
                           descriptor->path,
                           INTERFACE_DESCRIPTOR,
                           DESCRIPTOR_METHOD_READ_VALUE,
                           g_variant_new("(@a{sv})", options),
                           G_VARIANT_TYPE("(ay)"),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_descriptor_read_cb,
                           descriptor);
}

typedef struct binc_desc_write_data {
    GVariant *value;
    Descriptor *descriptor;
} WriteDescData;

static void binc_internal_descriptor_write_cb(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    WriteDescData *writeData = (WriteDescData *) user_data;
    Descriptor *descriptor = writeData->descriptor;
    g_assert(descriptor != NULL);

    GByteArray *byteArray = NULL;
    GError *error = NULL;
    GVariant *value = g_dbus_connection_call_finish(descriptor->connection, res, &error);

    if (writeData->value != NULL) {
        byteArray = g_variant_get_byte_array(writeData->value);
    }

    if (descriptor->on_write_cb != NULL) {
        descriptor->on_write_cb(descriptor->device, descriptor, byteArray, error);
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
        log_debug(TAG, "failed to call '%s' (error %d: %s)", DESCRIPTOR_METHOD_WRITE_VALUE,
                  error->code, error->message);
        g_clear_error(&error);
    }
}

void binc_descriptor_write(Descriptor *descriptor, const GByteArray *byteArray) {
    g_assert(descriptor != NULL);
    g_assert(byteArray != NULL);
    g_assert(byteArray->len > 0);

    GString *byteArrayStr = g_byte_array_as_hex(byteArray);
    log_debug(TAG, "writing <%s> to <%s>", byteArrayStr->str, descriptor->uuid);
    g_string_free(byteArrayStr, TRUE);

    GVariant *value = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data, byteArray->len, sizeof(guint8));

    WriteDescData *writeData = g_new0(WriteDescData, 1);
    writeData->value = g_variant_ref(value);
    writeData->descriptor = descriptor;

    guint16 offset = 0;
    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(builder, "{sv}", "offset", g_variant_new_uint16(offset));
    GVariant *options = g_variant_builder_end(builder);
    g_variant_builder_unref(builder);

    g_dbus_connection_call(descriptor->connection,
                           BLUEZ_DBUS,
                           descriptor->path,
                           INTERFACE_DESCRIPTOR,
                           DESCRIPTOR_METHOD_WRITE_VALUE,
                           g_variant_new("(@ay@a{sv})", value, options),
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           (GAsyncReadyCallback) binc_internal_descriptor_write_cb,
                           writeData);
}

void binc_descriptor_set_read_cb(Descriptor *descriptor, OnDescReadCallback callback) {
    g_assert(descriptor != NULL);
    g_assert(callback != NULL);

    descriptor->on_read_cb = callback;
}

void binc_descriptor_set_write_cb(Descriptor *descriptor, OnDescWriteCallback callback) {
    g_assert(descriptor != NULL);
    g_assert(callback != NULL);

    descriptor->on_write_cb = callback;
}

Device *binc_descriptor_get_device(const Descriptor *descriptor) {
    g_assert(descriptor != NULL);
    return descriptor->device;
}

Characteristic *binc_descriptor_get_char(const Descriptor *descriptor) {
    g_assert(descriptor != NULL);
    return descriptor->characteristic;
}
