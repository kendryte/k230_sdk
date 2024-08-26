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

#ifndef BINC_UTILITY_H
#define BINC_UTILITY_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

GString *g_byte_array_as_hex(const GByteArray *byteArray);

GList *g_variant_string_array_to_list(GVariant *value);

float binc_round_with_precision(float value, guint8 precision);

gchar *binc_date_time_format_iso8601(GDateTime *datetime);

gboolean is_lowercase(const char *str);

gboolean is_valid_uuid(const char *uuid);

char *path_to_address(const char *path);

GByteArray *g_variant_get_byte_array(GVariant *variant);

char* replace_char(char* str, char find, char replace);

#ifdef __cplusplus
}
#endif

#endif //BINC_UTILITY_H
