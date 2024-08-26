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

#ifndef BINC_DESCRIPTOR_H
#define BINC_DESCRIPTOR_H

#include <gio/gio.h>
#include "forward_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*OnDescReadCallback)(Device *device, Descriptor *descriptor, const GByteArray *byteArray, const GError *error);

typedef void (*OnDescWriteCallback)(Device *device, Descriptor *descriptor, const GByteArray *byteArray, const GError *error);

void binc_descriptor_read(Descriptor *descriptor);

void binc_descriptor_write(Descriptor *descriptor, const GByteArray *byteArray);

const char *binc_descriptor_get_uuid(const Descriptor *descriptor);

const char *binc_descriptor_to_string(const Descriptor *descriptor);

Characteristic *binc_descriptor_get_char(const Descriptor *descriptor);

Device *binc_descriptor_get_device(const Descriptor *descriptor);

#ifdef __cplusplus
}
#endif

#endif //BINC_DESCRIPTOR_H
