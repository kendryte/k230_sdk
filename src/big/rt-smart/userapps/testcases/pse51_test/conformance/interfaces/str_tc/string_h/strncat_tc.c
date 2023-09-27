#include <string.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
    char dest[20] = "abcde";
    char src[20] = "12345";
    const char *str_cmp = "abcde123";

    strncat(dest, src, 3);

    if (strcmp(dest, str_cmp))
    {
        perror("strerror fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}