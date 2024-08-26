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

#include "application.h"
#include "adapter.h"
#include "logger.h"
#include "characteristic.h"
#include "utility.h"
#include <errno.h>

#define GATT_SERV_INTERFACE "org.bluez.GattService1"
#define GATT_CHAR_INTERFACE "org.bluez.GattCharacteristic1"
#define GATT_DESC_INTERFACE "org.bluez.GattDescriptor1"

static const char *const TAG = "Application";

static const char *const CHARACTERISTIC_METHOD_READ_VALUE = "ReadValue";
static const char *const CHARACTERISTIC_METHOD_WRITE_VALUE = "WriteValue";
static const char *const CHARACTERISTIC_METHOD_STOP_NOTIFY = "StopNotify";
static const char *const CHARACTERISTIC_METHOD_START_NOTIFY = "StartNotify";
static const char *const CHARACTERISTIC_METHOD_CONFIRM = "Confirm";
static const char *const DESCRIPTOR_METHOD_READ_VALUE = "ReadValue";
static const char *const DESCRIPTOR_METHOD_WRITE_VALUE = "WriteValue";

static const gchar object_manager_xml[] =
        "<node name='/'>"
        "  <interface name='org.freedesktop.DBus.ObjectManager'>"
        "    <method name='GetManagedObjects'>"
        "        <arg type='a{oa{sa{sv}}}' name='object_paths_interfaces_and_properties' direction='out'/>"
        "    </method>"
        "  </interface>"
        "</node>";

static const gchar service_xml[] =
        "<node name='/'>"
        "  <interface name='org.freedesktop.DBus.Properties'>"
        "        <property type='s' name='UUID' access='read' />"
        "        <property type='b' name='primary' access='read' />"
        "        <property type='o' name='Device' access='read' />"
        "        <property type='ao' name='Characteristics' access='read' />"
        "        <property type='s' name='Includes' access='read' />"
        "  </interface>"
        "</node>";

static const gchar characteristic_xml[] =
        "<node name='/'>"
        "  <interface name='org.bluez.GattCharacteristic1'>"
        "        <method name='ReadValue'>"
        "               <arg type='a{sv}' name='options' direction='in' />"
        "               <arg type='ay' name='value' direction='out'/>"
        "        </method>"
        "        <method name='WriteValue'>"
        "               <arg type='ay' name='value' direction='in'/>"
        "               <arg type='a{sv}' name='options' direction='in' />"
        "        </method>"
        "        <method name='StartNotify'/>"
        "        <method name='StopNotify' />"
        "        <method name='Confirm' />"
        "  </interface>"
        "  <interface name='org.freedesktop.DBus.Properties'>"
        "    <property type='s' name='UUID' access='read' />"
        "    <property type='o' name='Service' access='read' />"
        "    <property type='ay' name='Value' access='readwrite' />"
        "    <property type='b' name='Notifying' access='read' />"
        "    <property type='as' name='Flags' access='read' />"
        "    <property type='ao' name='Descriptors' access='read' />"
        "  </interface>"
        "</node>";

static const gchar descriptor_xml[] =
        "<node name='/'>"
        "  <interface name='org.bluez.GattDescriptor1'>"
        "        <method name='ReadValue'>"
        "               <arg type='a{sv}' name='options' direction='in' />"
        "               <arg type='ay' name='value' direction='out'/>"
        "        </method>"
        "        <method name='WriteValue'>"
        "               <arg type='ay' name='value' direction='in'/>"
        "               <arg type='a{sv}' name='options' direction='in' />"
        "        </method>"
        "  </interface>"
        "  <interface name='org.freedesktop.DBus.Properties'>"
        "    <property type='s' name='UUID' access='read' />"
        "    <property type='o' name='Characteristic' access='read' />"
        "    <property type='ay' name='Value' access='readwrite' />"
        "    <property type='as' name='Flags' access='read' />"
        "  </interface>"
        "</node>";

struct binc_application {
    char *path;
    guint registration_id;
    GDBusConnection *connection;
    GHashTable *services;
    onLocalCharacteristicWrite on_char_write;
    onLocalCharacteristicRead on_char_read;
    onLocalCharacteristicUpdated on_char_updated;
    onLocalCharacteristicStartNotify on_char_start_notify;
    onLocalCharacteristicStopNotify on_char_stop_notify;
    onLocalDescriptorWrite on_desc_write;
    onLocalDescriptorRead on_desc_read;
};

typedef struct binc_local_service {
    char *path;
    char *uuid;
    guint registration_id;
    GHashTable *characteristics;
    Application *application;
} LocalService;

typedef struct local_characteristic {
    char *service_uuid;
    char *service_path;
    char *uuid;
    char *path;
    guint registration_id;
    GByteArray *value;
    guint permissions;
    GList *flags;
    gboolean notifying;
    GHashTable *descriptors;
    Application *application;
} LocalCharacteristic;

typedef struct local_descriptor {
    char *path;
    char *char_path;
    char *uuid;
    char *char_uuid;
    char *service_uuid;
    guint registration_id;
    GByteArray *value;
    guint permissions;
    GList *flags;
    Application *application;
} LocalDescriptor;

static void binc_local_desc_free(LocalDescriptor *localDescriptor) {
    g_assert(localDescriptor != NULL);

    log_debug(TAG, "freeing descriptor %s", localDescriptor->path);

    if (localDescriptor->registration_id != 0) {
        gboolean result = g_dbus_connection_unregister_object(localDescriptor->application->connection,
                                                              localDescriptor->registration_id);
        if (!result) {
            log_debug(TAG, "error: could not unregister descriptor %s", localDescriptor->path);
        }
        localDescriptor->registration_id = 0;
    }

    if (localDescriptor->value != NULL) {
        g_byte_array_free(localDescriptor->value, TRUE);
        localDescriptor->value = NULL;
    }

    g_free(localDescriptor->path);
    localDescriptor->path = NULL;

    g_free(localDescriptor->char_path);
    localDescriptor->char_path = NULL;

    g_free(localDescriptor->uuid);
    localDescriptor->uuid = NULL;

    g_free(localDescriptor->char_uuid);
    localDescriptor->char_uuid = NULL;

    g_free(localDescriptor->service_uuid);
    localDescriptor->service_uuid = NULL;

    if (localDescriptor->flags != NULL) {
        g_list_free_full(localDescriptor->flags, g_free);
        localDescriptor->flags = NULL;
    }

    g_free(localDescriptor);
}

static void binc_local_char_free(LocalCharacteristic *localCharacteristic) {
    g_assert(localCharacteristic != NULL);

    log_debug(TAG, "freeing characteristic %s", localCharacteristic->path);

    if (localCharacteristic->descriptors != NULL) {
        g_hash_table_destroy(localCharacteristic->descriptors);
        localCharacteristic->descriptors = NULL;
    }

    if (localCharacteristic->registration_id != 0) {
        gboolean result = g_dbus_connection_unregister_object(localCharacteristic->application->connection,
                                                              localCharacteristic->registration_id);
        if (!result) {
            log_debug(TAG, "error: could not unregister service %s", localCharacteristic->path);
        }
        localCharacteristic->registration_id = 0;
    }

    if (localCharacteristic->value != NULL) {
        g_byte_array_free(localCharacteristic->value, TRUE);
        localCharacteristic->value = NULL;
    }

    g_free(localCharacteristic->path);
    localCharacteristic->path = NULL;

    g_free(localCharacteristic->uuid);
    localCharacteristic->uuid = NULL;

    g_free(localCharacteristic->service_uuid);
    localCharacteristic->service_uuid = NULL;

    g_free(localCharacteristic->service_path);
    localCharacteristic->service_path = NULL;

    if (localCharacteristic->flags != NULL) {
        g_list_free_full(localCharacteristic->flags, g_free);
        localCharacteristic->flags = NULL;
    }

    g_free(localCharacteristic);
}

void binc_local_service_free(LocalService *localService) {
    g_assert(localService != NULL);

    log_debug(TAG, "freeing service %s", localService->path);

    if (localService->characteristics != NULL) {
        g_hash_table_destroy(localService->characteristics);
        localService->characteristics = NULL;
    }

    if (localService->registration_id != 0) {
        gboolean result = g_dbus_connection_unregister_object(localService->application->connection,
                                                              localService->registration_id);
        if (!result) {
            log_debug(TAG, "error: could not unregister service %s", localService->path);
        }
        localService->registration_id = 0;
    }

    g_free(localService->path);
    localService->path = NULL;

    g_free(localService->uuid);
    localService->uuid = NULL;

    g_free(localService);
}

typedef struct read_options {
    char *device;
    guint16 mtu;
    guint16 offset;
    char *link_type;
} ReadOptions;

void read_options_free(ReadOptions *options) {
    if (options->link_type != NULL) g_free(options->link_type);
    if (options->device != NULL) g_free(options->device);
    g_free(options);
}

static ReadOptions *parse_read_options(GVariant *params) {
    g_assert(g_str_equal(g_variant_get_type_string(params), "(a{sv})"));
    ReadOptions *options = g_new0(ReadOptions, 1);

    GVariantIter *optionsVariant;
    g_variant_get(params, "(a{sv})", &optionsVariant);

    GVariant *property_value;
    gchar *property_name;
    while (g_variant_iter_loop(optionsVariant, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, "offset")) {
            options->offset = g_variant_get_uint16(property_value);
        } else if (g_str_equal(property_name, "mtu")) {
            options->mtu = g_variant_get_uint16(property_value);
        } else if (g_str_equal(property_name, "device")) {
            options->device = path_to_address(g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "link")) {
            options->link_type = g_strdup(g_variant_get_string(property_value, NULL));
        }
    }
    g_variant_iter_free(optionsVariant);

    log_debug(TAG, "read with offset=%u, mtu=%u, link=%s, device=%s", (unsigned int) options->offset,
              (unsigned int) options->mtu, options->link_type, options->device);

    return options;
}

typedef struct write_options {
    char *write_type;
    char *device;
    guint16 mtu;
    guint16 offset;
    char *link_type;
} WriteOptions;

void write_options_free(WriteOptions *options) {
    if (options->link_type != NULL) g_free(options->link_type);
    if (options->device != NULL) g_free(options->device);
    if (options->write_type != NULL) g_free(options->write_type);
    g_free(options);
}

static WriteOptions *parse_write_options(GVariant *optionsVariant) {
    g_assert(g_str_equal(g_variant_get_type_string(optionsVariant), "a{sv}"));
    WriteOptions *options = g_new0(WriteOptions, 1);

    GVariantIter iter;
    g_variant_iter_init(&iter, optionsVariant);
    GVariant *property_value;
    gchar *property_name;
    while (g_variant_iter_loop(&iter, "{&sv}", &property_name, &property_value)) {
        if (g_str_equal(property_name, "offset")) {
            options->offset = g_variant_get_uint16(property_value);
        } else if (g_str_equal(property_name, "type")) {
            options->write_type = g_strdup(g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "mtu")) {
            options->mtu = g_variant_get_uint16(property_value);
        } else if (g_str_equal(property_name, "device")) {
            options->device = path_to_address(g_variant_get_string(property_value, NULL));
        } else if (g_str_equal(property_name, "link")) {
            options->link_type = g_strdup(g_variant_get_string(property_value, NULL));
        }
    }

    log_debug(TAG, "write with offset=%u, mtu=%u, link=%s, device=%s", (unsigned int) options->offset,
              (unsigned int) options->mtu, options->link_type, options->device);

    return options;
}

static void add_char_path(gpointer key, gpointer value, gpointer userdata) {
    LocalCharacteristic *localCharacteristic = (LocalCharacteristic *) value;
    g_variant_builder_add((GVariantBuilder *) userdata, "o", localCharacteristic->path);
}

static GVariant *binc_local_service_get_characteristics(const LocalService *localService) {
    g_assert(localService != NULL);

    GVariantBuilder *characteristics_builder = g_variant_builder_new(G_VARIANT_TYPE("ao"));
    g_hash_table_foreach(localService->characteristics, add_char_path, characteristics_builder);
    GVariant *result = g_variant_builder_end(characteristics_builder);
    g_variant_builder_unref(characteristics_builder);
    return result;
}

static void add_desc_path(gpointer key, gpointer value, gpointer userdata) {
    LocalDescriptor *localDescriptor = (LocalDescriptor *) value;
    g_variant_builder_add((GVariantBuilder *) userdata, "o", localDescriptor->path);
}

static GVariant *binc_local_characteristic_get_descriptors(const LocalCharacteristic *localCharacteristic) {
    g_assert(localCharacteristic != NULL);

    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("ao"));
    g_hash_table_foreach(localCharacteristic->descriptors, add_desc_path, builder);
    GVariant *result = g_variant_builder_end(builder);
    g_variant_builder_unref(builder);
    return result;
}

static GVariant *binc_local_characteristic_get_flags(const LocalCharacteristic *localCharacteristic) {
    g_assert(localCharacteristic != NULL);

    GVariantBuilder *flags_builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    for (GList *iterator = localCharacteristic->flags; iterator; iterator = iterator->next) {
        g_variant_builder_add(flags_builder, "s", (char *) iterator->data);
    }
    GVariant *result = g_variant_builder_end(flags_builder);
    g_variant_builder_unref(flags_builder);
    return result;
}

static GVariant *binc_local_descriptor_get_flags(const LocalDescriptor *localDescriptor) {
    g_assert(localDescriptor != NULL);

    GVariantBuilder *flags_builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    for (GList *iterator = localDescriptor->flags; iterator; iterator = iterator->next) {
        g_variant_builder_add(flags_builder, "s", (char *) iterator->data);
    }
    GVariant *result = g_variant_builder_end(flags_builder);
    g_variant_builder_unref(flags_builder);
    return result;
}

static void add_descriptors(GVariantBuilder *builder,
                            LocalCharacteristic *localCharacteristic) {
    // NOTE that the CCCD is automatically added by Bluez so no need to add it.
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, localCharacteristic->descriptors);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        LocalDescriptor *localDescriptor = (LocalDescriptor *) value;
        log_debug(TAG, "adding %s", localDescriptor->path);

        GVariantBuilder *descriptors_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));
        GVariantBuilder *desc_properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

        GByteArray *byteArray = localDescriptor->value;
        GVariant *byteArrayVariant = NULL;
        if (byteArray != NULL) {
            byteArrayVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data,
                                                         byteArray->len, sizeof(guint8));
            g_variant_builder_add(desc_properties_builder, "{sv}", "Value", byteArrayVariant);
        }
        g_variant_builder_add(desc_properties_builder, "{sv}", "UUID",
                              g_variant_new_string(localDescriptor->uuid));
        g_variant_builder_add(desc_properties_builder, "{sv}", "Characteristic",
                              g_variant_new("o", localDescriptor->char_path));
        g_variant_builder_add(desc_properties_builder, "{sv}", "Flags",
                              binc_local_descriptor_get_flags(localDescriptor));

        // Add the descriptor to result
        g_variant_builder_add(descriptors_builder, "{sa{sv}}", GATT_DESC_INTERFACE,
                              desc_properties_builder);
        g_variant_builder_unref(desc_properties_builder);
        g_variant_builder_add(builder, "{oa{sa{sv}}}", localDescriptor->path, descriptors_builder);
        g_variant_builder_unref(descriptors_builder);
    }
}

static void add_characteristics(GVariantBuilder *builder, LocalService *localService) {
    // Build service characteristics
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, localService->characteristics);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        LocalCharacteristic *localCharacteristic = (LocalCharacteristic *) value;
        log_debug(TAG, "adding %s", localCharacteristic->path);

        GVariantBuilder *characteristic_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));
        GVariantBuilder *char_properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

        // Build characteristic properties
        GByteArray *byteArray = localCharacteristic->value;
        GVariant *byteArrayVariant = NULL;
        if (byteArray != NULL) {
            byteArrayVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data,
                                                         byteArray->len, sizeof(guint8));
            g_variant_builder_add(char_properties_builder, "{sv}", "Value", byteArrayVariant);
        }
        g_variant_builder_add(char_properties_builder, "{sv}", "UUID",
                              g_variant_new_string(localCharacteristic->uuid));
        g_variant_builder_add(char_properties_builder, "{sv}", "Service",
                              g_variant_new("o", localService->path));
        g_variant_builder_add(char_properties_builder, "{sv}", "Flags",
                              binc_local_characteristic_get_flags(localCharacteristic));
        g_variant_builder_add(char_properties_builder, "{sv}", "Notifying",
                              g_variant_new("b", localCharacteristic->notifying));
        g_variant_builder_add(char_properties_builder, "{sv}", "Descriptors",
                              binc_local_characteristic_get_descriptors(localCharacteristic));

        // Add the characteristic to result
        g_variant_builder_add(characteristic_builder, "{sa{sv}}", GATT_CHAR_INTERFACE,
                              char_properties_builder);
        g_variant_builder_unref(char_properties_builder);
        g_variant_builder_add(builder, "{oa{sa{sv}}}", localCharacteristic->path, characteristic_builder);
        g_variant_builder_unref(characteristic_builder);

        add_descriptors(builder, localCharacteristic);
    }
}

static void add_services(Application *application, GVariantBuilder *builder) {
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, application->services);
    while (g_hash_table_iter_next(&iter, (gpointer) &key, &value)) {
        LocalService *localService = (LocalService *) value;
        log_debug(TAG, "adding %s", localService->path);
        GVariantBuilder *service_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sa{sv}}"));

        // Build service properties
        GVariantBuilder *service_properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(service_properties_builder, "{sv}", "UUID",
                              g_variant_new_string((char *) key));
        g_variant_builder_add(service_properties_builder, "{sv}", "Primary",
                              g_variant_new_boolean(TRUE));
        g_variant_builder_add(service_properties_builder, "{sv}", "Characteristics",
                              binc_local_service_get_characteristics(localService));

        // Add the service to result
        g_variant_builder_add(service_builder, "{sa{sv}}", GATT_SERV_INTERFACE, service_properties_builder);
        g_variant_builder_unref(service_properties_builder);
        g_variant_builder_add(builder, "{oa{sa{sv}}}", localService->path, service_builder);
        g_variant_builder_unref(service_builder);
        add_characteristics(builder, localService);
    }
}

static void binc_internal_application_method_call(GDBusConnection *conn,
                                                  const gchar *sender,
                                                  const gchar *path,
                                                  const gchar *interface,
                                                  const gchar *method,
                                                  GVariant *params,
                                                  GDBusMethodInvocation *invocation,
                                                  void *userdata) {

    Application *application = (Application *) userdata;
    g_assert(application != NULL);

    if (g_str_equal(method, "GetManagedObjects")) {
        GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{oa{sa{sv}}}"));
        if (application->services != NULL && g_hash_table_size(application->services) > 0) {
            add_services(application, builder);
        }
        GVariant *result = g_variant_builder_end(builder);
        g_variant_builder_unref(builder);

        g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&result, 1));
    }
}

static const GDBusInterfaceVTable application_method_table = {
        .method_call = binc_internal_application_method_call,
};

void binc_application_publish(Application *application, const Adapter *adapter) {
    g_assert(application != NULL);
    g_assert(adapter != NULL);

    GError *error = NULL;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(object_manager_xml, &error);
    if (error) {
        log_debug(TAG, "Unable to create manager node: %s\n", error->message);
        g_clear_error(&error);
        return;
    }

    application->registration_id = g_dbus_connection_register_object(application->connection,
                                                                     application->path,
                                                                     info->interfaces[0],
                                                                     &application_method_table,
                                                                     application,
                                                                     NULL,
                                                                     &error);
    g_dbus_node_info_unref(info);

    if (application->registration_id == 0 && error != NULL) {
        log_debug(TAG, "failed to publish application");
        g_clear_error(&error);
        return;
    }

    log_debug(TAG, "successfully published application");
}

Application *binc_create_application(const Adapter *adapter) {
    g_assert(adapter != NULL);

    Application *application = g_new0(Application, 1);
    application->connection = binc_adapter_get_dbus_connection(adapter);
    application->path = g_strdup("/org/bluez/bincapplication");
    application->services = g_hash_table_new_full(g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  (GDestroyNotify) binc_local_service_free);

    binc_application_publish(application, adapter);

    return application;
}

void binc_application_free(Application *application) {
    g_assert(application != NULL);

    log_debug(TAG, "freeing application %s", application->path);

    if (application->services != NULL) {
        g_hash_table_destroy(application->services);
        application->services = NULL;
    }

    if (application->registration_id != 0) {
        gboolean result = g_dbus_connection_unregister_object(application->connection, application->registration_id);
        if (!result) {
            log_debug(TAG, "error: could not unregister application %s", application->path);
        }
        application->registration_id = 0;
    }

    if (application->path != NULL) {
        g_free(application->path);
        application->path = NULL;
    }

    g_free(application);
}

static const GDBusInterfaceVTable service_table = {};

int binc_application_add_service(Application *application, const char *service_uuid) {
    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);

    GError *error = NULL;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(service_xml, &error);
    if (error) {
        log_debug(TAG, "Unable to create service node: %s\n", error->message);
        g_clear_error(&error);
        return EINVAL;
    }

    LocalService *localService = g_new0(LocalService, 1);
    localService->uuid = g_strdup(service_uuid);
    localService->application = application;
    localService->characteristics = g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            (GDestroyNotify) binc_local_char_free);
    localService->path = g_strdup_printf(
            "%s/service%d",
            application->path,
            g_hash_table_size(application->services));
    g_hash_table_insert(application->services, g_strdup(service_uuid), localService);

    localService->registration_id = g_dbus_connection_register_object(application->connection,
                                                                      localService->path,
                                                                      info->interfaces[0],
                                                                      &service_table,
                                                                      localService,
                                                                      NULL,
                                                                      &error);
    g_dbus_node_info_unref(info);

    if (localService->registration_id == 0) {
        log_debug(TAG, "failed to publish local service");
        log_debug(TAG, "Error %s", error->message);
        g_hash_table_remove(application->services, service_uuid);
        binc_local_service_free(localService);
        g_clear_error(&error);
        return EINVAL;
    }

    log_debug(TAG, "successfully published local service %s", service_uuid);
    return 0;
}


static LocalService *binc_application_get_service(const Application *application, const char *service_uuid) {
    g_return_val_if_fail (application != NULL, NULL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), NULL);

    return g_hash_table_lookup(application->services, service_uuid);
}

static GList *permissions2Flags(const guint permissions) {
    GList *list = NULL;

    if (permissions & GATT_CHR_PROP_READ) {
        list = g_list_append(list, g_strdup("read"));
    }
    if (permissions & GATT_CHR_PROP_WRITE_WITHOUT_RESP) {
        list = g_list_append(list, g_strdup("write-without-response"));
    }
    if (permissions & GATT_CHR_PROP_WRITE) {
        list = g_list_append(list, g_strdup("write"));
    }
    if (permissions & GATT_CHR_PROP_NOTIFY) {
        list = g_list_append(list, g_strdup("notify"));
    }
    if (permissions & GATT_CHR_PROP_INDICATE) {
        list = g_list_append(list, g_strdup("indicate"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_READ) {
        list = g_list_append(list, g_strdup("encrypt-read"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_WRITE) {
        list = g_list_append(list, g_strdup("encrypt-write"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_NOTIFY) {
        list = g_list_append(list, g_strdup("encrypt-notify"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_INDICATE) {
        list = g_list_append(list, g_strdup("encrypt-indicate"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_AUTH_READ) {
        list = g_list_append(list, g_strdup("encrypt-authenticated-read"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_AUTH_WRITE) {
        list = g_list_append(list, g_strdup("encrypt-authenticated-write"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_AUTH_NOTIFY) {
        list = g_list_append(list, g_strdup("encrypt-authenticated-notify"));
    }
    if (permissions & GATT_CHR_PROP_ENCRYPT_AUTH_INDICATE) {
        list = g_list_append(list, g_strdup("encrypt-authenticated-indicate"));
    }
    if (permissions & GATT_CHR_PROP_SECURE_READ) {
        list = g_list_append(list, g_strdup("secure-read"));
    }
    if (permissions & GATT_CHR_PROP_SECURE_WRITE) {
        list = g_list_append(list, g_strdup("secure-write"));
    }
    if (permissions & GATT_CHR_PROP_SECURE_NOTIFY) {
        list = g_list_append(list, g_strdup("secure-notify"));
    }
    if (permissions & GATT_CHR_PROP_SECURE_INDICATE) {
        list = g_list_append(list, g_strdup("secure-indicate"));
    }

    return list;
}

static int binc_characteristic_set_value(const Application *application, LocalCharacteristic *characteristic,
                                         GByteArray *byteArray) {
    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (characteristic != NULL, EINVAL);
    g_return_val_if_fail (byteArray != NULL, EINVAL);

    GString *byteArrayStr = g_byte_array_as_hex(byteArray);
    log_debug(TAG, "set value <%s> to <%s>", byteArrayStr->str, characteristic->uuid);
    g_string_free(byteArrayStr, TRUE);

    if (characteristic->value != NULL) {
        g_byte_array_free(characteristic->value, TRUE);
    }
    characteristic->value = byteArray;

    if (application->on_char_updated != NULL) {
        application->on_char_updated(characteristic->application, characteristic->service_uuid,
                                     characteristic->uuid, byteArray);
    }

    return 0;
}

static int binc_descriptor_set_value(const Application *application, LocalDescriptor *descriptor,
                                     GByteArray *byteArray) {
    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (descriptor != NULL, EINVAL);
    g_return_val_if_fail (byteArray != NULL, EINVAL);

    GString *byteArrayStr = g_byte_array_as_hex(byteArray);
    log_debug(TAG, "set value <%s> to <%s>", byteArrayStr->str, descriptor->uuid);
    g_string_free(byteArrayStr, TRUE);

    if (descriptor->value != NULL) {
        g_byte_array_free(descriptor->value, TRUE);
    }
    descriptor->value = byteArray;

//    if (application->on_char_updated != NULL) {
//        application->on_char_updated(characteristic->application, characteristic->service_uuid,
//                                     characteristic->uuid, byteArray);
//    }

    return 0;
}

static LocalCharacteristic *get_local_characteristic(const Application *application, const char *service_uuid,
                                                     const char *char_uuid) {

    g_return_val_if_fail (application != NULL, NULL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), NULL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), NULL);

    LocalService *service = binc_application_get_service(application, service_uuid);
    if (service != NULL) {
        return g_hash_table_lookup(service->characteristics, char_uuid);
    }
    return NULL;
}

static LocalDescriptor *get_local_descriptor(const Application *application, const char *service_uuid,
                                             const char *char_uuid, const char *desc_uuid) {

    g_return_val_if_fail (application != NULL, NULL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), NULL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), NULL);
    g_return_val_if_fail (is_valid_uuid(desc_uuid), NULL);

    LocalCharacteristic *characteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (characteristic != NULL) {
        return g_hash_table_lookup(characteristic->descriptors, desc_uuid);
    }
    return NULL;
}

static void binc_internal_descriptor_method_call(GDBusConnection *conn,
                                                 const gchar *sender,
                                                 const gchar *path,
                                                 const gchar *interface,
                                                 const gchar *method,
                                                 GVariant *params,
                                                 GDBusMethodInvocation *invocation,
                                                 void *userdata) {

    LocalDescriptor *localDescriptor = (LocalDescriptor *) userdata;
    g_assert(localDescriptor != NULL);

    Application *application = localDescriptor->application;
    g_assert(application != NULL);

    if (g_str_equal(method, DESCRIPTOR_METHOD_READ_VALUE)) {
        ReadOptions *options = parse_read_options(params);

        log_debug(TAG, "read descriptor <%s> by ", localDescriptor->uuid, options->device);

        const char *result = NULL;
        if (application->on_desc_read != NULL) {
            result = application->on_desc_read(localDescriptor->application, options->device,
                                               localDescriptor->service_uuid,
                                               localDescriptor->char_uuid, localDescriptor->uuid);
        }
        read_options_free(options);

        if (result) {
            g_dbus_method_invocation_return_dbus_error(invocation, result, "read descriptor error");
            log_debug(TAG, "read descriptor error");
            return;
        }

        if (localDescriptor->value != NULL) {
            GVariant *resultVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
                                                                localDescriptor->value->data,
                                                                localDescriptor->value->len,
                                                                sizeof(guint8));
            g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&resultVariant, 1));
        } else {
            g_dbus_method_invocation_return_dbus_error(invocation, BLUEZ_ERROR_FAILED, "no value for descriptor");
        }
    } else if (g_str_equal(method, DESCRIPTOR_METHOD_WRITE_VALUE)) {
        g_assert(g_str_equal(g_variant_get_type_string(params), "(aya{sv})"));
        GVariant *valueVariant, *optionsVariant;
        g_variant_get(params, "(@ay@a{sv})", &valueVariant, &optionsVariant);
        WriteOptions *options = parse_write_options(optionsVariant);
        g_variant_unref(optionsVariant);

        log_debug(TAG, "write descriptor <%s> by %s", localDescriptor->uuid, options->device);

        size_t data_length = 0;
        guint8 *data = (guint8 *) g_variant_get_fixed_array(valueVariant, &data_length, sizeof(guint8));
        GByteArray *byteArray = g_byte_array_sized_new(data_length);
        g_byte_array_append(byteArray, data, data_length);
        g_variant_unref(valueVariant);

        // Allow application to accept/reject the characteristic value before setting it
        const char *result = NULL;
        if (application->on_desc_write != NULL) {
            result = application->on_desc_write(localDescriptor->application,
                                                options->device,
                                                localDescriptor->service_uuid,
                                                localDescriptor->char_uuid,
                                                localDescriptor->uuid,
                                                byteArray);
        }
        write_options_free(options);

        if (result) {
            g_dbus_method_invocation_return_dbus_error(invocation, result, "write error");
            log_debug(TAG, "write error");
            return;
        }

        binc_descriptor_set_value(application, localDescriptor, byteArray);

        g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
    }
}

static const GDBusInterfaceVTable descriptor_table = {
        .method_call = binc_internal_descriptor_method_call,
};

int binc_application_add_descriptor(Application *application, const char *service_uuid,
                                    const char *char_uuid, const char *desc_uuid, guint permissions) {
    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);

    LocalCharacteristic *localCharacteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (localCharacteristic == NULL) {
        g_critical("characteristic %s does not exist", char_uuid);
        return EINVAL;
    }

    GError *error = NULL;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(descriptor_xml, &error);
    if (error) {
        log_debug(TAG, "Unable to create descriptor node: %s\n", error->message);
        g_clear_error(&error);
        return EINVAL;
    }

    LocalDescriptor *localDescriptor = g_new0(LocalDescriptor, 1);
    localDescriptor->uuid = g_strdup(desc_uuid);
    localDescriptor->application = application;
    localDescriptor->char_path = g_strdup(localCharacteristic->path);
    localDescriptor->char_uuid = g_strdup(char_uuid);
    localDescriptor->service_uuid = g_strdup(service_uuid);
    localDescriptor->flags = permissions2Flags(permissions);
    localDescriptor->path = g_strdup_printf("%s/desc%d",
                                            localCharacteristic->path,
                                            g_hash_table_size(localCharacteristic->descriptors));
    g_hash_table_insert(localCharacteristic->descriptors, g_strdup(desc_uuid), localDescriptor);

    // Register characteristic
    localDescriptor->registration_id = g_dbus_connection_register_object(application->connection,
                                                                         localDescriptor->path,
                                                                         info->interfaces[0],
                                                                         &descriptor_table,
                                                                         localDescriptor,
                                                                         NULL,
                                                                         &error);
    g_dbus_node_info_unref(info);

    if (localDescriptor->registration_id == 0) {
        log_debug(TAG, "failed to publish local characteristic");
        log_debug(TAG, "Error %s", error->message);
        g_clear_error(&error);
        g_hash_table_remove(localCharacteristic->descriptors, desc_uuid);
        return EINVAL;
    }

    log_debug(TAG, "successfully published local descriptor %s", desc_uuid);
    return 0;
}

int binc_application_set_char_value(const Application *application, const char *service_uuid,
                                    const char *char_uuid, GByteArray *byteArray) {

    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (service_uuid != NULL, EINVAL);
    g_return_val_if_fail (char_uuid != NULL, EINVAL);
    g_return_val_if_fail (byteArray != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), EINVAL);

    LocalCharacteristic *characteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (characteristic == NULL) {
        g_critical("%s: characteristic with uuid %s does not exist", G_STRFUNC, char_uuid);
        return EINVAL;
    }

    return binc_characteristic_set_value(application, characteristic, byteArray);
}

int binc_application_set_desc_value(const Application *application, const char *service_uuid,
                                    const char *char_uuid, const char *desc_uuid, GByteArray *byteArray) {

    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (service_uuid != NULL, EINVAL);
    g_return_val_if_fail (char_uuid != NULL, EINVAL);
    g_return_val_if_fail (byteArray != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), EINVAL);

    LocalDescriptor *descriptor = get_local_descriptor(application, service_uuid, char_uuid, desc_uuid);
    if (descriptor == NULL) {
        g_critical("%s: characteristic with uuid %s does not exist", G_STRFUNC, char_uuid);
        return EINVAL;
    }

    return binc_descriptor_set_value(application, descriptor, byteArray);
}

GByteArray *binc_application_get_char_value(const Application *application, const char *service_uuid,
                                            const char *char_uuid) {

    g_return_val_if_fail (application != NULL, NULL);
    g_return_val_if_fail (service_uuid != NULL, NULL);
    g_return_val_if_fail (char_uuid != NULL, NULL);
    g_return_val_if_fail (g_uuid_string_is_valid(service_uuid), NULL);
    g_return_val_if_fail (g_uuid_string_is_valid(char_uuid), NULL);

    LocalCharacteristic *characteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (characteristic != NULL) {
        return characteristic->value;
    }
    return NULL;
}


static void binc_internal_characteristic_method_call(GDBusConnection *conn,
                                                     const gchar *sender,
                                                     const gchar *path,
                                                     const gchar *interface,
                                                     const gchar *method,
                                                     GVariant *params,
                                                     GDBusMethodInvocation *invocation,
                                                     void *userdata) {

    LocalCharacteristic *characteristic = (LocalCharacteristic *) userdata;
    g_assert(characteristic != NULL);

    Application *application = characteristic->application;
    g_assert(application != NULL);

    if (g_str_equal(method, CHARACTERISTIC_METHOD_READ_VALUE)) {
        log_debug(TAG, "read <%s>", characteristic->uuid);
        ReadOptions *options = parse_read_options(params);

        // Allow application to accept/reject the characteristic value before setting it
        const char *result = NULL;
        if (application->on_char_read != NULL) {
            result = application->on_char_read(characteristic->application, options->device,
                                               characteristic->service_uuid,
                                               characteristic->uuid);
        }
        read_options_free(options);

        if (result) {
            g_dbus_method_invocation_return_dbus_error(invocation, result, "read characteristic error");
            log_debug(TAG, "read characteristic error '%s'", result);
            return;
        }

        // TODO deal with the offset & mtu parameter
        if (characteristic->value != NULL) {
            GVariant *resultVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
                                                                characteristic->value->data,
                                                                characteristic->value->len,
                                                                sizeof(guint8));
            g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&resultVariant, 1));
        } else {
            g_dbus_method_invocation_return_dbus_error(invocation, BLUEZ_ERROR_FAILED, "no value");
        }
    } else if (g_str_equal(method, CHARACTERISTIC_METHOD_WRITE_VALUE)) {
        log_debug(TAG, "write <%s>", characteristic->uuid);

        g_assert(g_str_equal(g_variant_get_type_string(params), "(aya{sv})"));
        GVariant *valueVariant, *optionsVariant;
        g_variant_get(params, "(@ay@a{sv})", &valueVariant, &optionsVariant);
        WriteOptions *options = parse_write_options(optionsVariant);
        g_variant_unref(optionsVariant);

        size_t data_length = 0;
        guint8 *data = (guint8 *) g_variant_get_fixed_array(valueVariant, &data_length, sizeof(guint8));
        GByteArray *byteArray = g_byte_array_sized_new(data_length);
        g_byte_array_append(byteArray, data, data_length);
        g_variant_unref(valueVariant);

        // Allow application to accept/reject the characteristic value before setting it
        const char *result = NULL;
        if (application->on_char_write != NULL) {
            result = application->on_char_write(characteristic->application, options->device,
                                                characteristic->service_uuid,
                                                characteristic->uuid, byteArray);
        }
        write_options_free(options);

        if (result) {
            g_dbus_method_invocation_return_dbus_error(invocation, result, "write error");
            log_debug(TAG, "write error");
            return;
        }

        // TODO deal with offset and mtu
        binc_characteristic_set_value(application, characteristic, byteArray);

        // Send properties changed signal with new value
        binc_application_notify(application, characteristic->service_uuid, characteristic->uuid, byteArray);

        g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
    } else if (g_str_equal(method, CHARACTERISTIC_METHOD_START_NOTIFY)) {
        log_debug(TAG, "start notify <%s>", characteristic->uuid);

        characteristic->notifying = TRUE;
        g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));

        if (application->on_char_start_notify != NULL) {
            application->on_char_start_notify(characteristic->application, characteristic->service_uuid,
                                              characteristic->uuid);
        }
    } else if (g_str_equal(method, CHARACTERISTIC_METHOD_STOP_NOTIFY)) {
        log_debug(TAG, "stop notify <%s>", characteristic->uuid);

        characteristic->notifying = FALSE;
        g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));

        if (application->on_char_stop_notify != NULL) {
            application->on_char_stop_notify(characteristic->application, characteristic->service_uuid,
                                             characteristic->uuid);
        }
    } else if (g_str_equal(method, CHARACTERISTIC_METHOD_CONFIRM)) {
        log_debug(TAG, "indication confirmed <%s>", characteristic->uuid);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
    }
}

GVariant *characteristic_get_property(GDBusConnection *connection,
                                      const gchar *sender,
                                      const gchar *object_path,
                                      const gchar *interface_name,
                                      const gchar *property_name,
                                      GError **error,
                                      gpointer user_data) {

    log_debug(TAG, "local characteristic get property : %s", property_name);
    LocalCharacteristic *characteristic = (LocalCharacteristic *) user_data;
    g_assert(characteristic != NULL);

    GVariant *ret = NULL;
    if (g_str_equal(property_name, "UUID")) {
        ret = g_variant_new_string(characteristic->uuid);
    } else if (g_str_equal(property_name, "Service")) {
        ret = g_variant_new_object_path(characteristic->path);
    } else if (g_str_equal(property_name, "Flags")) {
        ret = binc_local_characteristic_get_flags(characteristic);
    } else if (g_str_equal(property_name, "Notifying")) {
        ret = g_variant_new_boolean(characteristic->notifying);
    } else if (g_str_equal(property_name, "Value")) {
        ret = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, characteristic->value->data, characteristic->value->len,
                                        sizeof(guint8));
    }
    return ret;
}

static const GDBusInterfaceVTable characteristic_table = {
        .method_call = binc_internal_characteristic_method_call,
        .get_property = characteristic_get_property
};

int binc_application_add_characteristic(Application *application, const char *service_uuid,
                                        const char *char_uuid, guint permissions) {

    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), EINVAL);

    LocalService *localService = binc_application_get_service(application, service_uuid);
    if (localService == NULL) {
        g_critical("service %s does not exist", service_uuid);
        return EINVAL;
    }

    GError *error = NULL;
    GDBusNodeInfo *info = NULL;
    info = g_dbus_node_info_new_for_xml(characteristic_xml, &error);
    if (error) {
        log_debug(TAG, "Unable to create node: %s\n", error->message);
        g_clear_error(&error);
        return EINVAL;
    }

    LocalCharacteristic *characteristic = g_new0(LocalCharacteristic, 1);
    characteristic->service_uuid = g_strdup(service_uuid);
    characteristic->service_path = g_strdup(localService->path);
    characteristic->uuid = g_strdup(char_uuid);
    characteristic->permissions = permissions;
    characteristic->flags = permissions2Flags(permissions);
    characteristic->value = NULL;
    characteristic->application = application;
    characteristic->path = g_strdup_printf("%s/char%d",
                                           localService->path,
                                           g_hash_table_size(localService->characteristics));
    characteristic->descriptors = g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            (GDestroyNotify) binc_local_desc_free);
    g_hash_table_insert(localService->characteristics, g_strdup(char_uuid), characteristic);

    // Register characteristic
    characteristic->registration_id = g_dbus_connection_register_object(application->connection,
                                                                        characteristic->path,
                                                                        info->interfaces[0],
                                                                        &characteristic_table,
                                                                        characteristic,
                                                                        NULL,
                                                                        &error);
    g_dbus_node_info_unref(info);

    if (characteristic->registration_id == 0) {
        log_debug(TAG, "failed to publish local characteristic");
        log_debug(TAG, "Error %s", error->message);
        g_clear_error(&error);
        g_hash_table_remove(localService->characteristics, char_uuid);
        return EINVAL;
    }

    log_debug(TAG, "successfully published local characteristic %s", char_uuid);
    return 0;
}

const char *binc_application_get_path(const Application *application) {
    g_assert(application != NULL);
    return application->path;
}

void binc_application_set_char_read_cb(Application *application, onLocalCharacteristicRead callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_char_read = callback;
}

void binc_application_set_char_write_cb(Application *application, onLocalCharacteristicWrite callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_char_write = callback;
}

void binc_application_set_desc_read_cb(Application *application, onLocalDescriptorRead callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_desc_read = callback;
}

void binc_application_set_desc_write_cb(Application *application, onLocalDescriptorWrite callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_desc_write = callback;
}

void binc_application_set_char_start_notify_cb(Application *application, onLocalCharacteristicStartNotify callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_char_start_notify = callback;
}

void binc_application_set_char_stop_notify_cb(Application *application, onLocalCharacteristicStopNotify callback) {
    g_assert(application != NULL);
    g_assert(callback != NULL);

    application->on_char_stop_notify = callback;
}

int binc_application_notify(const Application *application, const char *service_uuid, const char *char_uuid,
                            const GByteArray *byteArray) {

    g_return_val_if_fail (application != NULL, EINVAL);
    g_return_val_if_fail (byteArray != NULL, EINVAL);
    g_return_val_if_fail (is_valid_uuid(service_uuid), EINVAL);
    g_return_val_if_fail (is_valid_uuid(char_uuid), EINVAL);

    LocalCharacteristic *characteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (characteristic == NULL) {
        g_critical("%s: characteristic %s does not exist", G_STRFUNC, service_uuid);
        return EINVAL;
    }

    GVariant *valueVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
                                                       byteArray->data,
                                                       byteArray->len,
                                                       sizeof(guint8));
    GVariantBuilder *properties_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(properties_builder, "{sv}", "Value", valueVariant);
    GVariantBuilder *invalidated_builder = g_variant_builder_new(G_VARIANT_TYPE("as"));

    GError *error = NULL;
    gboolean result = g_dbus_connection_emit_signal(application->connection,
                                                    NULL,
                                                    characteristic->path,
                                                    "org.freedesktop.DBus.Properties",
                                                    "PropertiesChanged",
                                                    g_variant_new("(sa{sv}as)",
                                                                  "org.bluez.GattCharacteristic1",
                                                                  properties_builder, invalidated_builder),
                                                    &error);

    g_variant_builder_unref(invalidated_builder);
    g_variant_builder_unref(properties_builder);

    if (result != TRUE) {
        if (error != NULL) {
            log_debug(TAG, "error emitting signal: %s", error->message);
            g_clear_error(&error);
        }
        return EINVAL;
    }

    GString *byteArrayStr = g_byte_array_as_hex(byteArray);
    log_debug(TAG, "notified <%s> on <%s>", byteArrayStr->str, characteristic->uuid);
    g_string_free(byteArrayStr, TRUE);
    return 0;
}

gboolean binc_application_char_is_notifying(const Application *application, const char *service_uuid,
                                            const char *char_uuid) {
    g_return_val_if_fail (application != NULL, FALSE);
    g_return_val_if_fail (is_valid_uuid(service_uuid), FALSE);
    g_return_val_if_fail (is_valid_uuid(char_uuid), FALSE);

    LocalCharacteristic *characteristic = get_local_characteristic(application, service_uuid, char_uuid);
    if (characteristic == NULL) {
        g_critical("%s: characteristic %s does not exist", G_STRFUNC, service_uuid);
        return FALSE;
    }

    return characteristic->notifying;
}
