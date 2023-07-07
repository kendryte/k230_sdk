/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

#define check_string_is_overlap(dest, dest_len, src, src_len) \
    (((dest) < (src) && ((dest) + (dest_len) + (src_len)) >= (src)) || \
    ((src) < (dest) && ((src) + (src_len)) >= (dest)))

/*
 * <RETURN VALUE>
 *    EOK                 Success
 *    EINVAL              Failed
 */
/* new fix: */
//int strcat_s(char *str_dest, unsigned int dest_max, const char *str_src)
int strcat_s(char *str_dest, size_t dest_max, const char *str_src)
{
    unsigned int dest_len;
    unsigned int src_len;

    secure_unused(dest_max);
    if (str_dest == NULL || str_src == NULL) {
        if (str_dest != NULL) {
            str_dest[0] = '\0';
        }
        return EINVAL;
    }

    calculate_string_length(str_dest, &dest_len);
    calculate_string_length(str_src, &src_len);

    if (check_string_is_overlap(str_dest, dest_len, str_src, src_len)) {
        str_dest[0] = '\0';
        return EINVAL;
    }

    memcpy(str_dest + dest_len, (char *)str_src, src_len + 1); /* \0 */
    return EOK;
}