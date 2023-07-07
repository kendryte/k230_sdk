/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#ifndef LIBSECURE
#define LIBSECURE

#include <securec.h>

#define secure_unused(x) ((x) = (x))

#define calculate_string_length(str, out_len) do { \
    const char *str_end = (const char *)(str); \
    while (*str_end != '\0') { \
        ++str_end; \
    } \
    *(out_len) = (unsigned int)(str_end - (str)); \
} while (0)

#endif