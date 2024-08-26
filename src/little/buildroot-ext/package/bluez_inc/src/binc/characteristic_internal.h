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

#ifndef BINC_CHARACTERISTIC_INTERNAL_H
#define BINC_CHARACTERISTIC_INTERNAL_H

#include "characteristic.h"

#ifdef __cplusplus
extern "C" {
#endif

Characteristic *binc_characteristic_create(Device *device, const char *path);

void binc_characteristic_free(Characteristic *characteristic);

void binc_characteristic_set_read_cb(Characteristic *characteristic, OnReadCallback callback);

void binc_characteristic_set_write_cb(Characteristic *characteristic, OnWriteCallback callback);

void binc_characteristic_set_notify_cb(Characteristic *characteristic, OnNotifyCallback callback);

void binc_characteristic_set_notifying_state_change_cb(Characteristic *characteristic,
                                                       OnNotifyingStateChangedCallback callback);

void binc_characteristic_set_service(Characteristic *characteristic, Service *service);

void binc_characteristic_set_service_path(Characteristic *characteristic, const char *service_path);

void binc_characteristic_set_flags(Characteristic *characteristic, GList *flags);

void binc_characteristic_set_uuid(Characteristic *characteristic, const char *uuid);

void binc_characteristic_set_mtu(Characteristic *characteristic, guint mtu);

void binc_characteristic_set_notifying(Characteristic *characteristic, gboolean notifying);

const char *binc_characteristic_get_service_path(const Characteristic *characteristic);

void binc_characteristic_add_descriptor(Characteristic *characteristic, Descriptor *descriptor);

#ifdef __cplusplus
}
#endif

#endif //BINC_CHARACTERISTIC_INTERNAL_H
