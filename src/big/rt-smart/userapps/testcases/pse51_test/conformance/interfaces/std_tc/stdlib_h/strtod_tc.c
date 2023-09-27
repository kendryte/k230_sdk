#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str[30] = "20.30300strtod test case";
    const char *str_cpy = "strtod test case";
    char *ptr = NULL;
    double ret;

    ret = strtod(str, &ptr);

    if (ret != 20.30300)
    {
        perror("strtod error\n");
        return PTS_FAIL;
    }

    if (strcmp(ptr, str_cpy))
    {
        perror("strtod error\n");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}