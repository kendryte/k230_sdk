#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    char *s = "http://www.rt-thread.com";
    char *str_cpy = "rt-thread";
    char str[20];

    memcpy(str, s + 11, 9);
    str[9] = '\0';

    if (strcmp(str, str_cpy))
    {
        perror("memcpy fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}