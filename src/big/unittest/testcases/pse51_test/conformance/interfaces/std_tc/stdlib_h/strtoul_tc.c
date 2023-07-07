#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str[30] = "2030300 This is test";
    char *str_cmp = " This is test";
    char *ptr;
    long ret;

    ret = strtoul(str, &ptr, 10);

    if (ret == 2030300ul && strcmp(str_cmp, str))
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strtoll error\n");
    return PTS_FAIL;
}