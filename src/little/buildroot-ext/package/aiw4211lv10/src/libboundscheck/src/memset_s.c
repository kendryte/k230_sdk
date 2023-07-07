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
//int memset_s(void *dest, unsigned int dest_max, int c, unsigned int count)
int memset_s(void *dest, size_t dest_max, int c, size_t count)
{
    secure_unused(dest_max);
    if (dest == NULL) {
        return EINVAL;
    }

    memset(dest, c, count);
    return EOK;
}