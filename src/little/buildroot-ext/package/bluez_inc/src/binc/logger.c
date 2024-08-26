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

#include "logger.h"
#include <glib.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/stat.h>

#define TAG "Logger"
#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 1024 * 64
#define MAX_LOGS 5

static struct {
    gboolean enabled;
    LogLevel level;
    FILE *fout;
    char filename[256];
    unsigned long maxFileSize;
    unsigned int maxFiles;
    size_t currentSize;
    LogEventCallback logCallback;
} LogSettings = {TRUE, LOG_DEBUG, NULL, "", MAX_FILE_SIZE, MAX_LOGS, 0, NULL};

static const char *log_level_names[] = {
        [LOG_DEBUG] = "DEBUG",
        [LOG_INFO] = "INFO",
        [LOG_WARN]  = "WARN",
        [LOG_ERROR]  = "ERROR"
};

void log_set_level(LogLevel level) {
    LogSettings.level = level;
}

void log_set_handler(LogEventCallback callback) {
    LogSettings.logCallback = callback;
}

void log_enabled(gboolean enabled) {
    LogSettings.enabled = enabled;
}

static void open_log_file() {
    LogSettings.fout = fopen(LogSettings.filename, "a");
    if (LogSettings.fout == NULL) {
        LogSettings.fout = stdout;
        return;
    }

    struct stat finfo;
    fstat(fileno(LogSettings.fout), &finfo);
    LogSettings.currentSize = finfo.st_size;
}

void log_set_filename(const char *filename, long max_size, int max_files) {
    g_assert(filename != NULL);
    g_assert(strlen(filename) > 0);

    LogSettings.maxFileSize = max_size ? max_size : MAX_FILE_SIZE;
    LogSettings.maxFiles = max_files ? max_files : MAX_LOGS;
    strncpy(LogSettings.filename, filename, sizeof(LogSettings.filename) - 1);
    open_log_file();
}

/**
 * Get the current UTC time in milliseconds since epoch
 * @return
 */
static long long current_timestamp_in_millis() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    return milliseconds;
}

/**
 * Returns a string representation of the current time year-month-day hours:minutes:seconds
 * @return newly allocated string, must be freed using g_free()
 */
static char *current_time_string() {
    GDateTime *now = g_date_time_new_now_local();
    char *time_string = g_date_time_format(now, "%F %R:%S");
    g_date_time_unref(now);

    char *result = g_strdup_printf("%s:%03lld", time_string, current_timestamp_in_millis() % 1000);
    g_free(time_string);
    return result;
}

static void log_log(const char *tag, const char *level, const char *message) {
    char *timestamp = current_time_string();
    int bytes_written;
    if ((bytes_written = fprintf(LogSettings.fout, "%s %s [%s] %s\n", timestamp, level, tag, message)) > 0) {
        LogSettings.currentSize += bytes_written;
        fflush(LogSettings.fout);
    }

    g_free(timestamp);
}

static char *get_log_name(int index) {
    if (index > 0) {
        return g_strdup_printf("%s.%d", LogSettings.filename, index);
    } else {
        return g_strdup(LogSettings.filename);
    }
}

static gboolean fileExists(const char *filename) {
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL) {
        return FALSE;
    } else {
        fclose(fp);
        return TRUE;
    }
}

static void rotate_log_files() {
    for (int i = LogSettings.maxFiles; i > 0; i--) {
        char *src = get_log_name(i - 1);
        char *dst = get_log_name(i);
        if (fileExists(dst)) {
            remove(dst);
        }

        if (fileExists(src)) {
            rename(src, dst);
        }

        g_free(src);
        g_free(dst);
    }
}

static void rotate_log_file_if_needed() {
    if ((LogSettings.currentSize < LogSettings.maxFileSize) ||
        LogSettings.fout == stdout || LogSettings.logCallback != NULL)
        return;

    g_assert(LogSettings.fout != NULL);
    fclose(LogSettings.fout);
    rotate_log_files();
    open_log_file();
}

void log_log_at_level(LogLevel level, const char *tag, const char *format, ...) {
    // Init fout to stdout if needed
    if (LogSettings.fout == NULL && LogSettings.logCallback == NULL) {
        LogSettings.fout = stdout;
    }

    rotate_log_file_if_needed();

    if (LogSettings.level <= level && LogSettings.enabled) {
        char buf[BUFFER_SIZE];
        va_list arg;
        va_start(arg, format);
        g_vsnprintf(buf, BUFFER_SIZE, format, arg);
        if (LogSettings.logCallback) {
            LogSettings.logCallback(level, tag, buf);
        } else {
            log_log(tag, log_level_names[level], buf);
        }
        va_end(arg);
    }
}

