#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char dest[20];
    char src[20];
    const char *str_cmp = "rtthread Smart";
    int len;

    strcpy(src, "rtthread Smart");
    len = strxfrm(dest, src, 20);

    if (strcmp(dest, str_cmp))
    {
        perror("strxfrm fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}