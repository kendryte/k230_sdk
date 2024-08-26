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

#ifndef BINC_ADVERTISEMENT_H
#define BINC_ADVERTISEMENT_H

#include <gio/gio.h>
#include "forward_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

Advertisement *binc_advertisement_create();

void binc_advertisement_free(Advertisement *advertisement);

void binc_advertisement_set_local_name(Advertisement *advertisement, const char* local_name);

void binc_advertisement_set_services(Advertisement *advertisement, const GPtrArray * service_uuids);

void binc_advertisement_set_manufacturer_data(Advertisement *advertisement, guint16 manufacturer_id, const GByteArray *byteArray);

void binc_advertisement_set_service_data(Advertisement *advertisement, const char* service_uuid, const GByteArray *byteArray);

const char *binc_advertisement_get_path(const Advertisement *advertisement);

void binc_advertisement_register(Advertisement *advertisement, const Adapter *adapter);

void binc_advertisement_unregister(Advertisement *advertisement, const Adapter *adapter);

#ifdef __cplusplus
}
#endif

#endif //BINC_ADVERTISEMENT_H
