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

#include "utility.h"
#include "math.h"

void bytes_to_hex(char *dest, const guint8 *src, int n) {
    const char xx[] = "0123456789abcdef";
    while (--n >= 0) dest[n] = xx[(src[n >> 1] >> ((1 - (n & 1)) << 2)) & 0xF];
}

GString *g_byte_array_as_hex(const GByteArray *byteArray) {
    int hexLength = (int) byteArray->len * 2;
    GString *result = g_string_sized_new(hexLength + 1);
    bytes_to_hex(result->str, byteArray->data, hexLength);
    result->str[hexLength] = 0;
    result->len = hexLength;
    return result;
}

GList *g_variant_string_array_to_list(GVariant *value) {
    g_assert(value != NULL);
    g_assert(g_str_equal(g_variant_get_type_string(value), "as"));

    GList *list = NULL;
    gchar *data;
    GVariantIter iter;

    g_variant_iter_init(&iter, value);
    while (g_variant_iter_loop(&iter, "s", &data)) {
        list = g_list_append(list, g_strdup(data));
    }
    return list;
}

float binc_round_with_precision(float value, guint8 precision) {
    int multiplier = (int) pow(10.0, precision);
    return roundf(value * multiplier) / multiplier;
}

gchar *binc_date_time_format_iso8601(GDateTime *datetime) {
    GString *outstr = NULL;
    gchar *main_date = NULL;
    gint64 offset;
    gchar *format = "%Y-%m-%dT%H:%M:%S";

    /* Main date and time. */
    main_date = g_date_time_format(datetime, format);
    outstr = g_string_new(main_date);
    g_free(main_date);

    /* Timezone. Format it as `%:::z` unless the offset is zero, in which case
     * we can simply use `Z`. */
    offset = g_date_time_get_utc_offset(datetime);

    if (offset == 0) {
        g_string_append_c (outstr, 'Z');
    } else {
        gchar *time_zone = g_date_time_format(datetime, "%:z");
        g_string_append(outstr, time_zone);
        g_free(time_zone);
    }

    return g_string_free(outstr, FALSE);
}

gboolean is_lowercase(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (g_ascii_isalpha(str[i]) && g_ascii_isupper(str[i])) return FALSE;
    }

    return TRUE;
}

gboolean is_valid_uuid(const char *uuid) {
    if (uuid == NULL) {
        g_critical("uuid is NULL");
        return FALSE;
    }

    if (!g_uuid_string_is_valid(uuid)) {
        g_critical("%s is not a valid UUID", uuid);
        return FALSE;
    }

    if (!is_lowercase(uuid)) {
        g_critical("%s is not entirely lowercase", uuid);
        return FALSE;
    }

    return TRUE;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

char *path_to_address(const char *path) {
    char *address = g_strdup_printf("%s", path+(strlen(path)-17));
    return replace_char(address, '_', ':');
}

/**
 * Get a byte array that wraps the data inside the variant.
 *
 * Only the GByteArray should be freed but not the content as it doesn't make a copy.
 * Content will be freed automatically when the variant is unref-ed.
 *
 * @param variant byte array of format 'ay'
 * @return GByteArray wrapping the data in the variant
 */
GByteArray *g_variant_get_byte_array(GVariant *variant) {
    g_assert(variant != NULL);
    g_assert(g_str_equal(g_variant_get_type_string(variant), "ay"));

    size_t data_length = 0;
    guint8 *data = (guint8 *) g_variant_get_fixed_array(variant, &data_length, sizeof(guint8));
    return g_byte_array_new_take(data, data_length);
}