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

#include "service.h"
#include "characteristic.h"
#include "utility.h"

struct binc_service {
    Device *device; // Borrowed
    const char *path; // Owned
    const char* uuid; // Owned
    GList *characteristics; // Owned
};

Service* binc_service_create(Device *device, const char* path, const char* uuid) {
    g_assert(device != NULL);
    g_assert(path != NULL);
    g_assert(is_valid_uuid(uuid));

    Service *service = g_new0(Service, 1);
    service->device = device;
    service->path = g_strdup(path);
    service->uuid = g_strdup(uuid);
    service->characteristics = NULL;
    return service;
}

void binc_service_free(Service *service) {
    g_assert(service != NULL);

    g_free((char*) service->path);
    service->path = NULL;

    g_free((char*) service->uuid);
    service->uuid = NULL;

    g_list_free(service->characteristics);
    service->characteristics = NULL;

    service->device = NULL;
    g_free(service);
}

const char* binc_service_get_uuid(const Service *service) {
    g_assert(service != NULL);
    return service->uuid;
}

Device *binc_service_get_device(const Service *service) {
    g_assert(service != NULL);
    return service->device;
}

void binc_service_add_characteristic(Service *service, Characteristic *characteristic) {
    g_assert(service != NULL);
    g_assert(characteristic != NULL);

    service->characteristics = g_list_append(service->characteristics, characteristic);
}

GList *binc_service_get_characteristics(const Service *service) {
    g_assert(service != NULL);
    return service->characteristics;
}

Characteristic *binc_service_get_characteristic(const Service *service, const char* char_uuid) {
    g_assert(service != NULL);
    g_assert(char_uuid != NULL);
    g_assert(is_valid_uuid(char_uuid));

    if (service->characteristics != NULL) {
        for (GList *iterator = service->characteristics; iterator; iterator = iterator->next) {
            Characteristic *characteristic = (Characteristic *) iterator->data;
            if (g_str_equal(char_uuid, binc_characteristic_get_uuid(characteristic))) {
                return characteristic;
            }
        }
    }
    return NULL;
}
