/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Author: CompanyName
 * Create: 2021-08-04
 */

#include "libsecure.h"

 /*
 * <RETURN VALUE>
 *    -1      Failed
 */
int sscanf_s(const char *buffer, const char *format, ...)
{
    int ret;
    va_list arg_list;

    if (buffer == NULL || format == NULL) {
        return -1;
    }

    int count = strlen(buffer);
    if (count == 0) {
        return -1;
    }

    va_start(arg_list, format);
    ret = vsscanf(buffer, format, arg_list);
    va_end(arg_list);

    if (ret < 0) {
        return -1;
    }

    return ret;
}
