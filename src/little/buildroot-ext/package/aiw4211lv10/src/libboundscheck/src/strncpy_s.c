/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

#define string_no_overlap(dest, src, len) \
    (((src) < (dest) && ((src) + (len)) < (dest)) || \
    ((dest) < (src) && ((dest) + (len)) < (src)))

#define calculate_strncpy_length(str, max_len, out_len) do { \
    const char *str_end_ = (const char *)(str); \
    unsigned int len_ = 0; \
    while (len_ < (max_len) && *str_end_ != L'\0') { \
        ++len_; \
        ++str_end_; \
    } \
    *(out_len) = len_; \
} while (0)

/*
 * <RETURN VALUE>
 *    EOK     Success
 *    EINVAL  Failed
 */
/* new fix: */
//int strncpy_s(char *str_dest, unsigned int dest_max, const char *str_src, unsigned int count)
int strncpy_s(char *str_dest, size_t dest_max, const char *str_src, size_t count)
{
    unsigned int src_strlen;
    secure_unused(dest_max);

    if (str_dest == NULL || str_src == NULL) {
        return EINVAL;
    }

    calculate_strncpy_length(str_src, count, &src_strlen);
    if (!string_no_overlap(str_dest, str_src, src_strlen)) {
        return EINVAL;
    }

    memcpy(str_dest, (char *)str_src, src_strlen);
    str_dest[src_strlen] = '\0';
    return EOK;
}