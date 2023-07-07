#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char src[50], dest[50];
    const char *str_cmp = "rtthread-rtthread-smart";

    strcpy(src, "-rtthread-smart");
    strcpy(dest, "rtthread");

    strcat(dest, src);

    if (strcmp(dest, str_cmp))
    {
        perror("strcat fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}