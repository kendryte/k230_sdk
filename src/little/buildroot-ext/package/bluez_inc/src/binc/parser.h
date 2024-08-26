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

#ifndef BINC_PARSER_H
#define BINC_PARSER_H

#include <glib.h>
#include <endian.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct parser_instance Parser;

/**
 * Create a parser for a byte array
 *
 * @param bytes the byte array
 * @param byteOrder either LITTLE_ENDIAN or BIG_ENDIAN
 * @return parser object
 */
Parser *parser_create(const GByteArray *bytes, int byteOrder);

void parser_set_offset(Parser *parser, guint offset);

void parser_free(Parser *parser);

guint8 parser_get_uint8(Parser *parser);

gint8 parser_get_sint8(Parser *parser);

guint16 parser_get_uint16(Parser *parser);

gint16 parser_get_sint16(Parser *parser);

guint32 parser_get_uint24(Parser *parser);

guint32 parser_get_uint32(Parser *parser);

double parser_get_sfloat(Parser *parser);

double parser_get_float(Parser *parser);

double parser_get_754half(Parser *parser);

double parser_get_754float(Parser *parser);

GDateTime* parser_get_date_time(Parser *parser);

GByteArray* binc_get_date_time();

GByteArray *binc_get_current_time();

GString *parser_get_string(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif //BINC_PARSER_H
