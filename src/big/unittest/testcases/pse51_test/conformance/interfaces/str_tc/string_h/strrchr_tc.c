#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    const char str[] = "https://www.rtthread.com";
    const char ch = '.';
    const char *str_cmp = ".com";
    char *ret;

    ret = strrchr(str, ch);

    if (strcmp(ret, str_cmp))
    {
        perror("strrchr fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}