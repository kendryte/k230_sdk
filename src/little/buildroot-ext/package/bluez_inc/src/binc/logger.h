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

#ifndef BINC_LOGGER_H
#define BINC_LOGGER_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LogLevel {
    LOG_DEBUG = 0, LOG_INFO = 1, LOG_WARN = 2, LOG_ERROR = 3
} LogLevel;

#define log_debug(tag, format, ...) log_log_at_level(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define log_info(tag, format, ...)  log_log_at_level(LOG_INFO, tag, format, ##__VA_ARGS__)
#define log_warn(tag, format, ...)  log_log_at_level(LOG_WARN, tag, format,  ##__VA_ARGS__)
#define log_error(tag, format, ...) log_log_at_level(LOG_ERROR, tag, format, ##__VA_ARGS__)

void log_log_at_level(LogLevel level, const char* tag, const char *format, ...);

void log_set_level(LogLevel level);

void log_set_filename(const char* filename, long max_size, int max_files);

typedef void (*LogEventCallback)(LogLevel level, const char *tag, const char *message);

void log_set_handler(LogEventCallback callback);

void log_enabled(gboolean enabled);

#ifdef __cplusplus
}
#endif

#endif //BINC_LOGGER_H
