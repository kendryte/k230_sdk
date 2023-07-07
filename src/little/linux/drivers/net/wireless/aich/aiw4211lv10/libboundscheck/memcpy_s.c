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
//int memcpy_s(void *dest, unsigned int dest_max, const void *src, unsigned int count)
int memcpy_s(void *dest, size_t dest_max, const void *src, size_t count)
{
    secure_unused(dest_max);
    if (dest == NULL || src == NULL) {
        return EINVAL;
    }

    if (!memory_check_not_over(dest, src, count)) {
        return EINVAL;
    }

    memcpy(dest, (void *)src, count);
    return EOK;
}