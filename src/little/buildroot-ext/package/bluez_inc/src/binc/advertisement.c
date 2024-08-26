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

#include "advertisement.h"
#include "adapter.h"
#include "logger.h"
#include "utility.h"

static const char *const TAG = "Advertisement";

struct binc_advertisement {
    char *path; // Owned
    char *local_name; // Owned
    GPtrArray *services; // Owned
    GHashTable *manufacturer_data; // Owned
    GHashTable *service_data; // Owned
    guint registration_id;
};

static void add_manufacturer_data(gpointer key, gpointer value, gpointer userdata) {
    GByteArray *byteArray = (GByteArray *) value;
    GVariant *byteArrayVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data,
                                                           byteArray->len, sizeof(guint8));
    guint16 manufacturer_id = *(int *) key;
    g_variant_builder_add((GVariantBuilder *) userdata, "{qv}", manufacturer_id, byteArrayVariant);
}

static void add_service_data(gpointer key, gpointer value, gpointer userdata) {
    GByteArray *byteArray = (GByteArray *) value;
    GVariant *byteArrayVariant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, byteArray->data,
                                                           byteArray->len, sizeof(guint8));
    g_variant_builder_add((GVariantBuilder *) userdata, "{sv}", (char *) key, byteArrayVariant);
}

GVariant *advertisement_get_property(GDBusConnection *connection,
                                     const gchar *sender,
                                     const gchar *object_path,
                                     const gchar *interface_name,
                                     const gchar *property_name,
                                     GError **error,
                                     gpointer user_data) {

    GVariant *ret = NULL;
    Advertisement *advertisement = user_data;
    g_assert(advertisement != NULL);

    if (g_str_equal(property_name, "Type")) {
        ret = g_variant_new_string("peripheral");
    } else if (g_str_equal(property_name, "LocalName")) {
        ret = advertisement->local_name ? g_variant_new_string(advertisement->local_name) : NULL;
    } else if (g_str_equal(property_name, "ServiceUUIDs")) {
        GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
        if (advertisement->services != NULL) {
            for (guint i = 0; i < advertisement->services->len; i++) {
                char *service_uuid = g_ptr_array_index(advertisement->services, i);
                g_variant_builder_add(builder, "s", service_uuid);
            }
        }
        ret = g_variant_builder_end(builder);
        g_variant_builder_unref(builder);
    } else if (g_str_equal(property_name, "ManufacturerData")) {
        GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{qv}"));
        if (advertisement->manufacturer_data != NULL && g_hash_table_size(advertisement->manufacturer_data) > 0) {
            g_hash_table_foreach(advertisement->manufacturer_data, add_manufacturer_data, builder);
        }
        ret = g_variant_builder_end(builder);
        g_variant_builder_unref(builder);
    } else if (g_str_equal(property_name, "ServiceData")) {
        GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
        if (advertisement->service_data != NULL && g_hash_table_size(advertisement->service_data) > 0) {
            g_hash_table_foreach(advertisement->service_data, add_service_data, builder);
        }
        ret = g_variant_builder_end(builder);
        g_variant_builder_unref(builder);
    }
    return ret;
}

static void advertisement_method_call(__attribute__((unused)) GDBusConnection *conn,
                                      __attribute__((unused)) const gchar *sender,
                                      __attribute__((unused)) const gchar *path,
                                      __attribute__((unused)) const gchar *interface,
                                      __attribute__((unused)) const gchar *method,
                                      __attribute__((unused)) GVariant *params,
                                      __attribute__((unused)) GDBusMethodInvocation *invocation,
                                      void *userdata) {
    log_debug(TAG, "advertisement method called");
}

static const GDBusInterfaceVTable advertisement_method_table = {
        .method_call = advertisement_method_call,
        .get_property = advertisement_get_property
};

void binc_advertisement_register(Advertisement *advertisement, const Adapter *adapter) {
    g_assert(advertisement != NULL);
    g_assert(adapter != NULL);

    static const char advertisement_xml[] =
            "<node name='/'>"
            "   <interface name='org.bluez.LEAdvertisement1'>"
            "       <method name='Release' />"
            "       <property name='Type' type='s' access='read'/>"
            "       <property name='LocalName' type='s' access='read'/>"
            "       <property name='ManufacturerData' type='a{qv}' access='read'/>"
            "       <property name='ServiceData' type='a{sv}' access='read'/>"
            "       <property name='ServiceUUIDs' type='as' access='read'/>"
            "   </interface>"
            "</node>";

    GError *error = NULL;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(advertisement_xml, &error);
    advertisement->registration_id = g_dbus_connection_register_object(binc_adapter_get_dbus_connection(adapter),
                                                                       advertisement->path,
                                                                       info->interfaces[0],
                                                                       &advertisement_method_table,
                                                                       advertisement, NULL, &error);

    if (error != NULL) {
        log_debug(TAG, "registering advertisement failed: %s", error->message);
        g_clear_error(&error);
    }
    g_dbus_node_info_unref(info);
}

void binc_advertisement_unregister(Advertisement *advertisement, const Adapter *adapter) {
    g_assert(advertisement != NULL);
    g_assert(adapter != NULL);

    gboolean result = g_dbus_connection_unregister_object(binc_adapter_get_dbus_connection(adapter),
                                                          advertisement->registration_id);
    if (!result) {
        log_debug(TAG, "failed to unregister advertisement");
    }
}

static void byte_array_free(GByteArray *byteArray) { g_byte_array_free(byteArray, TRUE); }

Advertisement *binc_advertisement_create() {
    Advertisement *advertisement = g_new0(Advertisement, 1);
    advertisement->path = g_strdup("/org/bluez/bincadvertisement");
    advertisement->manufacturer_data = g_hash_table_new_full(g_int_hash, g_int_equal, g_free,
                                                             (GDestroyNotify) byte_array_free);
    advertisement->service_data = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                                        (GDestroyNotify) byte_array_free);
    return advertisement;
}

void binc_advertisement_free(Advertisement *advertisement) {
    g_assert(advertisement != NULL);

    g_free(advertisement->path);
    advertisement->path = NULL;

    g_free(advertisement->local_name);
    advertisement->local_name = NULL;

    if (advertisement->services != NULL) {
        g_ptr_array_free(advertisement->services, TRUE);
        advertisement->services = NULL;
    }
    if (advertisement->manufacturer_data != NULL) {
        g_hash_table_destroy(advertisement->manufacturer_data);
        advertisement->manufacturer_data = NULL;
    }
    if (advertisement->service_data != NULL) {
        g_hash_table_destroy(advertisement->service_data);
        advertisement->service_data = NULL;
    }

    g_free(advertisement);
}

void binc_advertisement_set_local_name(Advertisement *advertisement, const char *local_name) {
    g_assert(advertisement != NULL);

    g_free(advertisement->local_name);
    advertisement->local_name = g_strdup(local_name);
}

const char *binc_advertisement_get_path(const Advertisement *advertisement) {
    g_assert(advertisement != NULL);
    return advertisement->path;
}

void binc_advertisement_set_services(Advertisement *advertisement, const GPtrArray *service_uuids) {
    g_assert(advertisement != NULL);
    g_assert(service_uuids != NULL);

    if (advertisement->services != NULL) {
        g_ptr_array_free(advertisement->services, TRUE);
    }
    advertisement->services = g_ptr_array_new_with_free_func(g_free);

    for (guint i = 0; i < service_uuids->len; i++) {
        g_ptr_array_add(advertisement->services, g_strdup(g_ptr_array_index(service_uuids, i)));
    }
}

void binc_advertisement_set_manufacturer_data(Advertisement *advertisement, guint16 manufacturer_id,
                                              const GByteArray *byteArray) {
    g_assert(advertisement != NULL);
    g_assert(advertisement->manufacturer_data != NULL);
    g_assert(byteArray != NULL);

    int man_id = manufacturer_id;
    g_hash_table_remove(advertisement->manufacturer_data, &man_id);

    int *key = g_new0 (int, 1);
    *key = manufacturer_id;

    GByteArray *value = g_byte_array_sized_new(byteArray->len);
    g_byte_array_append(value, byteArray->data, byteArray->len);

    g_hash_table_insert(advertisement->manufacturer_data, key, value);
}

void binc_advertisement_set_service_data(Advertisement *advertisement, const char *service_uuid,
                                         const GByteArray *byteArray) {
    g_assert(advertisement != NULL);
    g_assert(advertisement->service_data != NULL);
    g_assert(service_uuid != NULL);
    g_assert(is_valid_uuid(service_uuid));
    g_assert(byteArray != NULL);

    g_hash_table_remove(advertisement->service_data, service_uuid);

    GByteArray *value = g_byte_array_sized_new(byteArray->len);
    g_byte_array_append(value, byteArray->data, byteArray->len);

    g_hash_table_insert(advertisement->service_data, g_strdup(service_uuid), value);
}



