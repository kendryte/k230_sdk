/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

/*
 * <RETURN VALUE>
 *    return the number of bytes stored in str_dest, not counting the terminating null character.
 *    return -1 Failed
 */
/* new fix: */
//int sprintf_s(char *str_dest, unsigned int dest_max, const char *format, ...)
int sprintf_s(char *str_dest, size_t dest_max, const char *format, ...)
{
    int ret;
    va_list arg_list;

    secure_unused(dest_max);
    if (str_dest == NULL || format == NULL) {
        return -1;
    }

    va_start(arg_list, format);
    ret = vsprintf(str_dest, format, arg_list);
    va_end(arg_list);
    if (ret < 0) {
        str_dest[0] = '\0';
        return -1;
    }

    return ret;
}