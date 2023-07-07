/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

/*
 * <RETURN VALUE>
 *    EOK     Success
 *    EINVAL  Failed
 */
/* new fix: */
//int memmove_s(void *dest, unsigned int dest_max, const void *src, unsigned int count)
int memmove_s(void *dest, size_t dest_max, const void *src, size_t count)
{
    secure_unused(dest_max);
    if (dest == NULL || src == NULL || count <= 0) {
        return EINVAL;
    }

    if (dest == src) {
        return EOK;
    }

    memmove(dest, (void *)src, count);
    return EOK;
}