/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

#define memory_check_not_over(dest, src, count) \
    (((src) < (dest) && ((const char *)(src) + (count)) <= (char *)(dest)) || \
    ((dest) < (src) && ((char *)(dest) + (count)) <= (const char *)(src)))

/*
 * <RETURN VALUE>
 *    EOK     Success
 *    EINVAL  Failed
 */
/* new fix: */
//int strcpy_s(char *str_dest, unsigned int dest_max, const char *str_src)
int strcpy_s(char *str_dest, size_t dest_max, const char *str_src)
{
    unsigned int src_strlen;

    secure_unused(dest_max);
    if (str_dest == NULL || str_src == NULL || str_dest == str_src) {
        return EINVAL;
    }

    calculate_string_length(str_src, &src_strlen);
    ++src_strlen; /* include '\0' */
    if (!memory_check_not_over(str_dest, str_src, src_strlen)) {
        return EINVAL;
    }

    memcpy(str_dest, (char *)str_src, src_strlen);
    return EOK;
}