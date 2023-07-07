/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

/*
 * <RETURN VALUE>
 *    return  the number of characters written, not including the terminating null
 *    return -1  Failed
 */
/* new fix: */
//int vsnprintf_s(char *str_dest, unsigned int dest_max, unsigned int count, const char *format, va_list arg_list)
int vsnprintf_s(char *str_dest, size_t dest_max, size_t count, const char *format, va_list arg_list)
{
    int ret;

    secure_unused(dest_max);
    if (str_dest == NULL || format == NULL) {
        return -1;
    }

    ret = vsnprintf(str_dest, count + 1, format, arg_list);
    if (ret > (int)count) {
        str_dest[count] = '\0';
        return -1;
    }

    if (ret < 0) {
        str_dest[0] = '\0';
        return -1;
    }

    return ret;
}